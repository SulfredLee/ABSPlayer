#include "dashSegmentSelector.h"
#include "Logger.h"
#include <algorithm>
#include <ctime>

dashSegmentSelector::dashSegmentSelector()
{
    m_mpdFile = nullptr;
    InitStatus(m_videoStatus);
    InitStatus(m_audioStatus);
    InitStatus(m_subtitleStatus);
}

dashSegmentSelector::~dashSegmentSelector()
{}

void dashSegmentSelector::InitComponent(cmdReceiver* manager)
{
    segmentSelector::InitComponent(manager);
}

void dashSegmentSelector::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadMPD> msg)
{
    m_mpdFile = msg->GetAndMoveMPDFile();
    m_mpdFileURL = msg->GetURL();

    if (!IsStaticMedia(m_mpdFile))
        HandleDynamicMPDRefresh();

    // handle download process status
    m_videoStatus.m_numberSegment = 0;
    m_audioStatus.m_numberSegment = 0;
}

void dashSegmentSelector::ProcessMsg(std::shared_ptr<PlayerMsg_RefreshMPD> msg)
{
    m_mpdFile = msg->GetAndMoveMPDFile();
    HandleDynamicMPDRefresh();
}

void dashSegmentSelector::ProcessMsg(std::shared_ptr<PlayerMsg_Play> msg)
{
    // get current player status
    std::shared_ptr<PlayerMsg_Base> msgBase = m_msgFactory.CreateMsg(PlayerMsg_Type_GetPlayerStage);
    SendToManager(msgBase);
    std::shared_ptr<PlayerMsg_GetPlayerStage> msgGetStage = std::dynamic_pointer_cast<PlayerMsg_GetPlayerStage>(msgBase);
    if (msgGetStage->GetPlayerStage() == PlayerStage_Play)
    {
        HandleVideoSegment();
        HandleAudioSegment();
        HandleSubtitleSegment();
    }
    else
    {
        LOGMSG_ERROR("Cannot play movie. Current player stage is: %u", msgGetStage->GetPlayerStage());
    }
}

void dashSegmentSelector::ProcessMsg(std::shared_ptr<PlayerMsg_Pause> msg)
{
}

void dashSegmentSelector::ProcessMsg(std::shared_ptr<PlayerMsg_Stop> msg)
{
}

void dashSegmentSelector::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadFinish> msg)
{
    switch(msg->GetFileType())
    {
        case PlayerMsg_Type_DownloadVideo:
            {
                if (msg->GetResponseCode() == 200)
                {
                    m_videoStatus.m_downloadSpeed = msg->GetSpeed();
                    m_videoStatus.m_downloadTime = msg->GetDownloadTime();
                    if (!m_videoStatus.m_initFileReady)
                        m_videoStatus.m_initFileReady = true;
                    else
                        m_videoStatus.m_numberSegment++;
                }
                break;
            }
        case PlayerMsg_Type_DownloadAudio:
            {
                if (msg->GetResponseCode() == 200)
                {
                    m_audioStatus.m_downloadSpeed = msg->GetSpeed();
                    m_audioStatus.m_downloadTime = msg->GetDownloadTime();
                    if (!m_audioStatus.m_initFileReady)
                        m_audioStatus.m_initFileReady = true;
                    else
                        m_audioStatus.m_numberSegment++;
                }
                break;
            }
        case PlayerMsg_Type_DownloadSubtitle:
        default:
            break;
    }
}

void dashSegmentSelector::ProcessMsg(std::shared_ptr<PlayerMsg_ProcessNextSegment> msg)
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

void dashSegmentSelector::InitStatus(dashMediaStatus& status)
{
    status.m_initFileReady = false;
    status.m_downloadSpeed = 0;
    status.m_downloadTime = 0;
    status.m_playTime = 0;
    status.m_numberSegment = 0;
    status.m_mediaStartTime = 0;
    status.m_mediaEndTime = 0;
    status.m_prePeriodID = "";
}

