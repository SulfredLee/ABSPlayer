#include "DashSegmentSelector.h"
#include "Logger.h"
#include <algorithm>
#include <sys/time.h>
#include <ctime>
#include <stdio.h>

#define LIVE_TIME_LAG 3000 // 3 sec time lagging for live video
DashSegmentSelector::DashSegmentSelector()
{
    m_mpdFile = nullptr;
    InitStatus(m_videoStatus);
    InitStatus(m_audioStatus);
    InitStatus(m_subtitleStatus);
}

DashSegmentSelector::~DashSegmentSelector()
{}

void DashSegmentSelector::InitComponent(CmdReceiver* manager)
{
    SegmentSelector::InitComponent(manager);
}

// override
void DashSegmentSelector::ProcessMsg(SmartPointer<PlayerMsg_DownloadMPD> msg)
{
    m_mpdFile = msg->GetAndMoveMPDFile();
    m_mpdFileURL = msg->GetURL();

    // update status
    GetMediaDuration(m_videoStatus.m_mediaStartTime, m_videoStatus.m_mediaEndTime);
    GetMediaDuration(m_audioStatus.m_mediaStartTime, m_audioStatus.m_mediaEndTime);

    if (!IsStaticMedia(m_mpdFile))
        HandleDynamicMPDRefresh();
}

// override
void DashSegmentSelector::ProcessMsg(SmartPointer<PlayerMsg_RefreshMPD> msg)
{
    m_mpdFile = msg->GetAndMoveMPDFile();
    HandleDynamicMPDRefresh();
}

// override
void DashSegmentSelector::ProcessMsg(SmartPointer<PlayerMsg_Play> msg)
{
    // get current player status
    SmartPointer<PlayerMsg_Base> msgBase = m_msgFactory.CreateMsg(PlayerMsg_Type_GetPlayerStage);
    SendToManager(msgBase);
    SmartPointer<PlayerMsg_GetPlayerStage> msgGetStage = DynamicCast<PlayerMsg_GetPlayerStage>(msgBase);
    if (msgGetStage->GetPlayerStage() == PlayerStage_Play)
    {
        // prepare start time
        if (IsStaticMedia(m_mpdFile))
        {
            m_videoStatus.m_downloadTime = 0;
            m_audioStatus.m_downloadTime = 0;
        }
        else
        {
            m_videoStatus.m_downloadTime = GetCurrentDownloadTime(LIVE_TIME_LAG);
            m_audioStatus.m_downloadTime = m_videoStatus.m_downloadTime;
            LOGMSG_INFO("Live download time: %lu", m_videoStatus.m_downloadTime);
        }
        m_trickScale = 1000;
        HandleVideoSegment();
        HandleAudioSegment();
        HandleSubtitleSegment();
    }
    else
    {
        LOGMSG_ERROR("Cannot play movie. Current player stage is: %u", msgGetStage->GetPlayerStage());
    }
}

// override
void DashSegmentSelector::ProcessMsg(SmartPointer<PlayerMsg_Pause> msg)
{
}

// override
void DashSegmentSelector::ProcessMsg(SmartPointer<PlayerMsg_Stop> msg)
{
}

// override
void DashSegmentSelector::ProcessMsg(SmartPointer<PlayerMsg_UpdateDownloadTime> msg)
{
    switch (msg->GetFileType())
    {
        case PlayerMsg_Type_DownloadVideo:
            {
                m_videoStatus.m_downloadTime = GetNextDownloadTime(m_videoStatus, m_videoStatus.m_downloadTime);
                break;
            }
        case PlayerMsg_Type_DownloadAudio:
            {
                m_audioStatus.m_downloadTime = GetNextDownloadTime(m_audioStatus, m_audioStatus.m_downloadTime);
                break;
            }
        case PlayerMsg_Type_DownloadSubtitle:
            {
                break;
            }
        default:
            break;
    }
}

// override
void DashSegmentSelector::ProcessMsg(SmartPointer<PlayerMsg_DownloadFinish> msg)
{
    switch (msg->GetFileType())
    {
        case PlayerMsg_Type_DownloadVideo:
            {
                if (msg->GetResponseCode() == 200)
                {
                    m_videoStatus.m_downloadSpeed = msg->GetSpeed();
                    if (!m_videoStatus.m_initFileReady)
                        m_videoStatus.m_initFileReady = true;
                    else
                        m_videoStatus.m_downloadTime = GetNextDownloadTime(m_videoStatus, m_videoStatus.m_downloadTime);
                }
                break;
            }
        case PlayerMsg_Type_DownloadAudio:
            {
                if (msg->GetResponseCode() == 200)
                {
                    m_audioStatus.m_downloadSpeed = msg->GetSpeed();
                    if (!m_audioStatus.m_initFileReady)
                        m_audioStatus.m_initFileReady = true;
                    else
                        m_audioStatus.m_downloadTime = GetNextDownloadTime(m_audioStatus, m_audioStatus.m_downloadTime);
                }
                break;
            }
        case PlayerMsg_Type_DownloadSubtitle:
        default:
            break;
    }
}

// override
void DashSegmentSelector::ProcessMsg(SmartPointer<PlayerMsg_ProcessNextSegment> msg)
{
    switch(msg->GetSegmentType())
    {
        case PlayerMsg_Type_DownloadVideo:
            {
                HandleVideoSegment();
                break;
            }
        case PlayerMsg_Type_DownloadAudio:
            {
                HandleAudioSegment();
                break;
            }
        case PlayerMsg_Type_DownloadSubtitle:
        default:
            break;
    }
}

