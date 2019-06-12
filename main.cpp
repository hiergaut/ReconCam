#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp" // model
// #include "opencv2/videoio.hpp"

#include "opencv2/imgproc/imgproc.hpp" // bounding boxes
// #include <any>

// omp_set_num_threads(1);
// #define WITH_IPP OFF
// #define WITH_TBB OFF
// #define WITH_OPENMP OFF
// #define WITH_PTHREADS_PF OFF

#include <assert.h>
#include <cassert>
#include <ctime>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>

#include "color.hpp"
#include "function.hpp"

#include <dirent.h>

#ifdef PC
#define TIMELAPSE_INTERVAL 30 // 30 sec
#else
// #define TIMELAPSE_INTERVAL 1800 // 30 min
#define TIMELAPSE_INTERVAL 1200 // 20 min
#endif

#define NB_CAP_LEARNING_MODEL_FIRST 0
#define NB_CAP_FOCUS_BRIGHTNESS 10

// #define NB_CAP_MIN_FOR_REAL_MOTION 5
// #define MIN_MOV_DIST_TO_SAVE_OBJECT 10
#define NEW_OBJECT_MIN_DENSITY 10
#define MAX_ERROR_DIST_FOR_NEW_POS_OBJECT 100
#define MIN_MOV_YEARS_TO_SAVE_OBJECT 5
// #define DELTA_DIFF_MAX_DENSITY 2000

using namespace cv;

