#include "opencv2/highgui/highgui.hpp"
//#include "opencv2/imgproc.hpp"
//#include "opencv2/opencv.hpp"
//#include "opencv2/video.hpp" // model

//#define DETECTION
//#define TINY_YOLO

#ifdef DETECTION
#include <opencv2/dnn.hpp>
#endif

#include <dirent.h>
#include <iostream>
#include <list>
#include <set>
#include <string>

#include "Gpio.hpp"
#include "System.hpp"
#include "utils.hpp"

#ifdef PC
//#define TIMELAPSE_INTERVAL 20
#define TIMELAPSE_INTERVAL 600
#else
// #define TIMELAPSE_INTERVAL 1800 // 30 min
// #define TIMELAPSE_INTERVAL 1200 // 20 min
// #define TIMELAPSE_INTERVAL 300 // 5 min
#define TIMELAPSE_INTERVAL 600 // 10 min
#endif

#define NB_CAP_FOCUS_BRIGHTNESS 2
#define NB_CAP_LEARNING_MODEL_FIRST 5

#define NEW_OBJECT_MIN_DENSITY 10
#define MAX_ERROR_DIST_FOR_NEW_POS_OBJECT 100
#define MIN_MOV_YEARS_TO_SAVE_OBJECT 5

#define NON_BLACK_IMG_THRESHOLD 500

#define MAX_MOVEMENTS 10

//#define WIDTH 2592
//#define HEIGHT 1944

//#define WIDTH 1640
//#define HEIGHT 1232

//#define WIDTH 1296
//#define HEIGHT 972

#define WIDTH 640
#define HEIGHT 480

#ifdef PC
#define FPS 30
#else
#define FPS 15
#endif

#define HEADER colorHash(std::this_thread::get_id()) << "[" << std::this_thread::get_id() << "]\033[0m "

#ifdef DETECTION
cv::RNG rng(29791);

// Initialize the parameters
float confThreshold = 0.5; // Confidence threshold
float nmsThreshold = 0.4; // Non-maximum suppression threshold
int inpWidth = 416; // Width of network's input image
int inpHeight = 416; // Height of network's input image
std::vector<std::string> classes;

void drawPred(const Capture& capture, cv::Mat& frame, const cv::Scalar& color);
std::vector<cv::String> getOutputsNames(const cv::dnn::Net& net);
void detect(cv::dnn::Net& net, Capture& capture);
#endif

void openCamera(cv::VideoCapture& vCap, const std::string& stream, cv::Mat& inputFrame);

// ------------------------------- MAIN ---------------------------------------

int main(int argc, char** argv)
{
    std::cout << HEADER "OpenCV version : " << CV_VERSION << std::endl;
    std::cout << HEADER "Major version : " << CV_MAJOR_VERSION << std::endl;
    std::cout << HEADER "Minor version : " << CV_MINOR_VERSION << std::endl;
    std::cout << HEADER "Subminor version : " << CV_SUBMINOR_VERSION << std::endl;

    // if ( CV_MAJOR_VERSION < 3)
    // {
    //   // Old OpenCV 2 code goes here.
    // } else
    // {
    //   // New OpenCV 3 code goes here.
    // }
    // return 0;

    //    std::cout << std::fixed << std::setprecision(3);

    cv::CommandLineParser parser(
        argc, argv,
        "{s sensor      | -1        | gpio number of IR senror}"
        //        "{a and         | -1        | add detect sensor, (and logic)}"
        "{l light       | -1        | pin light up on movment}"
        "{d device      | 0         | /dev/video<device>}"
        "{stream        |           | camera/video src}"
        "{f flip        | false     | rotate image 180}"
        "{fd fullDetect | false     | detection with motion}"
        "{r repository  |           | save motion to specific repo}"
        "{p port        | -1        | remote port repository}"
        "{script        |           | launch script on recognize human}"
        "{help h        |           | help message}");

    if (parser.has("help")) {
        parser.printMessage();
        return 0;
    }
    sensorGpioNum = parser.get<int>("sensor");
    //    const bool noSensor = sensorGpioNum == -1;
    //    sensorAdditional = parser.get<int>("and");
    const int device = parser.get<int>("device");
    std::string stream = parser.get<std::string>("stream");
    const std::string remoteDir = parser.get<std::string>("repository");
    std::cout << "remoteDir = '" << remoteDir << "'" << std::endl;
    const int port = parser.get<int>("port");
    const bool flip180 = parser.get<bool>("flip");
    //    const bool fullDetect = parser.get<bool>("fullDetect");
    const int lightGpio = parser.get<int>("light");
    const std::string script = parser.get<std::string>("script");
#ifdef DETECTION
    const bool hasScript = !script.empty();
#endif
    const bool hasStream = !stream.empty();
    if (!parser.check()) {
        parser.printMessage();
        parser.printErrors();
        return 0;
    }

#ifdef PC
    gpioDir = "gpio/";

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

    std::string motionPath, motionStartTime, deviceId = "";

    std::size_t pos = deviceName.find_first_of("/");
    std::string rootDir = deviceName.substr(0, pos);

#ifdef DETECTION
    bool recordDetection = false;
    if (rootDir == "motion") {
        motionPath = deviceName.substr(7, 11);
        //        std::cout << motionPath << std::endl;
        motionStartTime = deviceName.substr(18, 8);
        //        std::cout << motionStartTime << std::endl;
        deviceId = deviceName.substr(deviceName.find_first_of("_") + 1, deviceName.find_last_of("/") - deviceName.find_first_of("_") - 1);
        //        std::cout << deviceId << std::endl;
        recordDetection = true;
    }
#endif

    const bool hasRemoteDir = !remoteDir.empty();
    const std::string motionRootDir = "motion/";
//    if (hasRemoteDir) {
//        motionRootDir = "/tmp/motion/";
//    } else {
//        motionRootDir = "motion/";
//    }
#ifdef DETECTION
    if (!recordDetection) {
#endif
        const std::string hostname = getHostname();
        deviceId = hostname + "_" + deviceName;
#ifdef DETECTION
    }
#endif

    cv::VideoCapture vCap;

//    cv::Mat inputFrame, mask;
#ifdef DETECTION
    cv::Mat drawing;
#endif

    int width = -1;
    int height = -1;

    std::cout << HEADER "check camera/stream" << std::endl;
    if (vCap.open(stream)) {
        vCap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
        vCap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);

        if ((int)vCap.get(cv::CAP_PROP_FRAME_WIDTH) != WIDTH
            || (int)vCap.get(cv::CAP_PROP_FRAME_HEIGHT) != HEIGHT)
            std::cout << HEADER "Warning! Can not adjust video capture properties!" << std::endl;

        width = vCap.get(cv::CAP_PROP_FRAME_WIDTH);
        height = vCap.get(cv::CAP_PROP_FRAME_HEIGHT);

        cv::Mat meter_image;
        if (vCap.read(meter_image)) {
            //            imwrite("/tmp/reconCamStartupTest.jpg", meter_image);
            vCap.release();
            std::cout << HEADER "camera/stream '" << stream << "' is ok" << std::endl;
            // return 0;
        } else {
            vCap.release();
            std::cout << HEADER "could not read frame" << std::endl;
            return -1; // can not read frame
        }
    } else {
        std::cout << HEADER "could not open video capture device " << stream << std::endl;
        return -2; // can not open video capture device
    }

    const cv::Size sizeScreen(width, height);

    if (sensorGpioNum != -1) {
        initGpio(sensorGpioNum, "in");
        gpioGetValue(sensorGpioNum);

        //        if (sensorAdditional != -1) {
        //            initGpio(sensorAdditional, "in");
        //            gpioGetValue(sensorAdditional);
        //        }

        if (lightGpio != -1) {
            initGpio(lightGpio, "out");
            gpioSetValue(lightGpio, 0);
        }
    }

