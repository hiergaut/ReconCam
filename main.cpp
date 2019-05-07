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

#define TIMELAPSE_INTERVAL 600 // secondes
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
	double dist;
	Point2f pos;
	int density;
	Point2f speedVector;
	Scalar color;
	int id;
	Capture bestCapture;
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

std::string gpioDir = "/tmp/gpio/";
// std::string gpioDir = "/sys/class/gpio/";

void initGpio(int gpio) {
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
	directionGpio << "in";
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
	usleep(100000);

	return val;
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
	return std::to_string(now->tm_hour) + ':' + std::to_string(now->tm_min) +
		   ':' + std::to_string(now->tm_sec);
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
		"{help h        |               | help message}"
		"{sensor        | -1            | gpio number of IR senror}"
		"{d device      | 0             | device camera, /dev/video0 by "
		"default}"
		"{r repository  |               | save motion to specific repository}"
		"{p port        | -1            | remote port repository}");

	if (parser.has("help")) {
		parser.printMessage();
		return 0;
	}
	int sensorGpioNum = parser.get<int>("@sensor");
	int device = parser.get<int>("device");
	std::string remoteDir = parser.get<std::string>("repository");
	int port = parser.get<int>("port");
	if (!parser.check()) {
		parser.printMessage();
		parser.printErrors();
		return 0;
	}

	bool hasRemoteDir = !remoteDir.empty();
	using ObjList = std::list<Object>;
	// std::string motionDir = "/tmp/motion/";
	std::string motionDir = "motion/";
	std::string hostname = getHostname();

	auto model = createBackgroundSubtractorKNN();
	VideoCapture vCap;
	// VideoWriter outputVideo;
	// auto model = createBackgroundSubtractorMOG2();

	std::string timelapseDir =
		motionDir + "timelapse_" + hostname + "_" + std::to_string(device);
	std::string cmd = "mkdir -p " + timelapseDir;
	system(cmd.c_str());

	ObjList objects;
	std::vector<Line> lines;
	std::vector<DeadObj> tombs;

	Mat inputFrame, mask, drawing;
	int iNewObj;
	size_t tickCapture;
	size_t cur;
	int iSec;
	int iCap;
	size_t tickTimeLapse = clock() + CLOCKS_PER_SEC; // take picture immediately
	// std::cout << "first tick TimeLapse " << tickTimeLapse << std::endl;

	if (sensorGpioNum != -1) {
		initGpio(sensorGpioNum);
		gpioGetValue(sensorGpioNum);
	}

	// --------------------------- INFINITE LOOP ------------------------------
	while (1) {
		while (sensorGpioNum == -1 || gpioGetValue(sensorGpioNum) == 0) {
			std::cout << "." << std::flush;
			usleep(1000000);
			tickTimeLapse -= CLOCKS_PER_SEC;
			// std::cout << "first tick TimeLapse " << tickTimeLapse <<
			// std::endl;

			cur = clock();
			if (cur > tickTimeLapse) {
				std::cout << std::endl;
				vCap.open(device);
				if (!vCap.isOpened()) {
					std::cout << "device not found";
					return 1;
				}
				vCap >> inputFrame;
				vCap.release();

				std::string saveLapse =
					timelapseDir + "/" + getCurTime() + ".jpg";
				imwrite(saveLapse, inputFrame);
				imwrite(timelapseDir + "/latest.jpeg", inputFrame);
				std::cout << "save lapse '" << saveLapse << "'" << std::endl;

				cmd = "convert " + timelapseDir + "/*.jpg " + timelapseDir +
					  "/timelapse.gif";
				std::cout << cmd << std::endl;
				system(cmd.c_str());

				if (hasRemoteDir) {
					if (port == -1) {
						cmd = "rsync -arv " + timelapseDir + " " + remoteDir;
					} else {
						cmd = "rsync -arv -e 'ssh -p " + std::to_string(port) +
							  "' " + timelapseDir + " " + remoteDir;
					}
					std::cout << cmd << std::endl;
					system(cmd.c_str());
				}

				tickTimeLapse = cur + TIMELAPSE_INTERVAL * CLOCKS_PER_SEC;
			}
		}
		std::cout << std::endl;

		// create new directory in /tmp/motion/
		std::string startTime = getCurTime();
		std::string tmpDir = motionDir + startTime + "_" + hostname + "_" +
							 std::to_string(device);
		cmd = "mkdir -p " + tmpDir;
		system(cmd.c_str());

		vCap.open(device);
		if (!vCap.isOpened()) {
			std::cout << "device not found";
			return 1;
		}

		// outputVideo.open(tmpDir + "/clip.avi", VideoWriter::fourcc('M', 'J',
		// 'P', 'G'), 2, Size(vCap.get(cv::CAP_PROP_FRAME_WIDTH),
		// 					  vCap.get(CAP_PROP_FRAME_HEIGHT)));
		// if (!outputVideo.isOpened()) {
		// 	std::cout << "could not open output video" << std::endl;
		// 	return 5;
		// }

		iNewObj = 0;
		iSec = 0;
		iCap = -1;
		tickCapture = clock() + CLOCKS_PER_SEC;
		lines.clear();
		tombs.clear();
		objects.clear();
		while (gpioGetValue(sensorGpioNum) == 1) {
			++iCap;
			// std::cout << "capture " << ++iCap << std::endl;
			int nbObjects = objects.size();

			vCap >> inputFrame;
			if (inputFrame.empty()) {
				std::cout << "Finished reading" << std::endl;
				break;
			}

			model->apply(inputFrame, mask);
			if (iCap < 20) {
				continue;
			}

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

			const int thresh = 10000;
			for (int i = 0; i < nbMovements; ++i) {
				nearestObj[i] = -1;
				distNearestObj[i] = thresh;
			}

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
				// new object
				if (iObj == -1) {
					Scalar color =
						Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
							   rng.uniform(0, 255));
					Capture cap{Mat(inputFrame, boundRect[i]).clone(),
								Mat(mask, boundRect[i]).clone(), contours[i],
								boundRect[i], static_cast<int>(mu[i].m00)};
					Object &&obj{
						0.0,		 mc[i], static_cast<int>(mu[i].m00),
						Vec2f(0, 0), color, iNewObj++,
						cap};
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
					if (obj.dist < 100) {
						tombs.push_back({obj.pos, obj.color});

						objects.erase(it++);
						++nbDeleteObj;

					}
					// save position of motionless object
					else {
						putText(drawing, "s", obj.pos + Point2f(-9, 9),
								FONT_HERSHEY_DUPLEX, 0.8, obj.color, 1);
						++it;
					}

				}
				// movement object
				else {
					if (mu[iMov].m00 > obj.bestCapture.density) {
						Capture cap{Mat(inputFrame, boundRect[iMov]).clone(),
									Mat(mask, boundRect[iMov]).clone(),
									contours[iMov], boundRect[iMov],
									static_cast<int>(mu[iMov].m00)};

						obj.bestCapture = cap;
					}
					obj.speedVector = mc[iMov] - obj.pos;
					obj.dist += norm(obj.speedVector);
					lines.push_back({obj.pos, mc[iMov], obj.color});
					obj.pos = mc[iMov];
					obj.density = mu[iMov].m00;

					rectangle(drawing, boundRect[iMov].tl(),
							  boundRect[iMov].br(), obj.color, 2);

					putText(drawing, std::to_string(obj.id),
							boundRect[iMov].br() + Point(5, 0),
							FONT_HERSHEY_DUPLEX, 0.5, obj.color, 1);
					putText(drawing, std::to_string(static_cast<int>(obj.dist)),
							boundRect[iMov].br() + Point(5, -20),
							FONT_HERSHEY_DUPLEX, 0.5, obj.color, 1);
					putText(drawing, std::to_string(obj.density),
							boundRect[iMov].br() + Point(5, -40),
							FONT_HERSHEY_DUPLEX, 0.5, obj.color, 1);
					line(drawing, obj.pos, obj.pos + obj.speedVector * 10,
						 Scalar(0, 0, 255), 1, LineTypes::LINE_AA);

					++it;
				}

				++iObj;
			}

			for (auto &obj : newObjects) {
				objects.push_back(obj);
			}

			putText(drawing, "nbObjs : " + std::to_string(nbObjects),
					Point(0, 30), FONT_HERSHEY_DUPLEX, 0.8, Scalar(0, 0, 255));

			for (size_t i = 0; i < lines.size(); ++i) {
				line(drawing, lines[i].p, lines[i].p2, lines[i].color, 1);
			}

			for (DeadObj obj : tombs) {
				putText(drawing, "x", obj.p + Point(-9, 9), FONT_HERSHEY_DUPLEX,
						0.8, obj.color, 1);
			}

