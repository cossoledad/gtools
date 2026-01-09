add_library(json_formatter_plugin SHARED
    plugin.cpp
)

target_include_directories(json_formatter_plugin
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(json_formatter_plugin
    PRIVATE
        imgui::imgui
        nlohmann_json::nlohmann_json
)

target_compile_definitions(json_formatter_plugin PRIVATE PLUGIN_BUILD)

set_target_properties(json_formatter_plugin PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
    ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/plugins"
)
