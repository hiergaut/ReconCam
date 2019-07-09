
#include "function.hpp"

#include <set>

const int side = 16;
const int rbins = side;
const int gbins = side;
const int bbins = side;
// const int hbins = side;
// const int sbins = side;
// const int histSize[] = {hbins, sbins};
const int histSize[] = {rbins, gbins, bbins};
// // hue varies from 0 to 179, see cvtColor
// const float hranges[] = {0, 180};
// const int hStep = (hranges[1] - hranges[0]) / hbins;
// // saturation varies from 0 (black-gray-white) to
// const float sranges[] = {0, 256};
// const int sStep = (sranges[1] - sranges[0]) / sbins;
const float rRanges[] = {0, 180};
const float gRanges[] = {0, 256};
const float bRanges[] = {0, 256};

const int rStep = (rRanges[1] - rRanges[0]) / rbins;
const int gStep = (gRanges[1] - gRanges[0]) / gbins;
const int bStep = (bRanges[1] - bRanges[0]) / bbins;

const int channels[] = {0, 1, 2};
// const float *ranges[] = {hranges, sranges};
const float *ranges[] = {rRanges, gRanges, bRanges};

// struct s_customCompare {
// 	inline bool operator()(const Color &left, const Color &right) const {
// 		return left.v > right.v;
// 	}
// };

typedef struct s_colorSorted {
	int r;
	int g;
	int b;
	int value;

	bool operator<(const s_colorSorted &right) const {
		if (value == right.value) {
			if (r == right.r) {
				if (g == right.g) {
					return b > right.b;
				}
				return g > right.g;
			}
			return r > right.r;
		}
		return value > right.value;
	}
} ColorSorted;

void Capture::buildNColors() const {

	Mat hsv;
	cvtColor(m_img, hsv, COLOR_BGR2HSV);

	MatND hist;
	calcHist(&hsv, 1, channels, m_mask, // do not use mask
			 hist, 3, histSize, ranges,
			 true, // the histogram is uniform
			 false);
	// double maxVal = -1;
	// minMaxLoc(hist, 0, &maxVal, 0, 0);

	// std::set<Color, s_customCompare> colors;
	std::set<ColorSorted> colors;

	// int scale = 10;
	// Mat histImg = Mat::zeros(hbins * scale, sbins * scale, CV_8UC3);
	// int maxIntensity = -100;
	for (int r = 0; r < rbins; ++r) {
		for (int g = 0; g < gbins; ++g) {
			for (int b = 0; b < bbins; ++b) {

				int binVal = hist.at<int>(r, g, b);
				// float v = cvRound(binVal * 255.f / maxVal);

				// float hTemp = (h + 0.5) * hStep / 179.0;
				// float sTemp = (s + 0.5) * sStep / 255.0;

				// colors.insert({(h + 0.5f) * hStep / 90.0f -1.0f , (s + 0.5f)
				// * sStep / 128.0f - 1.0f, v, h, s});
				// colors.insert({r * hStep / 87.0f - 1.0f, g, v, r, s});
				colors.insert({r * rStep, g * gStep, b * bStep, binVal});
				// colors.insert({hTemp, sTemp, v, h, s});
				// colors.insert({(h + 0.5) * hStep / 180.0 , (s + 0.5) * sStep
				// / 256.0, v, h, s});

				// Mat hsvColor(1, 1, CV_8UC3,
				// 			 // Scalar((s + 0.5) * hStep, (v + 0.5) * sStep,
				// vMax)); 			 Scalar((h + 0.5) * hStep,  (s + 0.5) *
				// sStep, v));

				// Mat bgrColor;
				// cvtColor(hsvColor, bgrColor, COLOR_HSV2BGR);

				// uchar *ptr = bgrColor.ptr();

				// Scalar color(ptr[0], ptr[1], ptr[2]);

				// rectangle(histImg, Point(h * scale, s * scale),
				// 		  Point((h + 1) * scale - 1, (s + 1) * scale - 1),
				// color, 		  FILLED);
			}
		}
	}
	std::vector<Color> firstThree;

	// id.colors.clear();
    assert(colors.size() >= 3);

	int cpt = 1;
	for (const ColorSorted &c : colors) {
		// std::cout << c.intensity << " " << c.h << " " << c.s << std::endl;
		// putText(histImg, std::to_string(cpt),
		// 		Point(c.x * scale + 1, c.y * scale + 9), FONT_HERSHEY_PLAIN,
		// 		0.8, Scalar(0, 0, 0));
		Color color(c.r, c.g, c.b);
		// std::cout << "color : " << color << std::endl;
		firstThree.push_back(color);

		if (cpt++ > 2)
			break;
	}
    // m_build = true;

    NColors col(firstThree);
    m_id = Identity(m_x, m_y, m_width, m_height, m_density, col);
    m_build = true;

	// return firstThree;
	// id.set_colors(firstThree);
	// histImg.copyTo(histImage);

	// return std::move(firstThree);
}

// Scalar toScalar(Color c) { return Scalar(c.m_r, c.m_g, c.m_b); }