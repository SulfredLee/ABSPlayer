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
    uint32_t targetDownloadSize = GetTargetDownloadSize_Video();
    // get url information
    segmentInfo videoSegmentInfo;
    segmentSelectorRet ret = GetSegmentInfo_Video(targetDownloadSize, videoSegmentInfo);
    if (ret == SegmentSelector_Video_EOS)
    {
        LOGMSG_INFO("Video_EOS");
        return;
    }
    else if (ret == SegmentSelector_Video_BOS)
    {
        LOGMSG_INFO("Video_BOS");
        return;
    }
    // generate url
    uint64_t nextDownloadTime;
    std::string targetURL = GetDownloadURL_Video(videoSegmentInfo, nextDownloadTime);

    LOGMSG_INFO("targetURL: %s", targetURL.c_str());

    if (targetURL == "Video_EOS")
    {
        LOGMSG_INFO("Video_EOS");
        return;
    }
    if (targetURL == "Video_BOS")
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
    uint32_t targetDownloadSize = GetTargetDownloadSize_Audio();
    // get url information
    segmentInfo audioSegmentInfo;
    segmentSelectorRet ret = GetSegmentInfo_Audio(targetDownloadSize, audioSegmentInfo);
    if (ret == SegmentSelector_Audio_EOS)
    {
        LOGMSG_INFO("Audio_EOS");
        return;
    }
    else if (ret == SegmentSelector_Audio_BOS)
    {
        LOGMSG_INFO("Audio_BOS");
        return;
    }
    // generate url
    uint64_t nextDownloadTime;
    std::string targetURL = GetDownloadURL_Audio(audioSegmentInfo, nextDownloadTime);

    LOGMSG_INFO("targetURL: %s", targetURL.c_str());

    if (targetURL == "Audio_EOS")
    {
        LOGMSG_INFO("Audio_EOS");
        return;
    }
    if (targetURL == "Audio_BOS")
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

uint32_t dashSegmentSelector::GetTargetDownloadSize_Video()
{
    return GetTargetDownloadSize(m_videoStatus, "Video");
}

segmentSelectorRet dashSegmentSelector::GetSegmentInfo_Video(uint32_t targetDownloadSize, segmentInfo& resultInfo)
{
    segmentSelectorRet ret = SegmentSelector_Okay;

    uint32_t selectedDownloadSize = 0;
    resultInfo.Representation.bandwidth = 0;
    segmentInfo lowestQualityInfo; lowestQualityInfo.Representation.bandwidth = 0xFFFFFFFF;
    segmentInfo highestQualityInfo; highestQualityInfo.Representation.bandwidth = 0;
    uint64_t accumulatePeriodDuration = 0;
    m_videoStatus.m_mediaEndTime = 0;
    m_videoStatus.m_mediaStartTime = 0;
    bool isHasPeriodDuration = false;
    // handle criteria
    std::string mediaType_Target = "video";
    uint64_t currentDownloadedTime = m_videoStatus.m_downloadTime;

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
            GetTimeString2MSec(period->GetStart(), m_videoStatus.m_mediaStartTime);
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
                if (mimeType.find(mediaType_Target) != std::string::npos) // found video representation
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
                                resultInfo = GetSegmentInfo_priv_Video(period, adaptationSet, segmentTemplate, representation);
                            }
                            if (lowestQualityInfo.Representation.bandwidth > bandwidth)
                            {
                                lowestQualityInfo = GetSegmentInfo_priv_Video(period, adaptationSet, segmentTemplate, representation);
                            }
                            if (highestQualityInfo.Representation.bandwidth < bandwidth)
                            {
                                highestQualityInfo = GetSegmentInfo_priv_Video(period, adaptationSet, segmentTemplate, representation);
                            }
                        }
                        else
                        {
                            if (highestQualityInfo.Representation.bandwidth < bandwidth)
                            {
                                highestQualityInfo = GetSegmentInfo_priv_Video(period, adaptationSet, segmentTemplate, representation);
                            }
                        }
                    }
                }
            }
        }
        accumulatePeriodDuration += timeMSec;
    }
    // handle media end time (MSec)
    m_videoStatus.m_mediaEndTime = accumulatePeriodDuration;
    if (m_mpdFile->GetMediaPresentationDuration().length())
    {
        isHasPeriodDuration = true;
        GetTimeString2MSec(m_mpdFile->GetMediaPresentationDuration(), m_videoStatus.m_mediaEndTime);
    }

    if ((isHasPeriodDuration || m_mpdFile->GetMediaPresentationDuration().length()) && m_videoStatus.m_mediaEndTime <= currentDownloadedTime)
        ret = SegmentSelector_Video_EOS;
    if (resultInfo.Representation.bandwidth == 0)
    {
        resultInfo = lowestQualityInfo;
    }
    // HACK
    resultInfo = highestQualityInfo;
    LOGMSG_INFO("selected bandwidth: %u", resultInfo.Representation.bandwidth);

    return ret;
}

