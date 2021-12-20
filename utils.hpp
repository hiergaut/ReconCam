#pragma once
// #ifndef __FUNCTION__
// #define __FUNCTION__

//#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp" // model
// #include "opencv2/videoio.hpp"
//#include "opencv2/imgproc/imgproc.hpp" // bounding boxes
//#include <opencv2/core/types.hpp>

//#include <cassert>
//#include <cstdio>
//#include <ctime>
//#include <fstream>
//#include <iostream>
//#include <list>
//#include <sstream>
//#include <string>
//#include <unistd.h>

//#include <dirent.h>
//#include <sstream>
//#include <vector>

// #include "LearningUI/Capture.h"
//#include "Identity.hpp"

//using namespace cv;

//typedef struct s_line {
//    cv::Point p;
//    cv::Point p2;
////    cv::Scalar color;
//} Line;


class Capture {
public:
    //	void buildNColors() const;

//    Capture(cv::Mat img, cv::Mat mask, std::vector<cv::Point> contour, cv::Rect rect, int x,
//        int y, int width, int height, int density)
//        : m_img { img }
//        , m_mask { mask }
//        , m_contour { contour }
//        , m_rect { rect }
//        , m_x { x }
//        , m_y { y }
//        , m_width { width }
//        , m_height { height }
//        , m_density { density }
//    {
//    }

//    ~Capture()
//    {
//        // if (m_id != nullptr) {
//        //     delete m_id;
//        //     m_id = nullptr;
//        // }
//    }

    //       private:
    cv::Mat m_img;
    cv::Mat m_mask;
    std::vector<cv::Point> m_contour;
    cv::Rect m_rect;
    int m_x;
    int m_y;
    int m_width;
    int m_height;
    double m_density;

//    std::string name;
    int label;
    double confidence;
    double detectDuration;

    // mutable Identity * m_id = nullptr;
    //    mutable Identity m_id;
    //    mutable bool m_build {false};
};

class Movement {
public:
    bool operator<(const Movement & right) const {
        return density < right.density;
    }

    cv::Point2i center;
    double density;
    bool found;
    cv::Rect boundRect;
    std::vector<cv::Point> contours;
};

//typedef struct s_object {
class Object {
public:
//    bool operator<(const Object & right) const
//    {
//        // return id < right.id;
//        if (age == 0 && right.age == 0) {
//            return density > right.density;
//        }
//        if (age == right.age) {
//            // 	return density > right.density;
//            return distance > right.distance;
//        }
//        return age > right.age;
//    }

    double distance; // from first pos
    cv::Point2i pos;
    double density;
    cv::Point2i speedVector;
    cv::Scalar color;
    int id;
    // Capture bestCapture;
//    int bestCapture;
    Capture biggestCapture;
//    std::vector<Capture> trace;
    cv::Point2i firstPos;
    uint age;
//    std::string name;
    std::vector<std::pair<cv::Point, cv::Point>> lines;
//    cv::Rect boundRect;
};


// struct s_ObjectCompare {
// 	inline bool operator()(const Object &left, const Object &right) const {
//         return left.age > right.age;
// 		// return left.v > right.v;
// 	}
// };

typedef struct s_deadObj {
    cv::Point p;
    cv::Scalar color;
} DeadObj;

// std::string gpioDir = "/sys/class/gpio/";

// #endif
