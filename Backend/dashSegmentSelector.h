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
    std::string id;
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
    std::string BaseURL;
};

struct MPD_MPD
{
    std::string minimumUpdatePeriod;
    std::string type; // static or dynamic
};

struct downloadInfo
{
    std::string BaseURL;
    MPD_MPD MPD;
    MPD_Period Period;
    MPD_AdaptationSet AdaptationSet;
    MPD_SegmentTemplate SegmentTemplate;
    MPD_Representation Representation;
    bool isHasSegmentTemplate;
};

struct dashMediaStatus
{
    bool m_initFileReady;
    uint32_t m_downloadSpeed; // bit per sec
    uint64_t m_downloadTime; // location that is already downloaded
    uint64_t m_playTime; // location that is already played
    uint32_t m_numberSegment;
    uint64_t m_mediaStartTime;
    uint64_t m_mediaEndTime;
    std::string m_prePeriodID;
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
    void ProcessMsg(std::shared_ptr<PlayerMsg_RefreshMPD> msg);
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
    bool IsEOS(const uint64_t& nextDownloadTime, const dashMediaStatus& inMediaStatus);
    bool IsBOS(const uint64_t& nextDownloadTime, const dashMediaStatus& inMediaStatus);
    bool GetTimeString2MSec(std::string timeStr, uint64_t& timeMSec);
    uint32_t GetSegmentDurationMSec(const downloadInfo& inDownloadInfo);
    uint32_t GetSegmentTimeMSec(const uint64_t& inTime, const downloadInfo& inDownloadInfo);
    void AppendSlash2Path(std::string& inPath);
    void HandleStringFormat(std::string& mediaStr, uint32_t data, std::string target);
    void HandleBaseURL(std::stringstream& ss, const downloadInfo& targetInfo);
    bool IsStaticMedia(std::shared_ptr<dash::mpd::IMPD> mpdFile);

    // video
    uint32_t GetTargetDownloadSize_Video();
    segmentSelectorRet GetDownloadInfo_Video(uint32_t targetDownloadSize, downloadInfo& resultInfo);
    std::string GetDownloadURL_Video(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    std::string GetInitFileURL_Video(const downloadInfo& targetInfo);
    std::string GetSegmentURL_Video(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    std::string GetSegmentURL_Video_Static(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    std::string GetSegmentURL_Video_Dynamic(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    downloadInfo GetDownloadInfo_priv_Video(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::ISegmentTemplate* segmentTemplate, dash::mpd::IRepresentation* representation);

    // audio
    uint32_t GetTargetDownloadSize_Audio();
    segmentSelectorRet GetDownloadInfo_Audio(uint32_t targetDownloadSize, downloadInfo& resultInfo);
    std::string GetDownloadURL_Audio(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    std::string GetInitFileURL_Audio(const downloadInfo& targetInfo);
    std::string GetSegmentURL_Audio(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    std::string GetSegmentURL_Audio_Static(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    std::string GetSegmentURL_Audio_Dynamic(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime);
    downloadInfo GetDownloadInfo_priv_Audio(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::ISegmentTemplate* segmentTemplate, dash::mpd::IRepresentation* representation);

    // Tools
    bool ReplaceSubstring(std::string& str, const std::string& from, const std::string& to);
    void ReplaceAllSubstring(std::string& str, const std::string& from, const std::string& to);
 private:
    DefaultMutex m_mutex;
    std::shared_ptr<dash::mpd::IMPD> m_mpdFile;
    std::string m_mpdFileURL;
    dashMediaStatus m_videoStatus;
    dashMediaStatus m_audioStatus;
    dashMediaStatus m_subtitleStatus;
    PlayerMsg_Factory m_msgFactory;
};

#endif
