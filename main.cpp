#include "opencv2/opencv.hpp"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp" // model

#include "opencv2/imgproc/imgproc.hpp" // bounding boxes

#include <assert.h>
#include <cassert>
#include <chrono>
#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <list>
#include <map>
#include <sstream>
#include <string>
#include <thread>
#include <unistd.h>
// #include <tuple>

// #include "Color.hpp"
#include <set>

#include "Gpio.hpp"
#include "System.hpp"
#include "function.hpp"
// #include "LearningUI/Capture.h"

#ifdef PC
#define TIMELAPSE_INTERVAL 20 // 30 sec
#else
// #define TIMELAPSE_INTERVAL 1800 // 30 min
// #define TIMELAPSE_INTERVAL 1200 // 20 min
// #define TIMELAPSE_INTERVAL 300 // 5 min
#define TIMELAPSE_INTERVAL 600 // 10 min
#endif

#define NB_CAP_LEARNING_MODEL_FIRST 5
#define NB_CAP_FOCUS_BRIGHTNESS 2

// #define NB_CAP_MIN_FOR_REAL_MOTION 5
// #define MIN_MOV_DIST_TO_SAVE_OBJECT 10
#define NEW_OBJECT_MIN_DENSITY 10
#define MAX_ERROR_DIST_FOR_NEW_POS_OBJECT 100
#define MIN_MOV_YEARS_TO_SAVE_OBJECT 5

#define MAX_MOVEMENTS 20
// #define DELTA_DIFF_MAX_DENSITY 2000
// #define WIDTH 1024
// #define HEIGHT 768
#define WIDTH 640
#define HEIGHT 480

using namespace cv;
using namespace std;

RNG rng(29791);

// ------------------------------- MAIN ---------------------------------------

