#include "VideoStream.h"
#include <opencv2/opencv.hpp>
#include <iostream>

VideoStream::VideoStream(){
    this->StartCapture();
}

void VideoStream::StartCapture() {
    std::cout << "VideoStream Instantiated" << std::endl;
    cv::VideoCapture cap(0, cv::CAP_DSHOW); // default camera
    if (!cap.isOpened()) {
        std::cerr << "ERROR: Could not open camera." << std::endl;
        return;
    }

    std::cout << "Camera opened: "
              << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x"
              << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    cv::Mat frame;
    while (true) {
        cap >> frame;

        if (frame.empty()) {
            std::cerr << "ERROR: Captured frame is empty." << std::endl;
            break;
        }

        cv::imshow("OpenCV Test - press 'q' to quit", frame);

        if (cv::waitKey(1) == 'q') {
            break;
        }
    }
}