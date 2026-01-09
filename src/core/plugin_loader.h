#pragma once

#include <filesystem>
#include <string>
#include <vector>

#include "plugin_api.h"

struct LoadedPlugin {
    std::string path;
    PluginInfo* info = nullptr;
    void* handle = nullptr;
};

struct PluginLoadResult {
    std::vector<LoadedPlugin> plugins;
    std::vector<std::string> errors;
};

PluginLoadResult LoadPlugins(const std::filesystem::path& directory);
void UnloadPlugins(std::vector<LoadedPlugin>& plugins);
std::filesystem::path GetDefaultPluginDir();