int main(int argc, char **argv) {
	// (void)argc;
	// (void)argv;

	cout << "OpenCV version : " << CV_VERSION << endl;
	cout << "Major version : " << CV_MAJOR_VERSION << endl;
	cout << "Minor version : " << CV_MINOR_VERSION << endl;
	cout << "Subminor version : " << CV_SUBMINOR_VERSION << endl;

	// if ( CV_MAJOR_VERSION < 3)
	// {
	//   // Old OpenCV 2 code goes here.
	// } else
	// {
	//   // New OpenCV 3 code goes here.
	// }
	// return 0;

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
		"{script        |           | launch script on recognize}"
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
	std::string script = parser.get<std::string>("script");
	bool hasScript = !script.empty();
	bool hasStream = !stream.empty();
	// bool onlyRec = parser.get<bool>("onlyRec");
	if (!parser.check()) {
		parser.printMessage();
		parser.printErrors();
		return 0;
	}

#ifdef PC
	gpioDir = "gpio/";

	// if (hasStream) {
	namedWindow("mask", WINDOW_AUTOSIZE);
	namedWindow("mask2", WINDOW_AUTOSIZE);
	namedWindow("drawing", WINDOW_AUTOSIZE);
	// }
#else
	gpioDir = "/sys/class/gpio/";
#endif

	std::string deviceName;
	if (!hasStream) {
		deviceName = std::to_string(device);
		stream = "/dev/video" + deviceName;
	} else {
		deviceName = stream;
	}

	bool hasRemoteDir = !remoteDir.empty();
	std::string motionDir;
	if (hasRemoteDir) {
		motionDir = "/tmp/motion/";

	} else {
		motionDir = "motion/";
	}
	std::string hostname = getHostname();

	VideoCapture vCap;

	std::string timelapseDir =
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

	std::set<Object> objects;
	std::vector<DeadObj> tombs;

	Mat inputFrame, mask, drawing;
	int iNewObj;
	int iCap;

	// int tickTimeLapse = 1; // take picture immediately

	// vCap.open(stream);
	// vCap.open(CAP_V4L2);
	// if (!vCap.isOpened()) {
	//     std::cout << "device not found" << std::endl;
	//     return 1;
	// }
	int width = -1;
	int height = -1;
	// Size sizeScreen(width, height);
	// vCap.release();

	cout <<"check camera" << endl;
    if(vCap.open(stream)) {
	   vCap.set(cv::CAP_PROP_FRAME_WIDTH , WIDTH);
	   vCap.set(cv::CAP_PROP_FRAME_HEIGHT ,HEIGHT);

	   if((int)vCap.get(cv::CAP_PROP_FRAME_WIDTH) != WIDTH
		   || (int)vCap.get(cv::CAP_PROP_FRAME_HEIGHT) != HEIGHT)
			std::cout << "Warning! Can not adjust video capture properties!" << std::endl;

		width = vCap.get(cv::CAP_PROP_FRAME_WIDTH);
		height = vCap.get(cv::CAP_PROP_FRAME_HEIGHT);

       Mat meter_image;
       if(vCap.read(meter_image)) {
            imwrite("/tmp/reconCamStartupTest.jpg", meter_image);
            vCap.release();
	        cout << "camera '" << stream << "' is ok" << endl;
            // return 0;
       } else {
           vCap.release();
	       cout << "could not read frame" << endl;
           return -1; // can not read frame
       }
    } else {
		 cout << "could not open video capture device " << stream << endl;
         return -2; // can not open video capture device
    }

	Size sizeScreen(width, height);

	// Mat notGreen = Mat::zeros(Size(width, height), CV_8UC3);
	// notGreen = Scalar(255, 0, 255);

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

	// std::map<std::string, std::tuple<Identity, Identity, Identity>> boxes;
	// std::vector<std::pair<std::string, Box>> boxes;
	std::map<std::string, Box> boxes;
	if (recon) {

		for (std::string filename : forEachFileInDir("learningFile/known/")) {
			std::string path = "learningFile/known/" + filename + "/";

			// Identity Min(path + "min.txt");
			// Identity Mean(path + "mean.txt");
			// Identity Max(path + "max.txt");
			Box box(path);

			// boxes[filename] = {Min, Mean, Max};
			// auto tuple = std::make_tuple(Min, Mean, Max);
			boxes.insert(std::make_pair(filename, std::move(box)));
			std::cout << "[RECON] boxes : insert " << filename << std::endl;
			// boxes[filename] = box;
		}
	}

	bool movsFound[MAX_MOVEMENTS];
	bool quit = false;

	const auto timelapseStart = std::chrono::high_resolution_clock::now();
	int timelapseCounter = 0;

		// auto end = std::chrono::high_resolution_clock::now();
		// auto duration =
		// 	std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
		// 		.count() /
		// 	1000.0;

	// --------------------------- INFINITE LOOP ------------------------------
	bool firstEntrance = true;
	while (1) {
		auto timelapseEnd = std::chrono::high_resolution_clock::now();
		auto timelapseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timelapseEnd - timelapseStart).count() / 1000.0f - timelapseCounter * TIMELAPSE_INTERVAL;
		// std::cout << "[TIMELAPSE] wait new motion, future lapse at "
				//   << tickTimeLapse << " sec " << std::endl;
		std::cout << "[TIMELAPSE] wait new motion, future lapse at "
				  << TIMELAPSE_INTERVAL - timelapseDuration << " sec " << std::endl;
		while (!hasMovement()) {

			std::cout << "." << std::flush;
			usleep(CLOCKS_PER_SEC);
			// --tickTimeLapse;

			timelapseEnd = std::chrono::high_resolution_clock::now();
			// timelapseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timelapseEnd - timelapseStart).count() / 1000.0f;
			timelapseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timelapseEnd - timelapseStart).count() / 1000.0f - timelapseCounter * TIMELAPSE_INTERVAL;
			if (timelapseDuration > TIMELAPSE_INTERVAL || firstEntrance) {
			// if (tickTimeLapse == 0) {
				std::cout << std::endl;
				vCap.open(stream);
				vCap.set(cv::CAP_PROP_FRAME_WIDTH , WIDTH);
				vCap.set(cv::CAP_PROP_FRAME_HEIGHT ,HEIGHT);
				//
				// vCap.open(CAP_V4L2);
				if (!vCap.isOpened()) {
					std::cout << "device not found";
					return 1;
				}
				for (int i = 0; i < NB_CAP_FOCUS_BRIGHTNESS + 100; ++i) {
					vCap >> inputFrame;
				}

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

				// tickTimeLapse = TIMELAPSE_INTERVAL;
				// std::cout << "[TIMELAPSE] future lapse at " << tickTimeLapse
						//   << " sec " << std::endl;

				// timelapseStart = timelapseEnd = std::chrono::high_resolution_clock::now();
				// timelapseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timelapseEnd - timelapseStart).count() / 1000.0f;
				if (firstEntrance) {
					firstEntrance = false;
				} else {
					++timelapseCounter;
				}
				timelapseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timelapseEnd - timelapseStart).count() / 1000.0f - timelapseCounter * TIMELAPSE_INTERVAL;
				std::cout << "[TIMELAPSE] future lapse at " << TIMELAPSE_INTERVAL - timelapseDuration
						  << " sec " << std::endl;
			}
		} // while (!hasMovement())
		std::cout << std::endl;

		if (lightGpio != -1 && isNight()) {
			gpioSetValue(lightGpio, 1);
		}

		std::string motionId = getCurTime() + "_" + hostname + "_" + deviceName;
		std::cout << "new event : " << motionId << std::endl;
		std::string newMotionDir = motionDir + motionId;
		cmd = "mkdir -p " + newMotionDir;
		system(cmd.c_str());

		auto start = std::chrono::high_resolution_clock::now();
		vCap.open(stream);
		vCap.set(cv::CAP_PROP_FRAME_WIDTH , WIDTH);
		vCap.set(cv::CAP_PROP_FRAME_HEIGHT ,HEIGHT);
		// vCap.open(CAP_V4L2);
		if (!vCap.isOpened()) {
			std::cout << "device not found";
			return 1;
		}

		std::string outputVideoFile = newMotionDir + "/video.webm";
		VideoWriter outputVideo = VideoWriter(
			outputVideoFile, cv::VideoWriter::fourcc('V', 'P', '8', '0'), 3,
			sizeScreen, true);

		// std::string outputVideoFile = newMotionDir + "/video.mp4";
		// VideoWriter outputVideo = VideoWriter(
		//     outputVideoFile, cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 20,
		//     sizeScreen, true);

		std::string outputVideoFileRec;
		VideoWriter outputVideoRec;
		outputVideoFileRec = newMotionDir + "/video.avi";
		outputVideoRec = VideoWriter(
			outputVideoFileRec, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 3,
			sizeScreen, true);

		if (!outputVideo.isOpened() || !outputVideoRec.isOpened()) {
			std::cout << "failed to write video" << std::endl;
			return 6;
		}

		iNewObj = 0;
		iCap = 0;
		tombs.clear();
		objects.clear();

		auto model = createBackgroundSubtractorMOG2();
		// auto model = createBackgroundSubtractorKNN();
		// auto model = createBackgroundSubtractorGMG();
		bool streamFinished = false;
		int nbObjects = 0;

		auto start2 = std::chrono::high_resolution_clock::now();
		vCap >> inputFrame;
		if (flip180) {
			flip(inputFrame, inputFrame, -1);
		}
		// outputVideo << inputFrame;
		outputVideoRec << inputFrame;
		drawing = inputFrame.clone();
		rectangle(drawing, Rect(640 - 50, 0, 50, 50), Scalar(255, 0, 0), -1);

		// ----------------------- WHILE HAS MOVEMENT
		while (1) {

			putText(drawing, "nbObjs : " + std::to_string(nbObjects),
					Point(0, 30), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 0), 5);
			putText(drawing, "nbObjs : " + std::to_string(nbObjects),
					Point(0, 30), FONT_HERSHEY_DUPLEX, 1,
					Scalar(255, 255, 255));

			putText(drawing, "frame : " + std::to_string(iCap), Point(0, 60),
					FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 0), 5);
			putText(drawing, "frame : " + std::to_string(iCap), Point(0, 60),
					FONT_HERSHEY_DUPLEX, 1, Scalar(255, 255, 255), 1);

			// std::cout << "frame : " << iCap << "\r" << std::flush;
			std::cout << "+" << std::flush;
			// auto end2 = std::chrono::high_resolution_clock::now();
			// double duration2 =
			// 	1000.0 /
			// 	std::chrono::duration_cast<std::chrono::milliseconds>(end2 -
			// 														  start2)
			// 		.count();
			auto end2 = std::chrono::high_resolution_clock::now();
			double duration2;
			if (quit) {
				duration2 =
					std::chrono::duration_cast<std::chrono::microseconds>(end2 -
																		  start)
						.count() /
					(1000000.0 * iCap);
			} else {
				duration2 =
					std::chrono::duration_cast<std::chrono::microseconds>(
						end2 - start2)
						.count() /
					1000000.0;
			}
			// std::cout << "recording fps : " << static_cast<double>(iCap) /
			// std::cout << "duration2 : " << duration2 << std::endl;
			// double fps = duration2;
			// std::cout << "fps : " << fps << std::endl;
			// duration
			//   << std::endl;
			// putText(drawing, "fps : " + std::to_string(fps), Point(0, 90),
			// 		FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 0), 5);
			// putText(drawing, "fps : " + std::to_string(fps), Point(0, 90),
			// 		FONT_HERSHEY_DUPLEX, 1, Scalar(255, 255, 255), 1);
			std::ostringstream fps;
			fps << std::fixed << std::setprecision(2) << 1.0 / duration2;
			putText(drawing, "fps : " + fps.str(), Point(0, 90),
					FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 0), 5);
			putText(drawing, "fps : " + fps.str(), Point(0, 90),
					FONT_HERSHEY_DUPLEX, 1, Scalar(255, 255, 255), 1);