#ifdef PC
    bool quit = false;
#endif

#ifdef DETECTION
    // Init DNN
    // Load names of classes
    std::string classesFile = PROJECT_DIR "yolo/coco.names";
    std::ifstream ifs(classesFile.c_str());
    std::string lineStr;
    while (getline(ifs, lineStr))
        classes.push_back(lineStr);

        // Give the configuration and weight files for the model
#ifdef TINY_YOLO
    cv::String modelConfiguration = PROJECT_DIR "yolo/yolov3-tiny.cfg";
    cv::String modelWeights = PROJECT_DIR "yolo/yolov3-tiny.weights";
#else
    cv::String modelConfiguration = PROJECT_DIR "yolo/yolov3.cfg";
    cv::String modelWeights = PROJECT_DIR "yolo/yolov3.weights";
#endif

    // Load the network
    cv::dnn::Net net = cv::dnn::readNetFromDarknet(modelConfiguration, modelWeights);

    net.setPreferableBackend(cv::dnn::DNN_TARGET_CPU);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CPU);
    cv::Mat blob;
#endif
    std::string cmd;

    const auto timelapseStart = std::chrono::high_resolution_clock::now();
    int64_t timelapseCounter = -1;
    //    std::list<std::thread> threads;

    // --------------------------- INFINITE LOOP ------------------------------
    while (1) {
        auto timelapseEnd = std::chrono::high_resolution_clock::now();
        auto timelapseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timelapseEnd - timelapseStart).count() / 1000.0f - timelapseCounter * TIMELAPSE_INTERVAL;

        std::cout << HEADER "[TIMELAPSE] wait new motion, future lapse at "
                  << TIMELAPSE_INTERVAL - timelapseDuration << " sec " << std::endl;

        // if no movement, wait for timelapse photo
        while ((!hasMovement() || timelapseDuration >= TIMELAPSE_INTERVAL) && !hasStream) {

            //            std::cout << colorHash(std::this_thread::get_id()) << "." << "\033[0m" << std::flush;
            std::cout << "." << std::flush;
            usleep(CLOCKS_PER_SEC);

            timelapseEnd = std::chrono::high_resolution_clock::now();
            timelapseDuration = std::chrono::duration_cast<std::chrono::milliseconds>(timelapseEnd - timelapseStart).count() / 1000.0f - timelapseCounter * TIMELAPSE_INTERVAL;
            if (timelapseDuration >= TIMELAPSE_INTERVAL) {
                std::cout << std::endl;
                std::cout << HEADER "[TIMELAPSE] open stream" << std::endl;

                if (lightGpio != -1) {
                    gpioSetValue(lightGpio, 1);
                }

                cv::Mat timelapseFrame;
                openCamera(vCap, stream, timelapseFrame);
                if (!vCap.isOpened()) {
                    std::cout << HEADER "[TIMELAPSE] device '" << stream << "' not found" << std::endl;
                    return 10;
                }
                //                }
                //                std::cout << std::endl;

                std::cout << HEADER "[TIMELAPSE] vCap.release()" << std::endl;
                vCap.release();

                if (lightGpio != -1) {
                    gpioSetValue(lightGpio, 0);
                }

                std::cout << HEADER "[TIMELAPSE] flip frame" << std::endl;
                if (flip180) {
                    flip(timelapseFrame, timelapseFrame, -1);
                }

                std::string timelapseStartTime = getCurTime();

                int line = 30;

                putText(timelapseFrame, deviceId,
                    cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 5);
                putText(timelapseFrame, deviceId,
                    cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1,
                    cv::Scalar(255, 255, 255));
                line += 30;

                putText(timelapseFrame, timelapseStartTime,
                    cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 5);
                putText(timelapseFrame, timelapseStartTime,
                    cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1,
                    cv::Scalar(255, 255, 255));

                std::string timelapseDir = motionRootDir + getYear() + "/" + getMonth() + "/" + getDay() + "/timelapse_" + deviceId;
                cmd = "mkdir -p " + timelapseDir;
                std::cout << HEADER "[TIMELAPSE] " << cmd << std::endl;
                system(cmd.c_str());

                std::string saveLapse = timelapseDir + "/" + timelapseStartTime + ".jpg";
                imwrite(saveLapse, timelapseFrame);
                imwrite(timelapseDir + "/latest.jpeg", timelapseFrame);
                std::cout << HEADER "[TIMELAPSE] save lapse '" << saveLapse << "'"
                          << std::endl;

                cmd = "convert -delay 3.3 -resize 320x240 " + timelapseDir + "/*.jpg " + timelapseDir + "/timelapse.gif";
                //                system(cmd.c_str());

                if (hasRemoteDir) {
                    if (port == -1) {
                        cmd += " && rsync -arv " + motionRootDir + " " + remoteDir;
                    } else {
                        cmd += " && rsync -arv -e 'ssh -p " + std::to_string(port) + "' " + motionRootDir + " " + remoteDir;
                    }
                    //                    std::thread t([cmd]() {
                    //                        std::cout << HEADER "[TIMELAPSE] " << cmd << std::endl;
                    //                        system(cmd.c_str());
                    //                        return 0;
                    //                    });
                }
                std::cout << HEADER "[TIMELAPSE] '" << cmd << "'" << std::endl;
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
#ifdef PC
        // if (hasStream) {
        cv::namedWindow("mask", cv::WINDOW_AUTOSIZE);
        cv::moveWindow("mask", 1920, 1080);
        namedWindow("mask2", cv::WINDOW_AUTOSIZE);
        cv::moveWindow("mask2", 1920 + 640, 1080);
        cv::namedWindow("mask3", cv::WINDOW_AUTOSIZE);
        cv::moveWindow("mask3", 1920 + 640 * 2, 1080);
        namedWindow("mask4", cv::WINDOW_AUTOSIZE);
        cv::moveWindow("mask4", 1920, 1080 + 540);
        namedWindow("mask5", cv::WINDOW_AUTOSIZE);
        cv::moveWindow("mask5", 1920 + 640, 1080 + 540);
        namedWindow("drawing", cv::WINDOW_AUTOSIZE);
        cv::moveWindow("drawing", 1920 + 640 * 2, 1080 + 540);

//    namedWindow("blobs", cv::WINDOW_AUTOSIZE);
//    cv::moveWindow("blobs", 1920 + 640, 1080 + 480);
// }
#endif
        cv::Mat inputFrame;
        openCamera(vCap, stream, inputFrame);
        if (!vCap.isOpened()) {
            std::cout << HEADER "[CAPTURE] device '" << stream << "' not found" << std::endl;
            return 9;
        }
        bool streamFinished = false;
        vCap >> inputFrame;
        if (inputFrame.empty()) {
            std::cout << HEADER "[CAPTURE] stream finished" << std::endl;
            streamFinished = true;
            break;
        }
        if (flip180) {
            flip(inputFrame, inputFrame, -1);
        }
        auto model = cv::createBackgroundSubtractorMOG2();
        int nMovement = 0;
        int iFrame = 0;

        const cv::Size filterSize(25, 25);

#ifndef DETECTION
        // wait for movement in background model
        while (nMovement == 0) {
            vCap >> inputFrame;
            if (inputFrame.empty()) {
                std::cout << HEADER "[CAPTURE] stream finished" << std::endl;
                streamFinished = true;
                break;
            }
            if (flip180) {
                flip(inputFrame, inputFrame, -1);
            }

            if (iFrame < NB_CAP_FOCUS_BRIGHTNESS) {
                // do nothing
                std::cout << "b" << std::flush;
#ifdef PC
                rectangle(inputFrame, cv::Rect(640 - 50, 0, 50, 50), cv::Scalar(0, 0, 255), -1);
#endif
            } else {

                cv::Mat gray;
                cv::cvtColor(inputFrame, gray, cv::COLOR_BGR2GRAY);
                //                                equalizeHist(gray, gray);
                cv::GaussianBlur(gray, gray, filterSize, 0);
//                cv::GaussianBlur(gray, gray, cv::Size(15, 15), 0);
#ifdef PC
                imshow("mask", gray);
#endif
                //                model->apply(inputFrame, mask);
                cv::Mat mask;
                model->apply(gray, mask);
//                model->apply(inputFrame, mask);
#ifdef PC
                imshow("drawing", mask);
#endif

                if (iFrame < NB_CAP_FOCUS_BRIGHTNESS + NB_CAP_LEARNING_MODEL_FIRST) {
                    // do nothing
                    std::cout << "l" << std::flush;
#ifdef PC
                    rectangle(inputFrame, cv::Rect(640 - 50, 0, 50, 50), cv::Scalar(255, 0, 0), -1);
#endif

                } else {

                    //                    nMovement = cv::countNonZero(mask);
                    //                    nMovement = isBlack(mask);
                    unsigned char* p = mask.data;
                    int counterNonBlack = 0;
                    for (int i = 0; i < mask.cols * mask.rows; ++i) {
                        if (p[i] != 0) {
                            ++counterNonBlack;
                            if (counterNonBlack > NON_BLACK_IMG_THRESHOLD)
                                break;
                        }
                    }
                    nMovement = counterNonBlack > NON_BLACK_IMG_THRESHOLD;
                    if (!nMovement)
                        std::this_thread::sleep_for(std::chrono::milliseconds(250));
                    std::cout << "w" << std::flush;
                }
            }
#ifdef PC
            imshow("mask3", inputFrame);
            cv::waitKey(1);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
#endif
            ++iFrame;
        } // while nMovement == 0

        // if (lightGpio != -1 && isNight()) {
        if (lightGpio != -1) {
            gpioSetValue(lightGpio, 1);
        }
        std::cout << std::endl;
#endif

#ifdef DETECTION
        if (!recordDetection) {
#endif
            motionPath = getYear() + "/" + getMonth() + "/" + getDay() + "/";
            motionStartTime = getCurTime();
#ifdef DETECTION
        }
#endif
        const std::string motionId = motionStartTime + "_" + deviceId;
        std::cout << HEADER "[CAPTURE] new event : " << motionId << std::endl;
        const std::string newMotionDir = motionRootDir + motionPath + motionId;
        cmd = "mkdir -p " + newMotionDir;
        system(cmd.c_str());

        //        std::string outputVideoFile = newMotionDir + "/video.webm";
        //        cv::VideoWriter outputVideo = cv::VideoWriter(
        //            outputVideoFile, cv::VideoWriter::fourcc('V', 'P', '8', '0'), FPS,
        //            sizeScreen, true);

#ifdef DETECTION
        std::string outputVideoFile = newMotionDir + "/detection.mp4";
        //#else
        //        std::string outputVideoFile = newMotionDir + "/record.mp4";
        cv::VideoWriter outputVideo = cv::VideoWriter(
            //            outputVideoFile, cv::VideoWriter::fourcc('M', 'P', '4', 'V'), FPS,
            outputVideoFile, cv::VideoWriter::fourcc('H', '2', '6', '4'), FPS,
            sizeScreen, true);
        if (!outputVideo.isOpened()) {
            //        if (!outputVideo.isOpened()) {
            std::cout << HEADER "[CAPTURE] failed to open mp4 video" << std::endl;
            return 6;
        }
#endif

        cv::VideoWriter outputVideoRec;
#ifdef DETECTION
        if (!recordDetection) {
#endif
            std::string outputVideoFileRec = newMotionDir + "/record.mp4";
            outputVideoRec = cv::VideoWriter(
                outputVideoFileRec, cv::VideoWriter::fourcc('H', '2', '6', '4'), FPS,
                //                outputVideoFileRec, cv::VideoWriter::fourcc('M', 'J', 'P', 'G'), FPS,
                sizeScreen, true);
            if (!outputVideoRec.isOpened()) {
                std::cout << HEADER "[CAPTURE] failed to open avi video" << std::endl;
                return 7;
            }
#ifdef DETECTION
        }
#endif

        // auto model = createBackgroundSubtractorKNN();
        // auto model = createBackgroundSubtractorGMG();

#ifdef DETECTION
        int iNewObj = 0;
        //        tombs.clear();
        //        objects.clear();
        std::list<DeadObj> tombs;
        assert(tombs.size() == 0);
        std::list<Object> objects;
        assert(objects.size() == 0);
#endif

        iFrame = 0;

        int line;
#ifdef DETECTION
        std::vector<typeof std::chrono::high_resolution_clock::now()> frameStarts;
        frameStarts.reserve(1000);
#else
        const auto videoStart = std::chrono::high_resolution_clock::now();
#endif

        // ----------------------- WHILE HAS MOVEMENT
#ifdef PC
        //        while ((hasMovement() || nMovement > 0) && !quit) {
        //        while (nMovement > 0 || hasMovement() || hasStream) {
        while (nMovement > 0 || hasStream) {
#else
        //        while (nMovement > 0 || hasMovement() || hasStream) {
        while (nMovement > 0 || hasStream) {
#endif

#ifdef DETECTION
            frameStarts.push_back(std::chrono::high_resolution_clock::now());
#endif
            vCap >> inputFrame;
            if (inputFrame.empty()) {
                std::cout << HEADER "[CAPTURE] stream finished" << std::endl;
                streamFinished = true;
                break;
            }
            if (flip180) {
                flip(inputFrame, inputFrame, -1);
            }
#ifdef DETECTION
            if (!recordDetection) {
#endif
                outputVideoRec << inputFrame;
#ifdef DETECTION
            }
#endif

#ifdef DETECTION
            drawing = inputFrame;
#endif

            // ------------------------ START

            if (iFrame < NB_CAP_FOCUS_BRIGHTNESS) {
#ifdef DETECTION
                rectangle(drawing, cv::Rect(640 - 50, 0, 50, 50), cv::Scalar(255, 0, 0),
                    -1);
#endif

            } else {

                cv::Mat gray;
                cv::cvtColor(inputFrame, gray, cv::COLOR_BGR2GRAY);
                //                                equalizeHist(gray, gray);
                cv::GaussianBlur(gray, gray, filterSize, 0);
//                cv::GaussianBlur(gray, gray, cv::Size(15, 15), 0);
#ifdef PC
                imshow("mask", gray);
#endif
                //                model->apply(inputFrame, mask);
                cv::Mat mask;
                model->apply(gray, mask);
//                model->apply(inputFrame, mask);
#ifdef PC
                imshow("mask2", mask);
#endif

                if (iFrame < NB_CAP_FOCUS_BRIGHTNESS + NB_CAP_LEARNING_MODEL_FIRST) {
#ifdef DETECTION
                    rectangle(drawing, cv::Rect(640 - 50, 0, 50, 50), cv::Scalar(0, 255, 0),
                        -1);
#endif

                } else {

#ifndef DETECTION
                    //                    nMovement = cv::countNonZero(mask);
                    //                    nMovement = isBlack(mask);
                    unsigned char* p = mask.data;
                    int counterNonBlack = 0;
                    for (int i = 0; i < mask.cols * mask.rows; ++i) {
                        if (p[i] != 0) {
                            ++counterNonBlack;
                            if (counterNonBlack > 0)
                                break;
                        }
                    }
                    nMovement = counterNonBlack > 0;
#else

                    // ------------------- BOUNDING MOVMENT ---------------------------
                    // threshold(mask, mask, 127, 255, THRESH_BINARY);
                    //                    medianBlur(mask, mask, 15);

                    auto kernel = getStructuringElement(cv::MORPH_ELLIPSE, cv::Size(5, 5));
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

                    threshold(mask, mask, 25, 255, cv::THRESH_BINARY);
                    dilate(mask, mask, kernel, cv::Point(-1, -1), 1);
#ifdef PC
                    imshow("mask3", mask);
                    // imshow("mask2", drawing);
#endif
                    //                                        cv::Mat maskContour;
                    //                    cv::dilate(mask, maskContour, kernel, cv::Point(-1, -1), 5);
                    cv::dilate(mask, mask, kernel, cv::Point(-1, -1), 5);

#ifdef PC
                    //                                        imshow("mask4", maskContour);
                    imshow("mask4", mask);
                    // imshow("mask2", drawing);
#endif

                    // ------------------- BOUNDING BOXING MOVEMENTS
                    std::vector<std::vector<cv::Point>> movContours;
                    std::vector<cv::Vec4i> hierarchy;
                    findContours(mask, movContours, hierarchy, cv::RETR_TREE,
                        //                    findContours(maskContour, movContours, hierarchy, cv::RETR_TREE,
                        cv::CHAIN_APPROX_SIMPLE, cv::Point(0, 0));

#ifdef PC
                    cv::Mat mat = inputFrame.clone();
                    for (int i = 0; i < movContours.size(); ++i) {
                        drawContours(mat, movContours, i, cv::Scalar(0, 255, 0), 2);
                        // imshow("mask2", drawing);
                    }
                    imshow("mask5", mat);
#endif

                    //#ifndef DETECTION
                    //                    for (int i = 0; i < movContours.size(); ++i) {
                    //                        drawContours(drawing, movContours, i, cv::Scalar(0, 255, 0), 2);
                    //                    }
                    //#endif

                    nMovement = movContours.size();
                    // to many movements -> no detection
                    if (nMovement > MAX_MOVEMENTS) {
                        rectangle(drawing, cv::Rect(640 - 50, 0, 50, 50), cv::Scalar(0, 0, 255),
                            -1);

                    }
                    //#ifdef DETECTION
                    else {

                        std::set<Movement> movements;
                        {
                            std::vector<std::vector<cv::Point>> contours_poly(nMovement);
                            std::vector<cv::Rect> movBoundRect(nMovement);
                            for (int i = 0; i < nMovement; ++i) {
                                approxPolyDP(cv::Mat(movContours[i]), contours_poly[i], 3, true);
                                movBoundRect[i] = boundingRect(cv::Mat(contours_poly[i]));
                            }

                            std::vector<cv::Moments> movMoments(nMovement);
                            for (int i = 0; i < nMovement; ++i) {
                                movMoments[i] = moments(movContours[i], true);
                            }

                            // drawing = inputFrame.clone();
                            std::vector<cv::Point2f> movCenters(nMovement);
                            for (int i = 0; i < nMovement; ++i) {
                                movCenters[i] = cv::Point2f(movMoments[i].m10 / movMoments[i].m00, movMoments[i].m01 / movMoments[i].m00);
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
                                    putText(drawing, "s", obj.pos + cv::Point2i(-9, 9),
                                        cv::FONT_HERSHEY_DUPLEX, 1, obj.color, 1);
                                    // ++it;
                                    //                                    newObjects.insert(std::move(obj));
                                }

                            }
                            // object moved
                            else {
                                //                                movsFound[iMovNearest] = true;
                                nearestMov->found = true;

                                cv::Point2i tl = nearestMov->boundRect.tl();
                                cv::Point2i tr = tl + cv::Point2i(nearestMov->boundRect.width + 5, 10);

                                // std::vector<std::vector<Point>> movContours
                                // {movCountours[iMovNearest]};
                                // std::cout << "rect : " << rect << std::endl;
                                //                                int density = nearestMov->density;
                                //                                assert(!inputFrame.empty());
                                // NColors colors = Capture::getPrimaryColors(img, m);

                                // if found best trace
                                //                                if (movMoments[iMovNearest].m00 > obj.trace[obj.bestCapture].m_density) {
                                if (nearestMov->density > obj.biggestCapture.m_density) {
                                    const cv::Rect& rect { nearestMov->boundRect };
                                    assert(!rect.empty());
                                    cv::Mat img = cv::Mat(inputFrame, rect).clone();
                                    cv::Mat m = cv::Mat(mask, rect).clone();
                                    obj.biggestCapture = { std::move(img), std::move(m), nearestMov->contours,
                                        rect, nearestMov->boundRect.x, nearestMov->boundRect.y, rect.width,
                                        rect.height, nearestMov->density, -1, 0.0 };
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
                                //                            cv::FONT_HERSHEY_DUPLEX, 0.5, obj.color, 1);
                                //                    } else {
                                putText(drawing, std::to_string(obj.id),
                                    tr + cv::Point(0, 0), cv::FONT_HERSHEY_DUPLEX, 0.5,
                                    obj.color, 1);
                                //                    }
                                putText(drawing,
                                    std::to_string(static_cast<int>(obj.distance)),
                                    tr + cv::Point(0, 20), cv::FONT_HERSHEY_DUPLEX, 0.5,
                                    obj.color, 1);
                                putText(drawing, std::to_string(obj.density),
                                    tr + cv::Point(0, 40), cv::FONT_HERSHEY_DUPLEX, 0.5,
                                    obj.color, 1);

                                cv::line(drawing, obj.pos, obj.pos + obj.speedVector,
                                    cv::Scalar(0, 0, 255), 1, cv::LineTypes::LINE_AA);

                                //                                obj.trace.emplace_back(std::move(cap));
                                // obj.trace.emplace_back(cap);
                                //                                newObjects.insert(std::move(obj));

                                //                                objects.emplace_front(obj);
                                //                                if (obj.age > 10) {
                                //                                    const cv::Rect& rect { nearestMov->boundRect };
                                //                                    cv::Mat img = cv::Mat(inputFrame, rect).clone();
                                //                                    //                                    cv::Mat img = cv;
                                //                                    cv::Mat m_mask = cv::Mat(mask, rect).clone();

                                //                                    //                assert(rect.size() == m_mask.size());
                                //                                    //                img.copyTo(cv::Mat(inputFrame, rect),
                                //                                    //                    mask);

                                //                                    //                                    inputFrame.copyTo(cv::Mat(img, rect),
                                //                                    //                                        cv::Mat(mask, rect));
                                //                                    Capture capture = { std::move(img), std::move(m_mask), nearestMov->contours,
                                //                                        rect, nearestMov->boundRect.x, nearestMov->boundRect.y, rect.width,
                                //                                        rect.height, nearestMov->density, -1, 0.0 };

                                //                                    detect(net, capture);
                                //                                    if (capture.confidence > 0.5) {
                                //                                        const auto& rect = capture.m_rect;
                                //                                        drawPred(capture.label, capture.confidence, rect.tl().x, rect.tl().y, rect.br().x, rect.br().y, drawing, obj.color);
                                //                                    }
                                //                                }

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
                                cv::Scalar color = cv::Scalar(rng.uniform(0, 255), rng.uniform(0, 255),
                                    rng.uniform(0, 255));

                                const cv::Rect& rect { mov.boundRect };
                                // std::cout << "rect : " << rect << std::endl;
                                assert(!inputFrame.empty());
                                assert(!rect.empty());
                                const cv::Mat& img = cv::Mat(inputFrame, rect).clone();
                                assert(!img.empty());
                                const cv::Mat& m = cv::Mat(mask, rect).clone();
                                assert(!m.empty());
                                // NColors colors = Capture::getPrimaryColors(img, m);
                                Capture cap({ std::move(img), std::move(m), mov.contours, rect,
                                    mov.center.x, mov.center.y, rect.width, rect.height,
                                    mov.density, -1, 0.0 });

                                const std::vector<std::vector<cv::Point>>& contour = { mov.contours };
                                drawContours(drawing, contour, 0, color, 2);

                                objects.emplace_back(Object { 0.0, mov.center, mov.density, cv::Point2i(0, 0),
                                    color, iNewObj++, std::move(cap),
                                    mov.center, 0u, {} });

                                //                                objects.insert(std::move(obj));
                                //                                objects.emplace_back(std::move(obj));
                            }
                        }
                        //            nbObjects = objects.size();

                        for (const auto& obj : objects) {
                            for (const auto& l : obj.lines) {
                                cv::line(drawing, l.first, l.second, obj.color, 2);
                            }
                        }

                        for (const DeadObj& obj : tombs) {
                            putText(drawing, "x", obj.p + cv::Point(-9, 9), cv::FONT_HERSHEY_DUPLEX,
                                1, obj.color, 1);
                        }

                    } // if (nMovement >= MAX_MOVEMENTS)
#endif

                } // if (iFrame < NB_CAP_FOCUS_BRIGHTNESS + NB_CAP_LEARNING_MODEL_FIRST)

            } // if (iFrame < NB_CAP_FOCUS_BRIGHTNESS)

#ifdef DETECTION
            line = 30;
            putText(drawing, deviceId,
                cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 5);
            putText(drawing, deviceId,
                cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1,
                cv::Scalar(255, 255, 255));
            line += 30;

            putText(drawing, motionStartTime,
                cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 5);
            putText(drawing, motionStartTime,
                cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1,
                cv::Scalar(255, 255, 255));
            line += 30;

            putText(drawing, "nbObjs : " + std::to_string(objects.size()),
                cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 5);
            putText(drawing, "nbObjs : " + std::to_string(objects.size()),
                cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1,
                cv::Scalar(255, 255, 255));
            line += 30;

            putText(drawing, "frame : " + std::to_string(iFrame), cv::Point(0, line),
                cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 5);
            putText(drawing, "frame : " + std::to_string(iFrame), cv::Point(0, line),
                cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 255, 255), 1);
            line += 30;
#endif

            // std::cout << "frame : " << iFrame << "\r" << std::flush;
            //            std::cout << colorHash(std::this_thread::get_id()) << "+" << std::flush << "\033[0m";
//#ifdef DEBUG
#ifdef DETECTION
            std::cout << "+" << std::flush;
#endif
            //#endif

#ifdef DETECTION
            const int iLastFrame = std::max(iFrame - 30, 0);
            const double framesDuration = std::chrono::duration_cast<std::chrono::milliseconds>(
                                              std::chrono::high_resolution_clock::now() - frameStarts[iLastFrame])
                                              .count()
                / 1000.0;
            //            frameStart = std::chrono::high_resolution_clock::now();

            //            std::ostringstream fps;
            //            fps << std::fixed << std::setprecision(2) << 1.0 / frameDuration;
            //            fps << 1.0 / frameDuration;
            //            double fps = 1.0 / frameDuration;
            const double fps = (iFrame - iLastFrame + 1.0) / framesDuration;
            char fpsBuf[10];
            sprintf(fpsBuf, "%.2f", fps);
            std::string fpsStr(fpsBuf);
            //            lastFrameDuration = frameDuration;
            putText(drawing, "fps : " + fpsStr, cv::Point(0, line),
                cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 5);
            putText(drawing, "fps : " + fpsStr, cv::Point(0, line),
                cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 255, 255), 1);
            line += 30;

            outputVideo << drawing;
#endif

#ifdef PC
#ifdef DETECTION
            cv::imshow("drawing", drawing);
#endif
            if (hasStream && !quit) {
                while (true) {
                    auto key = cv::waitKey(10);
                    if (key == 'q') {
                        quit = true;
                        break;
                    } else if (key == ' ')
                        break;
                }
            } else {
                if (cv::waitKey(1) == 'q') {
                    quit = true;
                    //                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
#endif
            ++iFrame;
            //            std::cout << "this thread : " << std::this_thread::get_id() << std::endl;

        } // while (hasMovement() || nMovement > 0 || hasStream)
#ifndef DETECTION
        for (int i = 0; i < iFrame; ++i) {
            std::cout << "+";
        }
#endif
        std::cout << std::endl;

        vCap.release();
#ifndef DETECTION
        cv::Mat drawing = inputFrame;
        line = 30;
        putText(drawing, deviceId,
            cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 5);
        putText(drawing, deviceId,
            cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1,
            cv::Scalar(255, 255, 255));
        line += 30;

        putText(drawing, motionStartTime,
            cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 5);
        putText(drawing, motionStartTime,
            cv::Point(0, line), cv::FONT_HERSHEY_DUPLEX, 1,
            cv::Scalar(255, 255, 255));
        line += 30;

        putText(drawing, "frame : " + std::to_string(iFrame), cv::Point(0, line),
            cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 5);
        putText(drawing, "frame : " + std::to_string(iFrame), cv::Point(0, line),
            cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 255, 255), 1);
        line += 30;
#endif
        //        auto end = std::chrono::high_resolution_clock::now();
#ifdef DETECTION
        auto videoDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - frameStarts[0]).count() / 1000.0;
#else
        auto videoDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - videoStart).count() / 1000.0;
