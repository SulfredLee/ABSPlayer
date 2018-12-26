#include "fileDownloader.h"
#include "Logger.h"

fileDownloader::fileDownloader()
{
    m_manager = NULL;
}

fileDownloader::~fileDownloader()
{
    stopThread();
}

void fileDownloader::InitComponent(cmdReceiver* manager)
{
    m_msgQ.InitComponent(1024 * 1024 * 10); // 10 MByte buffer for message queue
    m_manager = manager;
    startThread();
}

size_t fileDownloader::WriteFunction(void *contents, size_t size, size_t nmemb, void *userp)
{
    size_t realsize = size * nmemb;
    PlayerMsg_DownloadFile* msg = static_cast<PlayerMsg_DownloadFile*>(userp);

    msg->SetFile(static_cast<unsigned char*>(contents), realsize);

    return realsize;
}

void fileDownloader::ProcessMsg(PlayerMsg_Base* msg)
{
    LOGMSG_INFO("Process message %s", msg->GetMsgTypeName().c_str());

    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_DownloadFile:
        case PlayerMsg_Type_DownloadMPD:
        case PlayerMsg_Type_DownloadVideo:
        case PlayerMsg_Type_DownloadAudio:
        case PlayerMsg_Type_DownloadSubtitle:
            ProcessMsg(static_cast<PlayerMsg_DownloadFile*>(msg));
            break;
        default:
            break;
    }
}

void fileDownloader::ProcessMsg(PlayerMsg_DownloadFile* msg)
{
    CURL *curl_handle;
    CURLcode res;

    curl_global_init(CURL_GLOBAL_ALL);

    /* init the curl session */
    curl_handle = curl_easy_init();

    /* specify URL to get */
    curl_easy_setopt(curl_handle, CURLOPT_URL, msg->GetURL().c_str());

    /* send all data to this function  */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteFunction);

    /* we pass our 'chunk' struct to the callback function */
    curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, static_cast<void*>(msg));

    /* some servers don't like requests that are made without a user-agent
       field, so we provide one */
    curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");

    /* get it! */
    res = curl_easy_perform(curl_handle);

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

        LOGMSG_INFO("%lu bytes retrieved", msg->GetFileLength());
    }

    /* cleanup curl stuff */
    curl_easy_cleanup(curl_handle);

    /* we're done with libcurl, so clean it up */
    curl_global_cleanup();


    // finished download and alert manager
    PlayerMsg_Base* msgBase = static_cast<PlayerMsg_Base*>(msg);
    if (m_manager) m_manager->UpdateCMDReceiver(msgBase);
}

// override
void fileDownloader::UpdateCMDReceiver(PlayerMsg_Base*& msg)
{
    LOGMSG_INFO("Received message %s", msg->GetMsgTypeName().c_str());

    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_DownloadFile:
        case PlayerMsg_Type_DownloadMPD:
        case PlayerMsg_Type_DownloadVideo:
        case PlayerMsg_Type_DownloadAudio:
        case PlayerMsg_Type_DownloadSubtitle:
            if (!m_msgQ.AddMsg(msg))
            {
                if (msg)
                    delete msg;
            }
            msg = NULL;
            break;
        default:
            break;
    }
}

// override
void* fileDownloader::Main()
{
    LOGMSG_INFO("IN");

    while(isThreadRunning())
    {
        PlayerMsg_Base* msg = NULL;
        m_msgQ.GetMsg(msg);

        ProcessMsg(msg);

        if (msg)
        {
            delete msg;
            msg = NULL;
        }
    }

    LOGMSG_INFO("OUT");
    return NULL;
}
