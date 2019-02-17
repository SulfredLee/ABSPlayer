#ifndef FILE_DOWNLOADER_H
#define FILE_DOWNLOADER_H
#include "LinuxThread.h"
#include "CmdReceiver.h"
#include "PlayerMsgQ.h"
#include "PlayerMsg_Common.h"
#include "PlayerMsg_Factory.h"
#include "CountTimer.h"
#include "SmartPointer.h"

#include <stdio.h>
#include <curl/curl.h>

class FileDownloader : public LinuxThread, public CmdReceiver
{
 public:
    FileDownloader();
    ~FileDownloader();

    void InitComponent(CmdReceiver* manager, const std::string& thisName);
    void DeinitComponent();
    // override
    bool UpdateCMD(SmartPointer<PlayerMsg_Base> msg);
 private:
    static size_t WriteFunction(void *contents, size_t size, size_t nmemb, void *userp);
    static int ProgressFunction(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow);
    static size_t HeaderFunction(char *buffer, size_t size, size_t nitems, void *userdata);
    void SaveToPool(void *contents, size_t size);
    void SendPartOfMsg(SmartPointer<PlayerMsg_DownloadFile> msgFile, void *contents, size_t size);
    void ProcessMsg(SmartPointer<PlayerMsg_Base> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_DownloadFile> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_DownloadMPD> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_RefreshMPD> msg);

    CURLcode DownloadAFile(SmartPointer<PlayerMsg_DownloadFile> msg, CountTimer& countTimer, int32_t& responseCode);
    void SendToManager(SmartPointer<PlayerMsg_Base> msg);
    void SendDownloadFinishedMsg(const CountTimer& countTimer, SmartPointer<PlayerMsg_DownloadFile> msg);
    // override
    void* Main();
 private:
    PlayerMsgQ m_msgQ;
    CmdReceiver* m_manager;
    PlayerMsg_Factory m_msgFactory;
    CURL *m_curl_handle;
    // Handle sending small part of message
    SmartPointer<PlayerMsg_DownloadFile> m_msgPool;
    size_t m_msgPoolSize;
    size_t m_msgPoolCount;
};
#endif
