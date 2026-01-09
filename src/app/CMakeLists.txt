add_executable(conan_clang_demo
    main.cpp
    ${PROJECT_SOURCE_DIR}/src/imgui/imgui_impl_glfw.cpp
    ${PROJECT_SOURCE_DIR}/src/imgui/imgui_impl_opengl2.cpp
    ${PROJECT_SOURCE_DIR}/src/core/plugin_loader.cpp
)

target_include_directories(conan_clang_demo
    PRIVATE
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src/core
        ${PROJECT_SOURCE_DIR}/src/imgui
)

target_link_libraries(conan_clang_demo
    PRIVATE
        imgui::imgui
        ${GLFW_TARGET}
        OpenGL::GL
)

if (UNIX AND NOT APPLE)
    target_link_libraries(conan_clang_demo PRIVATE dl)
endif()

set_target_properties(conan_clang_demo PROPERTIES
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
            add_custom_command(TARGET conan_clang_demo POST_BUILD
                COMMAND ${CMAKE_COMMAND} -E copy_if_different
                    "${_dll}"
                    "$<TARGET_FILE_DIR:conan_clang_demo>"
            )
        endforeach()
    endforeach()
endif()

if (MSVC)
    target_compile_options(conan_clang_demo PRIVATE
        /W4
        /permissive-
    )

    set_property(TARGET conan_clang_demo PROPERTY
        MSVC_RUNTIME_LIBRARY "MultiThreadedDLL"
    )
else()
    target_compile_options(conan_clang_demo PRIVATE
        -Wall
        -Wextra
        -Wpedantic
    )
endif()
