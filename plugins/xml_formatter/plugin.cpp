#include <imgui.h>
#include <pugixml.hpp>

#include <cstdio>
#include <sstream>
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

void RenderXmlFormatter() {
    static std::string input_buf = "<root><item>hello</item><value>42</value></root>";
    static std::string output_buf;
    static char status_buf[256] = "Ready.";

    ImGui::Begin("XML Formatter");
    ImGui::Text("Input XML and format it automatically.");

    if (ImGui::Button("Format")) {
        pugi::xml_document doc;
        pugi::xml_parse_result result = doc.load_string(input_buf.c_str());
        if (result) {
            std::ostringstream oss;
            doc.save(oss, "  ", pugi::format_default, pugi::encoding_utf8);
            output_buf = oss.str();
            std::snprintf(status_buf, sizeof(status_buf), "%s", "Format OK.");
        } else {
            std::snprintf(status_buf, sizeof(status_buf), "Parse error: %s", result.description());
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
    if (ImGui::BeginTable("XmlPanels", 2, table_flags, ImVec2(0.0f, avail.y))) {
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
    "XML Formatter",
    &RenderXmlFormatter
};

} // namespace

extern "C" PLUGIN_API PluginInfo* GetPluginInfo() {
    return &g_plugin_info;
}
