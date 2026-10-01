#pragma once

#include <filesystem>
#include <functional>
#include <optional>
#include <string>
#include <vector>

namespace SohFilePicker {
using Callback = std::function<void(std::optional<std::filesystem::path>)>;

bool IsAvailable();
void PickFile(const std::string& title, const std::vector<std::string>& filters, Callback onResult);
void Pump();
bool IsStagedCopy(const std::filesystem::path& path);
std::string FilesAppFolder();
} // namespace SohFilePicker
