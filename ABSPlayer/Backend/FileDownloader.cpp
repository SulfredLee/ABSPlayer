#include "FileDownloader.h"
#include "Logger.h"
#include "IMPD.h"
#include "libdash.h"

FileDownloader::FileDownloader()
{
    m_manager = NULL;

    curl_global_init(CURL_GLOBAL_ALL);
    /* init the curl session */
    m_curl_handle = curl_easy_init();
}

FileDownloader::~FileDownloader()
{
    stopThread();
    std::shared_ptr<PlayerMsg_Dummy> msgDummy = std::make_shared<PlayerMsg_Dummy>();
    m_msgQ.AddMsg(std::static_pointer_cast<PlayerMsg_Base>(msgDummy));
    joinThread();

    /* cleanup curl stuff */
    curl_easy_cleanup(m_curl_handle);
    /* we're done with libcurl, so clean it up */
    curl_global_cleanup();

    LOGMSG_INFO("OUT");
}

void FileDownloader::InitComponent(CmdReceiver* manager, const std::string& thisName)
{
    m_msgQ.InitComponent(1024 * 1024 * 10); // 10 MByte buffer for message queue
    m_manager = manager;
    LOGMSG_CLASS_NAME(thisName);
    startThread();
}

void FileDownloader::DeinitComponent()
{
    m_manager = NULL;
}

size_t FileDownloader::WriteFunction(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    FileDownloader* downloader = static_cast<FileDownloader*>(userp);

    downloader->SaveToPool(contents, realsize);

    return realsize;
}

int FileDownloader::ProgressFunction(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    // Wait for future usage
    // FileDownloader* downloader = static_cast<FileDownloader*>(clientp);
    return 0;
}

size_t FileDownloader::HeaderFunction(char *buffer, size_t size, size_t nitems, void *userdata)
{
    // Wait for future usage
    // FileDownloader* downloader = static_cast<FileDownloader*>(userdata);
    size_t totalSize = size * nitems;
    return totalSize;
}

void FileDownloader::SaveToPool(void *contents, size_t size)
{
    switch (m_msgPool->GetMsgType())
    {
        case PlayerMsg_Type_DownloadVideo:
            SendPartOfMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadFile>(m_msgFactory.CreateMsg(PlayerMsg_Type_DownloadVideo)), contents, size);
            break;
        case PlayerMsg_Type_DownloadAudio:
            SendPartOfMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadFile>(m_msgFactory.CreateMsg(PlayerMsg_Type_DownloadAudio)), contents, size);
            break;
        case PlayerMsg_Type_DownloadSubtitle:
            SendPartOfMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadFile>(m_msgFactory.CreateMsg(PlayerMsg_Type_DownloadSubtitle)), contents, size);
            break;
        case PlayerMsg_Type_DownloadMPD:
        case PlayerMsg_Type_RefreshMPD:
            m_msgPool->SetFile(static_cast<unsigned char*>(contents), size);
            break;
        default:
            break;
    }
}

void FileDownloader::SendPartOfMsg(std::shared_ptr<PlayerMsg_DownloadFile> msgFile, void *contents, size_t size)
{
    msgFile->SetURL(m_msgPool->GetURL());
    msgFile->SetFile(static_cast<unsigned char*>(contents), size);
    m_msgPoolSize += size;
    msgFile->SetFileCount(m_msgPoolCount++);
    SendToManager(msgFile);
}

void FileDownloader::ProcessMsg(std::shared_ptr<PlayerMsg_Base> msg)
{
    LOGMSG_DEBUG("Process message %s from: %s", msg->GetMsgTypeName().c_str(), msg->GetSender().c_str());

    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_DownloadFile:
        case PlayerMsg_Type_DownloadVideo:
        case PlayerMsg_Type_DownloadAudio:
        case PlayerMsg_Type_DownloadSubtitle:
            ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadFile>(msg));
            break;
        case PlayerMsg_Type_DownloadMPD:
            ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadMPD>(msg));
            break;
        case PlayerMsg_Type_RefreshMPD:
            ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_RefreshMPD>(msg));
            break;
        default:
            break;
    }
}

