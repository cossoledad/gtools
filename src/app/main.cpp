#include <imgui.h>
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl2.h"

#include <GLFW/glfw3.h>

#include <filesystem>
#include <cstdio>
#include <chrono>
#include <thread>
#include <vector>
#include <cstdlib>

#include "plugin_loader.h"

namespace {

void SetWindowIcon(GLFWwindow* window) {
    const int width = 32;
    const int height = 32;
    std::vector<unsigned char> pixels(static_cast<size_t>(width * height * 4), 0);

    const float cx = (width - 1) * 0.5f;
    const float cy = (height - 1) * 0.5f;
    const float outer_radius = 14.0f;
    const float inner_radius = 9.0f;
    const float outer_r2 = outer_radius * outer_radius;
    const float inner_r2 = inner_radius * inner_radius;

    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const float dx = x - cx;
            const float dy = y - cy;
            const float dist2 = dx * dx + dy * dy;
            if (dist2 <= outer_r2) {
                const size_t idx = static_cast<size_t>((y * width + x) * 4);
                if (dist2 <= inner_r2) {
                    pixels[idx + 0] = 40;
                    pixels[idx + 1] = 120;
                    pixels[idx + 2] = 200;
                } else {
                    pixels[idx + 0] = 20;
                    pixels[idx + 1] = 80;
                    pixels[idx + 2] = 150;
                }
                pixels[idx + 3] = 255;
            }
        }
    }

    GLFWimage image;
    image.width = width;
    image.height = height;
    image.pixels = pixels.data();
    glfwSetWindowIcon(window, 1, &image);
}

} // namespace

int main() {
    if (!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 2);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

    GLFWwindow* window = glfwCreateWindow(960, 590, "gtools", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        return 1;
    }

    SetWindowIcon(window);

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
    std::vector<char> plugin_visible(plugin_result.plugins.size(), 1);
    bool single_mode = true;
    int single_index = plugin_result.plugins.empty() ? -1 : 0;
    int target_fps = 33;
    float font_scale = 1.0f;
    if (single_index >= 0) {
        std::fill(plugin_visible.begin(), plugin_visible.end(), 0);
        plugin_visible[static_cast<size_t>(single_index)] = 1;
    }
    ImVec4 clear_color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        auto frame_start = std::chrono::steady_clock::now();
        glfwPollEvents();

        ImGui_ImplOpenGL2_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        const float sidebar_width = 260.0f;
        ImGui::SetNextWindowPos(ImVec2(0.0f, 0.0f));
        ImGui::SetNextWindowSize(ImVec2(sidebar_width, io.DisplaySize.y));
        ImGuiWindowFlags host_flags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
        ImGui::Begin("Host", nullptr, host_flags);
        ImGui::Text("Loaded plugins: %d", static_cast<int>(plugin_result.plugins.size()));
        ImGui::SliderInt("Target FPS", &target_fps, 1, 60);
        if (ImGui::SliderFloat("Font scale", &font_scale, 0.5f, 2.0f)) {
            io.FontGlobalScale = font_scale;
        }
        if (ImGui::Checkbox("Single app mode", &single_mode)) {
            if (single_mode) {
                int fallback = single_index;
                if (fallback < 0) {
                    for (size_t i = 0; i < plugin_visible.size(); ++i) {
                        if (plugin_visible[i] != 0) {
                            fallback = static_cast<int>(i);
                            break;
                        }
                    }
                }
                if (fallback < 0 && !plugin_result.plugins.empty()) {
                    fallback = 0;
                }
                single_index = fallback;
                std::fill(plugin_visible.begin(), plugin_visible.end(), 0);
                if (single_index >= 0) {
                    plugin_visible[static_cast<size_t>(single_index)] = 1;
                }
            }
        }
        ImGui::SeparatorText("Plugins");
        for (size_t i = 0; i < plugin_result.plugins.size(); ++i) {
            const auto& plugin = plugin_result.plugins[i];
            const char* name = plugin.info && plugin.info->name ? plugin.info->name : plugin.path.c_str();
            ImGui::PushID(static_cast<int>(i));
            if (single_mode) {
                bool selected = single_index == static_cast<int>(i);
                if (ImGui::Selectable(name, selected)) {
                    single_index = static_cast<int>(i);
                    std::fill(plugin_visible.begin(), plugin_visible.end(), 0);
                    plugin_visible[i] = 1;
                }
            } else {
                bool visible = plugin_visible[i] != 0;
                if (ImGui::Checkbox("##plugin_visible", &visible)) {
                    plugin_visible[i] = visible ? 1 : 0;
                }
                ImGui::SameLine();
                ImGui::TextUnformatted(name);
            }
            ImGui::PopID();
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

        for (size_t i = 0; i < plugin_result.plugins.size(); ++i) {
            const auto& plugin = plugin_result.plugins[i];
            if (plugin_visible[i] != 0 && plugin.info && plugin.info->on_frame) {
                if (single_mode) {
                    ImVec2 content_pos(sidebar_width, 0.0f);
                    ImVec2 content_size(io.DisplaySize.x - sidebar_width, io.DisplaySize.y);
                    if (content_size.x < 0.0f) {
                        content_size.x = 0.0f;
                    }
                    ImGui::SetNextWindowPos(content_pos, ImGuiCond_Always);
                    ImGui::SetNextWindowSize(content_size, ImGuiCond_Always);
                }
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

        if (target_fps > 0) {
            const double target_frame_time = 1.0 / static_cast<double>(target_fps);
            auto frame_end = std::chrono::steady_clock::now();
            std::chrono::duration<double> elapsed = frame_end - frame_start;
            if (elapsed.count() < target_frame_time) {
                std::chrono::duration<double> sleep_time(target_frame_time - elapsed.count());
                std::this_thread::sleep_for(
                    std::chrono::duration_cast<std::chrono::milliseconds>(sleep_time));
            }
        }
    }

    ImGui_ImplOpenGL2_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    UnloadPlugins(plugin_result.plugins);

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}

#if defined(_WIN32)
#include <windows.h>

int WINAPI WinMain(HINSTANCE, HINSTANCE, LPSTR, int) {
    return main();
}
#endif
