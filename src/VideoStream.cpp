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
        cv::VideoCapture cap(0, cv::CAP_ANY);
        // cap.set(cv::CAP_PROP_FRAME_WIDTH, 2560);
        // cap.set(cv::CAP_PROP_FRAME_HEIGHT, 1440);

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
        auto lastFpsTime = std::chrono::steady_clock::now();
        double currentFps = 0.0;

        while (true) {
            cv::Mat tempFrame;
            bool readOk = cap.read(tempFrame);
            if (!readOk || tempFrame.empty()) continue;

            // Measure actual time since the previous accepted frame
            auto now = std::chrono::steady_clock::now();
            double deltaSec = std::chrono::duration<double>(now - lastFpsTime).count();
            lastFpsTime = now;
            if (deltaSec > 0.0) {
                double instantFps = 1.0 / deltaSec;
                currentFps = (currentFps == 0.0) ? instantFps : (currentFps * 0.9 + instantFps * 0.1);
            }

            //Draw detection rectangle
            cv::Rect rect(m_rectX, m_rectY, m_rectW, m_rectH);
            rect = rect & cv::Rect(0, 0, tempFrame.cols, tempFrame.rows);
            cv::rectangle(tempFrame, rect, cv::Scalar(0,255,0), 4);

            cv::putText(tempFrame, cv::format("FPS: %.1f", currentFps), cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 3);

            {
                std::lock_guard<std::mutex> lock(m_frameMutex);
                m_frame = tempFrame;
                m_textBoxRect = rect;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(30));

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

// Finds the shift between images, realign them then compare them
double calcSimilarity(const cv::Mat& img1, const cv::Mat& img2) {
    cv::Mat gray1, gray2;
    cv::cvtColor(img1, gray1, cv::COLOR_BGR2GRAY);
    cvtColor(img2, gray2, cv::COLOR_BGR2GRAY);
 
    cv::Mat f1, f2;
    gray1.convertTo(f1, CV_32F);
    gray2.convertTo(f2, CV_32F);
 
    // Hanning window improves phase correlation accuracy
    cv::Mat hann;
    cv::createHanningWindow(hann, f1.size(), CV_32F);
 
    cv::Point2d shift = cv::phaseCorrelate(f1, f2, hann);
 
    cv::Mat translation = (cv::Mat_<double>(2, 3) << 1, 0, shift.x, 0, 1, shift.y);
    cv::Mat aligned;
    cv::warpAffine(gray2, aligned, translation, gray2.size());
 
    cv::Mat result;
    cv::matchTemplate(gray1, aligned, result, cv::TM_CCOEFF_NORMED);
    double minVal, maxVal;
    cv::minMaxLoc(result, &minVal, &maxVal);
 
    return std::max(0.0, std::min(1.0, (maxVal + 1.0) / 2.0)); // TM_CCOEFF_NORMED range is [-1,1]
}

bool VideoStream::checkShiny() {
    cv::Mat croppedFrame;
    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        if (m_frame.empty() || m_textBoxRect.width <= 0 || m_textBoxRect.height <= 0)
            return false;
        croppedFrame = m_frame(m_textBoxRect).clone();
    }

    if (m_prevDetectionFrame.empty() || m_prevDetectionFrame.size() != croppedFrame.size()) {
        m_prevDetectionFrame = croppedFrame;
        std::cout << "First shiny check" << std::endl;
        return false;
    }

    double similarity = calcSimilarity(m_prevDetectionFrame, croppedFrame);
    std::cout << "Similarity Score: " << similarity << std::endl;

    if (m_avgSimilarity - similarity > 0.25){
        std::cout << "Average Score: " << m_avgSimilarity << std::endl;
        std::cout << "!!! SHINY DETECTED !!!" << std::endl;
        return true;
    }
    
    m_totalShinyChecks += 1;
    m_avgSimilarity = m_avgSimilarity + ((similarity - m_avgSimilarity) / m_totalShinyChecks);

    m_prevDetectionFrame = croppedFrame;
    //cv::imshow("OpenCV debug view", croppedFrame);
    return false;
}

