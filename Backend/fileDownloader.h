#ifndef FILE_DOWNLOADER_H
#define FILE_DOWNLOADER_H
#include "linuxThread.h"
#include "cmdReceiver.h"
#include "playerMsgQ.h"
#include "PlayerMsg_Common.h"
#include "PlayerMsg_Factory.h"
#include "CountTimer.h"

#include <stdio.h>
#include <curl/curl.h>
#include <memory>

class fileDownloader : public linuxThread, public cmdReceiver
{
 public:
    fileDownloader();
    ~fileDownloader();

    void InitComponent(cmdReceiver* manager);
    void DeinitComponent();
    // override
    void UpdateCMD(std::shared_ptr<PlayerMsg_Base> msg);
 private:
    static size_t WriteFunction(void *contents, size_t size, size_t nmemb, void *userp);
    void ProcessMsg(std::shared_ptr<PlayerMsg_Base> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadFile> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadMPD> msg);

    void SendToManager(std::shared_ptr<PlayerMsg_Base> msg);
    void SendDownloadFinishedMsg(const CountTimer& countTimer, std::shared_ptr<PlayerMsg_DownloadFile> msg);
    // override
    void* Main();
 private:
    playerMsgQ m_msgQ;
    cmdReceiver* m_manager;
    PlayerMsg_Factory m_msgFactory;
    CURL *m_curl_handle;
};
#endif