#endif
        const double videoFps = static_cast<double>(iFrame) / videoDuration;
        //        putText(drawing, "fps : " + std::to_string(fps), cv::Point(0, 90),
        //            cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 5);
        //        putText(drawing, "fps : " + std::to_string(fps), cv::Point(0, 90),
        //            cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 255, 255), 1);
        char videoFpsBuf[10];
        sprintf(videoFpsBuf, "%.2f", videoFps);
        std::string videoFpsStr(videoFpsBuf);
        putText(drawing, "video fps : " + videoFpsStr, cv::Point(0, line),
            cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 5);
        putText(drawing, "video fps : " + videoFpsStr, cv::Point(0, line),
            cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 255, 255), 1);
        line += 30;

        if (lightGpio != -1) {
            gpioSetValue(lightGpio, 0);
        }
#ifdef DETECTION
        if (!recordDetection) {
#endif
            outputVideoRec.release();
#ifdef DETECTION
        }
#endif

        //        auto it = threads.begin();
        //        while (it != threads.end()) {
        //            //        for (auto & thread : threads) {
        //            auto& thread = *it;

        //            if (thread.joinable()) {
        //                thread.join();
        //                it = threads.erase(it);
        //                continue;
        //            }
        //            ++it;
        //        }

        //        std::thread t([iFrame, newMotionDir, hasScript, script, motionId, hasRemoteDir, port, motionRootDir, remoteDir, &objects, net, &drawing, &outputVideo]() mutable {

        //        std::thread t([=, &threads, objects = std::move(objects), drawing = drawing.clone(), outputVideo = std::move(outputVideo)]() mutable {
        //            std::cout << HEADER "[CAPTURE] start new thread, nb object to detect = " << objects.size() << std::endl;

        //        std::thread t([=, objects = std::move(objects), &net, drawing = std::move(drawing)]() mutable {

        //            std::cout << HEADER "[CAPTURE] nb thread = " << threads.size() << std::endl;
        //            std::this_thread::sleep_for(std::chrono::seconds(120 * threads.size()));

