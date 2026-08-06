#pragma once
#include <wx/image.h>
#include <opencv2/opencv.hpp>
#include <mutex>

class VideoStream
{
public: 
    VideoStream();
    void StartCapture();
    wxImage GetWxImageFromFrame();
private:
    cv::Mat m_frame;
    std::mutex m_frameMutex;
};