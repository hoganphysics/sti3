#ifndef STI_ENGINE_ENGINECONFLICTPOLICY_H
#define STI_ENGINE_ENGINECONFLICTPOLICY_H

namespace STI
{
namespace Engine
{

class EngineID;

class EngineConflictPolicy
{
public:
	virtual ~EngineConflictPolicy() {}

	virtual bool parseWhenParsing(const EngineID& parsingID, const EngineID& engineID) const = 0;
    virtual bool playWhenParsing(const EngineID& parsingID, const EngineID& engineID) const = 0;
    virtual bool parseWhenPlaying(const EngineID& playingID, const EngineID& engineID) const = 0;
    
    virtual bool unloadAfterParsing(const EngineID& parsingID, const EngineID& engineID) const = 0;
    virtual bool unloadAfterPlaying(const EngineID& playingID, const EngineID& engineID) const = 0;
};

class EngineConflictPolicyDefault : public EngineConflictPolicy
{
public:
    virtual bool parseWhenParsing(const EngineID& parsingID, const EngineID& engineID) const override { return true; }
    virtual bool playWhenParsing(const EngineID& parsingID, const EngineID& engineID) const override { return true; }
    virtual bool parseWhenPlaying(const EngineID& playingID, const EngineID& engineID) const override { return true; }

    virtual bool unloadAfterParsing(const EngineID& parsingID, const EngineID& engineID) const override { return false; }
    virtual bool unloadAfterPlaying(const EngineID& playingID, const EngineID& engineID) const override { return false; }
};


} //Engine
} //STI

#endif