#ifdef DETECTION
        auto detectStart = std::chrono::high_resolution_clock::now();
        int nbRealObjects = 0;
        int nbHuman = 0;
        // draw all detected object movements in drawing capture
        for (Object& obj : objects) {

            Capture& bestCapture = obj.biggestCapture;
            // const cv::Mat &img = bestCapture.m_img;

            assert(!bestCapture.m_rect.empty());
            assert(!bestCapture.m_mask.empty());
            assert(bestCapture.m_rect.size() == bestCapture.m_mask.size());

            bestCapture.m_img.copyTo(cv::Mat(drawing, bestCapture.m_rect),
                bestCapture.m_mask);

            std::vector<std::vector<cv::Point>> movCountours { bestCapture.m_contour };
            drawContours(drawing, movCountours, 0, obj.color, 2);

            // if (obj.distance > MIN_MOV_DIST_TO_SAVE_OBJECT) {
            if (obj.age > MIN_MOV_YEARS_TO_SAVE_OBJECT) {
                ++nbRealObjects;

                // assert(obj.trace[obj.bestCapture] != nullptr);
                //                const Capture& bestCapture = obj.trace[obj.bestCapture];

                detect(net, bestCapture);
                if (bestCapture.confidence > 0.2) {

                    //                                        cv::Mat m;
                    //                                        cv::cvtColor(bestCapture.m_mask, m, cv::COLOR_GRAY2BGR);
                    //                                        m.copyTo(cv::Mat(drawing, bestCapture.m_rect));

                    //                                        std::vector<std::vector<cv::Point>> movCountours { bestCapture.m_contour };
                    //                                        drawContours(drawing, movCountours, 0, obj.color, 2);

                    drawPred(bestCapture, drawing, obj.color);
                    if (classes[bestCapture.label] == "person") {
                        ++nbHuman;
                    }
                } else {
                }
            }
        }

        auto detectDuration = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - detectStart)
                                  .count()
            / 1000.0;

        char detectDurationBuf[10];
        sprintf(detectDurationBuf, "%.2f", detectDuration);
        std::string detectDurationStr(detectDurationBuf);
        putText(drawing, "detection : " + detectDurationStr, cv::Point(0, line),
            cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(0, 0, 0), 5);
        putText(drawing, "detection : " + detectDurationStr, cv::Point(0, line),
            cv::FONT_HERSHEY_DUPLEX, 1, cv::Scalar(255, 255, 255), 1);
        line += 30;

        std::cout << HEADER "[CAPTURE] object detected : " << nbRealObjects << std::endl;
        std::cout << HEADER "[CAPTURE] human detected : " << nbHuman << std::endl;
        std::cout << HEADER "[CAPTURE] detect duration : " << detectDuration << std::endl;

        if (nbRealObjects > 0) {
            cmd = "touch " + newMotionDir + "/objectDetected.var";
            std::cout << HEADER << "[CAPTURE] " << cmd << std::endl;
            system(cmd.c_str());
        }