std::string dashSegmentSelector::GetDownloadURL_Video(const segmentInfo& videoSegmentInfo, uint64_t& nextDownloadTime)
{
    if (m_videoStatus.m_segmentInfo.Representation.id != videoSegmentInfo.Representation.id)
    {
        m_videoStatus.m_initFileReady = false;
    }
    m_videoStatus.m_segmentInfo = videoSegmentInfo;

    if (m_videoStatus.m_initFileReady)
    {
        if (!videoSegmentInfo.isHasSegmentTemplate)
            return "Video_EOS";
        return GetSegmentURL_Video(videoSegmentInfo, nextDownloadTime);
    }
    else
    {
        nextDownloadTime = m_videoStatus.m_downloadTime;
        return GetInitFileURL_Video(videoSegmentInfo);
    }
}

std::string dashSegmentSelector::GetInitFileURL_Video(const segmentInfo& targetInfo)
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

std::string dashSegmentSelector::GetSegmentURL_Video(const segmentInfo& videoSegmentInfo, uint64_t& nextDownloadTime)
{
    std::stringstream ss;
    HandleBaseURL(ss, videoSegmentInfo);

    std::string mediaStr;
    if (videoSegmentInfo.MPD.type == "static")
    {
        mediaStr = GetSegmentURL_Video_Static(videoSegmentInfo, nextDownloadTime);
    }
    else
    {
        mediaStr = GetSegmentURL_Video_Dynamic(videoSegmentInfo, nextDownloadTime);
    }

    if (mediaStr == "Video_EOS" || mediaStr == "Video_BOS")
        return mediaStr;

    if (mediaStr.length())
        ss << mediaStr;

    return ss.str();
}

std::string dashSegmentSelector::GetSegmentURL_Video_Static(const segmentInfo& videoSegmentInfo, uint64_t& nextDownloadTime)
{
    if (videoSegmentInfo.isHasSegmentTemplate)
    {
        std::string mediaStr = videoSegmentInfo.SegmentTemplate.media;
        ReplaceAllSubstring(mediaStr, "$RepresentationID$", videoSegmentInfo.Representation.id);

        if (mediaStr.find("$Number") != std::string::npos)
        {
            // get next segment number
            if (videoSegmentInfo.SegmentTemplate.startNumber > m_videoStatus.m_numberSegment || videoSegmentInfo.Period.id != m_videoStatus.m_prePeriodID)
                m_videoStatus.m_numberSegment = videoSegmentInfo.SegmentTemplate.startNumber;
            // handle period id
            m_videoStatus.m_prePeriodID = videoSegmentInfo.Period.id;
            uint32_t nextSegment = m_videoStatus.m_numberSegment;
            // get next download time
            nextDownloadTime = m_videoStatus.m_downloadTime + GetSegmentDurationMSec(videoSegmentInfo);
            // check if EOS or BOS
            if (IsEOS(nextDownloadTime, m_videoStatus))
                return "Video_EOS";
            if (IsBOS(nextDownloadTime, m_videoStatus))
                return "Video_BOS";
            HandleStringFormat(mediaStr, nextSegment, "$Number"); // $Number%06$
            HandleStringFormat(mediaStr, videoSegmentInfo.Representation.bandwidth, "$Bandwidth"); // $Bandwidth%06$
        }
        else if (mediaStr.find("$Time") != std::string::npos)
        {
            // get next segment number
            uint32_t nextSegment = m_videoStatus.m_numberSegment;
            // get next download time
            if (nextSegment < videoSegmentInfo.SegmentTemplate.SegmentTimeline.size())
                nextDownloadTime = GetSegmentTimeMSec(videoSegmentInfo.SegmentTemplate.SegmentTimeline[nextSegment], videoSegmentInfo);
            else
            {
                LOGMSG_ERROR("Out of range audio");
                return "Video_EOS";
            }
            // check if EOS or BOS
            if (IsEOS(nextDownloadTime, m_videoStatus))
                return "Video_EOS";
            if (IsBOS(nextDownloadTime, m_videoStatus))
                return "Video_BOS";
            // get the string format
            ReplaceAllSubstring(mediaStr, "$Time$", std::to_string(videoSegmentInfo.SegmentTemplate.SegmentTimeline[nextSegment]));
            LOGMSG_DEBUG("%s nextDownloadTime: %u", mediaStr.c_str(), videoSegmentInfo.SegmentTemplate.SegmentTimeline[nextSegment]);
        }
        else
        {
            return "Video_EOS";
        }
        return mediaStr;
    }
    else
    {
        return "";
    }
}

