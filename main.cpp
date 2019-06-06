#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp" // model
// #include "opencv2/videoio.hpp"

#include "opencv2/imgproc/imgproc.hpp" // bounding boxes

#include <cassert>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <unistd.h>

#ifdef PC
#define TIMELAPSE_INTERVAL 30 // 30 sec
#else
#define TIMELAPSE_INTERVAL 1800 // 30 min
#endif

#define THRESH_MOV_IS_OBJECT 50
#define NB_CAP_LEARNING_MODEL_FIRST 20
#define NB_CAP_FOCUS_BRIGHTNESS 10
#define NB_CAP_MIN_FOR_REAL_MOTION 5

#define NEW_OBJECT_MIN_DENSITY 10
#define MAX_DIST_NEW_POS 100000
#define MIN_MOV_YEARS_FOR_OBJECT 5
// #define TIMELAPSE_INTERVAL 30 // secondes

using namespace cv;

RNG rng(29791);

typedef struct s_capture {
	Mat img;
	Mat mask;
	std::vector<Point> contour;
	Rect rect;
	int density;
} Capture;

typedef struct s_object {
	double distance; // from first pos
	Point2f pos;
	int density;
	Point2f speedVector;
	Scalar color;
	int id;
	// Capture bestCapture;
	int bestCapture;
	std::vector<Capture> trace;
	Point2f firstPos;
	uint years;
} Object;

typedef struct s_line {
	Point p;
	Point p2;
	Scalar color;
} Line;

typedef struct s_deadObj {
	Point p;
	Scalar color;
} DeadObj;

std::string gpioDir;
// std::string gpioDir = "/sys/class/gpio/";

void initGpio(int gpio, std::string direction) {
	std::string gpioExportDir = gpioDir + "export";
	std::ofstream exportGpio(gpioExportDir.c_str());
	if (!exportGpio.is_open()) {
		std::cout << "unable to export gpio" << std::endl;
		perror(gpioExportDir.c_str());
		exit(1);
	}
	exportGpio << gpio;
	exportGpio.close();
	usleep(100000);

	std::string gpioNumDir = gpioDir + "gpio" + std::to_string(gpio) + "/";
	std::string gpioNumDirectionDir = gpioNumDir + "direction";
	std::ofstream directionGpio(gpioNumDirectionDir.c_str());
	if (!directionGpio.is_open()) {
		std::cout << "unable to set direction gpio" << std::endl;
		perror(gpioNumDirectionDir.c_str());
		exit(2);
	}
	directionGpio << direction;
	directionGpio.close();
	usleep(100000);
}

int gpioGetValue(int gpio) {
	std::string gpioValueFile =
		gpioDir + "gpio" + std::to_string(gpio) + "/value";
	std::ifstream getValueGpio(gpioValueFile.c_str());
	if (!getValueGpio.is_open()) {
		std::cout << "unable to get value gpio" << std::endl;
		perror(gpioValueFile.c_str());
		exit(3);
	}

	int val;
	getValueGpio >> val;
	getValueGpio.close();
	// usleep(100000);

	return val;
}

void gpioSetValue(int gpio, int value) {
	std::string gpioValueFile =
		gpioDir + "gpio" + std::to_string(gpio) + "/value";
	std::ofstream setValueGpio(gpioValueFile.c_str());
	if (!setValueGpio.is_open()) {
		std::cout << "unable to set value gpio" << std::endl;
		perror(gpioValueFile.c_str());
		exit(4);
	}

	setValueGpio << value;
	setValueGpio.close();
	// usleep(100000);
}

std::string getHostname() {
	std::string file = "/etc/hostname";
	std::ifstream getFile(file.c_str());
	if (!getFile.is_open()) {
		std::cout << "unable to open /etc/hostname" << std::endl;
		perror(file.c_str());
		exit(4);
	}

	std::string ret;
	getFile >> ret;
	getFile.close();

	return ret;
}

std::string getCurTime() {
	time_t t = time(0);
	tm *now = localtime(&t);

	int hour = now->tm_hour;
	std::string hour_str = std::to_string(hour);
	if (hour < 10) {
		hour_str = "0" + hour_str;
	}

	int min = now->tm_min;
	std::string min_str = std::to_string(min);
	if (min < 10)
		min_str = "0" + min_str;

	int sec = now->tm_sec;
	std::string sec_str = std::to_string(sec);
	if (sec < 10)
		sec_str = "0" + sec_str;

	return hour_str + ':' + min_str + ':' + sec_str;
}