#endif

        if (iFrame != 0) {
#ifdef DETECTION
            outputVideo << drawing;
            imwrite(newMotionDir + "/trace.jpg", drawing);
#else
            imwrite(newMotionDir + "/record.jpg", drawing);
#endif
        }
#ifdef DETECTION
        outputVideo.release();
#endif

#ifdef DETECTION
        if (nbHuman > 0) {
            cmd = "touch " + newMotionDir + "/humanDetected.var";
            std::cout << HEADER << "[CAPTURE] " << cmd << std::endl;
            system(cmd.c_str());
            if (hasScript) {
                //                imwrite("alert.jpg", drawing);

                //                cmd = "./" + script + " " + bestPath + " &";
                //                    cmd = script + " " + motionId + " &";
                cmd = script + " " + motionPath + motionId;
                system(cmd.c_str());

                std::cout << HEADER "[SCRIPT] run : " << cmd
                          << std::endl;
            }
        }
#endif

        // std::cout << "save video '" << outputVideoFile << "'" << std::endl;

        // std::cout << "save trace file '" << newMotionDir + "/trace.jpg'"
        //   << std::endl;

        //        thread = std::thread(foo);
        std::string cmd;
        if (iFrame > NB_CAP_FOCUS_BRIGHTNESS) {
            cmd = "touch " + newMotionDir + "/focusBrightnessDone.var";
            std::cout << HEADER << "[CAPTURE] " << cmd << std::endl;
            system(cmd.c_str());
        }

        if (iFrame > NB_CAP_FOCUS_BRIGHTNESS + NB_CAP_LEARNING_MODEL_FIRST) {
            cmd = "touch " + newMotionDir + "/learningModelDone.var";
            std::cout << HEADER << "[CAPTURE] " << cmd << std::endl;
            system(cmd.c_str());
        }

        if (hasRemoteDir) {
            if (port == -1) {
                cmd = "rsync -arv " + motionRootDir + " " + remoteDir;
            } else {
                cmd = "rsync -arv -e 'ssh -p " + std::to_string(port) + "' " + motionRootDir + " " + remoteDir;
            }
            //            std::thread t([cmd]() {
            std::cout << HEADER << "[CAPTURE] rsync start '" << cmd << "'" << std::endl;
            //#ifdef PC
            system((cmd).c_str());
            //#else
            //                system((cmd + " &").c_str());
            //#endif
            std::cout << HEADER << "[CAPTURE] rsync end " << cmd << std::endl;
            //            });
        }
        //            return 0;

        //        });
        //        t.detach();
        //        threads.push_back(std::move(t));

        //        threads.emplace_back(std::move(t));
        // }

