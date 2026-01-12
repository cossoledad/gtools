#include <imgui.h>
#include <nlohmann/json.hpp>

#include <cstdio>
#include <string>

#include "plugin_api.h"

namespace {

int InputTextResizeCallback(ImGuiInputTextCallbackData* data) {
    if (data->EventFlag == ImGuiInputTextFlags_CallbackResize) {
        auto* str = static_cast<std::string*>(data->UserData);
        str->resize(static_cast<size_t>(data->BufTextLen));
        data->Buf = const_cast<char*>(str->c_str());
    }
    return 0;
}

bool InputTextMultilineString(const char* label,
                              std::string* str,
                              const ImVec2& size,
                              ImGuiInputTextFlags flags) {
    if (str->capacity() < str->size() + 1) {
        str->reserve(str->size() + 1);
    }
    return ImGui::InputTextMultiline(
        label,
        const_cast<char*>(str->c_str()),
        str->capacity() + 1,
        size,
        flags | ImGuiInputTextFlags_CallbackResize,
        InputTextResizeCallback,
        str);
}

void RenderJsonFormatter() {
    static std::string input_buf = "{\"hello\":\"world\",\"value\":42}";
    static std::string output_buf;
    static char status_buf[256] = "Ready.";

    ImGui::Begin("JSON Formatter");
    ImGui::Text("Input JSON and format it automatically.");

    if (ImGui::Button("Format")) {
        try {
            auto parsed = nlohmann::json::parse(input_buf);
            output_buf = parsed.dump(4);
            std::snprintf(status_buf, sizeof(status_buf), "%s", "Format OK.");
        } catch (const std::exception& ex) {
            std::snprintf(status_buf, sizeof(status_buf), "Parse error: %s", ex.what());
        }
    }

    ImGui::SameLine();
    if (ImGui::Button("Clear")) {
        input_buf.clear();
        output_buf.clear();
        std::snprintf(status_buf, sizeof(status_buf), "%s", "Cleared.");
    }

    ImGui::SameLine();
    ImGui::Text("Status: %s", status_buf);
    ImGui::Separator();

    ImVec2 avail = ImGui::GetContentRegionAvail();
    ImGuiTableFlags table_flags = ImGuiTableFlags_Resizable | ImGuiTableFlags_SizingStretchProp;
    if (ImGui::BeginTable("JsonPanels", 2, table_flags, ImVec2(0.0f, avail.y))) {
        ImGui::TableNextColumn();
        ImGui::BeginChild("InputPanel", ImVec2(0.0f, 0.0f), true);
        ImGui::TextUnformatted("Input");
        ImGui::Separator();
        if (ImGui::BeginTabBar("InputTabs")) {
            if (ImGui::BeginTabItem("Edit")) {
                InputTextMultilineString(
                    "##Input",
                    &input_buf,
                    ImVec2(-1.0f, -1.0f),
                    ImGuiInputTextFlags_NoHorizontalScroll);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Wrap view")) {
                ImGui::BeginChild("WrapInputView", ImVec2(-1.0f, -1.0f), true);
                ImGui::PushTextWrapPos(0.0f);
                ImGui::TextUnformatted(input_buf.c_str());
                ImGui::PopTextWrapPos();
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        ImGui::TableNextColumn();
        ImGui::BeginChild("OutputPanel", ImVec2(0.0f, 0.0f), true);
        ImGui::TextUnformatted("Output");
        ImGui::Separator();
        if (ImGui::BeginTabBar("OutputTabs")) {
            if (ImGui::BeginTabItem("Edit")) {
                InputTextMultilineString(
                    "##Output",
                    &output_buf,
                    ImVec2(-1.0f, -1.0f),
                    ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_NoHorizontalScroll);
                ImGui::EndTabItem();
            }
            if (ImGui::BeginTabItem("Wrap view")) {
                ImGui::BeginChild("WrapOutputView", ImVec2(-1.0f, -1.0f), true);
                ImGui::PushTextWrapPos(0.0f);
                ImGui::TextUnformatted(output_buf.c_str());
                ImGui::PopTextWrapPos();
                ImGui::EndChild();
                ImGui::EndTabItem();
            }
            ImGui::EndTabBar();
        }
        ImGui::EndChild();

        ImGui::EndTable();
    }
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
