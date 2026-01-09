#include "RemoteProfileManager.h"

#include "convert/Convert_Profile.h"

using STI::Network::RemoteProfileManager;
using STI::TNetwork::TReferenceHolder;
using STI::Network::convert;
using ::STI::TNetwork::TProfileManager;
using STI::Device::Profile;
using STI::TNetwork::TProfile;
using STI::Device::ProfileType;
using STI::TNetwork::TProfileType;


RemoteProfileManager::RemoteProfileManager(::STI::TNetwork::TProfileManager_ptr manager)
: TReferenceHolder<TProfileManager>(manager, profileMutex)
{
}

RemoteProfileManager::~RemoteProfileManager()
{
}

void RemoteProfileManager::getProfiles(std::set<std::string>& names) const
{
	std::unique_lock<std::mutex> profileLock(profileMutex);

	if (isDisabled()) return;

	STI::TNetwork::TStringSeq_var tNames(new STI::TNetwork::TStringSeq);

	names.clear();

	try {
		getTRef()->getProfiles(tNames);	//remote call
		
		std::vector<std::string> namesVec;
		convert<STI::TNetwork::TStringSeq, std::vector<std::string>>(tNames, namesVec);		//only vector<string> is available
		
		names.insert(namesVec.begin(), namesVec.end());		//deep copy, but names should be very short
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}
}

bool RemoteProfileManager::getProfile(const std::string& name, std::shared_ptr<Profile>& profile) const
{
	std::unique_lock<std::mutex> profileLock(profileMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TProfile_var tProfile(new STI::TNetwork::TProfile);
	profile = std::make_shared<Profile>();

	bool success = false;

	try {
		success = getTRef()->getProfile(convert<std::string, ::CORBA::String_member>(name), tProfile);	//remote call

		convert<TProfile, Profile>(tProfile, *profile);
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

	return success && (profile != 0);
}

bool RemoteProfileManager::saveProfile(const std::shared_ptr<STI::Device::Profile>& profile)
{
	if (profile == 0) return false;

	std::unique_lock<std::mutex> profileLock(profileMutex);

	if (isDisabled()) return false;

	STI::TNetwork::TProfile_var tProfile(new STI::TNetwork::TProfile);
	convert<Profile, TProfile>(*profile, tProfile);

	bool success = false;

	try {
		success = getTRef()->saveProfile(tProfile);	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

	return success;
}

bool RemoteProfileManager::setReadOnly(const std::string& name, bool readOnly)
{
	std::unique_lock<std::mutex> profileLock(profileMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->setReadOnly(
			convert<std::string, ::CORBA::String_member>(name),
			static_cast<::CORBA::Boolean>(readOnly));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

	return success;
}

bool RemoteProfileManager::loadProfile(const std::string& name, const STI::Device::ProfileType& type, bool loadDependentDevices)
{
	std::unique_lock<std::mutex> profileLock(profileMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->loadProfile(
				convert<std::string, ::CORBA::String_member>(name), 
				convert<ProfileType, TProfileType>(type),
				static_cast<::CORBA::Boolean>(loadDependentDevices));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

	return success;
}

bool RemoteProfileManager::saveCurrentProfile(const std::string& name, const STI::Device::ProfileType& type, bool saveDependentDevices)
{
	std::unique_lock<std::mutex> profileLock(profileMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->saveCurrentProfile(
			convert<std::string, ::CORBA::String_member>(name),
			convert<ProfileType, TProfileType>(type),
			static_cast<::CORBA::Boolean>(saveDependentDevices));	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&) {
	}

	return success;
}


bool RemoteProfileManager::ping() const
{
	std::unique_lock<std::mutex> profileLock(profileMutex);

	if (isDisabled()) return false;

	bool success = false;

	try {
		success = getTRef()->ping();	//remote call
	}
	catch (CORBA::TRANSIENT&) {
	}
	catch (CORBA::SystemException&) {
	}
	catch (CORBA::Exception&)
	{
	}

	return success;
}
