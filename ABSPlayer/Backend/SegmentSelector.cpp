#include "SegmentSelector.h"
#include "Logger.h"

SegmentSelector::SegmentSelector()
{
}

SegmentSelector::~SegmentSelector()
{
    m_eventTimer.DeinitComponent();
    stopThread();
    std::shared_ptr<PlayerMsg_Dummy> msgDummy = std::make_shared<PlayerMsg_Dummy>();
    m_msgQ.AddMsg(std::static_pointer_cast<PlayerMsg_Base>(msgDummy));
}

void SegmentSelector::ProcessMsg(std::shared_ptr<PlayerMsg_Base> msg)
{
    LOGMSG_DEBUG("Process message %s from: %s", msg->GetMsgTypeName().c_str(), msg->GetSender().c_str());

    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_Play:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_Play>(msg));
                break;
            }
        case PlayerMsg_Type_Pause:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_Pause>(msg));
                break;
            }
        case PlayerMsg_Type_Stop:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_Stop>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadMPD:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadMPD>(msg));
                break;
            }
        case PlayerMsg_Type_RefreshMPD:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_RefreshMPD>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadFinish:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_DownloadFinish>(msg));
                break;
            }
        case PlayerMsg_Type_ProcessNextSegment:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_ProcessNextSegment>(msg));
                break;
            }
        case PlayerMsg_Type_UpdateDownloadTime:
            {
                ProcessMsg(std::dynamic_pointer_cast<PlayerMsg_UpdateDownloadTime>(msg));
                break;
            }
        default:
            break;
    }
}

void SegmentSelector::ProcessMsg(std::shared_ptr<PlayerMsg_DownloadMPD> msg)
{
    LOGMSG_INFO("Dummy Process message %s", msg->GetMsgTypeName().c_str());
}

void SegmentSelector::ProcessMsg(std::shared_ptr<PlayerMsg_RefreshMPD> msg)
{
    LOGMSG_INFO("Dummy Process message %s", msg->GetMsgTypeName().c_str());
}

void SegmentSelector::InitComponent(CmdReceiver* manager)
{
    LOGMSG_INFO("IN");
    m_manager = manager;
    m_msgQ.InitComponent(5 * 1024 * 1024);
    m_eventTimer.InitComponent(&m_msgQ);
    startThread();
}

void SegmentSelector::SendToManager(std::shared_ptr<PlayerMsg_Base> msg)
{
    msg->SetSender("SegmentSelector");
    if (m_manager) m_manager->UpdateCMD(msg);
}

// override
bool SegmentSelector::UpdateCMD(std::shared_ptr<PlayerMsg_Base> msg)
{
    LOGMSG_DEBUG("Received message %s from: %s", msg->GetMsgTypeName().c_str(), msg->GetSender().c_str());

    bool ret = true;
    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_ProcessNextSegment:
        case PlayerMsg_Type_UpdateDownloadTime:
        case PlayerMsg_Type_DownloadMPD:
        case PlayerMsg_Type_RefreshMPD:
        case PlayerMsg_Type_DownloadFinish:
        case PlayerMsg_Type_Play:
        case PlayerMsg_Type_Pause:
        case PlayerMsg_Type_Stop:
            {
                if (!m_msgQ.AddMsg(msg))
                {
                    LOGMSG_ERROR("AddMsg fail");
                    ret = false;
                }
                break;
            }
        default:
            break;
    }
    return ret;
}

// override
void* SegmentSelector::Main()
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
