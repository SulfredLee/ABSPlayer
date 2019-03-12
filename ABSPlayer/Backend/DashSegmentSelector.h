#ifndef DASH_SEGMENTSELECTOR_H
#define DASH_SEGMENTSELECTOR_H
#include "SegmentSelector.h"
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
    uint64_t periodStartTime;
};

struct MPD_AdaptationSet
{
    std::string mimeType;
    std::string BaseURL;
    std::string lang;
};

struct MPD_SegmentTemplate
{
    uint32_t startNumber;
    uint32_t timescale;
    uint32_t duration;
    uint64_t presentationTimeOffset; // MSec
    std::string media;
    std::string initialization;
    std::vector<uint64_t> SegmentTimeline;
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

struct SegmentInfo
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
    uint64_t m_numberSegment;
    uint64_t m_mediaStartTime;
    uint64_t m_mediaEndTime;
    SegmentInfo m_segmentInfo;
};

struct SegmentCriteria
{
    std::string mediaType;
    std::string language;
    uint64_t downloadTime;
    uint32_t downloadSize;
};

class DashSegmentSelector : public SegmentSelector
{
 public:
    DashSegmentSelector();
    ~DashSegmentSelector();

    // override
    void InitComponent(CmdReceiver* manager);
 private:
    // override
    void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadMPD> msg);
    // override
    void ProcessMsg(std::shared_ptr<PlayerMsg_RefreshMPD> msg);
    // override
    void ProcessMsg(std::shared_ptr<PlayerMsg_Play> msg);
    // override
    void ProcessMsg(std::shared_ptr<PlayerMsg_Pause> msg);
    // override
    void ProcessMsg(std::shared_ptr<PlayerMsg_Stop> msg);
    // override
    void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadFinish> msg);
    // override
    void ProcessMsg(std::shared_ptr<PlayerMsg_ProcessNextSegment> msg);
    // override
    void ProcessMsg(std::shared_ptr<PlayerMsg_UpdateDownloadTime> msg);

    void InitStatus(dashMediaStatus& status);

    void HandleVideoSegment();
    void HandleAudioSegment();
    void HandleSubtitleSegment();

    // video and audio
    uint32_t GetTargetDownloadSize(const dashMediaStatus& mediaStatus, std::string mediaType);
    std::vector<uint64_t> GetSegmentTimeline(dash::mpd::ISegmentTemplate* segmentTemplate, const std::string& periodID);
    bool IsEOS(const uint64_t& nextDownloadTime, const dashMediaStatus& inMediaStatus);
    bool IsBOS(const uint64_t& nextDownloadTime, const dashMediaStatus& inMediaStatus);
    bool GetTimeString2MSec(std::string timeStr, uint64_t& timeMSec);
    bool GetUTCDateTimeString2MSec(std::string timeStr, uint64_t& timeMSec); // input a UTC date time string
    bool GetLocalDateTimeString2MSec(std::string timeStr, uint64_t& timeMSec); // input a local date time string
    uint32_t GetSegmentDurationMSec(const SegmentInfo& inDownloadInfo);
    uint32_t GetSegmentTimeMSec(const uint64_t& inTime, const SegmentInfo& inDownloadInfo);
    std::string GetSegmentNumberFromTimeline(dashMediaStatus& mediaStatus, const SegmentInfo& segmentInfo);
    void AppendSlash2Path(std::string& inPath);
    void HandleStringFormat(std::string& mediaStr, uint64_t data, std::string target);
    void HandleBaseURL(std::stringstream& ss, const SegmentInfo& targetInfo);
    bool IsStaticMedia(std::shared_ptr<dash::mpd::IMPD> mpdFile);
    void GetSegmentInfo_Base(const SegmentCriteria& criteria, SegmentInfo& resultInfo);
    void GetSegmentInfo_Period(const SegmentCriteria& criteria, dash::mpd::IPeriod* period, SegmentInfo& resultInfo, const uint64_t& periodStartTime);
    void GetSegmentInfo_AdaptationSet(const SegmentCriteria& criteria, dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, SegmentInfo& resultInfo, const uint64_t& periodStartTime);
    void GetSegmentInfo_Representation(const SegmentCriteria& criteria, dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, SegmentInfo& resultInfo, const uint64_t& periodStartTime);
    void GetMediaDuration(uint64_t& startTime, uint64_t& endTime);
    SegmentInfo GetSegmentInfo_priv(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::ISegmentTemplate* segmentTemplate, dash::mpd::IRepresentation* representation, const uint64_t& periodStartTime);
    std::string GetDownloadURL(dashMediaStatus& mediaStatus, const SegmentInfo& videoDownloadInfo, std::string mediaType);
    std::string GetSegmentURL(dashMediaStatus& mediaStatus, const SegmentInfo& videoSegmentInfo, std::string mediaType);
    std::string GetSegmentURL_Static(dashMediaStatus& mediaStatus, const SegmentInfo& videoSegmentInfo);
    std::string GetInitFileURL(const SegmentInfo& targetInfo);
    std::string GetSegmentURL_Dynamic(dashMediaStatus& mediaStatus, const SegmentInfo& targetInfo);
    uint64_t GetNextDownloadTime(const dashMediaStatus& mediaStatus, uint64_t currentDownloadTime);
    std::string GetMimeType(dash::mpd::IAdaptationSet* adaptationSet);
    int32_t GetCurrentTimeZone();
    uint64_t GetCurrentDownloadTime(uint32_t liveDelayMSec, uint32_t timeShiftBufferDepthMSec = 0);
    uint64_t GetPeriodDuration(const std::string& periodID);
    uint64_t GetCurrentUTCTime();
    bool IsDownloadTimeTooOld(const uint64_t& currentDownloadTime);
    bool IsDownloadTimeTooEarly(const uint64_t& currentDownloadTime);
    std::string IsDownloadTimeValid(const uint64_t& currentDownloadTime);
    void UpdateMPDStaticInfo();

    // Tools
    bool ReplaceSubstring(std::string& str, const std::string& from, const std::string& to);
    void ReplaceAllSubstring(std::string& str, const std::string& from, const std::string& to);
    void HandleDynamicMPDRefresh();
    // Debug Tools
    void PrintTimeline(const std::vector<uint64_t>& timeline);
 private:
    DefaultMutex m_mutex;
    std::shared_ptr<dash::mpd::IMPD> m_mpdFile;
    std::string m_mpdFileURL;
    dashMediaStatus m_videoStatus;
    dashMediaStatus m_audioStatus;
    dashMediaStatus m_subtitleStatus;
    PlayerMsg_Factory m_msgFactory;
    int m_trickScale;
};

#endif
