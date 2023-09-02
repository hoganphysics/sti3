

#include <sti/device/ProfileManager.h>

#include <sti/device/Profile.h>
#include "MixedValuePy.h"
#include "ProfilePy.h"

#include <string>
#include <memory>

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/cast.h>

namespace py = pybind11;

using STI::Device::Profile;
using STI::Device::ProfileManager;
using STI::Python::ProfilePy;

void init_ProfileManager(py::module& m)
{

    py::class_<ProfileManager, std::shared_ptr<ProfileManager>>(m, "ProfileManager")
        .def("getProfiles",
            [](const ProfileManager& self) {
                std::set<std::string> names;
                self.getProfiles(names);
                return names;
            })
        .def("getProfile",
            [](const ProfileManager& self, const std::string& name) {
                std::shared_ptr<Profile> profile;
                std::shared_ptr<ProfilePy> profilePy;
                if (self.getProfile(name, profile) && profile != 0) {
                    profilePy = std::make_shared<ProfilePy>(*profile);
                    return profilePy;
                }
                //failed to find profile; return new
                profilePy = std::make_shared<ProfilePy>();
                profilePy->name = name;
                profilePy->type = STI::Device::ProfileType::All;
                return profilePy;
            }, py::arg("name"))
        //.def("saveProfile", &ProfileManager::saveProfile, py::arg("profile"))
        .def("saveProfile", [](ProfileManager& self, const std::shared_ptr<ProfilePy>& profilePy) {
                if (profilePy == 0) return false;

                auto profile = profilePy->toProfile();  //convert to STI::Device::Profile
                return self.saveProfile(profile);
            }, py::arg("profile"))

        .def("loadProfile", &ProfileManager::loadProfile, py::arg("name"), py::arg("type"), py::arg("loadDependentDevices"))
        .def("saveCurrentProfile", &ProfileManager::saveCurrentProfile, py::arg("name"), py::arg("type"), py::arg("saveDependentDevices"))
        
        ;

}