// ------------------------------- MAIN ---------------------------------------
int main(int argc, char **argv) {
	std::cout << std::fixed << std::setprecision(3);

	CommandLineParser parser(
		argc, argv,
		"{s sensor      | -1        | gpio number of IR senror}"
		"{a and         | -1        | add detect sensor, (and logic)}"
		"{l light       | -1        | pin light up on movment}"
		"{d device      | 0         | /dev/video<device>}"
		"{stream        |           | camera/video src}"
		"{f flip        | false     | rotate image 180}"
		"{r repository  |           | save motion to specific repo}"
		"{p port        | -1        | remote port repository}"
		// "{v vegetation  | false     | outside camera}"
		"{training      | false     | save movement capture for learning}"
		"{recon         | false     | recon event}"
		"{script        | false     | launch script on recognize}"
		// "{onlyRec       | false     | record video without treatment}"
		"{help h        |           | help message}");

	if (parser.has("help")) {
		parser.printMessage();
		return 0;
	}
	sensorGpioNum = parser.get<int>("sensor");
	sensorAdditional = parser.get<int>("and");
	int device = parser.get<int>("device");
	std::string stream = parser.get<std::string>("stream");
	std::string remoteDir = parser.get<std::string>("repository");
	int port = parser.get<int>("port");
	bool flip180 = parser.get<bool>("flip");
	int lightGpio = parser.get<int>("light");
	// bool hasVegetation = parser.get<bool>("vegetation");
	bool training = parser.get<bool>("training");
	bool recon = parser.get<bool>("recon");
	bool script = parser.get<bool>("script");
	// bool onlyRec = parser.get<bool>("onlyRec");
	if (!parser.check()) {
		parser.printMessage();
		parser.printErrors();
		return 0;
	}

#ifdef PC
	gpioDir = "gpio/";
	// namedWindow("inputFrame", WINDOW_AUTOSIZE);
	// resizeWindow("inputFrame", 320, 240);
	// moveWindow("inputFrame", 0, 0);
	// moveWindow("mask", 640, 0);

	namedWindow("mask", WINDOW_AUTOSIZE);
	namedWindow("mask2", WINDOW_AUTOSIZE);
	namedWindow("drawing", WINDOW_AUTOSIZE);
	// while (waitKey(100) != ' ');
#else
	gpioDir = "/sys/class/gpio/";
#endif

	// std::string device = stream;
	std::string deviceName;
	if (stream.empty()) {
		deviceName = std::to_string(device);
		stream = "/dev/video" + deviceName;
	} else {
		deviceName = stream;
	}
	// std::cout << "stream : " << stream << std::endl;
	// stream = "/dev/video0";

	bool hasRemoteDir = !remoteDir.empty();
	// using ObjList = std::list<Object>;
	std::string motionDir;
	if (hasRemoteDir) {
		motionDir = "/tmp/motion/";

	} else {
		motionDir = "motion/";
	}
	std::string hostname = getHostname();

	VideoCapture vCap;

	std::string timelapseDir =
		// motionDir + "timelapse_" + hostname;
		motionDir + "timelapse_" + hostname + "_" + deviceName;
	std::string cmd = "mkdir -p " + timelapseDir;
	system(cmd.c_str());

	std::string trainDir = "";
	if (training) {
		trainDir = "learningFile/newEvent/";
		cmd = "mkdir -p " + trainDir;
		system(cmd.c_str());

		cmd = "mkdir -p learningFile/known/";
		system(cmd.c_str());

		cmd = "mkdir -p learningFile/empty/";
		system(cmd.c_str());
	}

	// ObjList objects;
	std::set<Object> objects;
	// std::vector<Line> lines;
	std::vector<DeadObj> tombs;

	Mat inputFrame, mask, drawing;
	int iNewObj;
	int iCap;

	int tickTimeLapse = 1; // take picture immediately

	vCap.open(stream);
	// vCap.open("3car3person.avi");
	if (!vCap.isOpened()) {
		std::cout << "device not found" << std::endl;
		return 1;
	}
	int width = vCap.get(CAP_PROP_FRAME_WIDTH);
	int height = vCap.get(CAP_PROP_FRAME_HEIGHT);
	Size sizeScreen(width, height);
	vCap.release();

	Mat notGreen = Mat::zeros(Size(width, height), CV_8UC3);
	notGreen = Scalar(255, 0, 255);

	if (sensorGpioNum != -1) {
		initGpio(sensorGpioNum, "in");
		gpioGetValue(sensorGpioNum);

		if (sensorAdditional != -1) {
			initGpio(sensorAdditional, "in");
			gpioGetValue(sensorAdditional);
		}

		if (lightGpio != -1) {
			initGpio(lightGpio, "out");
			gpioSetValue(lightGpio, 0);
		}
	}

	std::map<std::string, std::tuple<Point3f, Point3f, Point3f>> boxes;
	if (recon) {

		for (std::string filename : forEachFileInDir("learningFile/known/")) {
			std::string path = "learningFile/known/" + filename + "/";
			Point3f Min = readPointFromFile(path + "min.txt");

			Point3f Mean = readPointFromFile(path + "mean.txt");
			Point3f Max = readPointFromFile(path + "max.txt");

			boxes[filename] = {Min, Mean, Max};
		}
	}

	// --------------------------- INFINITE LOOP ------------------------------
	while (1) {
		std::cout << "[TIMELAPSE] wait new motion, future lapse at "
				  << tickTimeLapse << " sec " << std::endl;
		while (!hasMovement()) {

			std::cout << "." << std::flush;
			usleep(CLOCKS_PER_SEC);
			--tickTimeLapse;

			if (tickTimeLapse == 0) {
				std::cout << std::endl;
				vCap.open(stream);
				if (!vCap.isOpened()) {
					std::cout << "device not found";
					return 1;
				}
				for (int i = 0; i < NB_CAP_FOCUS_BRIGHTNESS; ++i) {
					vCap >> inputFrame;
				}

				if (flip180) {
					flip(inputFrame, inputFrame, -1);
				}
				vCap.release();
				// model->apply(inputFrame, mask);

				std::string saveLapse =
					timelapseDir + "/" + getCurTime() + ".jpg";
				imwrite(saveLapse, inputFrame);
				imwrite(timelapseDir + "/latest.jpeg", inputFrame);
				std::cout << "[TIMELAPSE] save lapse '" << saveLapse << "'"
						  << std::endl;

				cmd = "convert " + timelapseDir + "/*.jpg " + timelapseDir +
					  "/timelapse.gif";
				std::cout << "[TIMELAPSE] " << cmd << std::endl;
				system(cmd.c_str());

				if (hasRemoteDir) {
					if (port == -1) {
						cmd = "rsync -arv " + timelapseDir + " " + remoteDir;
					} else {
						cmd = "rsync -arv -e 'ssh -p " + std::to_string(port) +
							  "' " + timelapseDir + " " + remoteDir;
					}
					std::cout << "[TIMELAPSE] " << cmd << std::endl;
					system(cmd.c_str());
				}

				tickTimeLapse = TIMELAPSE_INTERVAL;
				std::cout << "[TIMELAPSE] future lapse at " << tickTimeLapse
						  << " sec " << std::endl;
			}
		} // while (!hasMovement())
		std::cout << std::endl;

		if (lightGpio != -1 && isNight()) {
			gpioSetValue(lightGpio, 1);
		}

		// create new directory in /tmp/motion/
		// hostname + "_" + getDay() + "_" + getCurTime() + "_" + deviceName;
		// std::string startTime = getCurTime();
		std::string motionId = getCurTime() + "_" + hostname + "_" + deviceName;
		std::cout << "new event : " << motionId << std::endl;
		std::string newMotionDir = motionDir + motionId;
		// std::string newMotionDir = motionDir + startTime + "_" + hostname;
		cmd = "mkdir -p " + newMotionDir;
		system(cmd.c_str());

		vCap.open(stream);
		// vCap.open("3car3person.avi");
		if (!vCap.isOpened()) {
			std::cout << "device not found";
			return 1;
		}

		std::string outputVideoFile = newMotionDir + "/video.webm";
		VideoWriter outputVideo = VideoWriter(
			outputVideoFile, cv::VideoWriter::fourcc('V', 'P', '8', '0'), 3,
			sizeScreen, true);

		std::string outputVideoFileRec;
		VideoWriter outputVideoRec;
		// if (onlyRec) {
		outputVideoFileRec = newMotionDir + "/video.avi";
		outputVideoRec = VideoWriter(
			outputVideoFileRec, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 3,
			sizeScreen, true);
		// }
		// VideoWriter outputVideo(newMotionDir + "/video.ogv",
		// 						cv::VideoWriter::fourcc('T', 'H', 'E', 'O'), 2,
		// 						sizeScreen, true);
		if (!outputVideo.isOpened() || !outputVideoRec.isOpened()) {
			std::cout << "failed to write video" << std::endl;
			return 6;
		}

		vCap >> inputFrame;
		outputVideo << inputFrame;
		// if (onlyRec) {
		outputVideoRec << inputFrame;
		// }

		// outputVideo.open(newMotionDir + "/clip.avi", VideoWriter::fourcc('M',
		// 'J', 'P', 'G'), 2, Size(vCap.get(cv::CAP_PROP_FRAME_WIDTH),
		// 					  vCap.get(CAP_PROP_FRAME_HEIGHT)));

		iNewObj = 0;
		iCap = -1;
		// lines.clear();
		tombs.clear();
		objects.clear();

		// auto model = createBackgroundSubtractorKNN();
		auto model = createBackgroundSubtractorMOG2();
		// auto model = createBackgroundSubtractorGMG();
		bool streamFinished = false;
		drawing = inputFrame;

		// ----------------------- WHILE HAS MOVEMENT
		while (hasMovement()) {
			++iCap;
			int nbObjects = objects.size();
			// std::cout << "nbObjects = " << nbObjects << std::endl;

			vCap >> inputFrame;
			if (inputFrame.empty()) {
				std::cout << "Finished reading" << std::endl;
				streamFinished = true;
				break;
			}
			// if (onlyRec) {
			// outputVideo << inputFrame;
			outputVideoRec << inputFrame;
			// usleep(1000 * 300);
			// continue;
			// }
			// if (iCap < NB_CAP_FOCUS_BRIGHTNESS) {
			// 	continue;
			// }

			// ------------------- TRAINING MODEL -----------------------------
			// Mat grey;
			// if (hasVegetation) {
			// 	Mat inputWithoutGreen;
			// 	bitwise_and(inputFrame, notGreen, inputWithoutGreen);
			// 	cvtColor(inputWithoutGreen, grey, COLOR_BGR2GRAY);
			// 	// imshow("notGreen", inputWithoutGreen);
			// } else {
			// cvtColor(inputFrame, grey, COLOR_BGR2GRAY);
			// }
			// equalizeHist(grey, grey);
			// model->apply(grey, mask);
			model->apply(inputFrame, mask);
#ifdef PC
			// imshow("inputFrame", inputFrame);

			if (waitKey(100) == 'q') {
				return 0;
			}
			imshow("mask", mask);

			// imshow("inputFrame", inputFrame);
			// moveWindow("inputFrame", 100, 100);
			// resizeWindow("mask", 100, 100);
			// imshow("drawing", inputFrame);
#endif

			if (iCap < NB_CAP_FOCUS_BRIGHTNESS + NB_CAP_LEARNING_MODEL_FIRST) {
				// waitKey(300);
                imshow("drawing", inputFrame);
				outputVideo << inputFrame;
				continue;
			}

			// if (iCap > 150) {
			// if (waitKey(0) == 'q') {
			// 	return 0;
			// }
			// }
			// std::cout << "------------------------" << std::endl;
			// model->apply(inputFrame, mask, 0);

			// ------------------- BOUNDING MOVMENT ---------------------------
			// threshold(mask, mask, 127, 255, THRESH_BINARY);
			medianBlur(mask, mask, 11);

			// auto kernel = getStructuringElement(MORPH_RECT, Size(3, 3));
			// morphologyEx(mask, mask, MORPH_OPEN, kernel, Point(-1, -1), 1);
			// imshow("inputFrame", mask);
			// threshold(mask, mask, 127, 255, THRESH_BINARY);
			// medianBlur(mask, mask, 21);
			// medianBlur(mask, mask, 3);
			// medianBlur(mask, mask, 3);
			// medianBlur(mask, mask, 3);
			// blur(mask, mask,  Size(3, 3));
			// blur(mask, mask,  Size(3, 3));
			// blur(mask, mask,  Size(3, 3));
			// medianBlur(mask, mask, 9);
			// medianBlur(mask, mask, 9);

			// dilate(mask, mask, kernel, Point(-1, -1), 1);
			// threshold(mask, mask, 127, 255, THRESH_BINARY);
			// medianBlur(mask, mask, 21);
			// const int size = 100;
			// blur(mask, mask, Size(size, size));
			// blur(mask, mask, Size(size, size));

#ifdef PC
			imshow("mask2", mask);
			// imshow("mask2", drawing);
#endif
			// waitKey(300);
			// outputVideo << inputFrame;
			// drawing = inputFrame;
			// continue;

			// ------------------- BOUNDING BOXING MOVEMENTS
			std::vector<std::vector<Point>> contours;
			std::vector<Vec4i> hierarchy;
			findContours(mask, contours, hierarchy, RETR_TREE,
						 CHAIN_APPROX_SIMPLE, Point(0, 0));

			int nbMovements = contours.size();
			// if (nbMovements > 9) {
			// 	outputVideo << inputFrame;
			// 	continue;
			// }
			std::vector<std::vector<Point>> contours_poly(nbMovements);
			std::vector<Rect> boundRect(nbMovements);
			for (int i = 0; i < nbMovements; ++i) {
				approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
				boundRect[i] = boundingRect(Mat(contours_poly[i]));
			}

			std::vector<Moments> mu(nbMovements);
			for (int i = 0; i < nbMovements; ++i) {
				mu[i] = moments(contours[i], true);
			}

			drawing = inputFrame.clone();
			std::vector<Point2f> mc(nbMovements);
			for (int i = 0; i < nbMovements; ++i) {
				mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
			}

			// ------------------- FIND PREVIOUS OBJECTS POSITIONS
			// std::vector<int> nearestObj(nbMovements);
			// std::vector<double> distNearestObj(nbMovements);
			// std::vector<int> nearestMov(nbObjects);

			// for (int i = 0; i < nbObjects; ++i) {
			// 	nearestMov[i] = -1;
			// }

			// const int thresh = MAX_ERROR_DIST_FOR_NEW_POS_OBJECT;
			// for (int i = 0; i < nbMovements; ++i) {
			// 	nearestObj[i] = -1;
			// 	distNearestObj[i] = thresh;
			// }

			bool movFound[nbMovements] = {0};
			// find for each object the closest movement
			// int iObj = 0;
			// std::vector<Object&> objectsVector;
			// for (Object & obj : objects) {
			//     objectsVector.emplace_back(obj);
			// }
			std::set<Object> newObjects;
			// std::vector<std::unique_ptr<Object>> trash;
			// std::vector<Object> objectsVector(objects.begin(),
			// objects.end());
			std::set<Object>::iterator it = objects.begin();
			while (it != objects.end()) {
				// Object & obj = *it;
				// for (const Object &cobj : objects) {
				Object &obj = const_cast<Object &>(*it);
				// auto obj = *it;
				// for (auto &obj : objects) {
				// std::cout << "object " << obj.id << std::endl;
				// std::cout << "[" << obj.id << "] age:" << obj.age
				// 		  << ", dens:" << obj.density
				// 		  << ", dist:" << obj.distance << std::endl;

				int iMov = -1;
				int distMovMin = MAX_ERROR_DIST_FOR_NEW_POS_OBJECT;
				for (int i = 0; i < nbMovements; ++i) {
					// if (nearestObj[i] != -1) {
					if (!movFound[i]) {
						// double density = abs(obj.density - mu[i].m00);
						double density = 3.0 * fmax(obj.density, mu[i].m00) /
										 fmin(obj.density, mu[i].m00);

						// if (abs(density - 1.0) )
						// if (density > DELTA_DIFF_MAX_DENSITY) {
						// continue;
						// }

						double distance =
							norm((obj.pos + obj.speedVector) - mc[i]);

						// double dist = distance;

						// std::cout << "[" << obj.id << ", " << i << "] ";
						// std::cout << "distance : " << distance
						// 		  << ", density : " << density << std::endl;
						// double dist =
						// 	pow(norm((obj.pos + obj.speedVector) - mc[i]), 2) +
						double dist = distance + density;
						// abs(obj.density - mu[i].m00); pow(norm((obj.pos +
						// obj.speedVector) - mc[i]) +

						if (dist < distMovMin) {
							distMovMin = dist;
							iMov = i;
						}
					}
				}

				// std::cout << "[" << obj.id << "] ";
				// no event near object
				if (iMov == -1) {
					// std::cout << ", no movement near, ";
					// if (obj.distance < MIN_MOV_DIST_TO_SAVE_OBJECT) {
					if (obj.age < MIN_MOV_YEARS_TO_SAVE_OBJECT) {
						// std::cout << "object killed";
						tombs.push_back({obj.pos, obj.color});

						// Object & del = obj;
						// --it;
						// objects.erase(it++);
						// trash.push_back(std::make_unique<Object>(obj));
						// ++nbDeleteObj;

					}
					// save position of no moved object
					else {
						// std::cout << "object saved";
						putText(drawing, "s", obj.pos + Point2f(-9, 9),
								FONT_HERSHEY_DUPLEX, 1, obj.color, 1);
						// ++it;
						newObjects.insert(std::move(obj));
					}

				}
				// object moved
				else {
					// std::cout << "is moving ";
					movFound[iMov] = true;

					Point2i tl = boundRect[iMov].tl();
					Point2i tr = tl + Point2i(boundRect[iMov].width + 5, 10);

					Capture cap{Mat(inputFrame, boundRect[iMov]).clone(),
								Mat(mask, boundRect[iMov]).clone(),
								contours[iMov], boundRect[iMov],
								static_cast<int>(mu[iMov].m00)};

					// auto node = objects.extract(it);

					obj.trace.emplace_back(cap);

					// if found best trace
					if (mu[iMov].m00 > obj.trace[obj.bestCapture].density) {
						obj.bestCapture = obj.trace.size() - 1;
					}
					obj.speedVector = mc[iMov] - obj.pos;
					// obj.distance =
					// std::max(obj.distance, norm(mc[iMov] - obj.firstPos));
					obj.distance += norm(mc[iMov] - obj.firstPos);

					// lines.push_back({obj.pos, mc[iMov], obj.color});
					obj.lines.push_back({obj.pos, mc[iMov], obj.color});
					obj.pos = mc[iMov];
					obj.density = mu[iMov].m00;
					++obj.age;

					rectangle(drawing, tl, boundRect[iMov].br(), obj.color, 2);

					if (obj.name.compare("")) {
						putText(drawing, obj.name, tr + Point(0, 0),
								FONT_HERSHEY_DUPLEX, 0.5, obj.color, 1);
					} else {
						putText(drawing, std::to_string(obj.id),
								tr + Point(0, 0), FONT_HERSHEY_DUPLEX, 0.5,
								obj.color, 1);
					}
					putText(drawing,
							std::to_string(static_cast<int>(obj.distance)),
							tr + Point(0, 20), FONT_HERSHEY_DUPLEX, 0.5,
							obj.color, 1);
					putText(drawing, std::to_string(obj.density),
							tr + Point(0, 40), FONT_HERSHEY_DUPLEX, 0.5,
							obj.color, 1);

					line(drawing, obj.pos, obj.pos + obj.speedVector,
						 Scalar(0, 0, 255), 1, LineTypes::LINE_AA);

					if (recon && objects.size() <= 10) {
						if (!obj.name.compare("")) {
							Triplet triplet =
								getPrimaryColor(cap.img, cap.mask);
							putText(drawing, std::string(triplet),
									tr + Point(0, -20), FONT_HERSHEY_DUPLEX,
									0.5, obj.color, 1);

							std::string bestPath = "";
							double bestDist = 640 * 480;
							for (const auto &pair : boxes) {
								std::string path = pair.first;
								std::tuple<Point3f, Point3f, Point3f> bound =
									pair.second;
								const Point3f &Min = std::get<0>(bound);
								const Point3f &Mean = std::get<1>(bound);
								const Point3f &Max = std::get<2>(bound);

								if (triplet.in(Min, Max)) {
									double curDist = triplet.dist(Mean);
									if (curDist < bestDist) {
										curDist = bestDist;
										bestPath = path;
									}
								}
							}

							if (bestPath.compare("")) {
								// std::cout << "object " << obj.id
								// 		  << " was detected as : " << bestPath
								// 		  << std::endl;

								imwrite("alert.jpg", drawing);
								if (script) {
									cmd = "./script.sh " + bestPath + " &";
									system(cmd.c_str());
								}
								std::thread thread(
									thread_alert,
									std::string("alert/" + bestPath));
								thread.detach();

								obj.name = bestPath;
								boxes.erase(bestPath);
							}
						}
					}

					newObjects.insert(std::move(obj));
					// // if movement already chosen by other object
					// if (nearestObj[iMovMin] != -1) {
					// 	// if nearest by other
					// 	if (distMovMin < distNearestObj[iMovMin]) {
					// 		if (obj.age >
					// 			objectsVector[nearestObj[iMovMin]].age) {

					// 			nearestMov[nearestObj[iMovMin]] = -2;

					// 			distNearestObj[iMovMin] = distMovMin;
					// 			nearestObj[iMovMin] = iObj;

					// 			nearestMov[iObj] = iMovMin;
					// 		} else {
					// 			nearestMov[iObj] = -2;
					// 		}
					// 	} else {
					// 		nearestMov[iObj] = -2;
					// 	}
					// } else {
					// distNearestObj[iMov] = distMovMin;
					// nearestObj[iMov] = iObj;

					// ++it;
					// nearestMov[iObj] = iMov;
					// }
				} // if (iMov != -1) {

				// ++iObj;
				// std::cout << std::endl;
				++it;
				// ++it;
			} // while (it != objects.end()) {

			objects = std::move(newObjects);
			// for (auto &ptr : trash) {
			// 	objects.erase(*ptr);
			// }
			// new movement become new object if no previous object near
			// ObjList newObjects;
			// std::set<Object> newObjects;
			// std::set newObjects;
			for (int i = 0; i < nbMovements; ++i) {

				// int iObj = nearestObj[i];
				int density = mu[i].m00;
				// new object
				if (!movFound[i] && density > NEW_OBJECT_MIN_DENSITY) {
					// if (iObj == -1) {
					Scalar color =
						Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
							   rng.uniform(0, 255));

					Capture cap{Mat(inputFrame, boundRect[i]).clone(),
								Mat(mask, boundRect[i]).clone(), contours[i],
								boundRect[i], density};

					std::vector<std::vector<Point>> contour{contours[i]};
					drawContours(drawing, contour, 0, color, 2);
					// circle(drawing, mc[i], MAX_ERROR_DIST_FOR_NEW_POS_OBJECT,
					// 	   color, 1);
					// circle(drawing, mc[i], MAX_ERROR_DIST_FOR_NEW_POS_OBJECT,
					//    color, 1);

					Object obj{0.0,   mc[i],	 density, Vec2f(0, 0),
							   color, iNewObj++, 0,		  {std::move(cap)},
							   mc[i], 0};
					// newObjects.emplace_back(std::move(obj));
					// newObjects.insert(std::move(obj));
					objects.insert(std::move(obj));
				}
			}
			nbObjects = objects.size();

			// for (auto &obj : newObjects) {
			// 	objects.push_back(obj);
			// }
			// objects.insert(objects.end(), newObjects.begin(),
			// newObjects.end()); objects.insert()

			// objects.merge(newObjects);
			// putText(drawing, "nbMovs : " + std::to_string(nbMovements),
			// 		Point(0, 30), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 255));

			putText(drawing, "nbObjs : " + std::to_string(nbObjects),
					Point(0, 30), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 255));

			putText(drawing, "frame : " + std::to_string(iCap), Point(0, 60),
					FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 255));

			// for (size_t i = 0; i < lines.size(); ++i) {
			// 	line(drawing, lines[i].p, lines[i].p2, lines[i].color, 2);
			// }
			for (auto &obj : objects) {
				for (auto &l : obj.lines) {
					line(drawing, l.p, l.p2, obj.color, 2);
				}
			}

			for (DeadObj obj : tombs) {
				putText(drawing, "x", obj.p + Point(-9, 9), FONT_HERSHEY_DUPLEX,
						1, obj.color, 1);
			}

