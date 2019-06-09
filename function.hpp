#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp" // model
// #include "opencv2/videoio.hpp"

#include "opencv2/imgproc/imgproc.hpp" // bounding boxes
#include <opencv2/core/types.hpp>

#include <cassert>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <unistd.h>
#include <fstream>
#include <cstdio>

#include <dirent.h>
#include <sstream>
#include <vector>

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
	uint age;
	std::string name;
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

Point3f readPointFromFile(std::string path) {

	// std::ifstream file("learningFile/known/" + filename + "/min.txt");
	std::ifstream file(path);
	if (!file.is_open()) {
		std::cout << "cannot open file known" << std::endl;
		exit(1);
	}

	// while (getline(file, line)) {
	std::string line;
	getline(file, line);
	file.close();
	float h, h2, h3;
	sscanf(line.c_str(), "%f %f %f", &h, &h2, &h3);

	return std::move(Point3f(h, h2, h3));
}

std::vector<std::string> forEachFileInDir(std::string dirPath) {
	// std::string dirPath = "learningFile/known/";
	std::vector<std::string> files;
	DIR *dp;
	struct dirent *dirp;
	if ((dp = opendir(dirPath.c_str())) == NULL) {
		std::cout << "Error cannot opening " << dirPath << std::endl;
		exit(2);
	}
	while ((dirp = readdir(dp)) != NULL) {
		std::string filename(dirp->d_name);
		if (filename.compare(".") && filename.compare("..")) {
			files.push_back(std::move(filename));
		}
	}
	closedir(dp);

	return std::move(files);
}

void thread_alert(std::string filename) {
    std::cout << "new thread : " << filename << std::endl;
    std::fstream fs;
    fs.open(filename, std::ios::out);
    if (! fs.is_open()) {
        std::cout << "unable to open file : " << filename << std::endl;
    }
    fs.close();

    usleep(1000000 * 18);

    std::remove(filename.c_str());
}