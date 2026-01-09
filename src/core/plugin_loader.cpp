#include "plugin_loader.h"

#include <filesystem>
#include <sstream>

#if defined(_WIN32)
#include <windows.h>
#else
#include <dlfcn.h>
#include <unistd.h>
#endif

namespace {

#if defined(_WIN32)
using LibHandle = HMODULE;
#else
using LibHandle = void*;
#endif

bool HasLibraryExtension(const std::filesystem::path& path) {
#if defined(_WIN32)
    return path.extension() == ".dll";
#elif defined(__APPLE__)
    return path.extension() == ".dylib";
#else
    return path.extension() == ".so";
#endif
}

LibHandle OpenLibrary(const std::filesystem::path& path, std::string& error) {
#if defined(_WIN32)
    HMODULE handle = LoadLibraryA(path.string().c_str());
    if (!handle) {
        error = "LoadLibrary failed";
    }
    return handle;
#else
    LibHandle handle = dlopen(path.string().c_str(), RTLD_LAZY);
    if (!handle) {
        const char* dl_error = dlerror();
        error = dl_error ? dl_error : "dlopen failed";
    }
    return handle;
#endif
}

void CloseLibrary(LibHandle handle) {
#if defined(_WIN32)
    if (handle) {
        FreeLibrary(handle);
    }
#else
    if (handle) {
        dlclose(handle);
    }
#endif
}

PluginInfo* ResolvePluginInfo(LibHandle handle, std::string& error) {
#if defined(_WIN32)
    auto symbol = GetProcAddress(handle, "GetPluginInfo");
    if (!symbol) {
        error = "GetProcAddress(GetPluginInfo) failed";
        return nullptr;
    }
    auto func = reinterpret_cast<PluginInfo*(*)()>(symbol);
    return func();
#else
    void* symbol = dlsym(handle, "GetPluginInfo");
    if (!symbol) {
        const char* dl_error = dlerror();
        error = dl_error ? dl_error : "dlsym(GetPluginInfo) failed";
        return nullptr;
    }
    auto func = reinterpret_cast<PluginInfo*(*)()>(symbol);
    return func();
#endif
}

std::filesystem::path GetExecutablePath() {
#if defined(_WIN32)
    char buffer[MAX_PATH] = {};
    DWORD size = GetModuleFileNameA(nullptr, buffer, MAX_PATH);
    return std::filesystem::path(std::string(buffer, size));
#else
    char buffer[4096] = {};
    ssize_t size = readlink("/proc/self/exe", buffer, sizeof(buffer) - 1);
    if (size <= 0) {
        return {};
    }
    return std::filesystem::path(std::string(buffer, static_cast<size_t>(size)));
#endif
}

} // namespace

PluginLoadResult LoadPlugins(const std::filesystem::path& directory) {
    PluginLoadResult result;
    if (!std::filesystem::exists(directory)) {
        std::ostringstream oss;
        oss << "Plugin directory not found: " << directory.string();
        result.errors.push_back(oss.str());
        return result;
    }

    for (const auto& entry : std::filesystem::directory_iterator(directory)) {
        if (!entry.is_regular_file()) {
            continue;
        }
        const auto& path = entry.path();
        if (!HasLibraryExtension(path)) {
            continue;
        }

        std::string error;
        LibHandle handle = OpenLibrary(path, error);
        if (!handle) {
            result.errors.push_back(path.string() + ": " + error);
            continue;
        }

        PluginInfo* info = ResolvePluginInfo(handle, error);
        if (!info || !info->on_frame) {
            result.errors.push_back(path.string() + ": invalid plugin api");
            CloseLibrary(handle);
            continue;
        }

        LoadedPlugin plugin;
        plugin.path = path.string();
        plugin.info = info;
        plugin.handle = handle;
        result.plugins.push_back(plugin);
    }

    return result;
}

void UnloadPlugins(std::vector<LoadedPlugin>& plugins) {
    for (auto& plugin : plugins) {
        CloseLibrary(static_cast<LibHandle>(plugin.handle));
        plugin.handle = nullptr;
        plugin.info = nullptr;
    }
    plugins.clear();
}

std::filesystem::path GetDefaultPluginDir() {
    std::filesystem::path exe = GetExecutablePath();
    if (exe.empty()) {
        return {};
    }
    return exe.parent_path() / "plugins";
}
