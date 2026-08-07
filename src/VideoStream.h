#pragma once
#include <wx/image.h>
#include <opencv2/opencv.hpp>
#include <mutex>
#include <atomic>

class VideoStream
{
public: 
    VideoStream();
    void StartCapture();
    wxImage GetWxImageFromFrame();
    bool checkShiny();
    std::atomic<int> m_rectX = 78;
    std::atomic<int> m_rectY = 365;
    std::atomic<int> m_rectW = 293;
    std::atomic<int> m_rectH = 51;
private:
    cv::Mat m_frame;
    std::mutex m_frameMutex;
    cv::Mat m_prevDetectionFrame;
    cv::Rect m_textBoxRect;
    double m_avgSimilarity = 1;
    int m_totalShinyChecks = 0;
};