#ifdef PC
			imshow("drawing", drawing);
#endif
			outputVideo << drawing;
			if (quit) {
				break;
			}
			if (!hasMovement()) {
				break;
			}

			// ------------------------ START
			start2 = std::chrono::high_resolution_clock::now();
			++iCap;
			// int nbObjects = objects.size();

			vCap >> inputFrame;
			if (inputFrame.empty()) {
				std::cout << "Finished reading" << std::endl;
				streamFinished = true;
				break;
			}
			if (flip180) {
				flip(inputFrame, inputFrame, -1);
			}
			outputVideoRec << inputFrame;
			drawing = inputFrame.clone();
			if (iCap <= NB_CAP_FOCUS_BRIGHTNESS) {
				// #ifdef PC
				// putText(drawing, "auto brightness mode", Point(480, 30),
				// FONT_HERSHEY_DUPLEX, 1, Scalar(255, 255, 255), 1);
				rectangle(drawing, Rect(640 - 50, 0, 50, 50), Scalar(255, 0, 0),
						  -1);
				// 				imshow("drawing", drawing);
				// #endif
				continue;
			}

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
			if (waitKey(100) == 'q') {
				quit = true;
				continue;
				// return 0;
			}
			imshow("mask", mask);
#endif

			if (iCap <= NB_CAP_FOCUS_BRIGHTNESS + NB_CAP_LEARNING_MODEL_FIRST) {
				// if (iCap < NB_CAP_FOCUS_BRIGHTNESS) {
				// #ifdef PC
				// 				// imshow("mask", mask);
				// 				imshow("drawing", drawing);
				rectangle(drawing, Rect(640 - 50, 0, 50, 50), Scalar(0, 255, 0),
						  -1);
				// #endif
				// outputVideo << inputFrame;
				continue;
			}

			// ------------------- BOUNDING MOVMENT ---------------------------
			// threshold(mask, mask, 127, 255, THRESH_BINARY);
			medianBlur(mask, mask, 15);

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
			// medianBlur(mask, mask, 21);
			// const int size = 100;
			// blur(mask, mask, Size(size, size));
			// blur(mask, mask, Size(size, size));
			threshold(mask, mask, 0, 255, THRESH_BINARY);