#ifdef PC
        cv::destroyAllWindows();
#endif

        std::cout << HEADER "[CAPTURE] video duration : " << videoDuration << std::endl;
        std::cout << HEADER "[CAPTURE] nb capture : " << iFrame << std::endl;
        std::cout << HEADER "[CAPTURE] recording fps : " << videoFps
                  << std::endl;

#ifdef PC
        if (quit || streamFinished) {
            //            std::this_thread::sleep_for(std::chrono::seconds(5)); // wait rsync
            //            return 0;
            break;
        }
#else
        if (streamFinished) {
            //            std::this_thread::sleep_for(std::chrono::seconds(5)); // wait rsync
            //            return 0;
            break;
        }
#endif

    } // while (1)

    //        thread.join();
    //    for (auto& thread : threads) {
    //        if (thread.joinable())
    //            thread.join();
    //    }

    return 0;

} // end main

#ifdef DETECTION
// Draw the predicted bounding box
void drawPred(const Capture& capture, cv::Mat& frame, const cv::Scalar& color)
{
    const auto& rect = capture.m_rect;
    int left = rect.tl().x;
    int top = rect.tl().y;
    int right = rect.br().x;
    int bottom = rect.br().y;
    // Draw a rectangle displaying the bounding box
    cv::rectangle(frame, cv::Point(left, top + 3), cv::Point(right, bottom), color, 3);

    // Get the label for the class name and its confidence
    std::string label = cv::format("%i%%", int(capture.confidence * 100));
    if (!classes.empty()) {
        CV_Assert(capture.label < (int)classes.size());
        label = classes[capture.label] + ":" + label;
    }

    // Display the label at the top of the bounding box
    int baseLine;
    cv::Size labelSize = getTextSize(label, cv::FONT_HERSHEY_SIMPLEX, 0.5, 1, &baseLine);
    top = cv::max(top, labelSize.height);
    rectangle(frame, cv::Point(left - 2, top - round(1.5 * labelSize.height) + 2), cv::Point(left + round(labelSize.width) + 2, top + baseLine), color, cv::FILLED);
    //    putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0), 3.0);
    putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255), 1.0);
    putText(frame, cv::format("%.2fs", capture.detectDuration), cv::Point(left, bottom + 15), cv::FONT_HERSHEY_SIMPLEX, 0.5, color, 1.0);

    //    putText(frame, label, cv::Point(left, top), cv::FONT_HERSHEY_SIMPLEX, 0.75, cv::Scalar(0, 0, 0), 1.0);
    //    putText(frame, std::to_string(capture.detectDuration),
    //        capture.m_rect.tl() + cv::Point(0, 0), cv::FONT_HERSHEY_DUPLEX, 0.5,
    //        color, 1);
}

