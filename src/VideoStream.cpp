#include "VideoStream.h"
#include <opencv2/opencv.hpp>
#include <iostream>

VideoStream::VideoStream(){
    this->StartCapture();
}

void VideoStream::StartCapture() {
    std::cerr << "VideoStream Instantiated" << std::endl;
    cv::VideoCapture cap(0); // default camera
    if (!cap.isOpened()) {
        std::cerr << "ERROR: Could not open camera." << std::endl;
        return;
    }

    std::cout << "Camera opened: "
              << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x"
              << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;

    cv::Mat frame;
    cap >> frame;

    if (frame.empty()) {
        std::cerr << "ERROR: Captured frame is empty." << std::endl;
        return;
    }

    std::cout << "Captured frame: " << frame.cols << "x" << frame.rows
              << ", channels=" << frame.channels() << std::endl;

    cv::imshow("OpenCV Test - press any key to close", frame);
    cv::waitKey(0);
}