void FileDownloader::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadFile> msg)
{
    CountTimer countTimer;
    int32_t responseCode;
    CURLcode res = DownloadAFile(msg, countTimer, responseCode);
    /* check for errors */
    if(res != CURLE_OK)
    {
        LOGMSG_ERROR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
    }
    else
    {
        /*
         * Now, our chunk.memory points to a memory block that is chunk.size
         * bytes big and contains the remote file.
         *
         * Do something nice with it!
         */

        LOGMSG_INFO("%lu bytes retrieved, time spent: %f responseCode: %u", msg->GetFileLength() ? msg->GetFileLength() : m_msgPoolSize, countTimer.GetSecondDouble(), responseCode);

        msg->SetResponseCode(responseCode);

        // finished download and alert manager
        SendDownloadFinishedMsg(countTimer, msg);
    }
}

void FileDownloader::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadMPD> msg)
{
    CountTimer countTimer;
    int32_t responseCode;
    CURLcode res = DownloadAFile(msg, countTimer, responseCode);
    /* check for errors */
    if(res != CURLE_OK)
    {
        LOGMSG_ERROR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
    }
    else
    {
        /*
         * Now, our chunk.memory points to a memory block that is chunk.size
         * bytes big and contains the remote file.
         *
         * Do something nice with it!
         */

        LOGMSG_INFO("%lu bytes retrieved, time spent: %f responseCode: %u", msg->GetFileLength(), countTimer.GetSecondDouble(), responseCode);

        msg->SetResponseCode(responseCode);

        dash::IDASHManager* dashManager = CreateDashManager();
        dash::mpd::IMPD* mpdFile = dashManager->Open(const_cast<char*>(msg->GetURL().c_str()), msg->GetFile());
        msg->SetMPDFile(mpdFile);
        delete dashManager;

        // finished download and alert manager
        SendToManager(msg);
        SendDownloadFinishedMsg(countTimer, msg);
    }
}

void FileDownloader::ProcessMsg(std::shared_ptr<PlayerMsg_RefreshMPD> msg)
{
    CountTimer countTimer;
    int32_t responseCode;
    CURLcode res = DownloadAFile(msg, countTimer, responseCode);
    /* check for errors */
    if(res != CURLE_OK)
    {
        LOGMSG_ERROR("curl_easy_perform() failed: %s", curl_easy_strerror(res));
    }
    else
    {
        /*
         * Now, our chunk.memory points to a memory block that is chunk.size
         * bytes big and contains the remote file.
         *
         * Do something nice with it!
         */

        LOGMSG_INFO("%lu bytes retrieved, time spent: %f, responseCode: %u", msg->GetFileLength(), countTimer.GetSecondDouble(), responseCode);

        msg->SetResponseCode(responseCode);

        dash::IDASHManager* dashManager = CreateDashManager();
        dash::mpd::IMPD* mpdFile = dashManager->Open(const_cast<char*>(msg->GetURL().c_str()), msg->GetFile());
        msg->SetMPDFile(mpdFile);
        delete dashManager;

        // finished download and alert manager
        SendToManager(msg);
        SendDownloadFinishedMsg(countTimer, msg);
    }
}

void FileDownloader::SendToManager(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("FileDownloader");
    if (m_manager) m_manager->UpdateCMD(msg);
}

