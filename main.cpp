

// #include "opencv2/core.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/imgproc.hpp"
#include "opencv2/video.hpp"
#include "opencv2/videoio.hpp"

#include "opencv2/imgproc/imgproc.hpp" // bounding boxes

#include <cassert>
#include <iostream>
#include <list>
// #include <set>

using namespace cv;

RNG rng(29791);

typedef struct s_capture {
	Mat img;
	Mat mask;
	std::vector<Point> contour;
	// Point pos;
	Rect rect;
	int density;
} Capture;

typedef struct s_object {
	//   public:
	//   private:
	double dist;
	Point2f pos;
	int density;
	Point2f speedVector;
	Scalar color;
	int id;
	Capture bestCapture;
	// Point2f firstPos;
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

int main(int, char **) {
	// VideoCapture cap("1car.avi");
	VideoCapture cap("3car3person.avi");
	// VideoCapture cap(2);
	if (!cap.isOpened()) {
		std::cout << "file not found";
		return 1;
	}
	auto model = createBackgroundSubtractorKNN();
	// auto model = createBackgroundSubtractorMOG2();

	// std::vector<Object> objects;
	using ObjList = std::list<Object>;
	// std::list<Object> objects;
	ObjList objects;
	std::vector<Line> lines;
	std::vector<DeadObj> tombs;
	Mat inputFrame, frame, mask, drawing;
	int iNewObj = 0;
	for (int cpt = 0;; ++cpt) {
		int nbObjects = objects.size();
		// std::cout << "cpt = " << cpt << std::endl;

		cap >> inputFrame;
		if (inputFrame.empty()) {
			std::cout << "Finished reading" << std::endl;
			break;
		}

		// std::cout << "image = " << inputFrame.size;
		// const Size scaledSize(640, 640 * inputFrame.rows / inputFrame.cols);
		// resize(inputFrame, frame, scaledSize, 0, 0, INTER_LINEAR);
		model->apply(inputFrame, mask);
		if (cpt < 20) {
			continue;
		}

		const int size = 15;
		medianBlur(mask, mask, size);
		blur(mask, mask, Size(size, size));
		// medianBlur(mask, mask, size);
		// threshold(mask, mask, 0, 255, THRESH_BINARY);
		// blur(mask, mask, Size(size, size));
		// threshold(mask, mask, 0, 255, THRESH_BINARY);
		// blur(mask, mask, Size(size, size));
		// blur(mask, mask, Size(size, size));
		// medianBlur(mask, mask, size);
		// blur(mask, mask, Size(size, size));
		// medianBlur(mask, mask, size);

		// GaussianBlur(mask, mask, Size(11, 11), 0, 0);
		// threshold(mask, mask, 10, 255, THRESH_BINARY);
		threshold(mask, mask, 0, 255, THRESH_BINARY);

		// bitwise_not(mask, mask);
		// Canny(mask, mask, 0, 255);

		std::vector<std::vector<Point>> contours;
		std::vector<Vec4i> hierarchy;
		findContours(mask, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE,
					 Point(0, 0));
		// std::vector<Point2f> center(contours.size());

		int nbMovements = contours.size();
		std::vector<std::vector<Point>> contours_poly(nbMovements);
		std::vector<Rect> boundRect(nbMovements);
		for (int i = 0; i < nbMovements; ++i) {
			approxPolyDP(Mat(contours[i]), contours_poly[i], 3, true);
			boundRect[i] = boundingRect(Mat(contours_poly[i]));
			// minEnclosingCircle
		}

		std::vector<Moments> mu(nbMovements);
		// double max = -1;
		// int iMax = -1;
		for (int i = 0; i < nbMovements; ++i) {
			mu[i] = moments(contours[i], true);
			// 	if (mu[i].m00 > max) {
			// 		max = mu[i].m00;
			// 		iMax = i;
			// 	}
		}

		drawing = inputFrame;
		std::vector<Point2f> mc(nbMovements);
		for (int i = 0; i < nbMovements; ++i) {
			mc[i] = Point2f(mu[i].m10 / mu[i].m00, mu[i].m01 / mu[i].m00);
			// Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
			// rng.uniform(0, 255)); putText(drawing, "+", mc[i] -Point2f(10,
			// 0), FONT_HERSHEY_COMPLEX, 1.0, color, 2);
		}

		// Mat drawing =Mat::zeros(mask.size(), CV_8UC3);

		// if (max > 100.0) {
		// 	Rect rect = boundRect[iMax];
		// 	Mat best = drawing(rect);
		// 	Mat cropMask = mask(rect);

		// 	// std::cout << best.size() << std::endl;
		// 	// std::cout << cropMask.size() << std::endl;

		// 	for (int row = 0; row < best.rows; ++row) {
		// 		uchar *pixBest = best.ptr<uchar>(row);
		// 		uchar *pixMask = cropMask.ptr<uchar>(row);
		// 		for (int col = 0; col < best.cols; ++col) {
		// 			int mask = pixMask[col];
		// 			if (mask == 0) {

		// 				pixBest[3 * col] = 255;
		// 				pixBest[3 * col + 1] = 255;
		// 				pixBest[3 * col + 2] = 255;
		// 			}
		// 		}
		// 	}
		// 	// Mat best = mask(rect);
		// 	// std::cout << best.channels << std::endl;
		// 	// std::cout << mask(rect).channels << std::endl;
		// 	// bitwise_and(best, mask(rect), best);

		// 	imshow("best", best);
		// }

		std::vector<int> nearestObj(nbMovements);
		std::vector<double> distNearestObj(nbMovements);
		std::vector<int> nearestMov(nbObjects);
		// std::vector<bool> moveIsObject(nbMovements);
		// for (int i =0; i <nbMovements; ++i) {
		//     moveIsObject[i] =false;
		// }
		for (int i = 0; i < nbObjects; ++i) {
			nearestMov[i] = -1;
		}

		const int thresh = 10000;
		for (int i = 0; i < nbMovements; ++i) {
			nearestObj[i] = -1;
			distNearestObj[i] = thresh;
		}

		// for (int i = 0; i < nbMovements; ++i) {
		// 	// nearestObj[i] = -1;
		// 	// int iNearest = -1;
		// 	// distNearestObj[i] = 999999.0;
		// 	// auto objPos = objects[i].pos;
		// 	auto moveCenter = mc[i];
		// 	// for (int iObj = 0; iObj < nbObjects; ++iObj) {
		// 	int iObj = 0;
		// 	for (const auto &obj : objects) {
		// 		// std::cout << "loop 1 : " << moveCenter << std::endl;
		// 		double dist = norm((obj.pos + obj.speedVector) - moveCenter);
		// 		// double dist = norm(moveCenter - obj.pos);
		// 		// std::cout << "dist : " << dist << moveCenter << std::endl;
		// 		if (dist < distNearestObj[i]) {

		// 			distNearestObj[i] = dist;
		// 			nearestObj[i] = iObj;
		// 		}

		// 		++iObj;
		// 	}
		// }
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
				Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
									  rng.uniform(0, 255));
				// Object &&obj{0.0,		  mc[i],
				// static_cast<int>(mu[i].m00), 			 Vec2f(0, 0), color,

				// Rect rect {boundRect[i]};
				Capture cap{Mat(inputFrame, boundRect[i]).clone(),
							Mat(mask, boundRect[i]).clone(), contours[i],
							boundRect[i], mu[i].m00};
				// iNewObj++};
				Object &&obj{0.0,		  mc[i], static_cast<int>(mu[i].m00),
							 Vec2f(0, 0), color, iNewObj++,
							 cap};
				//  mc[i]};

				rectangle(drawing, boundRect[i].tl(), boundRect[i].br(),
						  Scalar(0, 255, 0), 2);

				// std::cout << "new object" << std::endl;
				newObjects.emplace_back(obj);
			}
			// previous object
			// else {
			// 	// int iMov = nearestMov[iObj];
			// }
			// drawContours(drawing, contours_poly, i, color, 1, 8,
			// std::vector<Vec4i>(), 0, Point());
			// rectangle(drawing, boundRect[i].tl(), boundRect[i].br(), color,
			// 2,
			//   8, 0);
			// putText(drawing,
			// 		std::to_string(static_cast<int>(std::floor(mu[i].m00))),
			// 		boundRect[i].tl() - Point2i(0, 10), FONT_HERSHEY_DUPLEX,
			// 		1.0, Scalar(0, 0, 255));
		}

		iObj = 0;
		auto it = objects.begin();
		int nbDeleteObj = 0;
		while (it != objects.end()) {
			// while (iObj < nbObjects) {
			Object &obj = *it;
			int iMov = nearestMov[iObj];
			// int minDist = 99999;
			// int minMov = -1;
			// for (int iMov = 0; iMov < nbMovements; ++iMov) {
			// 	if (nearestObj[iMov] == iObj) {
			// 		int dist = distNearestObj[iMov];
			// 		if (minDist > dist) {
			// 			minDist = dist;
			// 			minMov = iMov;
			// 		}
			// 	}
			// }

			// delete object if not moving (no event)
			if (iMov == -1) {
				if (obj.dist < 100) {
					tombs.push_back({obj.pos, obj.color});

					objects.erase(it++);
					// --iNewObj;
					++nbDeleteObj;
					// ++it;
					// putText(drawing, "d", obj.pos + Point2f(-9, 9),
					// FONT_HERSHEY_COMPLEX, 0.8, obj.color, 2);
					// std::cout << "delete object" << std::endl;
				} else {
					putText(drawing, "s", obj.pos + Point2f(-9, 9),
							FONT_HERSHEY_COMPLEX, 0.8, obj.color, 1);
					// std::cout << "standbye object" << std::endl;
					++it;
				}

			}
			// move object
			else {
				// Object &obj = objects[iObj];
				if (mu[iMov].m00 > obj.bestCapture.density) {
					Capture cap{Mat(inputFrame, boundRect[iMov]).clone(),
								Mat(mask, boundRect[iMov]).clone(),
								contours[iMov], boundRect[iMov], mu[iMov].m00};
					// iNewObj++};
					// Object &&obj{
					// 	0.0,		 mc[iMov],  static_cast<int>(mu[iMov].m00),
					// 	Vec2f(0, 0), obj.color, iNewObj++,
					// 	cap};

					obj.bestCapture = cap;
				}
				// assert(0 <= iMov && iMov < mc.size() && iMov < nbMovements);
				// std::cout << "loop 2 : " << mc[iMov] << obj.pos << std::endl;
				obj.speedVector = mc[iMov] - obj.pos;
				obj.dist += norm(obj.speedVector);
				// line(drawing, obj.pos, mc[i], obj.color);
				lines.push_back({obj.pos, mc[iMov], obj.color});
				obj.pos = mc[iMov];
				obj.density = mu[iMov].m00;

				rectangle(drawing, boundRect[iMov].tl(), boundRect[iMov].br(),
						  obj.color, 2);

				// std::cout << "loop 3 : " << iMov << "/" << nbMovements <<
				// std::endl; std::cout << "loop 3 : " << boundRect[iMov].tl()
				putText(drawing, "+", obj.pos + Point2f(-9, 9),
						FONT_HERSHEY_COMPLEX, 0.8, Scalar(0, 255, 255), 1);
				// << std::endl;
				putText(drawing, std::to_string(obj.id),
						boundRect[iMov].tl() - Point(0, 5),
						// boundRect[iMov].tl() - Point2i(0, 10),
						FONT_HERSHEY_DUPLEX, 0.8, obj.color, 1);
				putText(drawing, std::to_string(obj.dist),
						boundRect[iMov].tl() - Point(0, 25),
						// boundRect[iMov].tl() - Point2i(0, 10),
						FONT_HERSHEY_DUPLEX, 0.8, obj.color, 1);
				putText(drawing, std::to_string(obj.density),
						boundRect[iMov].tl() - Point(0, 45),
						// boundRect[iMov].tl() - Point2i(0, 10),
						FONT_HERSHEY_DUPLEX, 0.8, obj.color, 1);
				line(drawing, obj.pos, obj.pos + obj.speedVector * 5,
					 Scalar(0, 0, 255), 1);

				// std::cout << "move object " << std::endl;
				++it;
			}

			++iObj;
		}
		// std::cout << iObj << objects.size() << nbDeleteObj << std::endl;
		// assert(iObj == objects.size() + nbDeleteObj);

		// objects.emplace_back(newObjects);
		// objects += newObjects;
		// objects.push_back(std::move(newObjects));
		// objects.emplace_back(std::move(newObjects));
		for (auto &obj : newObjects) {
			objects.push_back(obj);
		}

		for (int i = 0; i < nbMovements; ++i) {
			putText(drawing, "+", mc[i] + Point2f(-9, 9), FONT_HERSHEY_COMPLEX,
					0.8, Scalar(0, 0, 0), 1);
			// rectangle(drawing, boundRect[i].tl(), boundRect[i].br(),
			// 		  Scalar(0, 0, 0), 2);
		}

		putText(drawing, "nbObjs : " + std::to_string(nbObjects), Point(0, 30),
				FONT_HERSHEY_DUPLEX, 0.8, Scalar(0, 0, 255));

		putText(drawing, "nbMovs : " + std::to_string(nbMovements),
				Point(0, 60), FONT_HERSHEY_DUPLEX, 0.8, Scalar(0, 0, 255));

		putText(drawing, "nbDeleteObj : " + std::to_string(nbDeleteObj),
				Point(0, 90), FONT_HERSHEY_DUPLEX, 0.8, Scalar(0, 0, 255));

		// Mat m;
		// int i =0;
		for (Object obj : objects) {
			// if (i++ != 1)
			// continue;
			// drawing(obj.bestCapture.rect) = obj.bestCapture.img;
			// Mat m = Mat(drawing, obj.bestCapture.rect);
			Mat m = obj.bestCapture.img;

			// m = obj.bestCapture.img;

			// obj.bestCapture.img.copyTo(m);

			// cvtColor(m, m, COLOR_BGR2GRAY);
			// bitwise_not(m, m);
			// Canny(m, m, 0, 255);

			// cvtColor(m, m, COLOR_GRAY2BGR);
			m.copyTo(Mat(drawing, obj.bestCapture.rect), obj.bestCapture.mask);
			// m = Scalar(0, 0, 0);
			// m = obj.bestCapture.img;
			// rectangle(drawing, obj.bestCapture.rect, obj.color, 2);
            std::vector<std::vector<Point>> contours {obj.bestCapture.contour};
			drawContours(drawing, contours, 0, obj.color, 2);
			// Mat m = Mat(drawing, obj.bestCapture.rect);
			// m = obj.bestCapture.img;
		}
		// imshow("m", m);

		for (size_t i = 0; i < lines.size(); ++i) {
			line(drawing, lines[i].p, lines[i].p2, lines[i].color, 2);
		}

		for (DeadObj obj : tombs) {
			putText(drawing, "d", obj.p + Point(-9, 9), FONT_HERSHEY_COMPLEX,
					0.8, obj.color, 1);
		}

		imshow("drawing", drawing);
		// imshow("image", inputFrame);
		imshow("mask", mask);
		if (waitKey(1) == 'q')
			break;
	}

    imwrite("result.jpg", drawing);
	std::cout << "done.\n";

	return 0;
}