void dashSegmentSelector::HandleVideoSegment()
{
    // get target criteria
    uint32_t targetDownloadSize = GetTargetDownloadSize(m_videoStatus, "Video");
    // get url information
    segmentInfo videoSegmentInfo;
    segmentSelectorRet ret = GetSegmentInfo(targetDownloadSize, "video", "*", m_videoStatus, videoSegmentInfo);
    if (ret == SegmentSelector_EOS)
    {
        LOGMSG_INFO("Video_EOS");
        return;
    }
    else if (ret == SegmentSelector_BOS)
    {
        LOGMSG_INFO("Video_BOS");
        return;
    }
    // generate url
    uint64_t nextDownloadTime;
    std::string targetURL = GetDownloadURL(m_videoStatus, videoSegmentInfo, nextDownloadTime, "video");

    LOGMSG_INFO("targetURL: %s", targetURL.c_str());

    if (targetURL == "Media_EOS")
    {
        LOGMSG_INFO("Video_EOS");
        return;
    }
    if (targetURL == "Media_BOS")
    {
        LOGMSG_INFO("Video_EOS");
        return;
    }
    if (targetURL.length())
    {
        // send download message
        std::shared_ptr<PlayerMsg_DownloadFile> msgDVideo = std::dynamic_pointer_cast<PlayerMsg_DownloadFile>(m_msgFactory.CreateMsg(PlayerMsg_Type_DownloadVideo));
        msgDVideo->SetURL(targetURL);
        msgDVideo->SetDownloadTime(nextDownloadTime);
        SendToManager(msgDVideo);
    }
}

void dashSegmentSelector::HandleAudioSegment()
{
    // get target criteria
    uint32_t targetDownloadSize = GetTargetDownloadSize(m_audioStatus, "Audio");
    // get url information
    segmentInfo audioSegmentInfo;
    segmentSelectorRet ret = GetSegmentInfo(targetDownloadSize, "audio", "*", m_audioStatus, audioSegmentInfo);
    if (ret == SegmentSelector_EOS)
    {
        LOGMSG_INFO("Audio_EOS");
        return;
    }
    else if (ret == SegmentSelector_BOS)
    {
        LOGMSG_INFO("Audio_BOS");
        return;
    }
    // generate url
    uint64_t nextDownloadTime;
    std::string targetURL = GetDownloadURL(m_audioStatus, audioSegmentInfo, nextDownloadTime, "audio");

    LOGMSG_INFO("targetURL: %s", targetURL.c_str());

    if (targetURL == "Media_EOS")
    {
        LOGMSG_INFO("Audio_EOS");
        return;
    }
    if (targetURL == "Media_BOS")
    {
        LOGMSG_INFO("Audio_BOS");
        return;
    }
    if (targetURL.length())
    {
        // send download message
        std::shared_ptr<PlayerMsg_DownloadFile> msgDAudio = std::dynamic_pointer_cast<PlayerMsg_DownloadFile>(m_msgFactory.CreateMsg(PlayerMsg_Type_DownloadAudio));
        msgDAudio->SetURL(targetURL);
        msgDAudio->SetDownloadTime(nextDownloadTime);
        SendToManager(msgDAudio);
    }
}

void dashSegmentSelector::HandleSubtitleSegment()
{
    if (m_subtitleStatus.m_initFileReady)
    {
    }
    else
    {
        // DownloadInitFile_Subtitle();
    }
}

