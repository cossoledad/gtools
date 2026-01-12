if (WIN32)
    add_executable(gtoolapp WIN32
        main.cpp
        ${PROJECT_SOURCE_DIR}/src/imgui/imgui_impl_glfw.cpp
        ${PROJECT_SOURCE_DIR}/src/imgui/imgui_impl_opengl2.cpp
        ${PROJECT_SOURCE_DIR}/src/core/plugin_loader.cpp
    )
else()
    add_executable(gtoolapp
        main.cpp
        ${PROJECT_SOURCE_DIR}/src/imgui/imgui_impl_glfw.cpp
        ${PROJECT_SOURCE_DIR}/src/imgui/imgui_impl_opengl2.cpp
        ${PROJECT_SOURCE_DIR}/src/core/plugin_loader.cpp
    )
endif()

target_include_directories(gtoolapp
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/core
        ${PROJECT_SOURCE_DIR}/src/imgui
)

target_link_libraries(gtoolapp
    PRIVATE
        imgui::imgui
        ${GLFW_TARGET}
        OpenGL::GL
)

if (UNIX AND NOT APPLE)
    target_link_libraries(gtoolapp PRIVATE dl)
endif()

set_target_properties(gtoolapp PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}"
)

if (WIN32)
    set(_bin_dirs "")
    if (CMAKE_BUILD_TYPE)
        string(TOUPPER "${CMAKE_BUILD_TYPE}" _cfg)
    endif()

    foreach(_pkg IN ITEMS imgui glfw)
        if (DEFINED ${_pkg}_BIN_DIRS_${_cfg})
            list(APPEND _bin_dirs ${${_pkg}_BIN_DIRS_${_cfg}})
        elseif (DEFINED ${_pkg}_BIN_DIRS)
            list(APPEND _bin_dirs ${${_pkg}_BIN_DIRS})
        endif()
    endforeach()

    list(REMOVE_DUPLICATES _bin_dirs)
    foreach(_dir IN LISTS _bin_dirs)
        file(GLOB _dlls "${_dir}/*.dll")
        foreach(_dll IN LISTS _dlls)
            add_custom_command(TARGET gtoolapp POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_dll}"
                    "$<TARGET_FILE_DIR:gtoolapp>"
            )
        endforeach()
    endforeach()
endif()

if (MSVC)
    target_compile_options(gtoolapp PRIVATE
        /W4
        /permissive-
    )

    set_property(TARGET gtoolapp PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    )
else()
    target_compile_options(gtoolapp PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
