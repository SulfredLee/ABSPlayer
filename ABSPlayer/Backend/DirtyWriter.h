#ifndef DIRTYWRITER_H
#define DIRTYWRITER_H
#include "LinuxThread.h"
#include "CmdReceiver.h"
#include "PlayerMsgQ.h"
#include "PlayerMsg_Factory.h"
#include "SmartPointer.h"

#include <string>
#include <vector>

class DirtyWriter : public LinuxThread, public CmdReceiver
{
 public:
    DirtyWriter();
    ~DirtyWriter();

    void InitComponent();
    // override
    bool UpdateCMD(SmartPointer<PlayerMsg_Base> msg);
 private:
    void ProcessMsg(SmartPointer<PlayerMsg_Base> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_DownloadVideo> msg);
    void ProcessMsg(SmartPointer<PlayerMsg_DownloadAudio> msg);
    // override
    void* Main();

    // Tools
    bool ReplaceSubstring(std::string& str, const std::string& from, const std::string& to);
    void ReplaceAllSubstring(std::string& str, const std::string& from, const std::string& to);
    std::string GetFolderPath(const std::string& fullPath);
    std::string GetFileName(const std::string& fillPath);
    bool isDirExist(const std::string& path);
    bool makePath(const std::string& path);
    void SaveFile(std::string fileName, const std::vector<unsigned char>& file, bool isAppend);
 private:
    PlayerMsgQ m_msgQ;
    PlayerMsg_Factory m_msgFactory;
};

#endif
