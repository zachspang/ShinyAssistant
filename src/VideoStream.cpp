#include "VideoStream.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>

VideoStream::VideoStream(){
    std::cout << "VideoStream Instantiated" << std::endl;
}

void VideoStream::StartCapture() {
    std::cout << "Starting capture" << std::endl;

    std::thread capThread([this]() {
        cv::VideoCapture cap(0);
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 1920);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1080);

        if (!cap.isOpened()) {
            std::cerr << "ERROR: Could not open camera." << std::endl;
            return;
        }

        // Let the camera warmup before trusting any frames
        cv::Mat warmup;
        for (int i = 0; i < 10; ++i) {
            cap.read(warmup);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }

        std::cout << "Camera opened: " << cap.get(cv::CAP_PROP_FRAME_WIDTH) << "x" << cap.get(cv::CAP_PROP_FRAME_HEIGHT) << std::endl;
        
        //while (true) {
        for (int i=0; i < 1200; i++){
            cv::Mat tempFrame;
            bool readOk = cap.read(tempFrame);
            if (!readOk || tempFrame.empty()) continue;

            {
                std::lock_guard<std::mutex> lock(m_frameMutex);
                m_frame = tempFrame;
            }

            //cv::imshow("OpenCV debug view", m_frame);
            //cv::waitKey(1);
        }
    });

    capThread.detach();
}

wxImage VideoStream::GetWxImageFromFrame(){
    cv::Mat frameCopy;
    {
        std::unique_lock<std::mutex> lock(m_frameMutex, std::try_to_lock);
        if (!lock.owns_lock() || m_frame.empty()) {
            return wxImage();
        }
        m_frame.copyTo(frameCopy);
    }
    
    cv::Mat rgbFrame;
    cv::cvtColor(frameCopy, rgbFrame, cv::COLOR_BGR2RGB);
    unsigned char* wxData = (unsigned char*)malloc(rgbFrame.rows * rgbFrame.cols * 3);
    if (!wxData) {
        return wxImage();
    }

    std::memcpy(wxData, rgbFrame.data, rgbFrame.rows * rgbFrame.cols * 3);

    return wxImage(rgbFrame.cols, rgbFrame.rows, wxData, false);
};