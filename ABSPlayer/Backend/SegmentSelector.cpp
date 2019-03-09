#include "SegmentSelector.h"
#include "Logger.h"

SegmentSelector::SegmentSelector()
{
}

SegmentSelector::~SegmentSelector()
{
    LOGMSG_INFO("Start OUT");
    m_eventTimer.DeinitComponent();
    stopThread();
    SmartPointer<PlayerMsg_Dummy> msgDummy = MakeSmartPointer<PlayerMsg_Dummy>();
    m_msgQ.AddMsg(StaticCast<PlayerMsg_Base>(msgDummy));
    joinThread();
    LOGMSG_INFO("OUT");
}

void SegmentSelector::ProcessMsg(SmartPointer<PlayerMsg_Base> msg)
{
    LOGMSG_DEBUG("Process message %s from: %s", msg->GetMsgTypeName().c_str(), msg->GetSender().c_str());

    switch(msg->GetMsgType())
    {
        case PlayerMsg_Type_Play:
            {
                ProcessMsg(DynamicCast<PlayerMsg_Play>(msg));
                break;
            }
        case PlayerMsg_Type_Pause:
            {
                ProcessMsg(DynamicCast<PlayerMsg_Pause>(msg));
                break;
            }
        case PlayerMsg_Type_Stop:
            {
                ProcessMsg(DynamicCast<PlayerMsg_Stop>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadMPD:
            {
                ProcessMsg(DynamicCast<PlayerMsg_DownloadMPD>(msg));
                break;
            }
        case PlayerMsg_Type_RefreshMPD:
            {
                ProcessMsg(DynamicCast<PlayerMsg_RefreshMPD>(msg));
                break;
            }
        case PlayerMsg_Type_DownloadFinish:
            {
                ProcessMsg(DynamicCast<PlayerMsg_DownloadFinish>(msg));
                break;
            }
        case PlayerMsg_Type_ProcessNextSegment:
            {
                ProcessMsg(DynamicCast<PlayerMsg_ProcessNextSegment>(msg));
                break;
            }
        case PlayerMsg_Type_UpdateDownloadTime:
            {
                ProcessMsg(DynamicCast<PlayerMsg_UpdateDownloadTime>(msg));
                break;
            }
        default:
            break;
    }
}

void SegmentSelector::ProcessMsg(SmartPointer<PlayerMsg_DownloadMPD> msg)
{
    LOGMSG_INFO("Dummy Process message %s", msg->GetMsgTypeName().c_str());
}

void SegmentSelector::ProcessMsg(SmartPointer<PlayerMsg_RefreshMPD> msg)
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

void SegmentSelector::SendToManager(SmartPointer<PlayerMsg_Base> msg)
{
    msg->SetSender("SegmentSelector");
    if (m_manager) m_manager->UpdateCMD(msg);
}

// override
bool SegmentSelector::UpdateCMD(SmartPointer<PlayerMsg_Base> msg)
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
        SmartPointer<PlayerMsg_Base> msg;
        m_msgQ.GetMsg(msg);

        ProcessMsg(msg);
    }

    LOGMSG_INFO("OUT");
    return NULL;
}