// Get the names of the output layers
std::vector<cv::String> getOutputsNames(const cv::dnn::Net& net)
{
    static std::vector<cv::String> names;
    if (names.empty()) {
        // Get the indices of the output layers, i.e. the layers with unconnected outputs
        std::vector<int> outLayers = net.getUnconnectedOutLayers();

        // get the names of all the layers in the network
        std::vector<cv::String> layersNames = net.getLayerNames();

        // Get the names of the output layers in names
        names.resize(outLayers.size());
        for (size_t i = 0; i < outLayers.size(); ++i)
            names[i] = layersNames[outLayers[i] - 1];
    }
    return names;
}

void detect(cv::dnn::Net& net, Capture& capture)
{
    const auto detectStart = std::chrono::high_resolution_clock::now();
    // Create a 4D blob from a frame.
    //                                    cv::Mat frame = cv::Mat(inputFrame, nearestMov->boundRect).clone();
    cv::Mat frame = capture.m_img;
    //                                    cv::Mat frame = cv::Mat(inputFrame).clone();
    cv::Mat blob;
    cv::dnn::blobFromImage(frame, blob, 1 / 255.0, cv::Size(inpWidth, inpHeight), cv::Scalar(0, 0, 0), true, false);

    // Sets the input to the network
    net.setInput(blob);

    // Runs the forward pass to get output of the output layers
    std::vector<cv::Mat> outs;
    net.forward(outs, getOutputsNames(net));

    // Remove the bounding boxes with low confidence
    //                postprocess(frame, outs, drawing);

    //                std::vector<int> classIds;
    //                std::vector<float> confidences;
    //                std::vector<cv::Rect> boxes;
    int iMax = -1;
    double confidenceMax = 0.0;

    for (size_t i = 0; i < outs.size(); ++i) {
        // Scan through all the bounding boxes output from the network and keep only the
        // ones with high confidence scores. Assign the box's class label as the class
        // with the highest score for the box.
        float* data = (float*)outs[i].data;
        for (int j = 0; j < outs[i].rows; ++j, data += outs[i].cols) {
            cv::Mat scores = outs[i].row(j).colRange(5, outs[i].cols);
            cv::Point classIdPoint;
            double confidence;
            // Get the value and location of the maximum score
            minMaxLoc(scores, 0, &confidence, 0, &classIdPoint);
            if (confidence > confThreshold && confidence > confidenceMax) {
                //                            int centerX = (int)(data[0] * frame.cols);
                //                            int centerY = (int)(data[1] * frame.rows);
                //                            int width = (int)(data[2] * frame.cols);
                //                            int height = (int)(data[3] * frame.rows);
                //                            int left = centerX - width / 2;
                //                            int top = centerY - height / 2;
                confidenceMax = confidence;
                iMax = classIdPoint.x;
                //                            classIds.push_back(classIdPoint.x);
                //                            confidences.push_back((float)confidence);
                //                            boxes.push_back(cv::Rect(left, top, width, height));
            }
        }
    }

    //    capture.name = classes[iMax];
    capture.label = iMax;
    capture.confidence = confidenceMax;

    const auto detectEnd = std::chrono::high_resolution_clock::now();
    double detectDuration = std::chrono::duration_cast<std::chrono::milliseconds>(detectEnd - detectStart).count() / 1000.0f;
    capture.detectDuration = detectDuration;
}
#endif

