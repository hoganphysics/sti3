#ifndef STI_ENGINE_ENGINEJOBID_H
#define STI_ENGINE_ENGINEJOBID_H

#include <sti/engine/ParseID.h>
#include <sti/engine/ShotID.h>


namespace STI
{
namespace Engine
{

enum class EventEngineJobType { Parse, Play, Sequence };


class EngineJobID
{
public:
    
    bool operator==(const EngineJobID& rhs) const
    {
        if(type == rhs.type) {
            switch(type) {
                case EventEngineJobType::Parse:
                    return pid == rhs.pid;
                break;
                case EventEngineJobType::Play:
                    return sid == rhs.sid;
                break;
                case EventEngineJobType::Sequence:
                    return seqid == rhs.seqid;
                break;
            } 
        }
        return false;
    }

    bool operator<(const EngineJobID& rhs) const
    {
        if(type == rhs.type) {
            switch(type) {
                case EventEngineJobType::Parse:
                    return pid < rhs.pid;
                break;
                case EventEngineJobType::Play:
                    return sid < rhs.sid;
                break;
                case EventEngineJobType::Sequence:
                    return seqid < rhs.seqid;
                break;
            } 
        }
        else {
            //different types
            
            STI::Utils::TimeStamp rhsTime;
            switch(rhs.type) {
                case EventEngineJobType::Parse:
                    rhsTime = rhs.pid.parseTimestamp;
                break;
                case EventEngineJobType::Play:
                    rhsTime = rhs.sid.submissionTime;
                break;
                case EventEngineJobType::Sequence:
                    rhsTime = rhs.seqid.timestamp;
                break;
            }

            switch(type) {
                case EventEngineJobType::Parse:
                    if(pid == rhs.pid) {
                        return true;
                    }
                    else {
                        return pid.parseTimestamp < rhsTime;
                    }
                break;
                case EventEngineJobType::Play:
                    if(rhs.type == EventEngineJobType::Parse && pid == rhs.pid) {
                        return false; //parse jobs are always before the associated play job
                    }
                    else {
                        return sid.submissionTime < rhsTime;
                    }
                break;
                case EventEngineJobType::Sequence:
                    return seqid.timestamp < rhsTime;
                break;
            } 
        }
        return false;
    }
    
    EventEngineJobType type;
    
    ParseID pid;
    ShotID sid;
    SequenceID seqid;

    STI::Utils::TimeStamp runTime;  //time job was run

};


} //Engine
} //STI

#endif
