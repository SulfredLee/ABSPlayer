#include "DashSegmentInfoGreper.h"
#include "Logger.h"

DashSegmentInfoGreper::DashSegmentInfoGreper()
{
}

DashSegmentInfoGreper::~DashSegmentInfoGreper()
{
}

bool DashSegmentInfoGreper::IsHasSegmentTemplate(std::shared_ptr<dash::mpd::IMPD> mpdFile)
{
    m_mpdFile = mpdFile;
    std::vector<dash::mpd::IPeriod *> periods = m_mpdFile->GetPeriods();
    for (size_t i = 0; i < periods.size(); i++) // loop for every period
    {
        dash::mpd::IPeriod* period = periods[i];
        std::vector<dash::mpd::IAdaptationSet *> adaptationsSets = period->GetAdaptationSets();
        for (size_t j = 0; j < adaptationsSets.size(); j++) // loop for every adaptation set
        {
            dash::mpd::IAdaptationSet* adaptationSet = adaptationsSets[j];
            std::vector<dash::mpd::IRepresentation *> representations = adaptationSet->GetRepresentation();
            for (size_t k = 0; k < representations.size(); k++) // loop for every representation
            {
                dash::mpd::IRepresentation* representation = representations[k];
                dash::mpd::ISegmentTemplate* segmentTemplate = representation->GetSegmentTemplate() ? representation->GetSegmentTemplate() : adaptationSet->GetSegmentTemplate();
                if (segmentTemplate)
                    return true;
            }
        }
    }

    return false;
}
