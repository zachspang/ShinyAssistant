#pragma once
#include <wx/wx.h>

// A wxStaticBitmap that automatically rescales its image to fit whatever
// size the sizer/window gives it, and supports swapping to a new image
// from disk or from memory.
class ScaledStaticBitmap : public wxStaticBitmap 
{
public:
    // Construct empty object
    ScaledStaticBitmap(wxWindow* parent, wxWindowID id = wxID_ANY) : wxStaticBitmap(parent, id, wxNullBitmap){
        Bind(wxEVT_SIZE, &ScaledStaticBitmap::OnResize, this);
    }

    // Construct with an initial image loaded from disk
    ScaledStaticBitmap(wxWindow* parent, const wxString& filePath, wxWindowID id = wxID_ANY) : wxStaticBitmap(parent, id, wxNullBitmap){
        Bind(wxEVT_SIZE, &ScaledStaticBitmap::OnResize, this);
        LoadImage(filePath);
    }

    // Load a new image from disk
    bool LoadImage(const wxString& filePath){
        wxImage img;
        if (!img.LoadFile(filePath)) return false;
        m_source_image = img;
        ApplyImage();
        return true;
    }

    // Swap to a new image already in memory
    void SetImage(const wxImage& img){
        m_source_image = img;
        ApplyImage();
    }

private:
    // Loaded image kept at its original resolution
    wxImage m_source_image;
    // Floor for how small the image can shrink. Without this the midWidth become the native width of the image which
    // is too large for a minimum
    int m_minWidth = 100; 

    // Sets MinSize of widget to (m_minWidth, height-that-preserves-ratio)
    void ApplyMinSize(){
        double ratio = (double)m_source_image.GetHeight() / (double)m_source_image.GetWidth();
        SetMinSize(wxSize(m_minWidth, std::max(1, (int)(m_minWidth * ratio))));
    }

    // If the new image is ok resets the MinSize for the new image and sets the wxBitmap to the image.
    void ApplyImage(){
        if (!m_source_image.IsOk()) return;
        ApplyMinSize();
        SetBitmap(wxBitmap(m_source_image));
    }

    // Reads the widgets pixel size and scales the image to fill that size
    void RescaleToCurrentSize(){
        wxSize size = GetSize();
        if (!m_source_image.IsOk() || size.x <= 0 || size.y <= 0) return;
        wxImage scaled = m_source_image.Scale(size.x, size.y, wxIMAGE_QUALITY_NEAREST);
        SetBitmap(wxBitmap(scaled));
    }

    // Event handler bound to exEVT_SIZE
    void OnResize(wxSizeEvent& evt){
        RescaleToCurrentSize();
        evt.Skip();
    }
};