#ifdef DISPLAY
			imshow("drawing", drawing);
			imshow("mask", mask);
#endif
			if (waitKey(1) == 'q')
				break;

			cur = clock();
			if (cur > tickCapture) {
				std::string saveCap =
					tmpDir + "/cap_" + std::to_string(iSec) + ".jpg";

				if (!imwrite(saveCap, drawing)) {
					std::cout << "failed to save cap" << std::endl;
					perror(tmpDir.c_str());
					return 2;
				}
				std::cout << "save capture '" << saveCap << "'" << std::endl;

				// outputVideo << drawing;

				tickCapture = cur + CLOCKS_PER_SEC;
				++iSec;
			}
		}

		for (Object obj : objects) {
			Mat m = obj.bestCapture.img;

			m.copyTo(Mat(drawing, obj.bestCapture.rect), obj.bestCapture.mask);
			std::vector<std::vector<Point>> contours{obj.bestCapture.contour};
			drawContours(drawing, contours, 0, obj.color, 2);
		}
		imwrite(tmpDir + "/trace.jpeg", drawing);
		// std::cout << "end capture " << startTime + "_" +
		// std::to_string(device)
		// 		  << std::endl;

		// cmd = "convert " + tmpDir + "/*.jpg " + tmpDir + "/clip.gif";
		// std::cout << cmd << std::endl;
		// system(cmd.c_str());

		if (hasRemoteDir) {
			if (port == -1) {
				cmd = "rsync -arv " + tmpDir + " " + remoteDir;
			} else {
				cmd = "rsync -arv -e 'ssh -p " + std::to_string(port) + "' " +
					  tmpDir + " " + remoteDir;
			}
			std::cout << cmd << std::endl;
			system(cmd.c_str());
		}

		vCap.release();
// outputVideo.release();
#ifdef DISPLAY
		destroyAllWindows();
#endif

	} // while (1)

	return 0;
}