#pragma once
#include <wx/image.h>
#include <opencv2/opencv.hpp>
#include <mutex>
#include <atomic>
#include <thread>

class VideoStream
{
public: 
    VideoStream();
    ~VideoStream();

    void StartCapture(int deviceIndex = 0); // Starts video capture
    void StopCapture(); // Joins m_captureThread if running
    void SwitchCamera(int deviceIndex); // Stops current capture, starts a new one on deviceIndex
    int GetWebcamCount(); // Counts number of webcams user has connected. Returns a default value of 2 with debugger attached
    wxImage GetWxImageFromFrame(); // Convert frame to wxImage
    bool checkShiny(); // Compares the current detection to the last, if it is very different assume it is a shiny
    std::atomic<int> m_rectX = 78;
    std::atomic<int> m_rectY = 365;
    std::atomic<int> m_rectW = 293;
    std::atomic<int> m_rectH = 51;
private:
    std::thread m_captureThread;
    std::atomic<bool> m_stopCapture{false};

    cv::Mat m_frame;
    std::mutex m_frameMutex;
    cv::Mat m_prevDetectionFrame;
    cv::Rect m_textBoxRect;

    double m_avgSimilarity = 1;
    int m_totalShinyChecks = 0;
};