#pragma once
// #ifndef __FUNCTION__
// #define __FUNCTION__

#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp" // model
// #include "opencv2/videoio.hpp"

#include "opencv2/imgproc/imgproc.hpp" // bounding boxes
#include <opencv2/core/types.hpp>

#include <cassert>
#include <cstdio>
#include <ctime>
#include <fstream>
#include <iostream>
#include <list>
#include <sstream>
#include <string>
#include <unistd.h>

#include <dirent.h>
#include <sstream>
#include <vector>

// #include "LearningUI/Capture.h"
#include "Identity.hpp"

using namespace cv;

typedef struct s_line {
	Point p;
	Point p2;
	Scalar color;
} Line;

class Capture {
  public:
	void buildNColors() const;

	Capture(Mat img, Mat mask, std::vector<Point> contour, Rect rect, int x,
			int y, int width, int height, int density)
		: m_img{img}, m_mask{mask}, m_contour{contour}, m_rect{rect}, m_x{x},
		  m_y{y}, m_width{width}, m_height{height}, m_density{density} {}

    ~Capture() {
        if (m_id != nullptr) {
            delete m_id;
        }
    }

	//   private:
	Mat m_img;
	Mat m_mask;
	std::vector<Point> m_contour;
	Rect m_rect;
	int m_x;
	int m_y;
	int m_width;
	int m_height;
	int m_density;

	mutable Identity * m_id = nullptr;
    // mutable bool m_build {false};
};

typedef struct s_object {
	// class Object {
	// public:

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
	std::vector<Line> lines;

	bool operator<(const struct s_object &right) const {
		// return id < right.id;
		if (age == 0 && right.age == 0) {
			return density > right.density;
		}
		if (age == right.age) {
			// 	return density > right.density;
			return distance > right.distance;
		}
		return age > right.age;
	}
} Object;

// struct s_ObjectCompare {
// 	inline bool operator()(const Object &left, const Object &right) const {
//         return left.age > right.age;
// 		// return left.v > right.v;
// 	}
// };

typedef struct s_deadObj {
	Point p;
	Scalar color;
} DeadObj;

// std::string gpioDir = "/sys/class/gpio/";

// #endif
