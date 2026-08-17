#include "WebhookNotifier.h"
#include "Logging.h"
#include <wx/mstream.h>
#include <curl/curl.h>
#include <nlohmann/json.hpp>

bool WebhookNotifier::SendShinyAlert(const wxImage& frame, const wxString& webhookUrl, const wxString& pingName) {
    if (webhookUrl.IsEmpty()) {
        Log("Webhook URL not set, skipping shiny alert");
        return false;
    }
    if (!frame.IsOk()) {
        Log("Cannot send shiny alert: invalid frame");
        return false;
    }

    //Encode the frame as PNG
    wxMemoryOutputStream pngStream;
    frame.SaveFile(pngStream, wxBITMAP_TYPE_PNG);
    wxStreamBuffer* buf = pngStream.GetOutputStreamBuffer();

    //Format payload content
    wxString content = pingName.IsEmpty()
    ? wxString("Shiny detected!")
    : wxString::Format("<@%s> Shiny detected!", pingName);

    nlohmann::json payload;
    payload["content"] = content.ToUTF8().data(); 

    wxString jsonPayload = wxString::FromUTF8(payload.dump());

    CURL* curl = curl_easy_init();
    if (!curl) {
        Log("curl_easy_init failed");
        return false;
    }

    curl_mime* mime = curl_mime_init(curl);

    curl_mimepart* jsonPart = curl_mime_addpart(mime);
    curl_mime_name(jsonPart, "payload_json");
    curl_mime_type(jsonPart, "application/json");
    wxScopedCharBuffer jsonUtf8 = jsonPayload.ToUTF8();
    curl_mime_data(jsonPart, jsonUtf8.data(), jsonUtf8.length());

    curl_mimepart* filePart = curl_mime_addpart(mime);
    curl_mime_name(filePart, "file");
    curl_mime_filename(filePart, "shiny.png");
    curl_mime_type(filePart, "image/png");
    curl_mime_data(filePart, (const char*)buf->GetBufferStart(), buf->GetBufferSize());

    wxScopedCharBuffer urlUtf8 = webhookUrl.ToUTF8();
    curl_easy_setopt(curl, CURLOPT_URL, urlUtf8.data());
    curl_easy_setopt(curl, CURLOPT_MIMEPOST, mime);
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);
    curl_easy_setopt(curl, CURLOPT_CAINFO, "ca-bundle.crt");

    CURLcode res = curl_easy_perform(curl);
    bool success = false;

    if (res == CURLE_OK) {
        long statusCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &statusCode);
        success = (statusCode >= 200 && statusCode < 300);
        Log(wxString::Format("Discord webhook response: %ld", statusCode));
    } else {
        Log(wxString::Format("Webhook send failed: %s", curl_easy_strerror(res)));
    }

    curl_mime_free(mime);
    curl_easy_cleanup(curl);

    return true;
}