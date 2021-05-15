#ifndef STI_ENGINE_ENGINEJOBID_H
#define STI_ENGINE_ENGINEJOBID_H

#include "ParseID.h"
#include "ShotID.h"


namespace STI
{
namespace Engine
{

enum class EventEngineJobType { Parse, Play };


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
            } 
        }
        else {
            //different types
            
            TimeStamp rhsTime;
            switch(rhs.type) {
                case EventEngineJobType::Parse:
                    rhsTime = rhs.pid.parseTimestamp;
                break;
                case EventEngineJobType::Play:
                    rhsTime = rhs.sid.submissionTime;
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
                    if(pid == rhs.pid) {
                        return false;
                    }
                    else {
                        return sid.submissionTime < rhsTime;
                    }
                break;
            } 
        }
        return false;
    }
    
    EventEngineJobType type;
    ParseID pid;
    ShotID sid;

};


} //Engine
} //STI

#endif
