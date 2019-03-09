#include "DirtyWriter.h"
#include "Logger.h"

#include <sys/types.h>
#include <sys/stat.h>

#include <fstream>

DirtyWriter::DirtyWriter()
{}

DirtyWriter::~DirtyWriter()
{
    LOGMSG_INFO("Start OUT");
    stopThread();
    m_msgQ.AddMsg(m_msgFactory.CreateMsg(PlayerMsg_Type_Dummy));
    joinThread();
    LOGMSG_INFO("OUT");
}

void DirtyWriter::InitComponent()
{
    m_msgQ.InitComponent(50 * 1024 * 1024); // 50 MByte
    startThread();
}

void DirtyWriter::ProcessMsg(std::shared_ptr<PlayerMsg_Base> msg)
{
    LOGMSG_DEBUG("Process message %s from: %s", msg->GetMsgTypeName().c_str(), msg->GetSender().c_str());

    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_DownloadVideo:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadVideo>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadAudio:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadAudio>(msg));
                break;
            }
        default:
            break;
    }
}

void DirtyWriter::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadVideo> msg)
{
    std::string fileURL = msg->GetURL();
    ReplaceSubstring(fileURL, "http://", "./");
    ReplaceSubstring(fileURL, "https://", "./");

    // get folder path
    std::string folderPath = GetFolderPath(fileURL);
    // create folder is not exist
    if (makePath(folderPath))
    {
        // Save file to disk
        std::vector<unsigned char> file = msg->GetFile();
        SaveFile(fileURL, file, msg->GetFileCount() ? true : false);
    }
}

void DirtyWriter::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadAudio> msg)
{
    std::string fileURL = msg->GetURL();
    ReplaceSubstring(fileURL, "http://", "./");
    ReplaceSubstring(fileURL, "https://", "./");

    // get folder path
    std::string folderPath = GetFolderPath(fileURL);
    // create folder is not exist
    if (makePath(folderPath))
    {
        // Save file to disk
        std::vector<unsigned char> file = msg->GetFile();
        SaveFile(fileURL, file, msg->GetFileCount() ? true : false);
    }
}

bool DirtyWriter::isDirExist(const std::string& path)
{
    struct stat info;
    if (stat(path.c_str(), &info) != 0)
    {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

bool DirtyWriter::makePath(const std::string& path)
{
    mode_t mode = 0755;
    int ret = mkdir(path.c_str(), mode);

    if (ret == 0)
        return true;

    switch (errno)
    {
    case ENOENT:
        // parent didn't exist, try to create it
        {
            size_t pos = path.find_last_of('/');
            if (pos == std::string::npos)
                return false;
            if (!makePath( path.substr(0, pos) ))
                return false;
        }
        // now, try to create again
        return 0 == mkdir(path.c_str(), mode);

    case EEXIST:
        // done!
        return isDirExist(path);

    default:
        return false;
    }
}

void DirtyWriter::SaveFile(std::string fileName, const std::vector<unsigned char>& file, bool isAppend)
{
    std::ofstream FHout;
    if (isAppend)
        FHout.open(fileName, std::ios::app | std::ios::binary);
    else
        FHout.open(fileName, std::ios::out | std::ios::binary);
    FHout.write((char*)&file[0], file.size());
    FHout.close();
}

bool DirtyWriter::ReplaceSubstring(std::string& str, const std::string& from, const std::string& to)
{
    size_t start_pos = str.find(from);
    if(start_pos == std::string::npos)
        return false;
    str.replace(start_pos, from.length(), to);
    return true;
}

void DirtyWriter::ReplaceAllSubstring(std::string& str, const std::string& from, const std::string& to)
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

std::string DirtyWriter::GetFolderPath(const std::string& fullPath)
{
    size_t found = fullPath.find_last_of("/\\");
    return fullPath.substr(0, found);
}

std::string DirtyWriter::GetFileName(const std::string& fullPath)
{
    size_t found = fullPath.find_last_of("/\\");
    return fullPath.substr(found + 1);
}

// override
bool DirtyWriter::UpdateCMD(std::shared_ptr<PlayerMsg_Base> msg)
{
    LOGMSG_DEBUG("Received message %s from: %s", msg->GetMsgTypeName().c_str(), msg->GetSender().c_str());

    bool ret = true;
    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_DownloadVideo:
        case PlayerMsg_Type_DownloadAudio:
        case PlayerMsg_Type_DownloadSubtitle:
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
void* DirtyWriter::Main()
{
    LOGMSG_INFO("IN");

    while(isThreadRunning())
    {
        std::shared_ptr<PlayerMsg_Base> msg;
        m_msgQ.GetMsg(msg);

        if (!isThreadRunning()) break;

        ProcessMsg(msg);
    }

    LOGMSG_INFO("OUT");
    return NULL;
}
