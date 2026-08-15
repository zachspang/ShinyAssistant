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
        ApplyMinSize();
    }

    ScaledBitmap(wxWindow* parent, const wxString& filePath, wxWindowID id = wxID_ANY) : wxWindow(parent, id){
        SetBackgroundStyle(wxBG_STYLE_PAINT);
        Bind(wxEVT_SIZE, &ScaledBitmap::OnResize, this);
        Bind(wxEVT_PAINT, &ScaledBitmap::OnPaint, this);
        m_source_image = wxImage();
        ApplyMinSize();
        LoadImage(filePath);
    }

    // Load a new image from disk
    bool LoadImage(const wxString& filePath){
        wxImage img;
        if (!img.LoadFile(filePath)) return false;
        SetImage(img);
        return true;
    }

    // Swap to a new image already in memory
    void SetImage(const wxImage& img){
        if (!img.IsOk()) return; //drop invalid images (e.g. frame from webcam still starting up)

        bool dimensionsChanged = !m_source_image.IsOk() || m_source_image.GetWidth()  != img.GetWidth() || m_source_image.GetHeight() != img.GetHeight();

        m_source_image = img;

        // Only touch the sizer's ratio layout when the source's actual aspect ratio changes, e.g. switching webcam sources
        if (dimensionsChanged) {
            ApplyRatio();
        }
        RescaleToCurrentSize();
    }

private:
    // Loaded image kept at its original resolution
    wxImage m_source_image;
    // Fixed floor for how small the widget can shrink, independent of aspect ratio.
    int m_minWidth = 160;
    int m_minHeight = 90;
    //Cached scaled image
    wxBitmap m_scaled_bitmap;

    // Sets MinSize of widget to (m_minWidth, m_minHeight)
    void ApplyMinSize(){
        SetMinSize(wxSize(m_minWidth, m_minHeight));
    }

    // Updates the containing sizer's Shaped() ratio to match the current
    // source image, so the widget's box always matches the real aspect ratio
    // whatever it happens to be for the current source.
    void ApplyRatio(){
        int w = m_source_image.GetWidth();
        int h = m_source_image.GetHeight();
        if (w <= 0 || h <= 0) return;

        if (wxSizer* sizer = GetContainingSizer()) {
            if (wxSizerItem* item = sizer->GetItem(this)) {
                item->SetRatio(w, h);
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

    // Event handler bound to wxEVT_SIZE
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