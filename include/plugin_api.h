#pragma once

#if defined(_WIN32)
#if defined(PLUGIN_BUILD)
#define PLUGIN_API __declspec(dllexport)
#else
#define PLUGIN_API __declspec(dllimport)
#endif
#else
#define PLUGIN_API __attribute__((visibility("default")))
#endif

struct PluginInfo {
    const char* name;
    void (*on_frame)();
};

extern "C" {
PLUGIN_API PluginInfo* GetPluginInfo();
}
