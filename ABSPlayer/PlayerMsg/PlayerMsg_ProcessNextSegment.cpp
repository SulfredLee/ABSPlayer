#include "PlayerMsg_ProcessNextSegment.h"

PlayerMsg_ProcessNextSegment::PlayerMsg_ProcessNextSegment()
{
}

PlayerMsg_ProcessNextSegment::~PlayerMsg_ProcessNextSegment()
{
}

// override
PlayerMsg_Type PlayerMsg_ProcessNextSegment::GetMsgType()
{
    return PlayerMsg_Type_ProcessNextSegment;
}

// override
std::string PlayerMsg_ProcessNextSegment::GetMsgTypeName()
{
    return "PlayerMsg_Type_ProcessNextSegment";
}

void PlayerMsg_ProcessNextSegment::SetSegmentType(PlayerMsg_Type segmentType)
{
    m_nextSegmentType = segmentType;
}

PlayerMsg_Type PlayerMsg_ProcessNextSegment::GetSegmentType()
{
    return m_nextSegmentType;
}