void DashSegmentSelector::InitStatus(dashMediaStatus& status)
{
    status.m_initFileReady = false;
    status.m_downloadSpeed = 0;
    status.m_downloadTime = 0;
    status.m_playTime = 0;
    status.m_numberSegment = 0;
    status.m_mediaStartTime = 0;
    status.m_mediaEndTime = 0;
}

void DashSegmentSelector::HandleVideoSegment()
{
    std::string validRtn = IsDownloadTimeValid(m_videoStatus.m_downloadTime);
    SmartPointer<PlayerMsg_DownloadFile> msgDVideo = DynamicCast<PlayerMsg_DownloadFile>(m_msgFactory.CreateMsg(PlayerMsg_Type_DownloadVideo));
    msgDVideo->SetErrorMsg(validRtn);

    if (validRtn == "")
    {
        // get target criteria
        uint32_t targetDownloadSize = GetTargetDownloadSize(m_videoStatus, "Video");
        // get url information
        SegmentInfo videoSegmentInfo;
        SegmentCriteria videoSegmentCriteria = {"video", "*", m_videoStatus.m_downloadTime, targetDownloadSize};
        GetSegmentInfo_Base(videoSegmentCriteria, videoSegmentInfo);
        // generate url
        std::string targetURL = GetDownloadURL(m_videoStatus, videoSegmentInfo, "video");
        // update segment information
        m_videoStatus.m_segmentInfo = videoSegmentInfo;

        LOGMSG_INFO("targetURL: %s", targetURL.c_str());

        if (targetURL == "Media_EOS")
        {
            LOGMSG_INFO("Video_EOS");
            msgDVideo->SetErrorMsg(targetURL);
        }
        else if (targetURL == "Media_BOS")
        {
            LOGMSG_INFO("Video_EOS");
            msgDVideo->SetErrorMsg(targetURL);
        }
        else if (targetURL.length())
        {
            // send download message
            msgDVideo->SetURL(targetURL);
        }
        else
        {
        }
    }
    else
    {
    }
    msgDVideo->SetDownloadTime(m_videoStatus.m_downloadTime);
    SendToManager(StaticCast<PlayerMsg_Base>(msgDVideo));
}

void DashSegmentSelector::HandleAudioSegment()
{
    std::string validRtn = IsDownloadTimeValid(m_audioStatus.m_downloadTime);
    SmartPointer<PlayerMsg_DownloadFile> msgDAudio = DynamicCast<PlayerMsg_DownloadFile>(m_msgFactory.CreateMsg(PlayerMsg_Type_DownloadAudio));
    msgDAudio->SetErrorMsg(validRtn);

    if (validRtn == "")
    {
        // get target criteria
        uint32_t targetDownloadSize = GetTargetDownloadSize(m_audioStatus, "Audio");
        // get url information
        SegmentInfo audioSegmentInfo;
        SegmentCriteria audioSegmentCriteria = {"audio", "*", m_audioStatus.m_downloadTime, targetDownloadSize};
        GetSegmentInfo_Base(audioSegmentCriteria, audioSegmentInfo);
        // generate url
        std::string targetURL = GetDownloadURL(m_audioStatus, audioSegmentInfo, "audio");
        // update segment information
        m_audioStatus.m_segmentInfo = audioSegmentInfo;

        LOGMSG_INFO("targetURL: %s", targetURL.c_str());

        if (targetURL == "Media_EOS")
        {
            LOGMSG_INFO("Audio_EOS");
            msgDAudio->SetErrorMsg(targetURL);
        }
        else if (targetURL == "Media_BOS")
        {
            LOGMSG_INFO("Audio_BOS");
            msgDAudio->SetErrorMsg(targetURL);
        }
        else if (targetURL.length())
        {
            // send download message
            msgDAudio->SetURL(targetURL);
        }
        else
        {
        }
    }
    else
    {
    }
    msgDAudio->SetDownloadTime(m_audioStatus.m_downloadTime);
    SendToManager(StaticCast<PlayerMsg_Base>(msgDAudio));
}

void DashSegmentSelector::HandleSubtitleSegment()
{
    if (m_subtitleStatus.m_initFileReady)
    {
    }
    else
    {
        // DownloadInitFile_Subtitle();
    }
}

