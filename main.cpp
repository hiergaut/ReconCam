#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc.hpp"
//#include "opencv2/opencv.hpp"
//#include "opencv2/video.hpp" // model

//#include <assert.h>
//#include <cassert>
//#include <chrono>
//#include <ctime>
#include <dirent.h>
#include <fstream>
#include <iomanip>
#include <iostream>
//#include <list>
//#include <map>
//#include <sstream>
//#include <string>
//#include <thread>
#include <list>
//#include <set>
#include <set>
#include <thread>
#include <unistd.h>

#include "Gpio.hpp"
#include "System.hpp"
#include "utils.hpp"

#define PC

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

#define NEW_OBJECT_MIN_DENSITY 10
#define MAX_ERROR_DIST_FOR_NEW_POS_OBJECT 100
#define MIN_MOV_YEARS_TO_SAVE_OBJECT 5

#define MAX_MOVEMENTS 20

#define WIDTH 640
#define HEIGHT 480

using namespace cv;
using namespace std;

RNG rng(29791);

// ------------------------------- MAIN ---------------------------------------

#define HEADER "[" << std::this_thread::get_id() << "] "

int main(int argc, char** argv)
{
    cout << HEADER "OpenCV version : " << CV_VERSION << endl;
    cout << HEADER "Major version : " << CV_MAJOR_VERSION << endl;
    cout << HEADER "Minor version : " << CV_MINOR_VERSION << endl;
    cout << HEADER "Subminor version : " << CV_SUBMINOR_VERSION << endl;

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
        "{script        |           | launch script on recognize}"
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
    std::string script = parser.get<std::string>("script");
    //    bool hasScript = !script.empty();
    bool hasStream = !stream.empty();
    if (!parser.check()) {
        parser.printMessage();
        parser.printErrors();
        return 0;
    }

#ifdef PC
    gpioDir = "gpio/";

    // if (hasStream) {
    namedWindow("mask", WINDOW_AUTOSIZE);
    moveWindow("mask", 1920, 1080);
    namedWindow("mask2", WINDOW_AUTOSIZE);
    moveWindow("mask2", 1920 + 640, 1080);
    namedWindow("drawing", WINDOW_AUTOSIZE);
    moveWindow("drawing", 1920 + 640 * 2, 1080);
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
    const std::string hostname = getHostname();

    VideoCapture vCap;

    std::string timelapseDir = motionDir + getYear() + "/" + getMonth() + "/" + getDay() + "/timelapse_" + hostname + "_" + deviceName;
    //    std::string timelapseDir = motionDir + "timelapse_" + hostname + "_" + deviceName;
    std::string cmd = "mkdir -p " + timelapseDir;
    system(cmd.c_str());

    //    std::set<Object> objects;
    //    std::vector<DeadObj> tombs;

    Mat inputFrame, mask, drawing;
    //        int iNewObj;
    int iFrame;

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

    cout << HEADER "check camera" << endl;
    if (vCap.open(stream)) {
        vCap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
        vCap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);

        if ((int)vCap.get(cv::CAP_PROP_FRAME_WIDTH) != WIDTH
            || (int)vCap.get(cv::CAP_PROP_FRAME_HEIGHT) != HEIGHT)
            std::cout << HEADER "Warning! Can not adjust video capture properties!" << std::endl;

        width = vCap.get(cv::CAP_PROP_FRAME_WIDTH);
        height = vCap.get(cv::CAP_PROP_FRAME_HEIGHT);

        Mat meter_image;
        if (vCap.read(meter_image)) {
            //            imwrite("/tmp/reconCamStartupTest.jpg", meter_image);
            vCap.release();
            cout << HEADER "camera '" << stream << "' is ok" << endl;
            // return 0;
        } else {
            vCap.release();
            cout << HEADER "could not read frame" << endl;
            return -1; // can not read frame
        }
    } else {
        cout << HEADER "could not open video capture device " << stream << endl;
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

//    bool movsFound[MAX_MOVEMENTS];
#ifdef PC
    bool quit = false;
#endif

    const auto timelapseStart = std::chrono::high_resolution_clock::now();
    int timelapseCounter = -1;

    // --------------------------- INFINITE LOOP ------------------------------
    while (1) {
        auto timelapseEnd = std::chrono::high_resolution_clock::now();
        auto timelapseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timelapseEnd - timelapseStart).count() / 1000.0f - timelapseCounter * TIMELAPSE_INTERVAL;
        // std::cout << "[TIMELAPSE] wait new motion, future lapse at "
        //   << tickTimeLapse << " sec " << std::endl;
        std::cout << HEADER "[TIMELAPSE] wait new motion, future lapse at "
                  << TIMELAPSE_INTERVAL - timelapseDuration << " sec " << std::endl;

        // if no movement, wait for timelapse photo
        while (!hasMovement()) {

            std::cout << "." << std::flush;
            usleep(CLOCKS_PER_SEC);
            // --tickTimeLapse;

            timelapseEnd = std::chrono::high_resolution_clock::now();
            // timelapseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timelapseEnd - timelapseStart).count() / 1000.0f;
            timelapseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timelapseEnd - timelapseStart).count() / 1000.0f - timelapseCounter * TIMELAPSE_INTERVAL;
            if (timelapseDuration > TIMELAPSE_INTERVAL) {
                // if (tickTimeLapse == 0) {
                std::cout << std::endl;
                vCap.open(stream);
                vCap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
                vCap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);
                //
                // vCap.open(CAP_V4L2);
                if (!vCap.isOpened()) {
                    std::cout << HEADER "device not found";
                    return 1;
                }
                for (int i = 0; i < NB_CAP_FOCUS_BRIGHTNESS + 100; ++i) {
                    vCap >> inputFrame;
                }

                if (flip180) {
                    flip(inputFrame, inputFrame, -1);
                }
                vCap.release();

                std::string saveLapse = timelapseDir + "/" + getCurTime() + ".jpg";
                imwrite(saveLapse, inputFrame);
                imwrite(timelapseDir + "/latest.jpeg", inputFrame);
                std::cout << HEADER "[TIMELAPSE] save lapse '" << saveLapse << "'"
                          << std::endl;

                cmd = "convert " + timelapseDir + "/*.jpg " + timelapseDir + "/timelapse.gif";
                std::cout << HEADER "[TIMELAPSE] " << cmd << std::endl;
                //                system(cmd.c_str());

                if (hasRemoteDir) {
                    if (port == -1) {
                        cmd += " && rsync -arv " + motionDir + " " + remoteDir;
                    } else {
                        cmd += " && rsync -arv -e 'ssh -p " + std::to_string(port) + "' " + motionDir + " " + remoteDir;
                    }
                    //                    std::thread t([cmd]() {
                    //                        std::cout << HEADER "[TIMELAPSE] " << cmd << std::endl;
                    //                        system(cmd.c_str());
                    //                        return 0;
                    //                    });
                }
                system(("(" + cmd + ") &").c_str());

                // timelapseStart = timelapseEnd = std::chrono::high_resolution_clock::now();
                // timelapseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timelapseEnd - timelapseStart).count() / 1000.0f;
                ++timelapseCounter;
                timelapseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timelapseEnd - timelapseStart).count() / 1000.0f - timelapseCounter * TIMELAPSE_INTERVAL;
                std::cout << HEADER "[TIMELAPSE] future lapse at " << TIMELAPSE_INTERVAL - timelapseDuration
                          << " sec " << std::endl;
            }
        } // while (!hasMovement())
        std::cout << std::endl;

        // movement detected !!!
        // start recording

        if (lightGpio != -1 && isNight()) {
            gpioSetValue(lightGpio, 1);
        }

        std::string motionId = getYear() + "/" + getMonth() + "/" + getDay() + "/" + getCurTime() + "_" + hostname + "_" + deviceName;
        std::cout << HEADER "new event : " << motionId << std::endl;
        std::string newMotionDir = motionDir + motionId;
        cmd = "mkdir -p " + newMotionDir;
        system(cmd.c_str());

        vCap.open(stream);
        vCap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
        vCap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);
        //        vCap.set(cv::CAP_PROP_FPS, 60);
        // vCap.open(CAP_V4L2);
        if (!vCap.isOpened()) {
            std::cout << HEADER "device not found";
            return 1;
        }

        std::string outputVideoFile = newMotionDir + "/video.webm";
        VideoWriter outputVideo = VideoWriter(
            outputVideoFile, cv::VideoWriter::fourcc('V', 'P', '8', '0'), 3,
            sizeScreen, true);

        //        std::string outputVideoFileRec = newMotionDir + "/video.mp4";
        //        VideoWriter outputVideoRec = VideoWriter(
        //            outputVideoFile, cv::VideoWriter::fourcc('M', 'P', '4', 'V'), 3,
        //            sizeScreen, true);

        std::string outputVideoFileRec;
        VideoWriter outputVideoRec;
        outputVideoFileRec = newMotionDir + "/video.avi";
        outputVideoRec = VideoWriter(
            outputVideoFileRec, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), 15,
            sizeScreen, true);

        if (!outputVideo.isOpened() || !outputVideoRec.isOpened()) {
            //        if (!outputVideo.isOpened()) {
            std::cout << HEADER "failed to write video" << std::endl;
            return 6;
        }
        auto videoStart = std::chrono::high_resolution_clock::now();

        int iNewObj = 0;
        //        tombs.clear();
        //        objects.clear();

        auto model = createBackgroundSubtractorMOG2();
        // auto model = createBackgroundSubtractorKNN();
        // auto model = createBackgroundSubtractorGMG();

        std::list<DeadObj> tombs;
        std::list<Object> objects;

        auto frameStart = std::chrono::high_resolution_clock::now();

        //        int nbObjects = 0;
        iFrame = 0;
        int nMovement = 0;
        // ----------------------- WHILE HAS MOVEMENT
        while (hasMovement() || nMovement > 0) {
            //            auto frameStart = std::chrono::high_resolution_clock::now();
            vCap >> inputFrame;
            assert(!inputFrame.empty());
            if (flip180) {
                flip(inputFrame, inputFrame, -1);
            }
            outputVideoRec << inputFrame;
            drawing = inputFrame;

            // ------------------------ START

            if (iFrame <= NB_CAP_FOCUS_BRIGHTNESS) {
                rectangle(drawing, Rect(640 - 50, 0, 50, 50), Scalar(255, 0, 0),
                    -1);

            } else {

                // equalizeHist(grey, grey);
                // model->apply(grey, mask);
                model->apply(inputFrame, mask);
#ifdef PC
                imshow("mask", mask);
#endif

                if (iFrame <= NB_CAP_FOCUS_BRIGHTNESS + NB_CAP_LEARNING_MODEL_FIRST) {
                    rectangle(drawing, Rect(640 - 50, 0, 50, 50), Scalar(0, 255, 0),
                        -1);

                } else {

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
                    std::vector<std::vector<Point>> movContours;
                    std::vector<Vec4i> hierarchy;
                    findContours(mask, movContours, hierarchy, RETR_TREE,
                        CHAIN_APPROX_SIMPLE, Point(0, 0));

                    nMovement = movContours.size();
                    // to many movements -> no detection
                    if (nMovement > MAX_MOVEMENTS) {
                        rectangle(drawing, Rect(640 - 50, 0, 50, 50), Scalar(0, 0, 255),
                            -1);

                    } else {

                        std::set<Movement> movements;
                        {
                            std::vector<std::vector<Point>> contours_poly(nMovement);
                            std::vector<Rect> movBoundRect(nMovement);
                            for (int i = 0; i < nMovement; ++i) {
                                approxPolyDP(Mat(movContours[i]), contours_poly[i], 3, true);
                                movBoundRect[i] = boundingRect(Mat(contours_poly[i]));
                            }

                            std::vector<Moments> movMoments(nMovement);
                            for (int i = 0; i < nMovement; ++i) {
                                movMoments[i] = moments(movContours[i], true);
                            }

                            // drawing = inputFrame.clone();
                            std::vector<Point2f> movCenters(nMovement);
                            for (int i = 0; i < nMovement; ++i) {
                                movCenters[i] = Point2f(movMoments[i].m10 / movMoments[i].m00, movMoments[i].m01 / movMoments[i].m00);
                            }

                            for (int i = 0; i < nMovement; ++i) {
                                movements.insert({ movCenters[i], movMoments[i].m00, false, movBoundRect[i], movContours[i] });
                            }
                        }

                        // ------------------- FIND PREVIOUS OBJECTS POSITIONS

                        //                        std::set<Object>::iterator it = objects.begin();
                        auto it = objects.begin();
                        while (it != objects.end()) {
                            //                        for (auto & obj : objects) {
                            //                            Object& obj = const_cast<Object&>(*it);
                            Object& obj = *it;

                            //                            int iMovNearest = -1;
                            Movement* nearestMov = nullptr;
                            double distMovMin = MAX_ERROR_DIST_FOR_NEW_POS_OBJECT;
                            //                            for (int i = 0; i < nMovement; ++i) {
                            for (const auto& mov : movements) {
                                //                                if (!movsFound[i]) {
                                if (!mov.found) {
                                    if (3.0 * mov.density / obj.density > distMovMin) {
                                        break;
                                    }
                                    double density = 3.0 * fmax(obj.density, mov.density) / fmin(obj.density, mov.density);
                                    //                                    double density = pow(obj.density - movMoments[i].m00, 2);
                                    double distance = norm(obj.pos + obj.speedVector - mov.center);
                                    double dist = distance + density;

                                    if (dist <= distMovMin) {
                                        distMovMin = dist;
                                        //                                        iMovNearest = i;
                                        nearestMov = const_cast<Movement*>(&mov);
                                    }
                                }
                            }

                            // no event near object
                            if (nearestMov == nullptr) {
                                // if (obj.distance < MIN_MOV_DIST_TO_SAVE_OBJECT) {
                                if (obj.age < MIN_MOV_YEARS_TO_SAVE_OBJECT) {
                                    tombs.push_back({ obj.pos, obj.color });
                                    it = objects.erase(it);
                                    continue;
                                }
                                // save position of no moved object
                                else {
                                    // std::cout << "object saved";
                                    putText(drawing, "s", obj.pos + Point2i(-9, 9),
                                        FONT_HERSHEY_DUPLEX, 1, obj.color, 1);
                                    // ++it;
                                    //                                    newObjects.insert(std::move(obj));
                                }

                            }
                            // object moved
                            else {
                                //                                movsFound[iMovNearest] = true;
                                nearestMov->found = true;

                                Point2i tl = nearestMov->boundRect.tl();
                                Point2i tr = tl + Point2i(nearestMov->boundRect.width + 5, 10);

                                // std::vector<std::vector<Point>> movContours
                                // {movCountours[iMovNearest]};
                                // std::cout << "rect : " << rect << std::endl;
                                //                                int density = nearestMov->density;
                                //                                assert(!inputFrame.empty());
                                // NColors colors = Capture::getPrimaryColors(img, m);

                                // if found best trace
                                //                                if (movMoments[iMovNearest].m00 > obj.trace[obj.bestCapture].m_density) {
                                if (nearestMov->density > obj.biggestCapture.m_density) {
                                    const Rect& rect { nearestMov->boundRect };
                                    assert(!rect.empty());
                                    Mat img = Mat(inputFrame, rect).clone();
                                    Mat m = Mat(mask, rect).clone();
                                    obj.biggestCapture = { std::move(img), std::move(m), nearestMov->contours,
                                        rect, nearestMov->boundRect.x, nearestMov->boundRect.y, rect.width,
                                        rect.height, nearestMov->density };
                                    //                                    obj.bestCapture = obj.trace.size() - 1;
                                    //                                    obj.biggestCapture = cap;
                                }
                                obj.speedVector = nearestMov->center - obj.pos;

                                //                                obj.distance += norm(nearestMov->center - obj.firstPos);
                                obj.distance += norm(obj.speedVector);

                                obj.lines.emplace_back(obj.pos, nearestMov->center);
                                obj.pos = nearestMov->center;
                                obj.density = nearestMov->density;
                                ++obj.age;

                                rectangle(drawing, tl, nearestMov->boundRect.br(), obj.color, 2);

                                //                    if (obj.name.compare("")) {
                                //                        putText(drawing, obj.name, tr + Point(0, 0),
                                //                            FONT_HERSHEY_DUPLEX, 0.5, obj.color, 1);
                                //                    } else {
                                putText(drawing, std::to_string(obj.id),
                                    tr + Point(0, 0), FONT_HERSHEY_DUPLEX, 0.5,
                                    obj.color, 1);
                                //                    }
                                putText(drawing,
                                    std::to_string(static_cast<int>(obj.distance)),
                                    tr + Point(0, 20), FONT_HERSHEY_DUPLEX, 0.5,
                                    obj.color, 1);
                                putText(drawing, std::to_string(obj.density),
                                    tr + Point(0, 40), FONT_HERSHEY_DUPLEX, 0.5,
                                    obj.color, 1);

                                line(drawing, obj.pos, obj.pos + obj.speedVector,
                                    Scalar(0, 0, 255), 1, LineTypes::LINE_AA);

                                //                                obj.trace.emplace_back(std::move(cap));
                                // obj.trace.emplace_back(cap);
                                //                                newObjects.insert(std::move(obj));

                                //                                objects.emplace_front(obj);

                            } // if (iMovNearest != -1)

                            ++it;
                        } // while (it != objects.end())

                        //                        objects = std::move(newObjects);

                        // non objects movements become new objects
                        //                        for (int i = 0; i < nMovement; ++i) {
                        for (const auto& mov : movements) {

                            //                            int density = mov.density;
                            // new object
                            if (!mov.found && mov.density > NEW_OBJECT_MIN_DENSITY) {
                                Scalar color = Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
                                    rng.uniform(0, 255));

                                const Rect& rect { mov.boundRect };
                                // std::cout << "rect : " << rect << std::endl;
                                assert(!inputFrame.empty());
                                assert(!rect.empty());
                                const Mat& img = Mat(inputFrame, rect).clone();
                                assert(!img.empty());
                                const Mat& m = Mat(mask, rect).clone();
                                assert(!m.empty());
                                // NColors colors = Capture::getPrimaryColors(img, m);
                                Capture cap({ std::move(img), std::move(m), mov.contours, rect,
                                    mov.center.x, mov.center.y, rect.width, rect.height,
                                    mov.density });

                                const std::vector<std::vector<Point>>& contour = { mov.contours };
                                drawContours(drawing, contour, 0, color, 2);

                                objects.emplace_back(Object { 0.0, mov.center, mov.density, Point2i(0, 0),
                                    color, iNewObj++, std::move(cap),
                                    mov.center, 0u, {} });

                                //                                objects.insert(std::move(obj));
                                //                                objects.emplace_back(std::move(obj));
                            }
                        }
                        //            nbObjects = objects.size();

                        for (const auto& obj : objects) {
                            for (const auto& l : obj.lines) {
                                line(drawing, l.first, l.second, obj.color, 2);
                            }
                        }

                        for (const DeadObj& obj : tombs) {
                            putText(drawing, "x", obj.p + Point(-9, 9), FONT_HERSHEY_DUPLEX,
                                1, obj.color, 1);
                        }

                    } // if (nMovement >= MAX_MOVEMENTS)

                } // if (iFrame <= NB_CAP_FOCUS_BRIGHTNESS + NB_CAP_LEARNING_MODEL_FIRST)

            } // if (iFrame <= NB_CAP_FOCUS_BRIGHTNESS)

            putText(drawing, "nbObjs : " + std::to_string(objects.size()),
                Point(0, 30), FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 0), 5);
            putText(drawing, "nbObjs : " + std::to_string(objects.size()),
                Point(0, 30), FONT_HERSHEY_DUPLEX, 1,
                Scalar(255, 255, 255));

            putText(drawing, "frame : " + std::to_string(iFrame), Point(0, 60),
                FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 0), 5);
            putText(drawing, "frame : " + std::to_string(iFrame), Point(0, 60),
                FONT_HERSHEY_DUPLEX, 1, Scalar(255, 255, 255), 1);

            // std::cout << "frame : " << iFrame << "\r" << std::flush;
            std::cout << "+" << std::flush;

            double frameDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                                       std::chrono::high_resolution_clock::now() - frameStart)
                                       .count()
                / 1000.0;
            frameStart = std::chrono::high_resolution_clock::now();

            //            std::ostringstream fps;
            //            fps << std::fixed << std::setprecision(2) << 1.0 / frameDuration;
            //            fps << 1.0 / frameDuration;
            double fps = 1.0 / frameDuration;
            putText(drawing, "fps : " + std::to_string(fps), Point(0, 90),
                FONT_HERSHEY_DUPLEX, 1, Scalar(0, 0, 0), 5);
            putText(drawing, "fps : " + std::to_string(fps), Point(0, 90),
                FONT_HERSHEY_DUPLEX, 1, Scalar(255, 255, 255), 1);

            outputVideo << drawing;
