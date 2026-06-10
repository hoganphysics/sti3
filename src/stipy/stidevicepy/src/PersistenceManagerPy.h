
#ifndef STI_PYTHON_PERSISTENCEMANAGER_H
#define STI_PYTHON_PERSISTENCEMANAGER_H

#include <sti/device/PersistenceManager.h>

#include <memory>
#include <string>



namespace STI
{
namespace Python
{

class PersistenceManagerPy
{
public:

    PersistenceManagerPy(const std::shared_ptr<STI::Device::PersistenceManager>& manager);
    ~PersistenceManagerPy();

    bool findShot(const STI::Engine::ShotID& sid);
    std::shared_ptr<STI::Engine::ParseResult> getParseResult(const STI::Engine::ParseID& pid);
    std::shared_ptr<STI::Engine::ShotResult> getShotResult(const STI::Engine::ShotID& sid);
    std::shared_ptr<STI::Engine::SequenceResult> getSequenceResult(const STI::Engine::SequenceID& id);
    std::shared_ptr<STI::Utils::FileHolder> makeFileHolder(const std::string& path, const std::string& filename);
    std::shared_ptr<STI::Utils::FileHolder> makeVirtualFileHolder(const STI::Utils::FileID& fileID);
    std::shared_ptr<STI::Utils::FileHolder> makeVirtualFileHolder(
        const std::shared_ptr<STI::Utils::VirtualFileHolder>& backingHolder);
    std::shared_ptr<STI::Utils::VirtualFileServer> makeVirtualFileServer();
    std::shared_ptr<STI::Utils::FileServer> getFileServer();
    std::string getBasePath() const;
    std::string getTemporaryPath() const;

	STI::Engine::MeasurementMap getMeasurements(const STI::Engine::ShotID& sid);

private:

    std::shared_ptr<STI::Device::PersistenceManager> persistenceManager;
};


} //Python
} //STI

#endif