bool dashSegmentSelector::ReplaceSubstring(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void dashSegmentSelector::ReplaceAllSubstring(std::string& str, const std::string& from, const std::string& to)
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

uint32_t dashSegmentSelector::GetSegmentDurationMSec(const segmentInfo& inDownloadInfo)
{
    return GetSegmentTimeMSec(inDownloadInfo.SegmentTemplate.duration, inDownloadInfo);
}

uint32_t dashSegmentSelector::GetSegmentTimeMSec(const uint64_t& inTime, const segmentInfo& inDownloadInfo)
{
    if (inDownloadInfo.SegmentTemplate.timescale > 0)
        return (static_cast<double>(inTime) / inDownloadInfo.SegmentTemplate.timescale) * 1000;
    else
        return inTime * 1000;
}

uint32_t dashSegmentSelector::GetTargetDownloadSize(const dashMediaStatus& mediaStatus, std::string mediaType)
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

bool dashSegmentSelector::IsEOS(const uint64_t& nextDownloadTime, const dashMediaStatus& inMediaStatus)
{
    LOGMSG_DEBUG("nextDownloadTime: %lu endTime: %lu", nextDownloadTime, inMediaStatus.m_mediaEndTime);
    if (inMediaStatus.m_segmentInfo.Period.duration.length())
        return nextDownloadTime > inMediaStatus.m_mediaEndTime;
    else
        return false;
}

bool dashSegmentSelector::IsBOS(const uint64_t& nextDownloadTime, const dashMediaStatus& inMediaStatus)
{
    if (inMediaStatus.m_segmentInfo.Period.start.length())
        return nextDownloadTime < inMediaStatus.m_mediaStartTime;
    else
        return false;
}

bool dashSegmentSelector::GetTimeString2MSec(std::string timeStr, uint64_t& timeMSec)
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

std::vector<uint32_t> dashSegmentSelector::GetSegmentTimeline(dash::mpd::ISegmentTemplate* segmentTemplate)
{
    std::vector<uint32_t> result;
    dash::mpd::ISegmentTimeline const * SegmentTimeline = segmentTemplate->GetSegmentTimeline();
    if (SegmentTimeline)
    {
        std::vector<dash::mpd::ITimeline *> timeline = SegmentTimeline->GetTimelines();
        for (size_t i = 0; i < timeline.size(); i++)
        {
            uint32_t startTime = timeline[i]->GetStartTime();
            uint32_t duration = timeline[i]->GetDuration();
            uint32_t repeatCount = timeline[i]->GetRepeatCount();
            if (i == 0)
            {
                result.push_back(startTime);
                result.push_back(startTime + duration);
                LOGMSG_DEBUG("time: %u time: %u", result[0], result[1]);
            }
            else
            {
                result.push_back(result.back() + duration);
                LOGMSG_DEBUG("time: %u", result.back());
            }
            for (uint32_t j = 0; j < repeatCount; j++)
            {
                result.push_back(result.back() + duration);
                LOGMSG_DEBUG("time: %u", result.back());
            }
        }
    }
    return result;
}

void dashSegmentSelector::AppendSlash2Path(std::string& inPath)
{
    if (inPath.size() && inPath.back() != '/')
        inPath.push_back('/');
}

void dashSegmentSelector::HandleStringFormat(std::string& mediaStr, uint32_t data, std::string target)
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
        LOGMSG_DEBUG("target: %s mediaStr: %s data: %u", target.c_str(), mediaStr.c_str(), data);
    }
}

void dashSegmentSelector::HandleBaseURL(std::stringstream& ss, const segmentInfo& targetInfo)
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

bool dashSegmentSelector::IsStaticMedia(std::shared_ptr<dash::mpd::IMPD> mpdFile)
{
    return mpdFile->GetType() == "static" ? true : false;
}

void dashSegmentSelector::HandleDynamicMPDRefresh()
{
    if (m_mpdFile->GetMinimumUpdatePeriod().find("Y") == std::string::npos)
    {
        uint64_t minimumUpdatePeriod;
        GetTimeString2MSec(m_mpdFile->GetMinimumUpdatePeriod(), minimumUpdatePeriod);

        LOGMSG_INFO("minimumUpdatePeriod: %lu", minimumUpdatePeriod);

        std::shared_ptr<PlayerMsg_RefreshMPD> msgRefresh = std::dynamic_pointer_cast<PlayerMsg_RefreshMPD>(m_msgFactory.CreateMsg(PlayerMsg_Type_RefreshMPD));
        msgRefresh->SetURL(m_mpdFileURL);
        msgRefresh->SetMinimumUpdatePeriod(minimumUpdatePeriod);
        SendToManager(msgRefresh);
    }
    else
    {
        LOGMSG_INFO("Not a reasonable Minimum update period: %s", m_mpdFile->GetMinimumUpdatePeriod().c_str());
    }
}

