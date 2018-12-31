#ifndef DIRTYWRITER_H
#define DIRTYWRITER_H
#include "linuxThread.h"
#include "cmdReceiver.h"
#include "playerMsgQ.h"
#include "PlayerMsg_Factory.h"

#include <memory>
#include <string>
#include <vector>

class dirtyWriter : public linuxThread, public cmdReceiver
{
 public:
    dirtyWriter();
    ~dirtyWriter();

    void InitComponent();
    // override
    bool UpdateCMD(std::shared_ptr<PlayerMsg_Base> msg);
 private:
    void ProcessMsg(std::shared_ptr<PlayerMsg_Base> msg);
    void ProcessMsg(std::shared_ptr<PlayerMsg_DownloadVideo> msg);
    // override
    void* Main();

    // Tools
    bool ReplaceSubstring(std::string& str, const std::string& from, const std::string& to);
    void ReplaceAllSubstring(std::string& str, const std::string& from, const std::string& to);
    std::string GetFolderPath(const std::string& fullPath);
    std::string GetFileName(const std::string& fillPath);
    bool isDirExist(const std::string& path);
    bool makePath(const std::string& path);
    void SaveFile(std::string fileName, const std::vector<unsigned char>& file);
 private:
    playerMsgQ m_msgQ;
    PlayerMsg_Factory m_msgFactory;
};

#endif
