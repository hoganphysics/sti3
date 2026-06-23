#include "Convert_PostProcessing.h"

#include <string>

using STI::Device::PostProcessingOptionInfo;
using STI::Device::PostProcessingTargetInfo;
using STI::TNetwork::TPostProcessingOptionInfo;
using STI::TNetwork::TPostProcessingTargetInfo;


//PostProcessingOptionInfo
template<>
bool STI::Network::convert<PostProcessingOptionInfo, TPostProcessingOptionInfo>(
        const PostProcessingOptionInfo& option, TPostProcessingOptionInfo& tOption)
{
    tOption.name = convert<std::string, CORBA::String_member>(option.name);
    tOption.description = convert<std::string, CORBA::String_member>(option.description);
    return true;
}

template<>
bool STI::Network::convert<TPostProcessingOptionInfo, PostProcessingOptionInfo>(
        const TPostProcessingOptionInfo& tOption, PostProcessingOptionInfo& option)
{
    option.name = convert<CORBA::String_member, std::string>(tOption.name);
    option.description = convert<CORBA::String_member, std::string>(tOption.description);
    return true;
}

template<>
PostProcessingOptionInfo STI::Network::convert<TPostProcessingOptionInfo, PostProcessingOptionInfo>(
        const TPostProcessingOptionInfo& tOption)
{
    PostProcessingOptionInfo option;
    convert<TPostProcessingOptionInfo, PostProcessingOptionInfo>(tOption, option);
    return option;
}


//PostProcessingTargetInfo
template<>
bool STI::Network::convert<PostProcessingTargetInfo, TPostProcessingTargetInfo>(
        const PostProcessingTargetInfo& info, TPostProcessingTargetInfo& tInfo)
{
    tInfo.name = convert<std::string, CORBA::String_member>(info.name);
    tInfo.description = convert<std::string, CORBA::String_member>(info.description);
    convert<PostProcessingOptionInfo, TPostProcessingOptionInfo>(info.options, tInfo.options);   //vector -> seq
    return true;
}

template<>
bool STI::Network::convert<TPostProcessingTargetInfo, PostProcessingTargetInfo>(
        const TPostProcessingTargetInfo& tInfo, PostProcessingTargetInfo& info)
{
    info.name = convert<CORBA::String_member, std::string>(tInfo.name);
    info.description = convert<CORBA::String_member, std::string>(tInfo.description);
    convert<TPostProcessingOptionInfo, PostProcessingOptionInfo>(tInfo.options, info.options);   //seq -> vector
    return true;
}

template<>
PostProcessingTargetInfo STI::Network::convert<TPostProcessingTargetInfo, PostProcessingTargetInfo>(
        const TPostProcessingTargetInfo& tInfo)
{
    PostProcessingTargetInfo info;
    convert<TPostProcessingTargetInfo, PostProcessingTargetInfo>(tInfo, info);
    return info;
}