std::string dashSegmentSelector::GetSegmentURL_Video_Dynamic(const segmentInfo& videoSegmentInfo, uint64_t& nextDownloadTime)
{
    if (videoSegmentInfo.isHasSegmentTemplate)
    {
        std::string mediaStr = videoSegmentInfo.SegmentTemplate.media;
        ReplaceAllSubstring(mediaStr, "$RepresentationID$", videoSegmentInfo.Representation.id);

        if (mediaStr.find("$Number") != std::string::npos)
        {
            // get next segment number
            std::time_t now = std::time(0); // get current time Sec counted from 1970 01 01 00:00
            uint32_t nextSegment;
            if (m_videoStatus.m_numberSegment == 0)
                m_videoStatus.m_numberSegment = (now - 5) / videoSegmentInfo.SegmentTemplate.duration;
            nextSegment = m_videoStatus.m_numberSegment;
            // get next download time
            nextDownloadTime = now;

            HandleStringFormat(mediaStr, nextSegment, "$Number"); // $Number%06$
            HandleStringFormat(mediaStr, videoSegmentInfo.Representation.bandwidth, "$Bandwidth"); // $Bandwidth%06$
        }
        return mediaStr;
    }
    else
    {
        return "";
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

uint32_t dashSegmentSelector::GetTargetDownloadSize_Audio()
{
    return GetTargetDownloadSize(m_audioStatus, "Audio");
}

segmentSelectorRet dashSegmentSelector::GetSegmentInfo_Audio(uint32_t targetDownloadSize, segmentInfo& resultInfo)
{
    segmentSelectorRet ret = SegmentSelector_Okay;

    uint32_t selectedDownloadSize = 0;
    resultInfo.Representation.bandwidth = 0;
    segmentInfo lowestQualityInfo; lowestQualityInfo.Representation.bandwidth = 0xFFFFFFFF;
    segmentInfo highestQualityInfo; highestQualityInfo.Representation.bandwidth = 0;
    uint64_t accumulatePeriodDuration = 0;
    m_audioStatus.m_mediaEndTime = 0;
    m_audioStatus.m_mediaStartTime = 0;
    bool isHasPeriodDuration = false;
    // handle criteria
    std::string mediaType_Target = "audio";
    std::string audioLang_Target = "*"; // "eng"
    uint64_t currentDownloadedTime = m_audioStatus.m_downloadTime;

    std::vector<dash::mpd::IPeriod *> periods = m_mpdFile->GetPeriods();
    for (size_t i = 0; i < periods.size(); i++) // loop for every period
    {
        dash::mpd::IPeriod* period = periods[i];
        uint64_t timeMSec = 0;
        GetTimeString2MSec(period->GetDuration(), timeMSec);
        // handle media start time
        if (i == 0 && period->GetStart().length())
        {
            GetTimeString2MSec(period->GetStart(), m_audioStatus.m_mediaStartTime);
        }
        bool isCorrectPeriod = false;
        if (period->GetDuration().length())
        {
            isHasPeriodDuration = true;
            if (accumulatePeriodDuration <= currentDownloadedTime && currentDownloadedTime < accumulatePeriodDuration + timeMSec) // select correct period
                isCorrectPeriod = true;
        }
        else
            isCorrectPeriod = true;
        if (isCorrectPeriod)
        {
            std::vector<dash::mpd::IAdaptationSet *> adaptationsSets = period->GetAdaptationSets();
            for (size_t j = 0; j < adaptationsSets.size(); j++) // loop for every adaptation set
            {
                dash::mpd::IAdaptationSet* adaptationSet = adaptationsSets[j];
                std::string mimeType = adaptationSet->GetMimeType();
                std::transform(mimeType.begin(), mimeType.end(), mimeType.begin(), ::tolower); // toupper
                if (mimeType.find(mediaType_Target) != std::string::npos) // found audio representation
                {
                    std::string audioLang = adaptationSet->GetLang();
                    if (audioLang == audioLang_Target || audioLang == "" || audioLang_Target == "*") // select audio language
                    {
                        if (audioLang_Target == "*")
                            audioLang_Target = audioLang;
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
                                    resultInfo = GetSegmentInfo_priv_Audio(period, adaptationSet, segmentTemplate, representation);
                                }
                                if (lowestQualityInfo.Representation.bandwidth > bandwidth)
                                {
                                    lowestQualityInfo = GetSegmentInfo_priv_Audio(period, adaptationSet, segmentTemplate, representation);
                                }
                                if (highestQualityInfo.Representation.bandwidth < bandwidth)
                                {
                                    highestQualityInfo = GetSegmentInfo_priv_Audio(period, adaptationSet, segmentTemplate, representation);
                                }
                            }
                            else
                            {
                                if (highestQualityInfo.Representation.bandwidth < bandwidth)
                                {
                                    highestQualityInfo = GetSegmentInfo_priv_Audio(period, adaptationSet, segmentTemplate, representation);
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
    m_audioStatus.m_mediaEndTime = accumulatePeriodDuration;
    if (m_mpdFile->GetMediaPresentationDuration().length())
    {
        isHasPeriodDuration = true;
        GetTimeString2MSec(m_mpdFile->GetMediaPresentationDuration(), m_audioStatus.m_mediaEndTime);
    }

    if ((isHasPeriodDuration || m_mpdFile->GetMediaPresentationDuration().length()) && m_audioStatus.m_mediaEndTime <= currentDownloadedTime)
        ret = SegmentSelector_Audio_EOS;
    if (resultInfo.Representation.bandwidth == 0)
    {
        resultInfo = lowestQualityInfo;
    }
    // HACK
    resultInfo = highestQualityInfo;
    LOGMSG_INFO("selected bandwidth: %u", resultInfo.Representation.bandwidth);

    return ret;
}

std::string dashSegmentSelector::GetDownloadURL_Audio(const segmentInfo& audioSegmentInfo, uint64_t& nextDownloadTime)
{
    if (m_audioStatus.m_segmentInfo.Representation.id != audioSegmentInfo.Representation.id)
    {
        m_audioStatus.m_initFileReady = false;
    }
    m_audioStatus.m_segmentInfo = audioSegmentInfo;

    if (m_audioStatus.m_initFileReady)
    {
        if (!audioSegmentInfo.isHasSegmentTemplate)
            return "Audio_EOS";
        return GetSegmentURL_Audio(audioSegmentInfo, nextDownloadTime);
    }
    else
    {
        nextDownloadTime = m_audioStatus.m_downloadTime;
        return GetInitFileURL_Audio(audioSegmentInfo);
    }
}

std::string dashSegmentSelector::GetInitFileURL_Audio(const segmentInfo& targetInfo)
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

std::string dashSegmentSelector::GetSegmentURL_Audio(const segmentInfo& targetInfo, uint64_t& nextDownloadTime)
{
    std::stringstream ss;
    HandleBaseURL(ss, targetInfo);

    std::string mediaStr;
    if (targetInfo.MPD.type == "static")
    {
        mediaStr = GetSegmentURL_Audio_Static(targetInfo, nextDownloadTime);
    }
    else
    {
        mediaStr = GetSegmentURL_Audio_Dynamic(targetInfo, nextDownloadTime);
    }

    if (mediaStr == "Audio_EOS" || mediaStr == "Audio_BOS")
        return mediaStr;

    if (mediaStr.length())
        ss << mediaStr;

    return ss.str();
}

std::string dashSegmentSelector::GetSegmentURL_Audio_Static(const segmentInfo& targetInfo, uint64_t& nextDownloadTime)
{
    if (targetInfo.isHasSegmentTemplate)
    {
        std::string mediaStr = targetInfo.SegmentTemplate.media;
        ReplaceAllSubstring(mediaStr, "$RepresentationID$", targetInfo.Representation.id);

        if (mediaStr.find("$Number") != std::string::npos)
        {
            // get next segment number
            if (targetInfo.SegmentTemplate.startNumber > m_audioStatus.m_numberSegment || targetInfo.Period.id != m_audioStatus.m_prePeriodID)
                m_audioStatus.m_numberSegment = targetInfo.SegmentTemplate.startNumber;
            // handle period id
            m_audioStatus.m_prePeriodID = targetInfo.Period.id;
            uint32_t nextSegment = m_audioStatus.m_numberSegment;
            // get next download time
            nextDownloadTime = m_audioStatus.m_downloadTime + GetSegmentDurationMSec(targetInfo);
            // check if EOS or BOS
            if (IsEOS(nextDownloadTime, m_audioStatus))
                return "Audio_EOS";
            if (IsBOS(nextDownloadTime, m_audioStatus))
                return "Audio_BOS";
            HandleStringFormat(mediaStr, nextSegment, "$Number"); // $Number%06$
            HandleStringFormat(mediaStr, targetInfo.Representation.bandwidth, "$Bandwidth"); // $Bandwidth%06$
        }
        else if (mediaStr.find("$Time") != std::string::npos)
        {
            // get next segment number
            uint32_t nextSegment = m_audioStatus.m_numberSegment;
            // get next download time
            if (nextSegment < targetInfo.SegmentTemplate.SegmentTimeline.size())
                nextDownloadTime = GetSegmentTimeMSec(targetInfo.SegmentTemplate.SegmentTimeline[nextSegment], targetInfo);
            else
            {
                LOGMSG_ERROR("Out of range audio");
                return "Audio_EOS";
            }
            // check if EOS or BOS
            if (IsEOS(nextDownloadTime, m_audioStatus))
                return "Audio_EOS";
            if (IsBOS(nextDownloadTime, m_audioStatus))
                return "Audio_BOS";
            // get the string format
            ReplaceAllSubstring(mediaStr, "$Time$", std::to_string(targetInfo.SegmentTemplate.SegmentTimeline[nextSegment]));
            LOGMSG_DEBUG("%s nextDownloadTime: %u", mediaStr.c_str(), targetInfo.SegmentTemplate.SegmentTimeline[nextSegment]);
        }
        else
        {
            return "Audio_EOS";
        }
        return mediaStr;
    }
    else
    {
        return "";
    }
}

std::string dashSegmentSelector::GetSegmentURL_Audio_Dynamic(const segmentInfo& targetInfo, uint64_t& nextDownloadTime)
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
            if (m_audioStatus.m_numberSegment == 0)
                m_audioStatus.m_numberSegment = (now - 5) / targetInfo.SegmentTemplate.duration;
            nextSegment = m_audioStatus.m_numberSegment;
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

segmentInfo dashSegmentSelector::GetSegmentInfo_priv_Audio(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::ISegmentTemplate* segmentTemplate, dash::mpd::IRepresentation* representation)
{
    segmentInfo resultInfo;

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
        resultInfo.SegmentTemplate.lang = representation->GetId();
        resultInfo.isHasSegmentTemplate = true;
    }
    else
    {
        resultInfo.isHasSegmentTemplate = false;
    }

    return resultInfo;
}

segmentInfo dashSegmentSelector::GetSegmentInfo_priv_Video(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::ISegmentTemplate* segmentTemplate, dash::mpd::IRepresentation* representation)
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
        resultInfo.SegmentTemplate.lang = "";
        resultInfo.isHasSegmentTemplate = true;
    }
    else
    {
        resultInfo.isHasSegmentTemplate = false;
    }

    return resultInfo;
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