void openCamera(cv::VideoCapture& vCap, const std::string& stream, cv::Mat& inputFrame)
{
    vCap.open(stream);
    vCap.set(cv::CAP_PROP_FRAME_WIDTH, WIDTH);
    vCap.set(cv::CAP_PROP_FRAME_HEIGHT, HEIGHT);
    vCap.set(cv::CAP_PROP_FPS, 30);
    // vCap.open(CAP_V4L2);
    if (!vCap.isOpened()) {
        std::cout << HEADER "[OPEN CAMERA] device '" << stream << "' not found" << std::endl;
        //            return 1;
        return;
    }

    //    cv::Mat inputFrame;
    vCap >> inputFrame;
    //                assert(!inputFrame.empty());
    int cpt = 0;
    while (inputFrame.empty() && cpt < 10) {
        vCap.release();
        std::cout << HEADER "[OPEN CAMERA] input frame " << cpt << " is empty" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100)); // wait rsync

        vCap.open(stream);
        if (!vCap.isOpened()) {
            std::cout << HEADER "[OPEN CAMERA] device '" << stream << "' not found" << std::endl;
            return;
        }
        vCap >> inputFrame;
        ++cpt;
    }
    if (inputFrame.empty()) {
        std::cout << HEADER "[OPEN CAMERA] all input frames are empty" << std::endl;
        vCap.release();
        return;
    }
}