bool DashSegmentSelector::ReplaceSubstring(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void DashSegmentSelector::ReplaceAllSubstring(std::string& str, const std::string& from, const std::string& to)
{
    if(from.empty())
        return;
    size_t start_pos = 0;
    while((start_pos = str.find(from, start_pos)) != std::string::npos)
    {
        str.replace(start_pos, from.length(), to);
        start_pos += to.length(); // In case 'to' contains 'from', like replacing 'x' with 'yx'
    }
}

uint32_t DashSegmentSelector::GetSegmentDurationMSec(const SegmentInfo& inDownloadInfo)
{
    return GetSegmentTimeMSec(inDownloadInfo.SegmentTemplate.duration, inDownloadInfo);
}

uint32_t DashSegmentSelector::GetSegmentTimeMSec(const uint64_t& inTime, const SegmentInfo& inDownloadInfo)
{
    if (inDownloadInfo.SegmentTemplate.timescale > 0)
        return (static_cast<double>(inTime) / inDownloadInfo.SegmentTemplate.timescale) * 1000;
    else
        return inTime * 1000;
}

uint32_t DashSegmentSelector::GetTargetDownloadSize(const dashMediaStatus& mediaStatus, std::string mediaType)
{
    uint32_t downloadSpeed = mediaStatus.m_downloadSpeed;
    uint64_t downloadTime = mediaStatus.m_downloadTime;
    uint64_t playTime = mediaStatus.m_playTime;

    // calculate the current network condition
    uint64_t bufferTime = (downloadTime < playTime) ? 0 : downloadTime - playTime; // bufferTime MSec
    bufferTime /= 1000;
    double availableDownloadTime = bufferTime * 0.7 > 1 ? bufferTime * 0.7 : 1; // assume we can always wait for 1 second
    uint32_t availableDownloadSize = availableDownloadTime * downloadSpeed;

    LOGMSG_INFO("%s downloadSpeed: %u bufferTime: %lu availableDownloadTime: %f availableDownloadSize: %u", mediaType.c_str(), downloadSpeed, bufferTime, availableDownloadTime, availableDownloadSize);
    return availableDownloadSize;
}

bool DashSegmentSelector::IsEOS(const uint64_t& nextDownloadTime, const dashMediaStatus& inMediaStatus)
{
    LOGMSG_DEBUG("nextDownloadTime: %lu endTime: %lu", nextDownloadTime, inMediaStatus.m_mediaEndTime);
    if (inMediaStatus.m_segmentInfo.Period.duration.length())
        return nextDownloadTime > inMediaStatus.m_mediaEndTime;
    else
        return false;
}

bool DashSegmentSelector::IsBOS(const uint64_t& nextDownloadTime, const dashMediaStatus& inMediaStatus)
{
    if (inMediaStatus.m_segmentInfo.Period.start.length())
        return nextDownloadTime < inMediaStatus.m_mediaStartTime;
    else
        return false;
}

bool DashSegmentSelector::GetTimeString2MSec(std::string timeStr, uint64_t& timeMSec)
{
    timeMSec = 0;
    if (timeStr.size() < 2)
        return false;
    if (timeStr.front() == 'P')
        timeStr.erase(0, 1);
    else
        return false;
    if (timeStr.front() == 'T')
        timeStr.erase(0, 1);
    else
        return false;

    size_t posHour = timeStr.find("H");
    if (posHour != std::string::npos)
    {
        std::string number = timeStr.substr(0, posHour);
        timeStr.erase(0, posHour + 1);
        timeMSec += std::stoull(number) * 3600 * 1000;
    }

    size_t posMinute = timeStr.find("M");
    if (posMinute != std::string::npos)
    {
        std::string number = timeStr.substr(0, posMinute);
        timeStr.erase(0, posMinute + 1);
        timeMSec += std::stoull(number) * 60 * 1000;
    }

    size_t posSecond = timeStr.find("S");
    if (posSecond != std::string::npos)
    {
        std::string number = timeStr.substr(0, posSecond);
        timeStr.erase(0, posSecond + 1);
        timeMSec += std::stod(number) * 1000;
    }

    return true;
}

bool DashSegmentSelector::GetDateTimeString2MSec(std::string timeStr, uint64_t& timeMSec)
{
    if (timeStr == "1970-01-01T00:00:00Z")
    {
        timeMSec = 0;
        return true;
    }
    else if (timeStr.length() == 20)
    {
        int year, month, day, hour, minute, second;
        sscanf(timeStr.c_str(), "%d-%d-%dT%d:%d:%dZ", &year, &month, &day, &hour, &minute, &second);

        if (year < 1970)
        {
            timeMSec = 0;
            return true;
        }

        struct tm timeInfo;
        timeInfo.tm_year = year - 1900;
        timeInfo.tm_mon = month - 1;
        timeInfo.tm_mday = day;
        timeInfo.tm_hour = hour;
        timeInfo.tm_min = minute;
        timeInfo.tm_sec = second;

        timeMSec = static_cast<uint64_t>(mktime(&timeInfo)) * 1000;

        return true;
    }
    else
    {
        LOGMSG_ERROR("timeStr format is not correct: %s", timeStr.c_str());
        return false;
    }

}

std::vector<uint64_t> DashSegmentSelector::GetSegmentTimeline(dash::mpd::ISegmentTemplate* segmentTemplate)
{
    std::vector<uint64_t> result;
    dash::mpd::ISegmentTimeline const * SegmentTimeline = segmentTemplate->GetSegmentTimeline();
    if (SegmentTimeline)
    {
        std::vector<dash::mpd::ITimeline *> timeline = SegmentTimeline->GetTimelines();
        for (size_t i = 0; i < timeline.size(); i++)
        {
            uint64_t startTime = timeline[i]->GetStartTime();
            uint32_t duration = timeline[i]->GetDuration();
            uint32_t repeatCount = timeline[i]->GetRepeatCount();
            if (i == 0)
            {
                result.push_back(startTime);
                result.push_back(startTime + duration);
                LOGMSG_DEBUG("time: %lu time: %lu", result[0], result[1]);
            }
            else
            {
                result.push_back(result.back() + duration);
                LOGMSG_DEBUG("time: %lu", result.back());
            }
            for (uint32_t j = 0; j < repeatCount; j++)
            {
                result.push_back(result.back() + duration);
                LOGMSG_DEBUG("time: %lu", result.back());
            }
        }
    }
    return result;
}

void DashSegmentSelector::AppendSlash2Path(std::string& inPath)
{
    if (inPath.size() && inPath.back() != '/')
        inPath.push_back('/');
}

void DashSegmentSelector::HandleStringFormat(std::string& mediaStr, uint64_t data, std::string target)
{
    if (mediaStr.find(target) != std::string::npos)
    {
        // get the string format
        size_t numberFormatStart = mediaStr.find(target) + target.size();
        size_t endDollar = mediaStr.find("$", numberFormatStart);
        std::string numberFormat = mediaStr.substr(numberFormatStart, endDollar - numberFormatStart);
        if (numberFormat.size())
        {
            char tempStr[20];
            sprintf(tempStr, numberFormat.c_str(), data);
            ReplaceAllSubstring(mediaStr, target + numberFormat + "$", tempStr);
        }
        else
        {
            ReplaceAllSubstring(mediaStr, target + "$", std::to_string(data));
        }
        LOGMSG_DEBUG("target: %s mediaStr: %s data: %lu", target.c_str(), mediaStr.c_str(), data);
    }
}

void DashSegmentSelector::HandleBaseURL(std::stringstream& ss, const SegmentInfo& targetInfo)
{
    if (targetInfo.Representation.BaseURL.find("http") != std::string::npos)
    {
        ss << targetInfo.Representation.BaseURL;
    }
    else if (targetInfo.AdaptationSet.BaseURL.find("http") != std::string::npos)
    {
        ss << targetInfo.AdaptationSet.BaseURL;
        ss << targetInfo.Representation.BaseURL;
    }
    else if (targetInfo.Period.BaseURL.find("http") != std::string::npos)
    {
        ss << targetInfo.Period.BaseURL;
        ss << targetInfo.AdaptationSet.BaseURL;
        ss << targetInfo.Representation.BaseURL;
    }
    else
    {
        ss << targetInfo.BaseURL;
        ss << targetInfo.Period.BaseURL;
        ss << targetInfo.AdaptationSet.BaseURL;
        ss << targetInfo.Representation.BaseURL;
    }
}

bool DashSegmentSelector::IsStaticMedia(SmartPointer<dash::mpd::IMPD> mpdFile)
{
    return mpdFile->GetType() == "static" ? true : false;
}

void DashSegmentSelector::HandleDynamicMPDRefresh()
{
    if (m_mpdFile->GetMinimumUpdatePeriod().find("Y") == std::string::npos)
    {
        uint64_t minimumUpdatePeriod;
        GetTimeString2MSec(m_mpdFile->GetMinimumUpdatePeriod(), minimumUpdatePeriod);

        LOGMSG_INFO("minimumUpdatePeriod: %s %lu", m_mpdFile->GetMinimumUpdatePeriod().c_str(), minimumUpdatePeriod);

        SmartPointer<PlayerMsg_RefreshMPD> msgRefresh = DynamicCast<PlayerMsg_RefreshMPD>(m_msgFactory.CreateMsg(PlayerMsg_Type_RefreshMPD));
        msgRefresh->SetURL(m_mpdFileURL);
        msgRefresh->SetMinimumUpdatePeriod(minimumUpdatePeriod);
        SendToManager(StaticCast<PlayerMsg_Base>(msgRefresh));
    }
    else
    {
        LOGMSG_INFO("Not a reasonable Minimum update period: %s", m_mpdFile->GetMinimumUpdatePeriod().c_str());
    }
}

void DashSegmentSelector::GetSegmentInfo_Base(const SegmentCriteria& criteria, SegmentInfo& resultInfo)
{
    resultInfo.Representation.bandwidth = 0;
    uint64_t accumulatePeriodDuration = 0;

    std::vector<dash::mpd::IPeriod *> periods = m_mpdFile->GetPeriods();
    for (size_t i = 0; i < periods.size(); i++) // loop for every period
    {
        dash::mpd::IPeriod* period = periods[i];
        // handle accumulate period
        uint64_t durationTimeMSec = 0;
        bool isCorrectPeriod = false;
        if (period->GetDuration().length()) // handle with duration case
        {
            GetTimeString2MSec(period->GetDuration(), durationTimeMSec);
            if (accumulatePeriodDuration <= criteria.downloadTime && criteria.downloadTime < accumulatePeriodDuration + durationTimeMSec) // select correct period
                isCorrectPeriod = true;
            accumulatePeriodDuration += durationTimeMSec;
        }
        else if (period->GetStart().length()) // handle with start case
        {
            uint64_t startTimeMSec, endTimeMSec; startTimeMSec = endTimeMSec = 0;
            GetTimeString2MSec(period->GetStart(), startTimeMSec);
            if (i < periods.size() - 1)
            {
                dash::mpd::IPeriod* nextPeriod = periods[i + 1];
                GetTimeString2MSec(nextPeriod->GetStart(), endTimeMSec);

                if (startTimeMSec <= criteria.downloadTime && criteria.downloadTime < endTimeMSec)
                    isCorrectPeriod = true;
            }
            else
            {
                LOGMSG_WARN("Force to use this period: %s", period->GetId().c_str());
                isCorrectPeriod = true;
            }
        }
        else
            isCorrectPeriod = true;

        if (isCorrectPeriod)
        {
            GetSegmentInfo_Period(criteria, period, resultInfo);
        }
    }

    LOGMSG_INFO("selected bandwidth: %u representation id: %s", resultInfo.Representation.bandwidth, resultInfo.Representation.id.c_str());
}

void DashSegmentSelector::GetSegmentInfo_Period(const SegmentCriteria& criteria, dash::mpd::IPeriod* period, SegmentInfo& resultInfo)
{
    LOGMSG_INFO("IN period id: %s", period->GetId().c_str());
    std::vector<dash::mpd::IAdaptationSet *> adaptationsSets = period->GetAdaptationSets();
    for (size_t j = 0; j < adaptationsSets.size(); j++) // loop for every adaptation set
    {
        dash::mpd::IAdaptationSet* adaptationSet = adaptationsSets[j];
        GetSegmentInfo_AdaptationSet(criteria, period, adaptationSet, resultInfo);
    }
    LOGMSG_DEBUG("OUT");
}

void DashSegmentSelector::GetSegmentInfo_AdaptationSet(const SegmentCriteria& criteria, dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, SegmentInfo& resultInfo)
{
    LOGMSG_DEBUG("IN");
    std::string mimeType = GetMimeType(adaptationSet);
    std::transform(mimeType.begin(), mimeType.end(), mimeType.begin(), ::tolower); // toupper
    if (mimeType.find(criteria.mediaType) != std::string::npos) // found representation
    {
        bool isCorrectAdaptationSet = false;
        std::string sourceLanguage = adaptationSet->GetLang();
        if (sourceLanguage == criteria.language || sourceLanguage == "" || criteria.language == "*") // select language
        {
            isCorrectAdaptationSet = true;
        }

        std::string roleValue = "";
        if (adaptationSet->GetRole().size())
            roleValue = adaptationSet->GetRole()[0]->GetValue();
        if (roleValue == "" || roleValue != "alternate")
            isCorrectAdaptationSet = true;
        else
        {
            isCorrectAdaptationSet = false;
            LOGMSG_INFO("Not a correct adaptation set: %s", roleValue.c_str());
        }

        if (isCorrectAdaptationSet)
        {
            GetSegmentInfo_Representation(criteria, period, adaptationSet, resultInfo);
        }
    }
    LOGMSG_DEBUG("OUT");
}

void DashSegmentSelector::GetSegmentInfo_Representation(const SegmentCriteria& criteria, dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, SegmentInfo& resultInfo)
{
    LOGMSG_DEBUG("IN");
    uint32_t selectedDownloadSize = 0;
    SegmentInfo lowestQualityInfo; lowestQualityInfo.Representation.bandwidth = 0xFFFFFFFF;
    SegmentInfo highestQualityInfo; highestQualityInfo.Representation.bandwidth = 0;

    std::vector<dash::mpd::IRepresentation *> representations = adaptationSet->GetRepresentation();
    dash::mpd::ISegmentTemplate* segmentTemplate = adaptationSet->GetSegmentTemplate();
    for (size_t k = 0; k < representations.size(); k++) // loop for every representation
    {
        dash::mpd::IRepresentation* representation = representations[k];
        uint32_t bandwidth = representation->GetBandwidth();

        if (segmentTemplate)
        {
            uint32_t segmentSize = bandwidth * static_cast<double>(segmentTemplate->GetDuration()) / segmentTemplate->GetTimescale();
            if (selectedDownloadSize < segmentSize && segmentSize < criteria.downloadSize) // if bandwidth is sutiable
            {
                selectedDownloadSize = segmentSize;
                resultInfo = GetSegmentInfo_priv(period, adaptationSet, segmentTemplate, representation);
            }
            if (lowestQualityInfo.Representation.bandwidth > bandwidth)
            {
                lowestQualityInfo = GetSegmentInfo_priv(period, adaptationSet, segmentTemplate, representation);
            }
            if (highestQualityInfo.Representation.bandwidth < bandwidth)
            {
                highestQualityInfo = GetSegmentInfo_priv(period, adaptationSet, segmentTemplate, representation);
            }
        }
        else
        {
            if (highestQualityInfo.Representation.bandwidth < bandwidth)
            {
                highestQualityInfo = GetSegmentInfo_priv(period, adaptationSet, segmentTemplate, representation);
            }
        }
    }

    if (resultInfo.Representation.bandwidth == 0)
    {
        resultInfo = lowestQualityInfo;
    }
    // HACK
    resultInfo = highestQualityInfo;
    LOGMSG_DEBUG("OUT");
}

void DashSegmentSelector::GetMediaDuration(uint64_t& startTime, uint64_t& endTime)
{
    startTime = 0xFFFFFFFFFFFFFFFF;
    endTime = 0;

    std::vector<dash::mpd::IPeriod *> periods = m_mpdFile->GetPeriods();
    for (size_t i = 0; i < periods.size(); i++)
    {
        dash::mpd::IPeriod* period = periods[i];
        uint64_t durationtimeMSec = 0;

        // handle start time
        GetTimeString2MSec(period->GetStart(), durationtimeMSec);
        if (startTime > durationtimeMSec)
            startTime = durationtimeMSec;
        // handle end time
        GetTimeString2MSec(period->GetDuration(), durationtimeMSec);
        endTime += durationtimeMSec;
    }

    if (m_mpdFile->GetMediaPresentationDuration().length())
    {
        GetTimeString2MSec(m_mpdFile->GetMediaPresentationDuration(), endTime);
    }
}

SegmentInfo DashSegmentSelector::GetSegmentInfo_priv(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::ISegmentTemplate* segmentTemplate, dash::mpd::IRepresentation* representation)
{
    SegmentInfo resultInfo;

    // handle MPD attributes
    resultInfo.MPD.minimumUpdatePeriod = m_mpdFile->GetMinimumUpdatePeriod();
    resultInfo.MPD.type = m_mpdFile->GetType();

    // handle outmost baseURL
    dash::mpd::IBaseUrl* BaseURL = m_mpdFile->GetMPDPathBaseUrl();
    resultInfo.BaseURL = BaseURL->GetUrl() + "/";

    // handle period
    resultInfo.Period.BaseURL = period->GetBaseURLs().size() ? period->GetBaseURLs()[0]->GetUrl() : std::string();
    AppendSlash2Path(resultInfo.Period.BaseURL);
    resultInfo.Period.duration = period->GetDuration();
    resultInfo.Period.start = period->GetStart();
    resultInfo.Period.id = period->GetId();
    // handle representation
    resultInfo.Representation.bandwidth = representation->GetBandwidth();
    resultInfo.Representation.id = representation->GetId();
    resultInfo.Representation.BaseURL = representation->GetBaseURLs().size() ? representation->GetBaseURLs()[0]->GetUrl() : std::string();
    // handle adaptationSet
    resultInfo.AdaptationSet.BaseURL = adaptationSet->GetBaseURLs().size() ? adaptationSet->GetBaseURLs()[0]->GetUrl() : std::string();
    resultInfo.AdaptationSet.lang = adaptationSet->GetLang();
    // handle segmentTemplate
    if (segmentTemplate)
    {
        resultInfo.SegmentTemplate.media = segmentTemplate->Getmedia();
        if (resultInfo.SegmentTemplate.media.find("Time") != std::string::npos)
            resultInfo.SegmentTemplate.SegmentTimeline = GetSegmentTimeline(segmentTemplate);
        resultInfo.SegmentTemplate.startNumber = segmentTemplate->GetStartNumber();
        resultInfo.SegmentTemplate.timescale = segmentTemplate->GetTimescale();
        resultInfo.SegmentTemplate.duration = segmentTemplate->GetDuration();
        resultInfo.SegmentTemplate.initialization = segmentTemplate->Getinitialization();
        resultInfo.isHasSegmentTemplate = true;
    }
    else
    {
        resultInfo.isHasSegmentTemplate = false;
    }

    return resultInfo;
}

std::string DashSegmentSelector::GetDownloadURL(dashMediaStatus& mediaStatus, const SegmentInfo& segmentInfo, std::string mediaType)
{
    if (mediaStatus.m_segmentInfo.Representation.id != segmentInfo.Representation.id)
    {
        mediaStatus.m_initFileReady = false;
    }

    if (mediaStatus.m_initFileReady)
    {
        if (!segmentInfo.isHasSegmentTemplate)
            return "Media_EOS";
        return GetSegmentURL(mediaStatus, segmentInfo, mediaType);
    }
    else
    {
        return GetInitFileURL(segmentInfo);
    }
}

std::string DashSegmentSelector::GetSegmentURL(dashMediaStatus& mediaStatus, const SegmentInfo& segmentInfo, std::string mediaType)
{
    std::stringstream ss;
    HandleBaseURL(ss, segmentInfo);

    std::string mediaStr;
    if (segmentInfo.MPD.type == "static")
    {
        mediaStr = GetSegmentURL_Static(mediaStatus, segmentInfo);
    }
    else
    {
        mediaStr = GetSegmentURL_Dynamic(mediaStatus, segmentInfo);
    }

    if (mediaStr == "Media_EOS" || mediaStr == "Media_BOS")
        return mediaStr;

    if (mediaStr.length())
        ss << mediaStr;

    return ss.str();
}

std::string DashSegmentSelector::GetSegmentURL_Static(dashMediaStatus& mediaStatus, const SegmentInfo& targetInfo)
{
    if (targetInfo.isHasSegmentTemplate)
    {
        // check if EOS or BOS
        if (IsEOS(mediaStatus.m_downloadTime, mediaStatus))
            return "Media_EOS";
        if (IsBOS(mediaStatus.m_downloadTime, mediaStatus))
            return "Media_BOS";

        std::string mediaStr = targetInfo.SegmentTemplate.media;
        ReplaceAllSubstring(mediaStr, "$RepresentationID$", targetInfo.Representation.id);

        if (mediaStr.find("$Number") != std::string::npos)
        {
            // get next segment number
            mediaStatus.m_numberSegment = mediaStatus.m_downloadTime / GetSegmentDurationMSec(targetInfo);
            mediaStatus.m_numberSegment += targetInfo.SegmentTemplate.startNumber;

            HandleStringFormat(mediaStr, mediaStatus.m_numberSegment, "$Number"); // $Number%06$
            HandleStringFormat(mediaStr, targetInfo.Representation.bandwidth, "$Bandwidth"); // $Bandwidth%06$
        }
        else if (mediaStr.find("$Time") != std::string::npos)
        {
            GetSegmentNumberFromTimeline(mediaStatus, targetInfo);
            // get the string format
            ReplaceAllSubstring(mediaStr, "$Time$", std::to_string(targetInfo.SegmentTemplate.SegmentTimeline[mediaStatus.m_numberSegment]));
            LOGMSG_DEBUG("%s nextDownloadTime: %lu", mediaStr.c_str(), targetInfo.SegmentTemplate.SegmentTimeline[mediaStatus.m_numberSegment]);
        }
        else
        {
            return "Media_EOS";
        }
        return mediaStr;
    }
    else
    {
        return "";
    }
}

std::string DashSegmentSelector::GetInitFileURL(const SegmentInfo& targetInfo)
{
    std::stringstream ss;
    HandleBaseURL(ss, targetInfo);

    if (targetInfo.isHasSegmentTemplate)
    {
        std::string initStr = targetInfo.SegmentTemplate.initialization;
        ReplaceAllSubstring(initStr, "$RepresentationID$", targetInfo.Representation.id);
        HandleStringFormat(initStr, targetInfo.Representation.bandwidth, "$Bandwidth");
        ss << initStr;
    }

    return ss.str();
}

std::string DashSegmentSelector::GetSegmentURL_Dynamic(dashMediaStatus& mediaStatus, const SegmentInfo& targetInfo)
{
    if (targetInfo.isHasSegmentTemplate)
    {
        std::string mediaStr = targetInfo.SegmentTemplate.media;
        ReplaceAllSubstring(mediaStr, "$RepresentationID$", targetInfo.Representation.id);

        if (mediaStr.find("$Number") != std::string::npos)
        {
            // get next segment number
            mediaStatus.m_numberSegment = mediaStatus.m_downloadTime / GetSegmentDurationMSec(targetInfo);

            HandleStringFormat(mediaStr, mediaStatus.m_numberSegment, "$Number"); // $Number%06$
            HandleStringFormat(mediaStr, targetInfo.Representation.bandwidth, "$Bandwidth"); // $Bandwidth%06$
        }
        else if (mediaStr.find("$Time") != std::string::npos)
        {
            GetSegmentNumberFromTimeline(mediaStatus, targetInfo);
            // get the string format
            ReplaceAllSubstring(mediaStr, "$Time$", std::to_string(targetInfo.SegmentTemplate.SegmentTimeline[mediaStatus.m_numberSegment]));
            HandleStringFormat(mediaStr, targetInfo.Representation.bandwidth, "$Bandwidth"); // $Bandwidth%06$

            LOGMSG_DEBUG("SegmentTimeline: %lu numberSegment: %lu", targetInfo.SegmentTemplate.SegmentTimeline[mediaStatus.m_numberSegment], mediaStatus.m_numberSegment);
        }
        else
        {
            return "Media_EOS";
        }
        return mediaStr;
    }
    else
    {
        return "";
    }
}

void DashSegmentSelector::GetSegmentNumberFromTimeline(dashMediaStatus& mediaStatus, const SegmentInfo& segmentInfo)
{
    // reset segment number
    if (segmentInfo.Period.id != mediaStatus.m_segmentInfo.Period.id)
        mediaStatus.m_numberSegment = 0;
    // get next segment number
    bool found = false;
    uint64_t i = 0;
    for (; i < segmentInfo.SegmentTemplate.SegmentTimeline.size() - 1; i++)
    {
        if (GetSegmentTimeMSec(segmentInfo.SegmentTemplate.SegmentTimeline[i], segmentInfo) <= mediaStatus.m_downloadTime
            && mediaStatus.m_downloadTime < GetSegmentTimeMSec(segmentInfo.SegmentTemplate.SegmentTimeline[i + 1], segmentInfo))
        {
            found = true;
            mediaStatus.m_numberSegment = i;
            break;
        }
    }
    if (!found)
    {
        mediaStatus.m_numberSegment = i;
    }
}

uint64_t DashSegmentSelector::GetNextDownloadTime(const dashMediaStatus& mediaStatus, uint64_t currentDownloadTime)
{
    uint64_t result = 0;
    std::string mediaStr = mediaStatus.m_segmentInfo.SegmentTemplate.media;

    if (mediaStr.find("$Number") != std::string::npos)
    {
        if (m_trickScale > 0)
        {
            result = currentDownloadTime + GetSegmentDurationMSec(mediaStatus.m_segmentInfo);
            if (result < currentDownloadTime + m_trickScale) result = currentDownloadTime + m_trickScale;
            if (result < currentDownloadTime) { LOGMSG_WARN("Overflow"); result = currentDownloadTime; } // overflow case
        }
        else
        {
            result = currentDownloadTime - GetSegmentDurationMSec(mediaStatus.m_segmentInfo);
            if (result > currentDownloadTime + m_trickScale) result = currentDownloadTime + m_trickScale;
            if (result > currentDownloadTime) { LOGMSG_WARN("Overflow"); result = currentDownloadTime; } // overflow case
        }
    }
    else if (mediaStr.find("$Time") != std::string::npos)
    {
        if (m_trickScale > 0)
        {
            if (mediaStatus.m_segmentInfo.SegmentTemplate.SegmentTimeline.size() > mediaStatus.m_numberSegment + 1)
            {
                result = GetSegmentTimeMSec(mediaStatus.m_segmentInfo.SegmentTemplate.SegmentTimeline[mediaStatus.m_numberSegment + 1], mediaStatus.m_segmentInfo);
                if (result < currentDownloadTime + m_trickScale) result = currentDownloadTime + m_trickScale;
                if (result < currentDownloadTime) result = GetSegmentTimeMSec(mediaStatus.m_segmentInfo.SegmentTemplate.SegmentTimeline[mediaStatus.m_numberSegment + 1], mediaStatus.m_segmentInfo); // overflow case
            }
            else
            {
                result = currentDownloadTime + m_trickScale;
                if (result < currentDownloadTime) { LOGMSG_WARN("Overflow"); result = currentDownloadTime; } // overflow case
            }
        }
        else
        {
            if (0 <= mediaStatus.m_numberSegment - 1)
            {
                result = GetSegmentTimeMSec(mediaStatus.m_segmentInfo.SegmentTemplate.SegmentTimeline[mediaStatus.m_numberSegment - 1], mediaStatus.m_segmentInfo);
                if (result > currentDownloadTime + m_trickScale) result = currentDownloadTime + m_trickScale;
                if (result > currentDownloadTime) result = GetSegmentTimeMSec(mediaStatus.m_segmentInfo.SegmentTemplate.SegmentTimeline[mediaStatus.m_numberSegment - 1], mediaStatus.m_segmentInfo); // overflow case
            }
            else
            {
                result = currentDownloadTime + m_trickScale;
                if (result > currentDownloadTime) { LOGMSG_WARN("Overflow"); result = currentDownloadTime; } // overflow case
            }
        }
    }
    else
    {
        LOGMSG_ERROR("No way to get next download time");
    }

    return result;
}

void DashSegmentSelector::PrintTimeline(const std::vector<uint64_t>& timeline)
{
    LOGMSG_INFO(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>");
    for (size_t i = 0; i < timeline.size(); i++)
    {
        LOGMSG_INFO("i: %lu %lu", i, timeline[i]);
    }
    LOGMSG_INFO("<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<");
}

std::string DashSegmentSelector::GetMimeType(dash::mpd::IAdaptationSet* adaptationSet)
{
    std::string mimeType = adaptationSet->GetMimeType();
    if (mimeType == "")
    {
        std::vector<dash::mpd::IRepresentation *> representations = adaptationSet->GetRepresentation();
        if (representations.size())
        {
            dash::mpd::IRepresentation* representation = representations.front();
            mimeType = representation->GetMimeType();
        }
    }
    return mimeType;
}

int32_t DashSegmentSelector::GetCurrentTimeZone()
{
    time_t curTime = std::time(0);
    LOGMSG_INFO("time zone checking utc: %s", asctime(gmtime(&curTime)));
    LOGMSG_INFO("time zone checking local: %s", asctime(localtime(&curTime)));

    struct tm* gmTM = gmtime(&curTime);
    time_t gmTime = mktime(gmTM);
    struct tm* localTM = localtime(&curTime);
    time_t localTime = mktime(localTM);

    LOGMSG_INFO("Time diff: %d", static_cast<int32_t>(localTime - gmTime));
    return static_cast<int32_t>((localTime - gmTime) / 3600.0 + 0.5);
}

uint64_t DashSegmentSelector::GetCurrentDownloadTime(uint32_t liveDelayMSec, uint32_t timeShiftBufferDepthMSec)
{
    if (timeShiftBufferDepthMSec)
        timeShiftBufferDepthMSec -= 5000; // Here we reduce 5 second so that we don't pick the edge
    struct timeval curTV;
    struct timezone curTZ;
    gettimeofday(&curTV, &curTZ);
    uint64_t startMSec = 0; GetDateTimeString2MSec(m_mpdFile->GetAvailabilityStarttime(), startMSec);
    LOGMSG_INFO("startMSec: %s %lu currentTime: %lu tz_minuteswest: %d tz_dsttime: %d", m_mpdFile->GetAvailabilityStarttime().c_str(), startMSec, static_cast<uint64_t>((curTV.tv_sec * 1000 + curTV.tv_usec / 1000.0) + 0.5), curTZ.tz_minuteswest, curTZ.tz_dsttime);

    if (startMSec)
        return (static_cast<uint64_t>(curTV.tv_sec) * 1000 + curTV.tv_usec / 1000.0) + 0.5 - liveDelayMSec - timeShiftBufferDepthMSec - startMSec - (GetCurrentTimeZone() * 3600 * 1000);
    else
        return (static_cast<uint64_t>(curTV.tv_sec) * 1000 + curTV.tv_usec / 1000.0) + 0.5 - liveDelayMSec - timeShiftBufferDepthMSec;
}

bool DashSegmentSelector::IsDownloadTimeTooOld(const uint64_t& currentDownloadTime)
{
    if (IsStaticMedia(m_mpdFile))
        return false;
    else
    {
        uint64_t timeShiftBufferDepthMSec = 0;
        GetTimeString2MSec(m_mpdFile->GetTimeShiftBufferDepth(), timeShiftBufferDepthMSec);
        uint64_t edgeDownloadTime = GetCurrentDownloadTime(0, timeShiftBufferDepthMSec);
        if (edgeDownloadTime >= currentDownloadTime)
            return true;
        else
            return false;
    }
}

bool DashSegmentSelector::IsDownloadTimeTooEarly(const uint64_t& currentDownloadTime)
{
    if (IsStaticMedia(m_mpdFile))
        return false;
    else
    {
        uint64_t latestDownloadTime = GetCurrentDownloadTime(0, 0);
        if (latestDownloadTime <= currentDownloadTime)
            return true;
        else
            return false;
    }
}

std::string DashSegmentSelector::IsDownloadTimeValid(const uint64_t& currentDownloadTime)
{
    uint64_t mediaStartTime, mediaEndTime; mediaStartTime = mediaEndTime = 0; GetMediaDuration(mediaStartTime, mediaEndTime);
    uint64_t timeShiftBufferDepthMSec = 0; GetTimeString2MSec(m_mpdFile->GetTimeShiftBufferDepth(), timeShiftBufferDepthMSec);
    uint64_t oldestDownloadTime = GetCurrentDownloadTime(0, timeShiftBufferDepthMSec);
    uint64_t latestDownloadTime = GetCurrentDownloadTime(0, 0);
    if (mediaEndTime && mediaEndTime <= currentDownloadTime)
    {
        LOGMSG_INFO("Media_EOS %s mediaEndTime: %lu downloadTime: %lu", IsStaticMedia(m_mpdFile) ? "VOD" : "Live", mediaEndTime, currentDownloadTime);
        return IsStaticMedia(m_mpdFile) ? "Live_Media_EOS" : "Media_EOS";
    }
    else if (currentDownloadTime < mediaStartTime)
    {
        LOGMSG_INFO("Media_BOS %s mediaStartTime: %lu downloadTime: %lu", IsStaticMedia(m_mpdFile) ? "VOD" : "Live", mediaStartTime, currentDownloadTime);
        return IsStaticMedia(m_mpdFile) ? "Live_Media_BOS" : "Media_BOS";
    }
    else if (!IsStaticMedia(m_mpdFile) && latestDownloadTime <= currentDownloadTime)
    {
        LOGMSG_INFO("Media_EOS %s latestDownloadTime: %lu downloadTime: %lu", IsStaticMedia(m_mpdFile) ? "VOD" : "Live", latestDownloadTime, currentDownloadTime);
        return IsStaticMedia(m_mpdFile) ? "Live_Media_EOS" : "Media_EOS";
    }
    else if (!IsStaticMedia(m_mpdFile) && oldestDownloadTime >= currentDownloadTime)
    {
        LOGMSG_INFO("Media_BOS %s oldestDownloadTime: %lu downloadTime: %lu", IsStaticMedia(m_mpdFile) ? "VOD" : "Live", oldestDownloadTime, currentDownloadTime);
        return IsStaticMedia(m_mpdFile) ? "Live_Media_BOS" : "Media_BOS";
    }
    else
    {
        return "";
    }
}
