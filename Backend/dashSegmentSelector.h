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

struct MPD_Period
{
    std::string BaseURL;
    std::string duration;
    std::string start;
};

struct MPD_AdaptationSet
{
    std::string mimeType;
    std::string BaseURL;
};

struct MPD_SegmentTemplate
{
    uint32_t startNumber;
    uint32_t timescale;
    uint32_t duration;
    std::string lang;
    std::string media;
    std::string initialization;
    std::vector<uint32_t> SegmentTimeline;
};

struct MPD_Representation
{
    uint32_t bandwidth;
    std::string id;
};

struct downloadInfo
{
    std::string BaseURL;
    MPD_Period Period;
    MPD_AdaptationSet AdaptationSet;
    MPD_SegmentTemplate SegmentTemplate;
    MPD_Representation Representation;
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
    std::vector<uint32_t> GetSegmentTimeline(dash::mpd::ISegmentTemplate* segmentTemplate);
    bool IsEOS(const uint64_t& nextDownloadTime, const downloadInfo& inDownloadInfo);
    bool IsBOS(const uint64_t& nextDownloadTime, const downloadInfo& inDownloadInfo);

    // video
    uint32_t GetTargetDownloadSize_Video();
    downloadInfo GetDownloadInfo_Video(uint32_t targetDownloadSize);
    std::string GetDownloadURL_Video(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    std::string GetInitFileURL_Video(const downloadInfo& targetInfo);
    std::string GetSegmentURL_Video(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    downloadInfo GetDownloadInfo_priv_Video(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::ISegmentTemplate* segmentTemplate, dash::mpd::IRepresentation* representation);

    // audio
    uint32_t GetTargetDownloadSize_Audio();
    downloadInfo GetDownloadInfo_Audio(uint32_t targetDownloadSize);
    std::string GetDownloadURL_Audio(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    std::string GetInitFileURL_Audio(const downloadInfo& targetInfo);
    std::string GetSegmentURL_Audio(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    downloadInfo GetDownloadInfo_priv_Audio(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::ISegmentTemplate* segmentTemplate, dash::mpd::IRepresentation* representation);

    // Tools
    bool ReplaceSubstring(std::string& str, const std::string& from, const std::string& to);
    void ReplaceAllSubstring(std::string& str, const std::string& from, const std::string& to);
    uint32_t GetSegmentDurationMSec(const downloadInfo& inDownloadInfo);
    uint32_t GetSegmentTimeMSec(const uint64_t& inTime, const downloadInfo& inDownloadInfo);
    bool GetTimeString2MSec(std::string timeStr, uint64_t& timeMSec);
    void AppendSlash2Path(std::string& inPath);
 private:
    DefaultMutex m_mutex;
    std::shared_ptr<dash::mpd::IMPD> m_mpdFile;
    dashMediaStatus m_videoStatus;
    dashMediaStatus m_audioStatus;
    dashMediaStatus m_subtitleStatus;
    PlayerMsg_Factory m_msgFactory;
};

#endif