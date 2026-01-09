#include <imgui.h>
#include <nlohmann/json.hpp>

#include <cstdio>

#include "plugin_api.h"

namespace {

void RenderJsonFormatter() {
    static char input_buf[8192] = "{\"hello\":\"world\",\"value\":42}";
    static char output_buf[8192] = "";
    static char status_buf[256] = "Ready.";

    ImGui::Begin("JSON Formatter");
    ImGui::Text("Input JSON and format it automatically.");
    ImGui::InputTextMultiline("Input", input_buf, sizeof(input_buf), ImVec2(-1.0f, 200.0f));

    if (ImGui::Button("Format")) {
        try {
            auto parsed = nlohmann::json::parse(input_buf);
            auto pretty = parsed.dump(4);
            std::snprintf(output_buf, sizeof(output_buf), "%s", pretty.c_str());
            std::snprintf(status_buf, sizeof(status_buf), "%s", "Format OK.");
        } catch (const std::exception& ex) {
            std::snprintf(status_buf, sizeof(status_buf), "Parse error: %s", ex.what());
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        input_buf[0] = '\0';
        output_buf[0] = '\0';
        std::snprintf(status_buf, sizeof(status_buf), "%s", "Cleared.");
    }

    ImGui::Text("Status: %s", status_buf);
    ImGui::InputTextMultiline("Output", output_buf, sizeof(output_buf), ImVec2(-1.0f, 200.0f),
        ImGuiInputTextFlags_ReadOnly);
    ImGui::End();
}

PluginInfo g_plugin_info = {
    "JSON Formatter",
    &RenderJsonFormatter
};

} // namespace

extern "C" PLUGIN_API PluginInfo* GetPluginInfo() {
    return &g_plugin_info;
}
