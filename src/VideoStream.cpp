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
        cap.set(cv::CAP_PROP_FRAME_WIDTH, 854);
        cap.set(cv::CAP_PROP_FRAME_HEIGHT, 480);

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

            // Comparing frame to the previous frame to see if a big change happened
            if (m_checkingChange) {
                cv::Mat prevFrameCopy;
                {
                    std::lock_guard<std::mutex> lock(m_frameMutex);
                    prevFrameCopy = m_prevFrame;
                }

                if (prevFrameCopy.empty()) {
                    // First frame since checking started, just set m_prevFrame
                    std::lock_guard<std::mutex> lock(m_frameMutex);
                    m_prevFrame = tempFrame.clone();
                } else {
                    double similarity = CalcChange(prevFrameCopy, tempFrame);
                    //Log(wxString::Format("Sim: %f", similarity));
                    if (similarity < 0.85) {
                        m_changeDetected = true;
                    }
                    std::lock_guard<std::mutex> lock(m_frameMutex);
                    m_prevFrame = tempFrame.clone();
                }
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
double VideoStream::CalcSimilarity(const cv::Mat& img1, const cv::Mat& img2) {
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

    // Warp the color image instead of gray so we dont lose color info before comparing
    cv::Mat alignedColor;
    cv::warpAffine(img2, alignedColor, translation, img2.size());

    // Compare each color channel separately
    std::vector<cv::Mat> ch1, ch2;
    cv::split(img1, ch1);
    cv::split(alignedColor, ch2);

    double structScore = 0.0;
    for (int i = 0; i < 3; i++) {
        cv::Mat f1c, f2c, result;
        ch1[i].convertTo(f1c, CV_32F);
        ch2[i].convertTo(f2c, CV_32F);
        cv::matchTemplate(f1c, f2c, result, cv::TM_CCOEFF_NORMED);
        double minVal, maxVal;
        cv::minMaxLoc(result, &minVal, &maxVal);
        structScore += std::max(0.0, std::min(1.0, (maxVal + 1.0) / 2.0)); // TM_CCOEFF_NORMED range is [-1,1]
    }
    structScore /= 3.0;

    // Direct pixel-wise color diff penalty
    cv::Mat diff;
    cv::absdiff(img1, alignedColor, diff);
    cv::Scalar meanDiff = cv::mean(diff); // per-channel mean abs diff, 0-255
    double avgColorDiff = (meanDiff[0] + meanDiff[1] + meanDiff[2]) / 3.0;
    double colorScore = 1.0 - (avgColorDiff / 255.0);

    // Take the min so a big color change can't be masked by good structural alignment elsewhere in the image
    double finalScore = std::min(structScore, colorScore);

    return std::max(0.0, std::min(1.0, finalScore));
}

// A lightweight change detector compared to CalcSimilarity
double VideoStream::CalcChange(const cv::Mat& img1, const cv::Mat& img2) {
    // Smaller working resoulution
    constexpr int kWorkSize = 160;

    cv::Mat g1, g2;
    cv::cvtColor(img1, g1, cv::COLOR_BGR2GRAY);
    cv::cvtColor(img2, g2, cv::COLOR_BGR2GRAY);
    cv::resize(g1, g1, cv::Size(kWorkSize, kWorkSize), 0, 0, cv::INTER_AREA);
    cv::resize(g2, g2, cv::Size(kWorkSize, kWorkSize), 0, 0, cv::INTER_AREA);

    // Color/brightness diff
    cv::Mat diff;
    cv::absdiff(g1, g2, diff);
    double meanDiff = cv::mean(diff)[0];
    double colorScore = 1.0 - (meanDiff / 255.0);

    // Texture/variance collapse: a flash (bright or dark from encounter starting) flattens the frame
    cv::Scalar mean1, stddev1, mean2, stddev2;
    cv::meanStdDev(g1, mean1, stddev1);
    cv::meanStdDev(g2, mean2, stddev2);
    double std1 = stddev1[0], std2 = stddev2[0];

    // Ratio of the smaller std to the larger; near 1.0 = similar texture level,
    // near 0.0 = one frame lost (or gained) most of its texture
    double textureScore = (std::max(std1, std2) < 1e-3)
        ? 1.0 // both frames are already flat; nothing to compare, don't penalize
        : std::min(std1, std2) / std::max(std1, std2);

    return std::min(colorScore, textureScore);
}

bool VideoStream::CheckShiny(const std::atomic<bool>& keepRunning) {
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

    double similarity = CalcSimilarity(m_prevDetectionFrame, croppedFrame);
    Log(wxString::Format("Similarity Score: %.2f", similarity));

    if (similarity < 0.9){
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

bool VideoStream::CheckChange() {
    // First call, start comparing frames in the cap loop
    if (!m_checkingChange.exchange(true)) {
        m_changeDetected = false;
        std::lock_guard<std::mutex> lock(m_frameMutex);
        m_prevFrame = cv::Mat();
        return false;
    }

    // Already watching, check if a big change was detected
    if (m_changeDetected.exchange(false)) {
        m_checkingChange = false;
        std::lock_guard<std::mutex> lock(m_frameMutex);
        m_prevFrame = cv::Mat();
        Log("Big change between frames detected");
        return true;
    }

    return false;
}

void VideoStream::StopCheckingChange() {
    if (m_checkingChange.exchange(false)) {
        m_changeDetected = false;
        std::lock_guard<std::mutex> lock(m_frameMutex);
        m_prevFrame = cv::Mat();
    }
}

void VideoStream::ResetDetectionFrame() {
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
        m_prevFrame = cv::Mat();
    }

    StartCapture(deviceIndex);
}