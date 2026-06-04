#pragma once

#include <chrono>
#include <filesystem>
#include <fstream>
#include <stdexcept>
#include <string>
#include <string_view>

namespace tests::common
{

class TemporaryDirectory
{
public:
    explicit TemporaryDirectory(std::string prefix = "symulator_test_")
    {
        const auto nonce = std::chrono::high_resolution_clock::now().time_since_epoch().count();
        path_ = std::filesystem::temp_directory_path() / (prefix + std::to_string(nonce));
        std::filesystem::create_directories(path_);
    }

    ~TemporaryDirectory() { std::filesystem::remove_all(path_); }

    const std::filesystem::path& path() const { return path_; }

private:
    std::filesystem::path path_;
};

inline void write_text(const std::filesystem::path& path, std::string_view content)
{
    if (path.has_parent_path())
    {
        std::filesystem::create_directories(path.parent_path());
    }

    std::ofstream file(path);
    if (!file)
    {
        throw std::runtime_error("failed to open file for writing: " + path.string());
    }

    file << content;
}

inline void create_minimal_fleet_tree(const std::filesystem::path& root)
{
    std::filesystem::create_directories(root / "vehicle_types");
    std::filesystem::create_directories(root / "vehicles");
    std::filesystem::create_directories(root / "trains");
}

}  // namespace tests::common
