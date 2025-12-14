#ifndef STI_DEVICE_FILEHOLDER_TESTS_SUPPORT_H
#define STI_DEVICE_FILEHOLDER_TESTS_SUPPORT_H

#include <sti/device/DeviceID.h>
#include <sti/utils/FileID.h>

#include <chrono>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <random>
#include <string>

namespace fileholder_test_support {

struct TempDir {
    std::filesystem::path path;

    TempDir(std::string prefix = "sti3-fileholder-") {
        auto base = std::filesystem::temp_directory_path();
        auto now = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        std::mt19937_64 rng(static_cast<uint64_t>(now));
        auto suffix = rng();
        path = base / (prefix + std::to_string(now) + "-" + std::to_string(suffix));
        std::filesystem::create_directories(path);
    }

    ~TempDir() {
        std::error_code ec;
        std::filesystem::remove_all(path, ec);
    }
};

inline std::string readFileToString(const std::filesystem::path& file) {
    std::ifstream ifs(file, std::ios::binary);
    return std::string(std::istreambuf_iterator<char>(ifs), std::istreambuf_iterator<char>());
}

inline STI::Device::DeviceID makeTestDeviceID() {
    return STI::Device::DeviceID("Local", "127.0.0.1", 1);
}

inline STI::Utils::FileID makeFileID(const STI::Device::DeviceID& device, const std::filesystem::path& path, const std::string& filename) {
    STI::Utils::FileID fid;
    fid.path = path.string();
    fid.filename = filename;
    fid.origin = device.getID();
    fid.persistenceLocation = device.getID();
    return fid;
}

} // namespace fileholder_test_support

#endif