segmentSelectorRet dashSegmentSelector::GetSegmentInfo(uint32_t targetDownloadSize, std::string mediaType, std::string language, dashMediaStatus& mediaStatus, segmentInfo& resultInfo)
{
    segmentSelectorRet ret = SegmentSelector_Okay;

    uint32_t selectedDownloadSize = 0;
    resultInfo.Representation.bandwidth = 0;
    segmentInfo lowestQualityInfo; lowestQualityInfo.Representation.bandwidth = 0xFFFFFFFF;
    segmentInfo highestQualityInfo; highestQualityInfo.Representation.bandwidth = 0;
    uint64_t accumulatePeriodDuration = 0;
    mediaStatus.m_mediaEndTime = 0;
    mediaStatus.m_mediaStartTime = 0;
    bool isHasPeriodDuration = false;
    // handle criteria
    std::string mediaType_Target = mediaType;
    std::string language_Target = language; // "eng"
    uint64_t currentDownloadedTime = mediaStatus.m_downloadTime;

    std::vector<dash::mpd::IPeriod *> periods = m_mpdFile->GetPeriods();
    for (size_t i = 0; i < periods.size(); i++) // loop for every period
    {
        dash::mpd::IPeriod* period = periods[i];
        // handle accumulate period
        uint64_t timeMSec = 0;
        GetTimeString2MSec(period->GetDuration(), timeMSec);
        // handle media start time
        if (i == 0 && period->GetStart().length())
        {
            GetTimeString2MSec(period->GetStart(), mediaStatus.m_mediaStartTime);
        }
        bool isCorrectPeriod = false;
        if (period->GetDuration().length()) // handle with duration case
        {
            isHasPeriodDuration = true;
            if (accumulatePeriodDuration <= currentDownloadedTime && currentDownloadedTime < accumulatePeriodDuration + timeMSec) // select correct period
                isCorrectPeriod = true;
        }
        else // handle no duration case
            isCorrectPeriod = true;
        if (isCorrectPeriod)
        {
            std::vector<dash::mpd::IAdaptationSet *> adaptationsSets = period->GetAdaptationSets();
            for (size_t j = 0; j < adaptationsSets.size(); j++) // loop for every adaptation set
            {
                dash::mpd::IAdaptationSet* adaptationSet = adaptationsSets[j];
                std::string mimeType = adaptationSet->GetMimeType();
                std::transform(mimeType.begin(), mimeType.end(), mimeType.begin(), ::tolower); // toupper
                if (mimeType.find(mediaType_Target) != std::string::npos) // found representation
                {
                    std::string sourceLanguage = adaptationSet->GetLang();
                    if (sourceLanguage == language_Target || sourceLanguage == "" || language_Target == "*") // select language
                    {
                        std::vector<dash::mpd::IRepresentation *> representations = adaptationsSets[j]->GetRepresentation();
                        dash::mpd::ISegmentTemplate* segmentTemplate = adaptationSet->GetSegmentTemplate();
                        for (size_t k = 0; k < representations.size(); k++) // loop for every representation
                        {
                            dash::mpd::IRepresentation* representation = representations[k];
                            uint32_t bandwidth = representation->GetBandwidth();

                            if (segmentTemplate)
                            {
                                uint32_t segmentSize = bandwidth * static_cast<double>(segmentTemplate->GetDuration()) / segmentTemplate->GetTimescale();
                                if (selectedDownloadSize < segmentSize && segmentSize < targetDownloadSize) // if bandwidth is sutiable
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
                    }
                }
            }
        }
        accumulatePeriodDuration += timeMSec;
    }
    // handle media end time (MSec)
    mediaStatus.m_mediaEndTime = accumulatePeriodDuration;
    if (m_mpdFile->GetMediaPresentationDuration().length())
    {
        isHasPeriodDuration = true;
        GetTimeString2MSec(m_mpdFile->GetMediaPresentationDuration(), mediaStatus.m_mediaEndTime);
    }

    if ((isHasPeriodDuration || m_mpdFile->GetMediaPresentationDuration().length()) && mediaStatus.m_mediaEndTime <= currentDownloadedTime)
        ret = SegmentSelector_EOS;
    if (resultInfo.Representation.bandwidth == 0)
    {
        resultInfo = lowestQualityInfo;
    }
    // HACK
    resultInfo = highestQualityInfo;
    LOGMSG_INFO("selected bandwidth: %u representation id: %s", resultInfo.Representation.bandwidth, resultInfo.Representation.id.c_str());

    return ret;
}

segmentInfo dashSegmentSelector::GetSegmentInfo_priv(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::ISegmentTemplate* segmentTemplate, dash::mpd::IRepresentation* representation)
{
    segmentInfo resultInfo;

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

std::string dashSegmentSelector::GetDownloadURL(dashMediaStatus& mediaStatus, const segmentInfo& segmentInfo, uint64_t& nextDownloadTime, std::string mediaType)
{
    if (mediaStatus.m_segmentInfo.Representation.id != segmentInfo.Representation.id)
    {
        mediaStatus.m_initFileReady = false;
    }
    mediaStatus.m_segmentInfo = segmentInfo;

    if (mediaStatus.m_initFileReady)
    {
        if (!segmentInfo.isHasSegmentTemplate)
            return "Media_EOS";
        return GetSegmentURL(mediaStatus, segmentInfo, nextDownloadTime, mediaType);
    }
    else
    {
        nextDownloadTime = mediaStatus.m_downloadTime;
        return GetInitFileURL(segmentInfo);
    }
}

std::string dashSegmentSelector::GetSegmentURL(dashMediaStatus& mediaStatus, const segmentInfo& segmentInfo, uint64_t& nextDownloadTime, std::string mediaType)
{
    std::stringstream ss;
    HandleBaseURL(ss, segmentInfo);

    std::string mediaStr;
    if (segmentInfo.MPD.type == "static")
    {
        mediaStr = GetSegmentURL_Static(mediaStatus, segmentInfo, nextDownloadTime);
    }
    else
    {
        mediaStr = GetSegmentURL_Dynamic(mediaStatus, segmentInfo, nextDownloadTime);
    }

    if (mediaStr == "Media_EOS" || mediaStr == "Media_BOS")
        return mediaStr;

    if (mediaStr.length())
        ss << mediaStr;

    return ss.str();
}

std::string dashSegmentSelector::GetSegmentURL_Static(dashMediaStatus& mediaStatus, const segmentInfo& segmentInfo, uint64_t& nextDownloadTime)
{
    if (segmentInfo.isHasSegmentTemplate)
    {
        std::string mediaStr = segmentInfo.SegmentTemplate.media;
        ReplaceAllSubstring(mediaStr, "$RepresentationID$", segmentInfo.Representation.id);

        if (mediaStr.find("$Number") != std::string::npos)
        {
            // get next segment number
            if (segmentInfo.SegmentTemplate.startNumber > mediaStatus.m_numberSegment || segmentInfo.Period.id != mediaStatus.m_prePeriodID)
                mediaStatus.m_numberSegment = segmentInfo.SegmentTemplate.startNumber;
            // handle period id
            mediaStatus.m_prePeriodID = segmentInfo.Period.id;
            uint32_t nextSegment = mediaStatus.m_numberSegment;
            // get next download time
            nextDownloadTime = mediaStatus.m_downloadTime + GetSegmentDurationMSec(segmentInfo);
            // check if EOS or BOS
            if (IsEOS(nextDownloadTime, mediaStatus))
                return "Media_EOS";
            if (IsBOS(nextDownloadTime, mediaStatus))
                return "Media_BOS";
            HandleStringFormat(mediaStr, nextSegment, "$Number"); // $Number%06$
            HandleStringFormat(mediaStr, segmentInfo.Representation.bandwidth, "$Bandwidth"); // $Bandwidth%06$
        }
        else if (mediaStr.find("$Time") != std::string::npos)
        {
            // get next segment number
            uint32_t nextSegment = mediaStatus.m_numberSegment;
            // get next download time
            if (nextSegment < segmentInfo.SegmentTemplate.SegmentTimeline.size())
                nextDownloadTime = GetSegmentTimeMSec(segmentInfo.SegmentTemplate.SegmentTimeline[nextSegment], segmentInfo);
            else
            {
                LOGMSG_ERROR("Out of range audio");
                return "Media_EOS";
            }
            // check if EOS or BOS
            if (IsEOS(nextDownloadTime, mediaStatus))
                return "Media_EOS";
            if (IsBOS(nextDownloadTime, mediaStatus))
                return "Media_BOS";
            // get the string format
            ReplaceAllSubstring(mediaStr, "$Time$", std::to_string(segmentInfo.SegmentTemplate.SegmentTimeline[nextSegment]));
            LOGMSG_DEBUG("%s nextDownloadTime: %u", mediaStr.c_str(), segmentInfo.SegmentTemplate.SegmentTimeline[nextSegment]);
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

std::string dashSegmentSelector::GetInitFileURL(const segmentInfo& targetInfo)
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

std::string dashSegmentSelector::GetSegmentURL_Dynamic(dashMediaStatus& mediaStatus, const segmentInfo& targetInfo, uint64_t& nextDownloadTime)
{
    if (targetInfo.isHasSegmentTemplate)
    {
        std::string mediaStr = targetInfo.SegmentTemplate.media;
        ReplaceAllSubstring(mediaStr, "$RepresentationID$", targetInfo.Representation.id);

        if (mediaStr.find("$Number") != std::string::npos)
        {
            // get next segment number
            std::time_t now = std::time(0); // get current time Sec counted from 1970 01 01 00:00
            uint32_t nextSegment;
            if (mediaStatus.m_numberSegment == 0)
                mediaStatus.m_numberSegment = (now - 5) / targetInfo.SegmentTemplate.duration;
            nextSegment = mediaStatus.m_numberSegment;
            // get next download time
            nextDownloadTime = now;

            HandleStringFormat(mediaStr, nextSegment, "$Number"); // $Number%06$
            HandleStringFormat(mediaStr, targetInfo.Representation.bandwidth, "$Bandwidth"); // $Bandwidth%06$
        }
        return mediaStr;
    }
    else
    {
        return "";
    }
}
