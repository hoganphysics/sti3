#ifndef STI_ENGINE_RAWEVENTTARGETCHANNEL_H
#define STI_ENGINE_RAWEVENTTARGETCHANNEL_H



#include <string>


namespace STI
{
namespace Engine
{


class RawEventTargetChannel
{
public:

    RawEventTargetChannel(const std::string& name);
    RawEventTargetChannel(unsigned short channel);

    bool isAbstract() const;
    std::string name() const;
    unsigned short channel() const;

    void setChannel(unsigned short channel);

    bool operator<(const RawEventTargetChannel& rhs) const;
	bool operator==(const RawEventTargetChannel& rhs) const;
	bool operator!=(const RawEventTargetChannel& rhs) const;

    template<class Archive>
    void serialize(Archive& archive);

private:

    bool _isAbstract;
    std::string _name;
    unsigned short _channel;
};


} //Engine
} //STI

#endif