#ifdef PC
            imshow("drawing", drawing);
            if (waitKey(1) == 'q') {
                quit = true;
                break;
            }
#endif
            ++iFrame;
            //            std::cout << "this thread : " << std::this_thread::get_id() << std::endl;

        } // while (hasMovement())

        vCap.release();
        //        auto end = std::chrono::high_resolution_clock::now();
        auto videoDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - videoStart)
                                 .count()
            / 1000.0;

        if (lightGpio != -1) {
            gpioSetValue(lightGpio, 0);
        }

        int nbRealObjects = 0;
        // draw all detected object movements in drawing capture
        for (const Object& obj : objects) {

            // if (obj.distance > MIN_MOV_DIST_TO_SAVE_OBJECT) {
            if (obj.age > MIN_MOV_YEARS_TO_SAVE_OBJECT) {

                // assert(obj.trace[obj.bestCapture] != nullptr);
                //                const Capture& bestCapture = obj.trace[obj.bestCapture];
                const Capture& bestCapture = obj.biggestCapture;
                // const Mat &img = bestCapture.m_img;

                assert(!bestCapture.m_rect.empty());
                assert(!bestCapture.m_mask.empty());
                assert(bestCapture.m_rect.size() == bestCapture.m_mask.size());

                bestCapture.m_img.copyTo(Mat(drawing, bestCapture.m_rect),
                    bestCapture.m_mask);
                std::vector<std::vector<Point>> movCountours { bestCapture.m_contour };
                drawContours(drawing, movCountours, 0, obj.color, 2);
                ++nbRealObjects;
            }
        }

        outputVideo << drawing;
        outputVideo.release();
        outputVideoRec.release();
        // std::cout << "save video '" << outputVideoFile << "'" << std::endl;

        imwrite(newMotionDir + "/trace.jpg", drawing);
        // std::cout << "save trace file '" << newMotionDir + "/trace.jpg'"
        //   << std::endl;
        if (nbRealObjects > 0) {
            cmd = "touch " + newMotionDir + "/objectDetected.var";
            std::cout << HEADER << cmd << std::endl;
            system(cmd.c_str());
        }

        if (hasRemoteDir) {
            if (port == -1) {
                cmd = "rsync -arv " + motionDir + " " + remoteDir;
            } else {
                cmd = "rsync -arv -e 'ssh -p " + std::to_string(port) + "' " + motionDir + " " + remoteDir;
            }
            //            std::thread t([cmd]() {
            std::cout << HEADER << cmd << std::endl;
            system((cmd + " &").c_str());
            //            });
        }
        // }

#ifdef PC
        destroyAllWindows();
#endif

        std::cout << HEADER "object detected : " << nbRealObjects << std::endl;
        std::cout << HEADER "duration : " << videoDuration << std::endl;
        std::cout << HEADER "nb capture : " << iFrame << std::endl;
        std::cout << HEADER "recording fps : " << static_cast<double>(iFrame) / videoDuration
                  << std::endl;

#ifdef PC
        if (quit) {
            std::this_thread::sleep_for(std::chrono::seconds(5)); // wait rsync
            return 0;
        }
#endif

    } // while (1)

    return 0;
}
