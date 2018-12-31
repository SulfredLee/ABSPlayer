#ifndef PLAYERMSG_PROCESSNEXTSEGMENT_H
#define PLAYERMSG_PROCESSNEXTSEGMENT_H
#include "PlayerMsg_Base.h"
#include <string>

class PlayerMsg_ProcessNextSegment : public PlayerMsg_Base
{
 public:
    PlayerMsg_ProcessNextSegment();
    ~PlayerMsg_ProcessNextSegment();

    // override
    PlayerMsg_Type GetMsgType();
    // override
    std::string GetMsgTypeName();

    void SetSegmentType(PlayerMsg_Type segmentType);

    PlayerMsg_Type GetSegmentType();
 private:
    PlayerMsg_Type m_nextSegmentType;
};

#endif
