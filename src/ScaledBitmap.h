#pragma once
#include <wx/wx.h>
#include <wx/dcbuffer.h>

// A custom-drawn widget that rescales its image to fit whatever size
// the sizer/window gives it. Double-buffered to avoid flicker when
// swapping images rapidly (e.g. video frames).
class ScaledBitmap : public wxWindow
{
public:
    ScaledBitmap(wxWindow* parent, wxWindowID id = wxID_ANY): wxWindow(parent, id){
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        Bind(wxEVT_SIZE, &ScaledBitmap::OnResize, this);
        Bind(wxEVT_PAINT, &ScaledBitmap::OnPaint, this);
        m_source_image = wxImage();
    }

    ScaledBitmap(wxWindow* parent, const wxString& filePath, wxWindowID id = wxID_ANY) : wxWindow(parent, id){
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        Bind(wxEVT_SIZE, &ScaledBitmap::OnResize, this);
        Bind(wxEVT_PAINT, &ScaledBitmap::OnPaint, this);
        m_source_image = wxImage();
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
    //Cached scaled image
    wxBitmap m_scaled_bitmap;

    // Sets MinSize of widget to (m_minWidth, height-that-preserves-ratio)
    void ApplyMinSize(){
        if (!m_source_image.IsOk()) return;
        double ratio = (double)m_source_image.GetHeight() / (double)m_source_image.GetWidth();
        SetMinSize(wxSize(m_minWidth, std::max(1, (int)(m_minWidth * ratio))));

        //Need to update sizers aspect ratio since it is normally only computed when the this is created
        if (wxSizer* sizer = GetContainingSizer()) {
            if (wxSizerItem* item = sizer->GetItem(this)) {
                int w = m_source_image.GetWidth();
                int h = m_source_image.GetHeight();
                if (w > 0 && h > 0) item->SetRatio(w, h);
            }
        }
        //Force a layout update
        if (wxWindow* parent = GetParent()) parent->Layout();
    }

    void RescaleToCurrentSize(){
        wxSize size = GetSize();
        if (!m_source_image.IsOk() || size.x <= 0 || size.y <= 0) return;
        wxImage scaled = m_source_image.Scale(size.x, size.y, wxIMAGE_QUALITY_NEAREST);
        m_scaled_bitmap = wxBitmap(scaled);
        // false = don't erase background before repainting
        Refresh(false);
    }

    void ApplyImage(){
        if (!m_source_image.IsOk()) return;
        ApplyMinSize();
        RescaleToCurrentSize();
    }

    // Event handler bound to exEVT_SIZE
    void OnResize(wxSizeEvent& evt){
        RescaleToCurrentSize();
        evt.Skip();
    }

    void OnPaint(wxPaintEvent& evt){
        // draws to an off-screen buffer, then blits once
        wxAutoBufferedPaintDC dc(this); 
        dc.SetBackground(wxBrush(GetBackgroundColour()));
        dc.Clear();
        if (m_scaled_bitmap.IsOk()) {
            dc.DrawBitmap(m_scaled_bitmap, 0, 0, false);
        }
    }
};