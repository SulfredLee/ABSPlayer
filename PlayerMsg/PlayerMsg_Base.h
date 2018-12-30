#ifndef PLAYERMSG_BASE_H
#define PLAYERMSG_BASE_H
#include <string>

enum PlayerMsg_Type
{
    PlayerMsg_Type_Base,
    PlayerMsg_Type_Dummy,
    PlayerMsg_Type_Open,
    PlayerMsg_Type_Play,
    PlayerMsg_Type_Pause,
    PlayerMsg_Type_Stop,
    PlayerMsg_Type_GetPlayerStage,
    PlayerMsg_Type_DownloadFile,
    PlayerMsg_Type_DownloadMPD,
    PlayerMsg_Type_DownloadVideo,
    PlayerMsg_Type_DownloadAudio,
    PlayerMsg_Type_DownloadSubtitle,
    PlayerMsg_Type_DownloadFinish,
    PlayerMsg_Type_DownloadFinishV,
    PlayerMsg_Type_DownloadFinishA,
    PlayerMsg_Type_DownloadFinishS,
    PlayerMsg_Type_End
};

class PlayerMsg_Base
{
 public:
    PlayerMsg_Base();
    virtual ~PlayerMsg_Base();

    virtual PlayerMsg_Type GetMsgType();
    virtual std::string GetMsgTypeName();
    virtual int GetMsgSize(); // only report message size when message has media data
    void SetSender(std::string sender);
    std::string GetSender();
 protected:
    std::string m_sender;
};

#endif
