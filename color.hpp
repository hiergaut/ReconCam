
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>

using namespace cv;

#include <fstream>
#include <iostream>
#include <set>
#include <vector>

class Color {
  public:
	float h;
	int s;
	float v;
	// int intensity;
    int x;
    int y;


	friend std::ostream &operator<<(std::ostream &out, Color &c) {
		// return out << "[" << c.h << ", " << c.s << ", " << c.v << "]";
		return out << c.h << " " << c.s << " " << c.v;
		// return out << c.h << " " << c.s << " " << c.v;
	}
};

// using Triplet = std::vector<Color>;
class Triplet {
  public:
	std::vector<Color> colors;

	void write(std::string file) {
		std::ofstream out;
		out.open(file);
		out << colors[0] << std::endl;
		out << colors[1] << std::endl;
		out << colors[2] << std::endl;
		out.close();
	}

    bool in(const std::pair<Point3f, Point3f> & bound) const {
        Point3f Min = bound.first;
        Point3f Max = bound.second;

        Point3f cur (colors[0].h, colors[1].h, colors[2].h);

        return (Min.x < cur.x && cur.x < Max.x && Min.y < cur.y && cur.y < Max.y && Min.z < cur.z && cur.z < Max.z);

    }
};

struct s_customCompare {
	bool operator()(const Color &left, const Color &right) {
		return left.v > right.v;
	}
};

const int side = 30;
const int hbins = side;
const int sbins = side;
const int histSize[] = {hbins, sbins};
// hue varies from 0 to 179, see cvtColor
const float hranges[] = {0, 180};
const int hStep = (hranges[1] - hranges[0]) / hbins;
// saturation varies from 0 (black-gray-white) to
const float sranges[] = {0, 256};
const int sStep = (sranges[1] - sranges[0]) / sbins;

const int channels[] = {0, 1};
const float *ranges[] = {hranges, sranges};

Triplet getPrimaryColor(const Mat &src, const Mat &mask) {
	Mat hsv;

	cvtColor(src, hsv, COLOR_BGR2HSV);

	MatND hist;
	// we compute the histogram from the 0-th and 1-st channels

	calcHist(&hsv, 1, channels, mask, // do not use mask
			 hist, 2, histSize, ranges,
			 true, // the histogram is uniform
			 false);
	// std::cout << hist.at<float>(0, 0, 0) << std::endl;
	double maxVal = -1;
	minMaxLoc(hist, 0, &maxVal, 0, 0);

	std::set<Color, s_customCompare> colors;
	for (int h = 0; h < hbins; ++h) {
		for (int s = 0; s < sbins; ++s) {
			float binVal = hist.at<float>(h, s);
			float v = cvRound(binVal * 255 / maxVal);

			colors.insert({h * hStep / 87.0f -1.0f , s, v, h, s});
		}
	}

	int cpt = 1;
	Triplet firstThree;
	for (Color c : colors) {

		firstThree.colors.push_back(c);

		if (cpt++ > 2)
			break;
	}

	return std::move(firstThree);
}
/**
 * @function main
 */
Triplet hsvHist(const Mat &src, const Mat &mask, Mat &histImage) {
	Mat hsv;
	cvtColor(src, hsv, COLOR_BGR2HSV);

	MatND hist;
	calcHist(&hsv, 1, channels, mask, // do not use mask
			 hist, 2, histSize, ranges,
			 true, // the histogram is uniform
			 false);
	double maxVal = -1;
	minMaxLoc(hist, 0, &maxVal, 0, 0);

	std::set<Color, s_customCompare> colors;

	int scale = 10;
	Mat histImg = Mat::zeros(hbins * scale, sbins * scale, CV_8UC3);
	// int maxIntensity = -100;
	for (int h = 0; h < hbins; ++h) {
		for (int s = 0; s < sbins; ++s) {
			float binVal = hist.at<float>(h, s);
			float v = cvRound(binVal * 255.f / maxVal);

            // float hTemp = (h + 0.5) * hStep / 179.0;
            // float sTemp = (s + 0.5) * sStep / 255.0;

			// colors.insert({(h + 0.5f) * hStep / 90.0f -1.0f , (s + 0.5f) * sStep / 128.0f - 1.0f, v, h, s});
			colors.insert({h * hStep / 87.0f -1.0f , s, v, h, s});
			// colors.insert({hTemp, sTemp, v, h, s});
			// colors.insert({(h + 0.5) * hStep / 180.0 , (s + 0.5) * sStep / 256.0, v, h, s});

			Mat hsvColor(1, 1, CV_8UC3,
						 // Scalar((s + 0.5) * hStep, (v + 0.5) * sStep, vMax));
						 Scalar((h + 0.5) * hStep,  (s + 0.5) * sStep, v));

			Mat bgrColor;
			cvtColor(hsvColor, bgrColor, COLOR_HSV2BGR);

			uchar *ptr = bgrColor.ptr();

			Scalar color(ptr[0], ptr[1], ptr[2]);

			rectangle(histImg, Point(h * scale, s * scale),
					  Point((h + 1) * scale - 1, (s + 1) * scale - 1), color,
					  FILLED);
		}
	}

	int cpt = 1;
	Triplet firstThree;
	for (Color c : colors) {
		// std::cout << c.intensity << " " << c.h << " " << c.s << std::endl;
		putText(histImg, std::to_string(cpt),
				Point(c.x * scale + 1, c.y * scale + 9), FONT_HERSHEY_PLAIN,
				0.8, Scalar(0, 0, 0));

		firstThree.colors.push_back(c);

		if (cpt++ > 2)
			break;
	}

	histImg.copyTo(histImage);

	return std::move(firstThree);
}