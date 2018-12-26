#ifndef FILE_DOWNLOADER_H
#define FILE_DOWNLOADER_H
#include "linuxThread.h"
#include "cmdReceiver.h"
#include "playerMsgQ.h"
#include "PlayerMsg_DownloadFile.h"

#include <stdio.h>
#include <curl/curl.h>

class fileDownloader : public linuxThread, public cmdReceiver
{
 public:
    fileDownloader();
    ~fileDownloader();

    void InitComponent(cmdReceiver* manager);
    void DeinitComponent();
    // override
    void UpdateCMDReceiver(PlayerMsg_Base*& msg);
 private:
    static size_t WriteFunction(void *contents, size_t size, size_t nmemb, void *userp);
    void ProcessMsg(PlayerMsg_Base* msg);
    void ProcessMsg(PlayerMsg_DownloadFile* msg);
    // override
    void* Main();
 private:
    playerMsgQ m_msgQ;
    cmdReceiver* m_manager;
};
#endif
