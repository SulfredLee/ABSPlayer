#include "dashSegmentSelector.h"
#include "Logger.h"
#include <algorithm>

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
    LOGMSG_INFO("Process message %s", msg->GetMsgTypeName().c_str());
    m_mpdFile = msg->GetAndMoveMPDFile();

    // handle download process status
    m_videoStatus.m_numberSegment = 0;
    m_audioStatus.m_numberSegment = 0;
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
                m_videoStatus.m_downloadSpeed = msg->GetSpeed();
                m_videoStatus.m_downloadTime = msg->GetDownloadTime();
                if (!m_videoStatus.m_initFileReady)
                    m_videoStatus.m_initFileReady = true;
                else
                    m_videoStatus.m_numberSegment++;
                break;
            }
        case PlayerMsg_Type_DownloadAudio:
            {
                m_audioStatus.m_downloadSpeed = msg->GetSpeed();
                m_audioStatus.m_downloadTime = msg->GetDownloadTime();
                if (!m_audioStatus.m_initFileReady)
                    m_audioStatus.m_initFileReady = true;
                else
                    m_audioStatus.m_numberSegment++;
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
}

void dashSegmentSelector::HandleVideoSegment()
{
    // get target criteria
    uint32_t targetDownloadSize = GetTargetDownloadSize_Video();
    // get url information
    downloadInfo videoDownloadInfo = GetDownloadInfo_Video(targetDownloadSize);
    // generate url
    uint64_t nextDownloadTime;
    std::string targetURL = GetDownloadURL_Video(videoDownloadInfo, nextDownloadTime);

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
    downloadInfo audioDownloadInfo = GetDownloadInfo_Audio(targetDownloadSize);
    // generate url
    uint64_t nextDownloadTime;
    std::string targetURL = GetDownloadURL_Audio(audioDownloadInfo, nextDownloadTime);

    LOGMSG_INFO("targetURL: %s", targetURL.c_str());

    if (targetURL == "Audio_EOS")
    {
        LOGMSG_INFO("Audio_EOS");
        return;
    }
    if (targetURL == "Audio_BOS")
    {
        LOGMSG_INFO("Audio_EOS");
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

downloadInfo dashSegmentSelector::GetDownloadInfo_Video(uint32_t targetDownloadSize)
{
    uint32_t selectedDownloadSize = 0;
    downloadInfo resultInfo; resultInfo.Representation.bandwidth = 0;
    downloadInfo lowestQualityInfo; lowestQualityInfo.Representation.bandwidth = 0xFFFFFFFF;
    downloadInfo highestQualityInfo; highestQualityInfo.Representation.bandwidth = 0;
    std::vector<dash::mpd::IPeriod *> periods = m_mpdFile->GetPeriods();
    for (size_t i = 0; i < periods.size(); i++) // loop for every period
    {
        dash::mpd::IPeriod* period = periods[i];
        std::vector<dash::mpd::IAdaptationSet *> adaptationsSets = period->GetAdaptationSets();
        for (size_t j = 0; j < adaptationsSets.size(); j++) // loop for every adaptation set
        {
            dash::mpd::IAdaptationSet* adaptationSet = adaptationsSets[j];
            std::string mimeType = adaptationSet->GetMimeType();
            std::transform(mimeType.begin(), mimeType.end(), mimeType.begin(), ::tolower); // toupper
            if (mimeType.find("video") != std::string::npos) // found video representation
            {
                std::vector<dash::mpd::IRepresentation *> representations = adaptationsSets[j]->GetRepresentation();
                dash::mpd::ISegmentTemplate* segmentTemplate = adaptationSet->GetSegmentTemplate();
                for (size_t k = 0; k < representations.size(); k++) // loop for every representation
                {
                    dash::mpd::IRepresentation* representation = representations[k];

                    if (segmentTemplate)
                    {
                        uint32_t bandwidth = representation->GetBandwidth();
                        uint32_t segmentSize = bandwidth * static_cast<double>(segmentTemplate->GetDuration()) / segmentTemplate->GetTimescale();
                        if (selectedDownloadSize < segmentSize && segmentSize < targetDownloadSize) // if bandwidth is sutiable
                        {
                            selectedDownloadSize = segmentSize;
                            resultInfo = GetDownloadInfo_priv_Video(period, adaptationSet, segmentTemplate, representation);
                        }
                        if (lowestQualityInfo.Representation.bandwidth > bandwidth)
                        {
                            lowestQualityInfo = GetDownloadInfo_priv_Video(period, adaptationSet, segmentTemplate, representation);
                        }
                        if (highestQualityInfo.Representation.bandwidth < bandwidth)
                        {
                            highestQualityInfo = GetDownloadInfo_priv_Video(period, adaptationSet, segmentTemplate, representation);
                        }
                    }
                }
            }
        }
    }

    if (resultInfo.Representation.bandwidth == 0)
    {
        resultInfo = lowestQualityInfo;
    }
    // HACK
    resultInfo = highestQualityInfo;
    LOGMSG_INFO("selected bandwidth: %u", resultInfo.Representation.bandwidth);

    return resultInfo;
}

std::string dashSegmentSelector::GetDownloadURL_Video(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime)
{
    if (m_videoStatus.m_downloadInfo.Representation.id != videoDownloadInfo.Representation.id)
    {
        m_videoStatus.m_downloadInfo = videoDownloadInfo;
        m_videoStatus.m_initFileReady = false;
    }

    if (m_videoStatus.m_initFileReady)
    {
        return GetSegmentURL_Video(videoDownloadInfo, nextDownloadTime);
    }
    else
    {
        nextDownloadTime = m_videoStatus.m_downloadTime;
        return GetInitFileURL_Video(videoDownloadInfo);
    }
}

std::string dashSegmentSelector::GetInitFileURL_Video(const downloadInfo& targetInfo)
{
    std::stringstream ss;
    ss << targetInfo.BaseURL;
    ss << targetInfo.Period.BaseURL;
    ss << targetInfo.AdaptationSet.BaseURL;
    if (targetInfo.SegmentTemplate.initialization.find("$RepresentationID$") != std::string::npos)
    {
        std::string initStr = targetInfo.SegmentTemplate.initialization;
        ReplaceAllSubstring(initStr, "$RepresentationID$", targetInfo.Representation.id);
        ss << initStr;

        return ss.str();
    }
    else
    {
        return std::string();
    }
}

std::string dashSegmentSelector::GetSegmentURL_Video(const downloadInfo& videoDownloadInfo, uint64_t& nextDownloadTime)
{
    std::stringstream ss;
    ss << videoDownloadInfo.BaseURL;
    ss << videoDownloadInfo.Period.BaseURL;
    ss << videoDownloadInfo.AdaptationSet.BaseURL;
    if (videoDownloadInfo.SegmentTemplate.media.find("$RepresentationID$") != std::string::npos)
    {
        std::string mediaStr = videoDownloadInfo.SegmentTemplate.media;
        ReplaceAllSubstring(mediaStr, "$RepresentationID$", videoDownloadInfo.Representation.id);

        if (mediaStr.find("$Number") != std::string::npos)
        {
            // get next segment number
            uint32_t nextSegment = m_videoStatus.m_downloadTime / GetSegmentDurationMSec(videoDownloadInfo);
            nextSegment++;
            if (videoDownloadInfo.SegmentTemplate.startNumber > nextSegment)
                nextSegment = videoDownloadInfo.SegmentTemplate.startNumber;
            // get next download time
            nextDownloadTime = nextSegment * GetSegmentDurationMSec(videoDownloadInfo);
            // check if EOS or BOS
            if (IsEOS(nextDownloadTime, videoDownloadInfo))
                return "Video_EOS";
            if (IsBOS(nextDownloadTime, videoDownloadInfo))
                return "Video_BOS";
            // get the string format
            size_t numberFormatStart = mediaStr.find("$Number") + 7;
            size_t endDollar = mediaStr.find("$", numberFormatStart);
            std::string numberFormat = mediaStr.substr(numberFormatStart, endDollar - numberFormatStart);
            char numberStr[20];
            sprintf(numberStr, numberFormat.c_str(), nextSegment);
            // replace the string
            ReplaceAllSubstring(mediaStr, "$Number" + numberFormat + "$", numberStr);
            LOGMSG_DEBUG("%s numberFormat: %s numberStr: %s nextSegment: %u", mediaStr.c_str(), numberFormat.c_str(), numberStr, nextSegment);
        }
        else
        {
            // get next segment number
            uint32_t nextSegment = m_videoStatus.m_numberSegment;
            // get next download time
            if (nextSegment < videoDownloadInfo.SegmentTemplate.SegmentTimeline.size())
                nextDownloadTime = GetSegmentTimeMSec(videoDownloadInfo.SegmentTemplate.SegmentTimeline[nextSegment], videoDownloadInfo);
            else
            {
                LOGMSG_ERROR("Out of range audio");
                return "Video_EOS";
            }
            // check if EOS or BOS
            if (IsEOS(nextDownloadTime, videoDownloadInfo))
                return "Video_EOS";
            if (IsBOS(nextDownloadTime, videoDownloadInfo))
                return "Video_BOS";
            // get the string format
            ReplaceAllSubstring(mediaStr, "$Time$", std::to_string(videoDownloadInfo.SegmentTemplate.SegmentTimeline[nextSegment]));
            LOGMSG_DEBUG("%s nextDownloadTime: %u", mediaStr.c_str(), videoDownloadInfo.SegmentTemplate.SegmentTimeline[nextSegment]);
        }
        ss << mediaStr;

        return ss.str();
    }
    else
    {
        return std::string();
    }
}

uint32_t dashSegmentSelector::GetSegmentDurationMSec(const downloadInfo& inDownloadInfo)
{
    return GetSegmentTimeMSec(inDownloadInfo.SegmentTemplate.duration, inDownloadInfo);
}

uint32_t dashSegmentSelector::GetSegmentTimeMSec(const uint64_t& inTime, const downloadInfo& inDownloadInfo)
{
    return (static_cast<double>(inTime) / inDownloadInfo.SegmentTemplate.timescale) * 1000;
}

uint32_t dashSegmentSelector::GetTargetDownloadSize_Audio()
{
    return GetTargetDownloadSize(m_audioStatus, "Audio");
}

downloadInfo dashSegmentSelector::GetDownloadInfo_Audio(uint32_t targetDownloadSize)
{
    uint32_t selectedDownloadSize = 0;
    downloadInfo resultInfo; resultInfo.Representation.bandwidth = 0;
    downloadInfo lowestQualityInfo; lowestQualityInfo.Representation.bandwidth = 0xFFFFFFFF;
    downloadInfo highestQualityInfo; highestQualityInfo.Representation.bandwidth = 0;
    std::string mediaType_Target = "audio";
    std::string audioLang_Target = "eng";

    std::vector<dash::mpd::IPeriod *> periods = m_mpdFile->GetPeriods();
    for (size_t i = 0; i < periods.size(); i++) // loop for every period
    {
        dash::mpd::IPeriod* period = periods[i];
        std::vector<dash::mpd::IAdaptationSet *> adaptationsSets = period->GetAdaptationSets();
        for (size_t j = 0; j < adaptationsSets.size(); j++) // loop for every adaptation set
        {
            dash::mpd::IAdaptationSet* adaptationSet = adaptationsSets[j];
            std::string mimeType = adaptationSet->GetMimeType();
            std::transform(mimeType.begin(), mimeType.end(), mimeType.begin(), ::tolower); // toupper
            if (mimeType.find(mediaType_Target) != std::string::npos) // found audio representation
            {
                std::string audioLang = adaptationSet->GetLang();
                if (audioLang == audioLang_Target || audioLang == "") // select audio language
                {
                    std::vector<dash::mpd::IRepresentation *> representations = adaptationsSets[j]->GetRepresentation();
                    dash::mpd::ISegmentTemplate* segmentTemplate = adaptationSet->GetSegmentTemplate();
                    for (size_t k = 0; k < representations.size(); k++) // loop for every representation
                    {
                        dash::mpd::IRepresentation* representation = representations[k];

                        if (segmentTemplate)
                        {
                            uint32_t bandwidth = representation->GetBandwidth();
                            uint32_t segmentSize = bandwidth * static_cast<double>(segmentTemplate->GetDuration()) / segmentTemplate->GetTimescale();
                            if (selectedDownloadSize < segmentSize && segmentSize < targetDownloadSize) // if bandwidth is sutiable
                            {
                                selectedDownloadSize = segmentSize;
                                resultInfo = GetDownloadInfo_priv_Audio(period, adaptationSet, segmentTemplate, representation);
                            }
                            if (lowestQualityInfo.Representation.bandwidth > bandwidth)
                            {
                                lowestQualityInfo = GetDownloadInfo_priv_Audio(period, adaptationSet, segmentTemplate, representation);
                            }
                            if (highestQualityInfo.Representation.bandwidth < bandwidth)
                            {
                                highestQualityInfo = GetDownloadInfo_priv_Audio(period, adaptationSet, segmentTemplate, representation);
                            }
                        }
                    }
                }
            }
        }
    }

    if (resultInfo.Representation.bandwidth == 0)
    {
        resultInfo = lowestQualityInfo;
    }
    // HACK
    resultInfo = highestQualityInfo;
    LOGMSG_INFO("selected bandwidth: %u", resultInfo.Representation.bandwidth);

    return resultInfo;
}

std::string dashSegmentSelector::GetDownloadURL_Audio(const downloadInfo& audioDownloadInfo, uint64_t& nextDownloadTime)
{
    if (m_audioStatus.m_downloadInfo.Representation.id != audioDownloadInfo.Representation.id)
    {
        m_audioStatus.m_downloadInfo = audioDownloadInfo;
        m_audioStatus.m_initFileReady = false;
    }

    if (m_audioStatus.m_initFileReady)
    {
        return GetSegmentURL_Audio(audioDownloadInfo, nextDownloadTime);
    }
    else
    {
        nextDownloadTime = m_audioStatus.m_downloadTime;
        return GetInitFileURL_Audio(audioDownloadInfo);
    }
}

std::string dashSegmentSelector::GetInitFileURL_Audio(const downloadInfo& targetInfo)
{
    std::stringstream ss;
    dash::mpd::IBaseUrl* baseURL = m_mpdFile->GetMPDPathBaseUrl();
    ss << baseURL->GetUrl() << "/";
    ss << targetInfo.Period.BaseURL;
    ss << targetInfo.AdaptationSet.BaseURL;
    if (targetInfo.SegmentTemplate.initialization.find("$RepresentationID$") != std::string::npos)
    {
        std::string initStr = targetInfo.SegmentTemplate.initialization;
        ReplaceAllSubstring(initStr, "$RepresentationID$", targetInfo.Representation.id);
        ss << initStr;

        return ss.str();
    }
    else
    {
        return std::string();
    }
}

std::string dashSegmentSelector::GetSegmentURL_Audio(const downloadInfo& targetInfo, uint64_t& nextDownloadTime)
{
    std::stringstream ss;
    dash::mpd::IBaseUrl* baseURL = m_mpdFile->GetMPDPathBaseUrl();
    ss << baseURL->GetUrl() << "/";
    ss << targetInfo.Period.BaseURL;
    ss << targetInfo.AdaptationSet.BaseURL;
    if (targetInfo.SegmentTemplate.media.find("$RepresentationID$") != std::string::npos)
    {
        std::string mediaStr = targetInfo.SegmentTemplate.media;
        ReplaceAllSubstring(mediaStr, "$RepresentationID$", targetInfo.Representation.id);

        if (mediaStr.find("$Number") != std::string::npos)
        {
            // get next segment number
            uint32_t nextSegment = m_audioStatus.m_downloadTime / GetSegmentDurationMSec(targetInfo);
            nextSegment++;
            if (targetInfo.SegmentTemplate.startNumber > nextSegment)
                nextSegment = targetInfo.SegmentTemplate.startNumber;
            // get next download time
            nextDownloadTime = nextSegment * GetSegmentDurationMSec(targetInfo);
            // check if EOS or BOS
            if (IsEOS(nextDownloadTime, targetInfo))
                return "Audio_EOS";
            if (IsBOS(nextDownloadTime, targetInfo))
                return "Audio_BOS";
            // get the string format
            size_t numberFormatStart = mediaStr.find("$Number") + 7;
            size_t endDollar = mediaStr.find("$", numberFormatStart);
            std::string numberFormat = mediaStr.substr(numberFormatStart, endDollar - numberFormatStart);
            char numberStr[20];
            sprintf(numberStr, numberFormat.c_str(), nextSegment);
            // replace the string
            ReplaceAllSubstring(mediaStr, "$Number" + numberFormat + "$", numberStr);
            LOGMSG_DEBUG("%s numberFormat: %s numberStr: %s nextSegment: %u", mediaStr.c_str(), numberFormat.c_str(), numberStr, nextSegment);
        }
        else
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
            if (IsEOS(nextDownloadTime, targetInfo))
                return "Audio_EOS";
            if (IsBOS(nextDownloadTime, targetInfo))
                return "Audio_BOS";
            // get the string format
            ReplaceAllSubstring(mediaStr, "$Time$", std::to_string(targetInfo.SegmentTemplate.SegmentTimeline[nextSegment]));
            LOGMSG_DEBUG("%s nextDownloadTime: %u", mediaStr.c_str(), targetInfo.SegmentTemplate.SegmentTimeline[nextSegment]);
        }
        ss << mediaStr;

        return ss.str();
    }
    else
    {
        return std::string();
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

downloadInfo dashSegmentSelector::GetDownloadInfo_priv_Audio(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::ISegmentTemplate* segmentTemplate, dash::mpd::IRepresentation* representation)
{
    downloadInfo resultInfo;

    dash::mpd::IBaseUrl* BaseURL = m_mpdFile->GetMPDPathBaseUrl();
    resultInfo.BaseURL = BaseURL->GetUrl() + "/";

    // handle period
    resultInfo.Period.BaseURL = period->GetBaseURLs().size() ? period->GetBaseURLs()[0]->GetUrl() : std::string();
    AppendSlash2Path(resultInfo.Period.BaseURL);
    resultInfo.Period.duration = period->GetDuration();
    resultInfo.Period.start = period->GetStart();
    // handle representation
    resultInfo.Representation.bandwidth = representation->GetBandwidth();
    resultInfo.Representation.id = representation->GetId();
    // handle adaptationSet
    resultInfo.AdaptationSet.BaseURL = adaptationSet->GetBaseURLs().size() ? adaptationSet->GetBaseURLs()[0]->GetUrl() : std::string();
    // handle segmentTemplate
    resultInfo.SegmentTemplate.media = segmentTemplate->Getmedia();
    if (resultInfo.SegmentTemplate.media.find("Time") != std::string::npos)
        resultInfo.SegmentTemplate.SegmentTimeline = GetSegmentTimeline(segmentTemplate);
    resultInfo.SegmentTemplate.startNumber = segmentTemplate->GetStartNumber();
    resultInfo.SegmentTemplate.timescale = segmentTemplate->GetTimescale();
    resultInfo.SegmentTemplate.duration = segmentTemplate->GetDuration();
    resultInfo.SegmentTemplate.initialization = segmentTemplate->Getinitialization();
    resultInfo.SegmentTemplate.lang = representation->GetId();

    return resultInfo;
}

downloadInfo dashSegmentSelector::GetDownloadInfo_priv_Video(dash::mpd::IPeriod* period, dash::mpd::IAdaptationSet* adaptationSet, dash::mpd::ISegmentTemplate* segmentTemplate, dash::mpd::IRepresentation* representation)
{
    downloadInfo resultInfo;

    dash::mpd::IBaseUrl* BaseURL = m_mpdFile->GetMPDPathBaseUrl();
    resultInfo.BaseURL = BaseURL->GetUrl() + "/";

    // handle period
    resultInfo.Period.BaseURL = period->GetBaseURLs().size() ? period->GetBaseURLs()[0]->GetUrl() : std::string();
    AppendSlash2Path(resultInfo.Period.BaseURL);
    resultInfo.Period.duration = period->GetDuration();
    resultInfo.Period.start = period->GetStart();
    // handle representation
    resultInfo.Representation.bandwidth = representation->GetBandwidth();
    resultInfo.Representation.id = representation->GetId();
    // handle adaptationSet
    resultInfo.AdaptationSet.BaseURL = adaptationSet->GetBaseURLs().size() ? adaptationSet->GetBaseURLs()[0]->GetUrl() : std::string();
    // handle segmentTemplate
    resultInfo.SegmentTemplate.media = segmentTemplate->Getmedia();
    if (resultInfo.SegmentTemplate.media.find("Time") != std::string::npos)
        resultInfo.SegmentTemplate.SegmentTimeline = GetSegmentTimeline(segmentTemplate);
    resultInfo.SegmentTemplate.startNumber = segmentTemplate->GetStartNumber();
    resultInfo.SegmentTemplate.timescale = segmentTemplate->GetTimescale();
    resultInfo.SegmentTemplate.duration = segmentTemplate->GetDuration();
    resultInfo.SegmentTemplate.initialization = segmentTemplate->Getinitialization();
    resultInfo.SegmentTemplate.lang = "";

    return resultInfo;
}

bool dashSegmentSelector::IsEOS(const uint64_t& nextDownloadTime, const downloadInfo& inDownloadInfo)
{
    uint64_t durationMSec = 0;
    if (GetTimeString2MSec(inDownloadInfo.Period.duration, durationMSec))
        return nextDownloadTime > durationMSec;
    else
        return false;
}

bool dashSegmentSelector::IsBOS(const uint64_t& nextDownloadTime, const downloadInfo& inDownloadInfo)
{
    uint64_t startMSec = 0;
    if (GetTimeString2MSec(inDownloadInfo.Period.start, startMSec))
        return nextDownloadTime < startMSec;
    else
        return false;
}

bool dashSegmentSelector::GetTimeString2MSec(std::string timeStr, uint64_t& timeMSec)
{
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

    timeMSec = 0;
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
