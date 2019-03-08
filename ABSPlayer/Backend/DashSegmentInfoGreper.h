#ifndef DASH_SEGMENT_INFO_GREPER_H
#define DASH_SEGMENT_INFO_GREPER_H
#include "SmartPointer.h"
#include "IMPD.h"
#include "libdash.h"

class DashSegmentInfoGreper
{
 public:
    DashSegmentInfoGreper();
    ~DashSegmentInfoGreper();

    bool IsHasSegmentTemplate(SmartPointer<dash::mpd::IMPD> mpdFile);
    // TODO: get message to download init data
    // TODO: generate segment structure
 private:
    SmartPointer<dash::mpd::IMPD> m_mpdFile;
};

#endif