std::string getDay() {
	time_t t = time(0);
	tm *now = localtime(&t);

	int year = now->tm_year + 1900;
	int month = now->tm_mon + 1;
	int day = now->tm_mday;

	return std::to_string(year) + ":" + std::to_string(month) + ":" +
		   std::to_string(day);
}

int sensorGpioNum;
int sensorAdditional;

bool hasMovement() {
	bool ret = false;

	if (sensorGpioNum != -1) {
		ret = gpioGetValue(sensorGpioNum) == 1;

		if (sensorAdditional != -1) {
			ret = ret && gpioGetValue(sensorAdditional) == 1;
		}
	}

	return ret;
}

bool isNight() {
	time_t t = time(0);
	tm *now = localtime(&t);

	int hour = now->tm_hour;

	return 19 < hour && hour < 23;
}

// ------------------------------- MAIN ---------------------------------------
int main(int argc, char **argv) {
	// VideoCapture cap("1car.avi");
	// VideoCapture cap("3car3person.avi");
	// if (!cap.isOpened()) {
	// 	std::cout << "file not found";
	// 	return 1;
	// }
	CommandLineParser parser(
		argc, argv,
		"{s sensor      | -1            | gpio number of IR senror}"
		"{a and         | -1            | add detect sensor, and logic}"
		"{l light       | -1            | light up on movment}"
		// "{n not         | -1             | sensor disable motion}"
		"{d device      | 0             | device camera, /dev/video0 by "
		"default}"
		"{f flip        | false         | rotate image 180}"
		"{r repository  | "
		"            | save motion to specific repository}"
		"{p port        | -1            | remote port repository}"
		"{help h        |               | help message}"
		"{vegetation v  | false         | outside camera}"
		"{inVideo       |               | video in}"
		"{training take | false         | save movement capture for post "
		"learning}"
		//
	);

	if (parser.has("help")) {
		parser.printMessage();
		return 0;
	}
	sensorGpioNum = parser.get<int>("sensor");
	// int sensorNotMov = parser.get<int>("not");
	sensorAdditional = parser.get<int>("and");
	int device = parser.get<int>("device");
	std::string remoteDir = parser.get<std::string>("repository");
	int port = parser.get<int>("port");
	bool flip180 = parser.get<bool>("flip");
	int lightGpio = parser.get<int>("light");
	bool hasVegetation = parser.get<bool>("vegetation");
	bool training = parser.get<bool>("training");
	// std::string inVideo = parser.get<std::string>("inVideo");
	// std::cout << "inVideo = " << inVideo << std::endl;
	if (!parser.check()) {
		parser.printMessage();
		parser.printErrors();
		return 0;
	}

	// if (sensorGpioNum == -1) {
#ifdef PC
	gpioDir = "gpio/";
#else
	// } else {
	gpioDir = "/sys/class/gpio/";
#endif
	// }

	// bool hasVideo = ! inVideo.empty();
	bool hasRemoteDir = !remoteDir.empty();
	using ObjList = std::list<Object>;
	std::string motionDir;
	if (hasRemoteDir) {
		motionDir = "/tmp/motion/";
		// trainDir = "/tmp/learning/";

	} else {
		motionDir = "motion/";
		// trainDir = "learning/";
	}
	std::string hostname = getHostname();

	auto model = createBackgroundSubtractorKNN();
	// auto model = createBackgroundSubtractorMOG2();

	VideoCapture vCap;

	std::string timelapseDir =
		motionDir + "timelapse_" + hostname + "_" + std::to_string(device);
	std::string cmd = "mkdir -p " + timelapseDir;
	system(cmd.c_str());

	std::string trainDir = "";
	if (training) {
		trainDir = "learningFile/newEvent/";
		cmd = "mkdir -p " + trainDir;
		system(cmd.c_str());

		cmd = "mkdir -p learningFile/known/";
		system(cmd.c_str());
	}

	ObjList objects;
	std::vector<Line> lines;
	std::vector<DeadObj> tombs;

	Mat inputFrame, mask, drawing;
	int iNewObj;
	// size_t tickCapture;
	// size_t cur;
	// int iSec;
	int iCap;
	// size_t tickTimeLapse = clock() + CLOCKS_PER_SEC; // take picture
	// immediately

	int tickTimeLapse = 1; // take picture immediately

	vCap.open(device);
	if (!vCap.isOpened()) {
		std::cout << "device not found";
		return 1;
	}
	int width = vCap.get(CAP_PROP_FRAME_WIDTH);
	int height = vCap.get(CAP_PROP_FRAME_HEIGHT);
	Size sizeScreen(width, height);
	vCap.release();

	Mat notGreen = Mat::zeros(Size(width, height), CV_8UC3);
	notGreen = Scalar(255, 0, 255);
	// std::cout << "first tick TimeLapse " << tickTimeLapse << std::endl;

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

	// bool firstLapse = true;

	// if (sensorNotMov != -1) {
	// 	initGpio(sensorNotMov);
	// 	gpioGetValue(sensorNotMov);
	// }

	// --------------------------- INFINITE LOOP ------------------------------
	// bool isNotMov = false;
	// if (sensorNotMov != -1) {
	// 	isNotMov = gpioGetValue(sensorNotMov) == 1;
	// }
	while (1) {
		// std::cout << "wait new motion, future lapse at " << tickTimeLapse <<
		// "clock = " << clock()
		std::cout << "[TIMELAPSE] wait new motion, future lapse at "
				  << tickTimeLapse << " sec " << std::endl;
		while (!hasMovement()) {

			std::cout << "." << std::flush;
			usleep(CLOCKS_PER_SEC);
			// tickTimeLapse -= CLOCKS_PER_SEC;
			--tickTimeLapse;
			// std::cout << "first tick TimeLapse " << tickTimeLapse <<
			// std::endl;

			// cur = clock();
			// if (cur > tickTimeLapse) {
			if (tickTimeLapse == 0) {
				std::cout << std::endl;
				// std::cout << "[TIMELAPSE] cur > tickTimelapse : take new
				// lapse" << std::endl;
				vCap.open(device);
				// openCap(vCap);
				if (!vCap.isOpened()) {
					std::cout << "device not found";
					return 1;
				}
				for (int i = 0; i < NB_CAP_FOCUS_BRIGHTNESS; ++i)
					vCap >> inputFrame;

				if (flip180) {
					flip(inputFrame, inputFrame, -1);
				}
				vCap.release();

				std::string saveLapse =
					timelapseDir + "/" + getCurTime() + ".jpg";
				imwrite(saveLapse, inputFrame);
				imwrite(timelapseDir + "/latest.jpeg", inputFrame);
				std::cout << "[TIMELAPSE] save lapse '" << saveLapse << "'"
						  << std::endl;

				// std::string gifFile = timelapseDir + "/timelapse.gif";
				// if (firstLapse) {
				cmd = "convert " + timelapseDir + "/*.jpg " + timelapseDir +
					  "/timelapse.gif";
				// cmd = "convert " + saveLapse + " " + gifFile;
				// firstLapse = false;
				// } else {
				// cmd =
				// "convert " + gifFile + " " + saveLapse + " " + gifFile;
				// }
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

				// tickTimeLapse = clock() + TIMELAPSE_INTERVAL *
				// CLOCKS_PER_SEC;
				tickTimeLapse = TIMELAPSE_INTERVAL;
				std::cout << "[TIMELAPSE] future lapse at " << tickTimeLapse
						  << " sec " << std::endl;
				// std::cout << "TIMELAPSE: tickTimeLapse = " << tickTimeLapse
				// << ", clock = " << clock() << std::endl;
			}
			// if (sensorNotMov != -1) {
			// 	isNotMov = gpioGetValue(sensorNotMov) == 1;
			// }
		} // while (!hasMovement())
		std::cout << std::endl;

		if (lightGpio != -1 && isNight()) {
			gpioSetValue(lightGpio, 1);
		}

		// create new directory in /tmp/motion/
		std::string startTime = getCurTime();
		std::string newMotionDir = motionDir + startTime + "_" + hostname +
								   "_" + std::to_string(device);
		cmd = "mkdir -p " + newMotionDir;
		system(cmd.c_str());

		vCap.open(device);
		if (!vCap.isOpened()) {
			std::cout << "device not found";
			return 1;
		}

		// VideoWriter outputVideo(newMotionDir + "/video.avi",
		// 						cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 10,
		// 						sizeScreen, true);
		std::string outputVideoFile = newMotionDir + "/video.webm";
		VideoWriter outputVideo(outputVideoFile,
								cv::VideoWriter::fourcc('V', 'P', '8', '0'), 2,
								sizeScreen, true);
		// VideoWriter outputVideo(newMotionDir + "/video.ogv",
		// 						cv::VideoWriter::fourcc('T', 'H', 'E', 'O'), 2,
		// 						sizeScreen, true);
		if (!outputVideo.isOpened()) {
			std::cout << "failed to write video" << std::endl;
			return 6;
		}

		vCap >> inputFrame;
		// std::string saveCap = newMotionDir + "/cap_00.jpg";
		// std::cout << "save immediate capture '" << saveCap << "'" <<
		// std::endl; if (!imwrite(saveCap, inputFrame)) { 	std::cout << "failed
		// to save immediate cap" << std::endl; perror(newMotionDir.c_str());
		// 	return 2;
		// }
		outputVideo << inputFrame;

		// outputVideo.open(newMotionDir + "/clip.avi", VideoWriter::fourcc('M',
		// 'J', 'P', 'G'), 2, Size(vCap.get(cv::CAP_PROP_FRAME_WIDTH),
		// 					  vCap.get(CAP_PROP_FRAME_HEIGHT)));
		// if (!outputVideo.isOpened()) {
		// 	std::cout << "could not open output video" << std::endl;
		// 	return 5;
		// }

		iNewObj = 0;
		// iSec = 0;
		iCap = -1;
		// tickCapture = clock() + CLOCKS_PER_SEC;
		// tickCapture = clock() + CLOCKS_PER_SEC; // take immediate capture
		lines.clear();
		tombs.clear();
		objects.clear();

		while (hasMovement()) {
			++iCap;
			// std::cout << "capture " << ++iCap << std::endl;
			int nbObjects = objects.size();

			vCap >> inputFrame;
			if (inputFrame.empty()) {
				std::cout << "Finished reading" << std::endl;
				break;
			}

			// ------------------- TRAINING MODEL -----------------------------
			Mat grey;
			if (hasVegetation) {
				Mat inputWithoutGreen;
				bitwise_and(inputFrame, notGreen, inputWithoutGreen);
				cvtColor(inputWithoutGreen, grey, COLOR_BGR2GRAY);
				// imshow("notGreen", inputWithoutGreen);
			} else {
				cvtColor(inputFrame, grey, COLOR_BGR2GRAY);
			}
			equalizeHist(grey, grey);
			model->apply(grey, mask);

			if (iCap < NB_CAP_FOCUS_BRIGHTNESS + NB_CAP_LEARNING_MODEL_FIRST) {
				continue;
			}

			// ------------------- BOUNDING MOVMENT ---------------------------
			const int size = 15;
			medianBlur(mask, mask, size);
			blur(mask, mask, Size(size, size));

			threshold(mask, mask, 0, 255, THRESH_BINARY);

			std::vector<std::vector<Point>> contours;
			std::vector<Vec4i> hierarchy;
			findContours(mask, contours, hierarchy, RETR_TREE,
						 CHAIN_APPROX_SIMPLE, Point(0, 0));

			int nbMovements = contours.size();
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

			std::vector<int> nearestObj(nbMovements);
			std::vector<double> distNearestObj(nbMovements);
			std::vector<int> nearestMov(nbObjects);

			for (int i = 0; i < nbObjects; ++i) {
				nearestMov[i] = -1;
			}

			const int thresh = MAX_DIST_NEW_POS;
			for (int i = 0; i < nbMovements; ++i) {
				nearestObj[i] = -1;
				distNearestObj[i] = thresh;
			}

			// find for each object the closest movement
			int iObj = 0;
			for (const auto &obj : objects) {

				int iMovMin = -1;
				int distMovMin = thresh;
				for (int i = 0; i < nbMovements; ++i) {
					double dist =
						pow(norm((obj.pos + obj.speedVector) - mc[i]), 2) +
						abs(obj.density - mu[i].m00);

					if (dist < distMovMin) {
						distMovMin = dist;
						iMovMin = i;
					}
				}

				if (iMovMin != -1) {
					if (nearestObj[iMovMin] != -1) {
						if (distMovMin < distNearestObj[iMovMin]) {
							nearestMov[nearestObj[iMovMin]] = -1;

							distNearestObj[iMovMin] = distMovMin;
							nearestObj[iMovMin] = iObj;

							nearestMov[iObj] = iMovMin;
						}
					} else {
						distNearestObj[iMovMin] = distMovMin;
						nearestObj[iMovMin] = iObj;

						nearestMov[iObj] = iMovMin;
					}
				}

				++iObj;
			}

			// new movement become new object if no previous object near
			ObjList newObjects;
			for (int i = 0; i < nbMovements; ++i) {

				int iObj = nearestObj[i];
				int density = mu[i].m00;
				// new object
				if (iObj == -1 && density > NEW_OBJECT_MIN_DENSITY) {
					Scalar color =
						Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
							   rng.uniform(0, 255));

					Capture cap{Mat(inputFrame, boundRect[i]).clone(),
								Mat(mask, boundRect[i]).clone(), contours[i],
								boundRect[i], density};
					// std::vector<Capture> firstCap {cap};

					Object &&obj{0.0,   mc[i],	 density, Vec2f(0, 0),
								 color, iNewObj++, 0,		{std::move(cap)},
								 mc[i], 0};
					newObjects.emplace_back(obj);
				}
			}

			iObj = 0;
			auto it = objects.begin();
			int nbDeleteObj = 0;
			while (it != objects.end()) {
				Object &obj = *it;
				int iMov = nearestMov[iObj];

				// delete object if not moving (no event)
				if (iMov == -1) {
					// delete passing object event
					if (obj.distance < THRESH_MOV_IS_OBJECT) {
						tombs.push_back({obj.pos, obj.color});

						objects.erase(it++);
						++nbDeleteObj;

					}
					// save position of motionless object
					else {
						putText(drawing, "s", obj.pos + Point2f(-9, 9),
								FONT_HERSHEY_DUPLEX, 1, obj.color, 1);
						++it;
					}

				}
				// movement object
				else {
					Capture cap{Mat(inputFrame, boundRect[iMov]).clone(),
								Mat(mask, boundRect[iMov]).clone(),
								contours[iMov], boundRect[iMov],
								static_cast<int>(mu[iMov].m00)};

					obj.trace.emplace_back(std::move(cap));

					if (mu[iMov].m00 > obj.trace[obj.bestCapture].density) {
						Capture cap{Mat(inputFrame, boundRect[iMov]).clone(),
									Mat(mask, boundRect[iMov]).clone(),
									contours[iMov], boundRect[iMov],
									static_cast<int>(mu[iMov].m00)};

						obj.bestCapture = obj.trace.size();
					}
					obj.speedVector = mc[iMov] - obj.pos;
					// obj.dist += norm(obj.speedVector);
					obj.distance =
						std::max(obj.distance, norm(mc[iMov] - obj.firstPos));
					lines.push_back({obj.pos, mc[iMov], obj.color});
					obj.pos = mc[iMov];
					obj.density = mu[iMov].m00;
					++obj.years;

					Point2i tl = boundRect[iMov].tl();
					rectangle(drawing, tl, boundRect[iMov].br(), obj.color, 2);

                    tl += Point2i(boundRect[iMov].width + 5, 10);

					putText(drawing, std::to_string(obj.id),
							tl + Point(0, 0), FONT_HERSHEY_DUPLEX, 0.5,
							obj.color, 1);
					putText(drawing,
								std::to_string(static_cast<int>(obj.distance)),
							tl + Point(0, 20), FONT_HERSHEY_DUPLEX, 0.5,
							obj.color, 1);
					putText(drawing, std::to_string(obj.density),
							tl + Point(0, 40), FONT_HERSHEY_DUPLEX, 0.5,
							obj.color, 1);

					line(drawing, obj.pos, obj.pos + obj.speedVector * 2,
						 Scalar(0, 0, 255), 1, LineTypes::LINE_AA);

					++it;
				}

				++iObj;
			}

			for (auto &obj : newObjects) {
				objects.push_back(obj);
			}

			putText(drawing, "nbObjs : " + std::to_string(nbObjects),
					Point(0, 30), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 255));

			putText(drawing, "frame : " + std::to_string(iCap), Point(0, 60),
					FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 255));

			for (size_t i = 0; i < lines.size(); ++i) {
				line(drawing, lines[i].p, lines[i].p2, lines[i].color, 2);
			}

			for (DeadObj obj : tombs) {
				putText(drawing, "x", obj.p + Point(-9, 9), FONT_HERSHEY_DUPLEX,
						1, obj.color, 1);
			}

