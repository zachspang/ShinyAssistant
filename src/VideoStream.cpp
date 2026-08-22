#include "VideoStream.h"
#include "Logging.h"
#include <opencv2/opencv.hpp>
#include <iostream>
#include <thread>
#include <wx/string.h>
#include <windows.h>

VideoStream::VideoStream(){
    Log("VideoStream Instantiated");
}

VideoStream::~VideoStream(){
    StopCapture();
}

int VideoStream::GetWebcamCount() {
    /* Opening captures while debugging with GDB takes a really long time
       and I dont really care enough to troubleshoot it more than I 
       already have since it works fine without a debugger attached.
       If a debugger is present counting the webcams is just skipped.
       Defaulted to 2 because assumption is 1 webcam + 1 virtual cam
    */
    if (IsDebuggerPresent()) return 2;

    int maxTested = 10;
    int count = 0;
    for (int i = 0; i < maxTested; i++) {
        cv::VideoCapture cap(i);
        if (cap.isOpened()) {
            count++;
            cap.release();
        }
    }
    Log(wxString::Format("Detected Webcams: %d", count));
    return count;
}

void VideoStream::StartCapture(int deviceIndex) {
    Log(wxString::Format("Starting capture on device %d", deviceIndex));
    m_stopCapture = false;

    m_captureThread = std::thread([this, deviceIndex]() {
        cv::VideoCapture cap(deviceIndex, cv::CAP_ANY);

        if (!cap.isOpened()) {
            Log("ERROR: Could not open camera.");
            std::cerr << "ERROR: Could not open camera." << std::endl;
            return;
        }

        // Let the camera warmup before trusting any frames
        cv::Mat warmup;
        for (int i = 0; i < 10 && !m_stopCapture; i++) {
            cap.read(warmup);
            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }

        Log(wxString::Format("Camera opened %.0fx%.0f", cap.get(cv::CAP_PROP_FRAME_WIDTH), cap.get(cv::CAP_PROP_FRAME_HEIGHT) ));
        auto lastFpsTime = std::chrono::steady_clock::now();
        double currentFps = 0.0;

        while (!m_stopCapture) {
            cv::Mat tempFrame;
            bool readOk = cap.read(tempFrame);
            if (!readOk || tempFrame.empty()) continue;

            // // Measure actual time since the previous accepted frame
            // auto now = std::chrono::steady_clock::now();
            // double deltaSec = std::chrono::duration<double>(now - lastFpsTime).count();
            // lastFpsTime = now;
            // if (deltaSec > 0.0) {
            //     double instantFps = 1.0 / deltaSec;
            //     currentFps = (currentFps == 0.0) ? instantFps : (currentFps * 0.9 + instantFps * 0.1);
            // }
            // cv::putText(tempFrame, cv::format("FPS: %.1f", currentFps), cv::Point(10, 50), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 3);

            //Draw detection rectangle
            cv::Rect rect(m_rectX, m_rectY, m_rectW, m_rectH);
            rect = rect & cv::Rect(0, 0, tempFrame.cols, tempFrame.rows);
            cv::rectangle(tempFrame, rect, cv::Scalar(0,255,0), 4);

            {
                std::lock_guard<std::mutex> lock(m_frameMutex);
                m_frame = tempFrame;
                m_textBoxRect = rect;
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(30));
        }

        cap.release();
        Log(wxString::Format("Capture stopped on device %d", deviceIndex));
    });
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

bool VideoStream::checkShiny(const std::atomic<bool>& keepRunning) {
    cv::Mat croppedFrame;
    {
        std::lock_guard<std::mutex> lock(m_frameMutex);
        if (m_frame.empty() || m_textBoxRect.width <= 0 || m_textBoxRect.height <= 0)
            return false;
        croppedFrame = m_frame(m_textBoxRect).clone();
    }

    if (m_prevDetectionFrame.empty() || m_prevDetectionFrame.size() != croppedFrame.size()) {
        m_prevDetectionFrame = croppedFrame;
        Log("First shiny check assuming this is not shiny, if it is shiny you have 10 seconds to stop the macro");

        //Sleep in small chunks to respond to the thread joining
        for (int i = 0; i < 10000; i += 50) {
            if (!keepRunning) break;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
        return false;
    }

    double similarity = calcSimilarity(m_prevDetectionFrame, croppedFrame);
    Log(wxString::Format("Similarity Score: %.2f", similarity));

    if (m_avgSimilarity - similarity > 0.17){
        Log(wxString::Format("Average Score: %.2f", m_avgSimilarity));
        Log("!!! SHINY DETECTED !!!");
        return true;
    }
    
    m_totalShinyChecks += 1;
    m_avgSimilarity = m_avgSimilarity + ((similarity - m_avgSimilarity) / m_totalShinyChecks);

    m_prevDetectionFrame = croppedFrame;
    //cv::imshow("OpenCV debug view", croppedFrame);
    return false;
}

void VideoStream::resetDetectionFrame() {
    m_prevDetectionFrame.release();
}

void VideoStream::StopCapture() {
    m_stopCapture = true;
    if (m_captureThread.joinable()) m_captureThread.join();
}

void VideoStream::SwitchCamera(int deviceIndex) {
    StopCapture();

    {
        // Clear the old frame so GetWxImageFromFrame() doesn't briefly hand back
        // a stale frame from the previous camera while the new one warms up.
        std::lock_guard<std::mutex> lock(m_frameMutex);
        m_frame = cv::Mat();
    }

    StartCapture(deviceIndex);
}