#ifdef PC
			imshow("drawing", drawing);
			// imshow("mask", mask);
			// imshow("grey", grey);
			// if (waitKey(300) == 'q')
			// break;
#endif

			outputVideo << drawing;

		} // while (hasMovement())
		vCap.release();

		// std::string motionId = getDay() + "_" + getCurTime() + "_" + hostname
		// + "_" + deviceName;
		std::string trainingPath = trainDir + getDay() + "_" + motionId;
		// getHostname() + "_" + std::to_string(device) + "_" +
		// hostname + "_" + getDay() + "_" + getCurTime() + "_" + deviceName;

		int nbRealObjects = 0;
		for (const Object &obj : objects) {
			// assert(obj.trace[obj.bestCapture]);

			// if (obj.distance > MIN_MOV_DIST_TO_SAVE_OBJECT) {
			if (obj.age > MIN_MOV_YEARS_TO_SAVE_OBJECT) {

				const Capture &bestCapture = obj.trace[obj.bestCapture];
				const Mat &m = bestCapture.img;

				if (bestCapture.rect.empty()) {
					assert(!bestCapture.img.empty());
					imshow("bestCapture", bestCapture.img);
					waitKey(0);
					continue;
				}
				assert(!bestCapture.rect.empty());
				assert(!bestCapture.mask.empty());
				assert(bestCapture.rect.size() == bestCapture.mask.size());

				m.copyTo(Mat(drawing, bestCapture.rect), bestCapture.mask);
				std::vector<std::vector<Point>> contours{bestCapture.contour};
				drawContours(drawing, contours, 0, obj.color, 2);
				++nbRealObjects;

				if (training) {
					// std::string newMotionDir = motionDir + startTime + "_" +
					// hostname;
					std::string newTrainingFile =
						trainingPath +
						// getHostname() + "_" + std::to_string(device) + "_" +
						// getHostname() + "_" + getDay() + "_" + getCurTime() +
						"_" + std::to_string(obj.id) + "_";

					Mat a;
					for (size_t i = 0; i < obj.trace.size(); ++i) {
						const Capture &cap{obj.trace[i]};
						const Mat &img{cap.img};
						const Mat &mask{cap.mask};

						const std::string dir =
							newTrainingFile + std::to_string(i) + "/";
						std::cout << "new training event '" << dir << "'"
								  << std::endl;

						cmd = "mkdir -p " + dir;
						system(cmd.c_str());

						img.copyTo(a, cap.mask);
						imwrite(dir + "image.jpg", a);

						Mat histImg;
						Triplet triplet = hsvHist(img, mask, histImg);
						imwrite(dir + "hist.jpg", histImg);

						triplet.write(dir + "primary.txt");
					}
				}
			}
		}

		// if (onlyRec) {
		// 	drawing = inputFrame;
		// 	outputVideoRec << drawing;
		// 	outputVideoRec.release();
		// 	// } else if (streamFinished) {
		// 	// drawing = inputFrame;
		// }

		outputVideo << drawing;
		// outputVideoFileRec << drawing;
		outputVideo.release();
		outputVideoRec.release();
		std::cout << "save video '" << outputVideoFile << "'" << std::endl;

		// if (iCap >= NB_CAP_FOCUS_BRIGHTNESS + NB_CAP_LEARNING_MODEL_FIRST +
		// 				NB_CAP_MIN_FOR_REAL_MOTION) {

		imwrite(newMotionDir + "/trace.jpg", drawing);
		std::cout << "save trace file '" << newMotionDir + "/trace.jpg'"
				  << std::endl;

		if (hasRemoteDir) {
			if (port == -1) {
				cmd = "rsync -arv " + newMotionDir + " " + remoteDir;
			} else {
				cmd = "rsync -arv -e 'ssh -p " + std::to_string(port) + "' " +
					  newMotionDir + " " + remoteDir;
			}
			std::cout << cmd << std::endl;
			system(cmd.c_str());
		}
		// }

#ifdef PC
		destroyAllWindows();
#endif

		if (lightGpio != -1) {
			gpioSetValue(lightGpio, 0);
		}

		if (streamFinished) {
			return 0;
		}
	} // while (1)

	return 0;
}