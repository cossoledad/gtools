#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

#include <GLFW/glfw3.h>

#include <filesystem>
#include <cstdio>

#include "plugin_loader.h"

int main() {
    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(1280, 720, "gtools", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO();
    const char* font_candidates[] = {
        "C:/Windows/Fonts/msyh.ttc",
        "C:/Windows/Fonts/msyh.ttf",
        "C:/Windows/Fonts/simhei.ttf",
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttc",
        "/usr/share/fonts/truetype/noto/NotoSansCJK-Regular.ttf",
        "/usr/share/fonts/opentype/noto/NotoSansCJK-Regular.ttc"
    };
    for (const char* path : font_candidates) {
        if (std::filesystem::exists(path)) {
            ImFont* font = io.Fonts->AddFontFromFileTTF(
                path, 18.0f, nullptr, io.Fonts->GetGlyphRangesChineseFull());
            if (font) {
                io.FontDefault = font;
                break;
            }
        }
    }
    // ImGui::StyleColorsDark();
    ImGui::StyleColorsLight();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL2_Init();

    PluginLoadResult plugin_result = LoadPlugins(GetDefaultPluginDir());
    ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        ImGui::Begin("Host");
        ImGui::Text("Loaded plugins: %d", static_cast<int>(plugin_result.plugins.size()));
        for (const auto& plugin : plugin_result.plugins) {
            if (plugin.info && plugin.info->name) {
                ImGui::BulletText("%s", plugin.info->name);
            }
        }
        if (!plugin_result.errors.empty()) {
            ImGui::Separator();
            ImGui::Text("Plugin load errors:");
            for (const auto& err : plugin_result.errors) {
                ImGui::TextWrapped("%s", err.c_str());
            }
        }
        // ImGui::ColorEdit3("Clear color", reinterpret_cast<float*>(&clear_color));
        if (ImGui::Button("Quit")) {
            glfwSetWindowShouldClose(window, GLFW_TRUE);
        }
        ImGui::End();

        for (const auto& plugin : plugin_result.plugins) {
            if (plugin.info && plugin.info->on_frame) {
                plugin.info->on_frame();
            }
        }

        ImGui::Render();
        int display_w = 0;
        int display_h = 0;
        glfwGetFramebufferSize(window, &display_w, &display_h);
        glViewport(0, 0, display_w, display_h);
        glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w);
        glClear(GL_COLOR_BUFFER_BIT);
        ImGui_ImplOpenGL2_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    UnloadPlugins(plugin_result.plugins);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