#ifdef PC
			imshow("drawing", drawing);
			imshow("mask", mask);
			imshow("grey", grey);
			if (waitKey(300) == 'q')
				break;
#endif

			// cur = clock();
			// if (cur > tickCapture) {
			//     ++iSec;

			// 	std::string iSec_str = std::to_string(iSec);
			// 	if (iSec < 10) {
			// 		iSec_str = "0" + iSec_str;
			// 	}

			// 	std::string saveCap = newMotionDir + "/cap_" + iSec_str +
			// ".jpg"; 	std::cout << "save capture '" << saveCap << "'" <<
			// std::endl; 	if (!imwrite(saveCap, drawing)) { 		std::cout <<
			// "failed to save cap" << std::endl;
			// perror(newMotionDir.c_str()); 		return 2;
			// 	}

			// 	// outputVideo << drawing;

			// 	tickCapture = cur + CLOCKS_PER_SEC;
			// }

			outputVideo << drawing;
			// if (sensorNotMov != -1) {
			// 	isNotMov = gpioGetValue(sensorNotMov) == 1;
			// }

		} // while (hasMovement())
		vCap.release();

		int nbRealObjects = 0;
		// if (objects.size() > 0) {

		for (Object obj : objects) {
			Capture &bestCapture = obj.trace[obj.bestCapture];
			Mat m = bestCapture.img;

			m.copyTo(Mat(drawing, bestCapture.rect), bestCapture.mask);
			std::vector<std::vector<Point>> contours{bestCapture.contour};
			drawContours(drawing, contours, 0, obj.color, 2);

			if (obj.distance > THRESH_MOV_IS_OBJECT &&
				obj.years > MIN_MOV_YEARS_FOR_OBJECT) {
				++nbRealObjects;

				if (training) {
					std::string newTrainingFile =
						getHostname() + "_" + std::to_string(device) + "_" +
						getDay() + "_" + getCurTime() + "_" +
						std::to_string(obj.id) + "_";

					Mat a;
                    // Mat img;
					// for (const Capture &cap : obj.trace) {
					for (size_t i = 0; i < obj.trace.size(); ++i) {
						const Capture &cap = obj.trace[i];
                        Mat img = cap.img;
						std::string file = trainDir + newTrainingFile +
										   std::to_string(i) + ".jpg";

						cap.img.copyTo(a, cap.mask);
						imwrite(file, a);

						std::cout << "save training file '" << file << "'"
								  << std::endl;
					}
				}
			}
		}

		outputVideo << drawing;
		outputVideo.release();
		std::cout << "save video '" << outputVideoFile << "'" << std::endl;

		// if (nbRealObjects > 0) {
		if (iCap >= NB_CAP_FOCUS_BRIGHTNESS + NB_CAP_LEARNING_MODEL_FIRST +
						NB_CAP_MIN_FOR_REAL_MOTION) {
			// if (true) {
			// if (iSec > 3) {
			imwrite(newMotionDir + "/trace.jpg", drawing);
			std::cout << "save trace file '" << newMotionDir + "/trace.jpg'"
					  << std::endl;
			// std::cout << "end capture " << startTime + "_" +
			// std::to_string(device)
			// 		  << std::endl;

			// cmd = "convert " + newMotionDir + "/*.jpg " + newMotionDir +
			// "/clip.gif"; std::cout << cmd << std::endl; system(cmd.c_str());

			if (hasRemoteDir) {
				if (port == -1) {
					cmd = "rsync -arv " + newMotionDir + " " + remoteDir;
				} else {
					cmd = "rsync -arv -e 'ssh -p " + std::to_string(port) +
						  "' " + newMotionDir + " " + remoteDir;
				}
				std::cout << cmd << std::endl;
				system(cmd.c_str());
			}
		}

// outputVideo.release();
#ifdef PC
		destroyAllWindows();
#endif

		if (lightGpio != -1) {
			gpioSetValue(lightGpio, 0);
		}

	} // while (1)

	return 0;
}