#ifndef DASH_SEGMENTSELECTOR_H
#define DASH_SEGMENTSELECTOR_H
#include "segmentSelector.h"
#include "DefaultMutex.h"
#include "PlayerMsg_Factory.h"

#include "IMPD.h"
#include "libdash.h"

#include <set>
#include <map>
#include <string>
#include <sstream>

struct downloadInfo
{
    uint32_t m_bandwidth;
    std::string m_baseURL;
    uint32_t m_startNumber;
    uint32_t m_timescale;
    uint32_t m_duration;
    std::string m_media;
    std::string m_initialization;
    std::string m_lang;
    std::string m_representationID;
};

struct dashMediaStatus
{
    bool m_initFileReady;
    uint32_t m_downloadSpeed; // bit per sec
    uint64_t m_downloadTime; // location that is already downloaded
    uint64_t m_playTime; // location that is already played
    uint32_t m_numberSegment;
    downloadInfo m_downloadInfo;
};

class dashSegmentSelector : public segmentSelector
{
 public:
    dashSegmentSelector();
    ~dashSegmentSelector();

    // override
    void InitComponent(cmdReceiver* manager);
 private:
    void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadMPD> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_Play> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_Pause> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_Stop> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadFinish> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_ProcessNextSegment> msg);

    void InitStatus(dashMediaStatus& status);

    void HandleVideoSegment();
    void HandleAudioSegment();
    void HandleSubtitleSegment();

    // video and audio
    uint32_t GetTargetDownloadSize(const dashMediaStatus& mediaStatus, std::string mediaType);

    // video
    uint32_t GetTargetDownloadSize_Video();
    downloadInfo GetDownloadInfo_Video(uint32_t targetDownloadSize);
    std::string GetDownloadURL_Video(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    std::string GetInitFileURL_Video(const downloadInfo& targetInfo);
    std::string GetSegmentURL_Video(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);

    // audio
    uint32_t GetTargetDownloadSize_Audio();
    downloadInfo GetDownloadInfo_Audio(uint32_t targetDownloadSize);
    std::string GetDownloadURL_Audio(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    std::string GetInitFileURL_Audio(const downloadInfo& targetInfo);
    std::string GetSegmentURL_Audio(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);

    // Tools
    bool ReplaceSubstring(std::string& str, const std::string& from, const std::string& to);
    void ReplaceAllSubstring(std::string& str, const std::string& from, const std::string& to);
    uint32_t GetSegmentDurationMSec(const downloadInfo& inDownloadInfo);
 private:
    DefaultMutex m_mutex;
    std::shared_ptr<dash::mpd::IMPD> m_mpdFile;
    dashMediaStatus m_videoStatus;
    dashMediaStatus m_audioStatus;
    dashMediaStatus m_subtitleStatus;
    PlayerMsg_Factory m_msgFactory;
};

#endif