void FileDownloader::SendDownloadFinishedMsg(const CountTimer& countTimer, std::shared_ptr<PlayerMsg_DownloadFile> msg)
{
    uint32_t downloadSpeed;
    if (msg->GetFileLength())
        downloadSpeed = msg->GetFileLength() / countTimer.GetSecondDouble() * 8; // bit per second
    else
        downloadSpeed = m_msgPoolSize / countTimer.GetSecondDouble() * 8; // bit per second
    std::shared_ptr<PlayerMsg_DownloadFinish> msgFinish = std::dynamic_pointer_cast<PlayerMsg_DownloadFinish>(m_msgFactory.CreateMsg(PlayerMsg_Type_DownloadFinish));
    msgFinish->SetFileName(msg->GetURL());
    msgFinish->SetSize(msg->GetFileLength());
    msgFinish->SetSpeed(downloadSpeed);
    msgFinish->SetTimeSpent(countTimer.GetMSecond());
    msgFinish->SetFileType(msg->GetMsgType());
    msgFinish->SetDownloadTime(msg->GetDownloadTime());
    msgFinish->SetResponseCode(msg->GetResponseCode());

    SendToManager(msgFinish);
}

// override
bool FileDownloader::UpdateCMD(std::shared_ptr<PlayerMsg_Base> msg)
{
    LOGMSG_DEBUG("Received message %s from: %s", msg->GetMsgTypeName().c_str(), msg->GetSender().c_str());

    bool ret = true;
    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_DownloadFile:
        case PlayerMsg_Type_DownloadMPD:
        case PlayerMsg_Type_DownloadVideo:
        case PlayerMsg_Type_DownloadAudio:
        case PlayerMsg_Type_DownloadSubtitle:
        case PlayerMsg_Type_RefreshMPD:
            {
                if (!m_msgQ.AddMsg(msg))
                {
                    LOGMSG_ERROR("AddMsg fail");
                    ret = false;
                }
            }
            break;
        default:
            break;
    }
    return ret;
}

// override
void* FileDownloader::Main()
{
    LOGMSG_INFO("IN");

    while(isThreadRunning())
    {
        std::shared_ptr<PlayerMsg_Base> msg;
        m_msgQ.GetMsg(msg);

        ProcessMsg(msg);
    }

    LOGMSG_INFO("OUT");
    return NULL;
}

CURLcode FileDownloader::DownloadAFile(std::shared_ptr<PlayerMsg_DownloadFile> msg, CountTimer& countTimer, int32_t& responseCode)
{
    // handle message pool so that we can download and process part of the message
    m_msgPool = msg;
    m_msgPoolSize = 0;
    m_msgPoolCount = 0;

    CURLcode res;

    /* specify URL to get */
    curl_easy_setopt(m_curl_handle, CURLOPT_URL, msg->GetURL().c_str());

    LOGMSG_INFO("Going to Download: %s", msg->GetURL().c_str());

    /* send all data to this function  */
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEFUNCTION, WriteFunction);
    curl_easy_setopt(m_curl_handle, CURLOPT_WRITEDATA, static_cast<void*>(this));
    // monitor progress
    curl_easy_setopt(m_curl_handle, CURLOPT_PROGRESSFUNCTION, ProgressFunction);
    curl_easy_setopt(m_curl_handle, CURLOPT_PROGRESSDATA, static_cast<void*>(this));
    // monitor header
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERFUNCTION, HeaderFunction);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADERDATA, static_cast<void*>(this));

    /* some servers don't like requests that are made without a user-agent
       field, so we provide one */
    curl_easy_setopt(m_curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    // other
    curl_easy_setopt(m_curl_handle, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(m_curl_handle, CURLOPT_VERBOSE, 0L);
    curl_easy_setopt(m_curl_handle, CURLOPT_NOBODY ,0);
    curl_easy_setopt(m_curl_handle, CURLOPT_HEADER, 0);
    curl_easy_setopt(m_curl_handle, CURLOPT_SSL_VERIFYPEER, 0L);
    curl_easy_setopt(m_curl_handle, CURLOPT_SSL_VERIFYHOST, 0L);

    countTimer.Start();
    /* get it! */
    res = curl_easy_perform(m_curl_handle);
    countTimer.Stop();

    res = curl_easy_getinfo(m_curl_handle, CURLINFO_RESPONSE_CODE, &responseCode);
    return res;
}