#ifdef PC
			imshow("mask2", mask);
			// imshow("mask2", drawing);
#endif

			// ------------------- BOUNDING BOXING MOVEMENTS
			std::vector<std::vector<Point>> contours;
			std::vector<Vec4i> hierarchy;
			findContours(mask, contours, hierarchy, RETR_TREE,
						 CHAIN_APPROX_SIMPLE, Point(0, 0));

			int nbMovements = contours.size();
			if (nbMovements >= MAX_MOVEMENTS) {
				rectangle(drawing, Rect(640 - 50, 0, 50, 50), Scalar(0, 0, 255),
						  -1);
				// outputVideo << inputFrame;
				// #ifdef PC
				// 				imshow("drawing", drawing);
				// #endif
				continue;
			}
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

			// drawing = inputFrame.clone();
			std::vector<Point2f> mc(nbMovements);
			for (int i = 0; i < nbMovements; ++i) {
				mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
			}

			// ------------------- FIND PREVIOUS OBJECTS POSITIONS
			for (int i = 0; i < MAX_MOVEMENTS; ++i) {
				movsFound[i] = 0;
			}
			std::set<Object> newObjects;

			std::set<Object>::iterator it = objects.begin();
			while (it != objects.end()) {
				Object &obj = const_cast<Object &>(*it);

				int iMov = -1;
				int distMovMin = MAX_ERROR_DIST_FOR_NEW_POS_OBJECT;
				for (int i = 0; i < nbMovements; ++i) {
					if (!movsFound[i]) {
						double density = 3.0 * fmax(obj.density, mu[i].m00) /
										 fmin(obj.density, mu[i].m00);

						// if (density > DELTA_DIFF_MAX_DENSITY) {
						// continue;
						// }

						double distance =
							norm((obj.pos + obj.speedVector) - mc[i]);

						double dist = distance + density;

						if (dist < distMovMin) {
							distMovMin = dist;
							iMov = i;
						}
					}
				}

				// no event near object
				if (iMov == -1) {
					// if (obj.distance < MIN_MOV_DIST_TO_SAVE_OBJECT) {
					if (obj.age < MIN_MOV_YEARS_TO_SAVE_OBJECT) {
						tombs.push_back({obj.pos, obj.color});

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
					movsFound[iMov] = true;

					Point2i tl = boundRect[iMov].tl();
					Point2i tr = tl + Point2i(boundRect[iMov].width + 5, 10);

					// std::vector<std::vector<Point>> contours
					// {contours[iMov]};
					const Rect &rect{boundRect[iMov]};
					// std::cout << "rect : " << rect << std::endl;
					int density = mu[iMov].m00;
					assert(!inputFrame.empty());
					assert(!rect.empty());
					Mat img = Mat(inputFrame, rect).clone();
					Mat m = Mat(mask, rect).clone();
					// NColors colors = Capture::getPrimaryColors(img, m);
					Capture cap(std::move(img), std::move(m), contours[iMov],
								rect, mc[iMov].x, mc[iMov].y, rect.width,
								rect.height, density);

					// if found best trace
					if (mu[iMov].m00 > obj.trace[obj.bestCapture].m_density) {
						obj.bestCapture = obj.trace.size() - 1;
					}
					obj.speedVector = mc[iMov] - obj.pos;

					obj.distance += norm(mc[iMov] - obj.firstPos);

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

					// if (recon && objects.size() <= 10) {
					if (recon) {
						// if (obj.age > MIN_MOV_YEARS_TO_SAVE_OBJECT) {
						// std::cout << "recon object " << std::endl;
						if (!obj.name.compare("")) {
							if (!cap.m_build) {
								cap.buildNColors();
							}
							assert(cap.m_build);

							// std::cout << "build done " << std::endl;
							// std::cout <<  std::flush;
							// Triplet triplet =
							// 	getPrimaryColor(cap.img, cap.mask);
							// putText(drawing, std::string(triplet),
							// 		tr + Point(0, -20), FONT_HERSHEY_DUPLEX,
							// 		0.5, obj.color, 1);
							// cap.getPrimaryColors();

							std::string bestPath = "";
							const double maxDist = 100000000;
							double bestDist = maxDist;
							// assert(boxes.size() == 0);
							for (const auto &pair : boxes) {
								std::string path = pair.first;
								// std::cout << "path : " << path << std::endl;
								auto box = pair.second;
								// std::tuple<Identity, Identity, Identity>
								// bound = pair.second;
								// const Identity &Min = std::get<0>(bound);
								// const Identity &Mean = std::get<1>(bound);
								// const Identity &Max = std::get<2>(bound);

								// const Identity &cur{cap.m_id};
								// assert(cap.m_build);
								const Identity &cur = cap.m_id;
								// std::cout << "id : " << cur  << std::endl;

								// if (triplet.in(Min, Max)) {
								if (box.in(cur)) {
									// std::cout << "boxin " << path <<
									// std::endl; if (Min <= cur && cur <= Max)
									// { double curDist = triplet.dist(Mean);
									double curDist = cur - box.center();
									assert(curDist < 100000000);

									if (curDist < bestDist) {
										bestDist = curDist;
										bestPath = path;
									}
								}
							}

							// if (bestPath.compare("")) {
							if (bestDist != maxDist) {
								std::cout
									<< "[RECON] find object : " << bestPath
									<< std::endl;

								if (hasScript) {
									imwrite("alert.jpg", drawing);

									cmd = "./" + script + " " + bestPath + " &";
									system(cmd.c_str());

									std::cout << "[SCRIPT] run : " << cmd
											  << std::endl;
								}
								// std::thread thread(
								// 	thread_alert,
								// 	std::string("alert/" + bestPath));
								// thread.detach();

								obj.name = bestPath;
								boxes.erase(bestPath);
							}
						}
					}

					obj.trace.emplace_back(std::move(cap));
					// obj.trace.emplace_back(cap);
					newObjects.insert(std::move(obj));
				} // if (iMov != -1) {

				++it;
			} // while (it != objects.end()) {

			objects = std::move(newObjects);

			// non objects movements become new objects
			for (int i = 0; i < nbMovements; ++i) {

				int density = mu[i].m00;
				// new object
				if (!movsFound[i] && density > NEW_OBJECT_MIN_DENSITY) {
					Scalar color =
						Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
							   rng.uniform(0, 255));

					const Rect &rect{boundRect[i]};
					// std::cout << "rect : " << rect << std::endl;
					assert(!inputFrame.empty());
					assert(!rect.empty());
					const Mat &img = Mat(inputFrame, rect).clone();
					assert(!img.empty());
					const Mat &m = Mat(mask, rect).clone();
					assert(!m.empty());
					// NColors colors = Capture::getPrimaryColors(img, m);
					Capture cap(std::move(img), std::move(m), contours[i], rect,
								mc[i].x, mc[i].y, rect.width, rect.height,
								density);

					std::vector<std::vector<Point>> contour{contours[i]};
					drawContours(drawing, contour, 0, color, 2);

					Object obj{0.0,   mc[i],	 density, Vec2f(0, 0),
							   color, iNewObj++, 0,		  {std::move(cap)},
							   mc[i], 0,		 "",	  {}};

					objects.insert(std::move(obj));
				}
			}
			nbObjects = objects.size();

			for (auto &obj : objects) {
				for (auto &l : obj.lines) {
					line(drawing, l.p, l.p2, obj.color, 2);
				}
			}

			for (DeadObj obj : tombs) {
				putText(drawing, "x", obj.p + Point(-9, 9), FONT_HERSHEY_DUPLEX,
						1, obj.color, 1);
			}
			// putText(drawing, "nbObjs : " + std::to_string(nbObjects),
			// 		Point(0, 30), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 0), 5);
			// putText(drawing, "nbObjs : " + std::to_string(nbObjects),
			// 		Point(0, 30), FONT_HERSHEY_DUPLEX, 1,
			// 		Scalar(255, 255, 255));

			// putText(drawing, "frame : " + std::to_string(iCap), Point(0, 60),
			// 		FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 0), 5);
			// putText(drawing, "frame : " + std::to_string(iCap), Point(0, 60),
			// 		FONT_HERSHEY_DUPLEX, 1, Scalar(255, 255, 255), 1);

			// // auto end2 = std::chrono::high_resolution_clock::now();
			// // double duration2 =
			// // 	1000.0 /
			// // 	std::chrono::duration_cast<std::chrono::milliseconds>(end2 -
			// // start2)
			// // 		.count();
			// auto end2 = std::chrono::high_resolution_clock::now();
			// auto duration2 =
			// 	std::chrono::duration_cast<std::chrono::milliseconds>(end2 -
			// 														  start2)
			// 		.count() /
			// 	1000.0;
			// // std::cout << "recording fps : " << static_cast<double>(iCap) /
			// std::cout << "duration2 : " << duration2 << std::endl;
			// double fps = 1.0 / duration2;
			// std::cout << "fps : " << fps << std::endl;
			// // duration
			// //   << std::endl;
			// putText(drawing, "fps : " + std::to_string(fps), Point(0, 90),
			// 		FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 0), 5);
			// putText(drawing, "fps : " + std::to_string(fps), Point(0, 90),
			// 		FONT_HERSHEY_DUPLEX, 1, Scalar(255, 255, 255), 1);
			// // std::ostringstream fps;
			// // fps << std::fixed << std::setprecision(2) << 1.0 / duration2;
			// // putText(drawing, "fps : " + fps.str(), Point(0, 90),
			// // 		FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 0), 5);
			// // putText(drawing, "fps : " + fps.str(), Point(0, 90),
			// // 		FONT_HERSHEY_DUPLEX, 1, Scalar(255, 255, 255), 1);

			// for (auto &obj : objects) {
			// 	for (auto &l : obj.lines) {
			// 		line(drawing, l.p, l.p2, obj.color, 2);
			// 	}
			// }

			// for (DeadObj obj : tombs) {
			// 	putText(drawing, "x", obj.p + Point(-9, 9), FONT_HERSHEY_DUPLEX,
			// 			1, obj.color, 1);
			// }

			// #ifdef PC
			// 			imshow("drawing", drawing);
			// 			// imshow("mask", mask);
			// 			// imshow("grey", grey);
			// 			// if (waitKey(300) == 'q')
			// 			// break;
			// #endif

			// outputVideo << drawing;

		} // while (hasMovement())
		vCap.release();
		auto end = std::chrono::high_resolution_clock::now();
		auto duration =
			std::chrono::duration_cast<std::chrono::milliseconds>(end - start)
				.count() /
			1000.0;

		if (lightGpio != -1) {
			gpioSetValue(lightGpio, 0);
		}

		std::string trainingPath = trainDir + getDay() + "_" + motionId;

		int nbRealObjects = 0;
		for (const Object &obj : objects) {

			// if (obj.distance > MIN_MOV_DIST_TO_SAVE_OBJECT) {
			if (obj.age > MIN_MOV_YEARS_TO_SAVE_OBJECT) {

				// assert(obj.trace[obj.bestCapture] != nullptr);
				const Capture &bestCapture = obj.trace[obj.bestCapture];
				// const Mat &img = bestCapture.m_img;

				assert(!bestCapture.m_rect.empty());
				// if (bestCapture.m_rect.empty()) {
				// 	assert(!bestCapture.m_img.empty());
				// 	imshow("bestCapture", bestCapture.m_img);
				// 	waitKey(0);
				// 	continue;
				// }
				// assert(!bestCapture.m_rect.empty());
				assert(!bestCapture.m_mask.empty());
				assert(bestCapture.m_rect.size() == bestCapture.m_mask.size());

				bestCapture.m_img.copyTo(Mat(drawing, bestCapture.m_rect),
										 bestCapture.m_mask);
				std::vector<std::vector<Point>> contours{bestCapture.m_contour};
				drawContours(drawing, contours, 0, obj.color, 2);
				++nbRealObjects;

				if (training) {
					std::string newTrainingFile =
						trainingPath + "_" + std::to_string(obj.id) + "_";

					// Mat a;
					for (size_t i = 0; i < obj.trace.size(); ++i) {
						const Capture &cap = obj.trace[i];
						if (!cap.m_build) {
							cap.buildNColors();
						}
						// const Mat &img{cap.m_img};
						// const Mat &m{cap.m_mask};

						const std::string dir =
							newTrainingFile + std::to_string(i) + "/";
						std::cout << "[TRAINING] new training event '" << dir
								  << "'" << std::endl;

						cmd = "mkdir -p " + dir;
						system(cmd.c_str());

						assert(cap.m_img.size == cap.m_mask.size);
						assert(cap.m_img.dims == cap.m_mask.dims);
						// Mat a(cap.m_width, cap.m_height, CV_8UC3,
						// 	  Scalar(255, 0, 0));

						Mat a(cap.m_mask.size(), CV_8UC3,
							  Scalar(255, 255, 255));

						// a.copyTo(cap.m_img, cap.m_mask);
						// bestCapture.m_img.copyTo(Mat(drawing,
						// bestCapture.m_rect),
						// bestCapture.m_mask);
						cap.m_img.copyTo(a, cap.m_mask);
						// cap.m_img.copyTo(a);

						// cap.m_mask.copyTo(a);
						assert(cap.m_build);
						Identity &id = cap.m_id;
						// rectangle(a, Rect(0, 0, 20, 20), Scalar(first.r(),
						// first.g(), first.b()));

						int w = cap.m_width;
						int h = cap.m_height;
						int hStep = w / 5;
						int vStep = h / 5;
						// Scalar scalar(id[0]);
						// rectangle(a, Rect(0, 0, hStep * 3, vStep +2),
						// Scalar(0, 255, 0), -1);
						for (int j = 0; j < 3; ++j) {
							const Color col = id(j);
							Vec3b hsv(col.m_r, col.m_g, col.m_b);
							Mat3b hsvMat(hsv);
							// Vec3b bgr;
							Mat3b bgrMat;
							cvtColor(hsvMat, bgrMat, COLOR_HSV2BGR);
							Vec3b bgr(bgrMat.at<Vec3b>(0, 0));


							rectangle(a, Rect(hStep * j, 0, hStep, vStep),
									  bgr, -1);
							// rectangle(a, Rect(hStep * j, 0, hStep, vStep),
									//   Scalar(col.m_r, col.m_g, col.m_b), -1);
							Vec3b hsv2(col.m_r, 255, 255);
							Mat3b hsvMat2(hsv2);
							// Vec3b bgr;
							Mat3b bgrMat2;
							cvtColor(hsvMat2, bgrMat2, COLOR_HSV2BGR);
							Vec3b bgr2(bgrMat2.at<Vec3b>(0, 0));

							rectangle(a, Rect(hStep * j, vStep, hStep, vStep),
									  bgr2, -1);
						}

						imwrite(dir + "image.jpg", a);
						// imwrite(dir + "image.jpg", cap.m_img);
						// continue;

						// Mat histImg;
						// Triplet triplet = hsvHist(img, mask, histImg);
						// imwrite(dir + "hist.jpg", histImg);
						// cap.getPrimaryColors();
						cap.m_id.write(dir + "identity.txt");

						// triplet.write(dir + "primary.txt");
					}
				}
			}
		}

		outputVideo << drawing;
		outputVideo.release();
		outputVideoRec.release();
		// std::cout << "save video '" << outputVideoFile << "'" << std::endl;


		// if (iCap >= NB_CAP_FOCUS_BRIGHTNESS + NB_CAP_LEARNING_MODEL_FIRST +
		// 				NB_CAP_MIN_FOR_REAL_MOTION) {

		imwrite(newMotionDir + "/trace.jpg", drawing);
		// std::cout << "save trace file '" << newMotionDir + "/trace.jpg'"
		//   << std::endl;
		if (nbRealObjects > 0) {
			cmd = "touch " + newMotionDir + "/objectDetected.var";
			std::cout << cmd << std::endl;
			system(cmd.c_str());
		}

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


		std::cout << "object detected : " << nbRealObjects << std::endl;
		std::cout << "duration : " << duration << std::endl;
		std::cout << "nb capture : " << iCap << std::endl;
		std::cout << "recording fps : " << static_cast<double>(iCap) / duration
				  << std::endl;

		if (streamFinished || quit) {
			return 0;
		}
		// return 0;
	} // while (1)

	return 0;
}
