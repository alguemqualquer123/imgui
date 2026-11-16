cmake_minimum_required(VERSION 3.16)
project(AmberUI VERSION 1.0.0 LANGUAGES CXX)

# -----------------------------------------------------------------------------
# Options
# -----------------------------------------------------------------------------
option(AMBERUI_BUILD_EXAMPLES "Build the Amber UI example applications" ON)
option(AMBERUI_BUILD_TESTS "Build Amber UI headless tests" ON)
option(AMBERUI_BUILD_EXTENSIONS "Build optional NodeGraph and Timeline extensions" ON)
option(AMBERUI_INSTALL "Generate install rules and CMake package files" ON)
set(AMBERUI_IMGUI_TARGET "ImGui" CACHE STRING "Existing Dear ImGui CMake target to reuse")
set(AMBERUI_IMGUI_DIR "" CACHE PATH "Directory containing imgui.cpp (empty = auto-detect)")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# -----------------------------------------------------------------------------
# Dear ImGui dependency
# Amber UI never compiles a second copy of Dear ImGui when the host already
# provides a target. The embedding application remains responsible for backends.
# -----------------------------------------------------------------------------
if (NOT TARGET ${AMBERUI_IMGUI_TARGET})
    if (NOT AMBERUI_IMGUI_DIR)
        if (EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/../imgui.cpp")
            get_filename_component(AMBERUI_IMGUI_DIR "${CMAKE_CURRENT_SOURCE_DIR}/.." ABSOLUTE)
        endif()
    endif()

    if (AMBERUI_IMGUI_DIR)
        add_library(${AMBERUI_IMGUI_TARGET} STATIC
            "${AMBERUI_IMGUI_DIR}/imgui.cpp"
            "${AMBERUI_IMGUI_DIR}/imgui_draw.cpp"
            "${AMBERUI_IMGUI_DIR}/imgui_tables.cpp"
            "${AMBERUI_IMGUI_DIR}/imgui_widgets.cpp"
            "${AMBERUI_IMGUI_DIR}/imgui_demo.cpp"
        )
        target_include_directories(${AMBERUI_IMGUI_TARGET} SYSTEM PUBLIC "${AMBERUI_IMGUI_DIR}")
        target_compile_features(${AMBERUI_IMGUI_TARGET} PUBLIC cxx_std_17)
        set_target_properties(${AMBERUI_IMGUI_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    else()
        include(FetchContent)
        FetchContent_Declare(imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG docking
            GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(imgui)
        add_library(${AMBERUI_IMGUI_TARGET} STATIC
            "${imgui_SOURCE_DIR}/imgui.cpp"
            "${imgui_SOURCE_DIR}/imgui_draw.cpp"
            "${imgui_SOURCE_DIR}/imgui_tables.cpp"
            "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
            "${imgui_SOURCE_DIR}/imgui_demo.cpp"
        )
        target_include_directories(${AMBERUI_IMGUI_TARGET} SYSTEM PUBLIC "${imgui_SOURCE_DIR}")
        target_compile_features(${AMBERUI_IMGUI_TARGET} PUBLIC cxx_std_17)
        set_target_properties(${AMBERUI_IMGUI_TARGET} PROPERTIES POSITION_INDEPENDENT_CODE ON)
    endif()
endif()

# -----------------------------------------------------------------------------
# Core library
# -----------------------------------------------------------------------------
add_library(AmberUI STATIC
    src/Core/AmberUI.cpp
    src/Core/ThemeManager.cpp
    src/Core/ThemeTokens.cpp
    src/Themes/Presets.cpp
    src/Styling/StyleApplier.cpp
    src/Animation/Animation.cpp
    src/Typography/Typography.cpp
    src/Widgets/Icons.cpp
    src/Widgets/BasicWidgets.cpp
    src/Widgets/PropertyWidgets.cpp
    src/Widgets/DataWidgets.cpp
    src/Widgets/ShellWidgets.cpp
    src/Serialization/Json.cpp
    src/Serialization/ThemeSerialization.cpp
)
add_library(AmberUI::AmberUI ALIAS AmberUI)

target_include_directories(AmberUI
    PUBLIC
        $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
        $<INSTALL_INTERFACE:include>
    PRIVATE
        ${CMAKE_CURRENT_SOURCE_DIR}/src
)
target_compile_features(AmberUI PUBLIC cxx_std_17)
target_compile_definitions(AmberUI PUBLIC AMBERUI_ASSETS_DIR="${CMAKE_CURRENT_SOURCE_DIR}/assets")
# Build-tree consumers reuse the host ImGui target; the exported package cannot
# reference it, so installed consumers provide Dear ImGui themselves.
target_link_libraries(AmberUI PUBLIC $<BUILD_INTERFACE:${AMBERUI_IMGUI_TARGET}>)
set_target_properties(AmberUI PROPERTIES POSITION_INDEPENDENT_CODE ON)

if (MSVC)
    target_compile_options(AmberUI PRIVATE /W4 /permissive-)
else()
    target_compile_options(AmberUI PRIVATE -Wall -Wextra -Wpedantic)
endif()

# -----------------------------------------------------------------------------
# Optional extensions
# -----------------------------------------------------------------------------
if (AMBERUI_BUILD_EXTENSIONS)
    add_library(AmberUINodeGraph STATIC extensions/NodeGraph/src/NodeGraph.cpp)
    add_library(AmberUI::NodeGraph ALIAS AmberUINodeGraph)
    target_include_directories(AmberUINodeGraph PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extensions/NodeGraph/include> $<INSTALL_INTERFACE:include>)
    target_link_libraries(AmberUINodeGraph PUBLIC AmberUI)
    target_compile_features(AmberUINodeGraph PUBLIC cxx_std_17)
    set_target_properties(AmberUINodeGraph PROPERTIES POSITION_INDEPENDENT_CODE ON EXPORT_NAME NodeGraph)

    add_library(AmberUITimeline STATIC extensions/Timeline/src/Timeline.cpp)
    add_library(AmberUI::Timeline ALIAS AmberUITimeline)
    target_include_directories(AmberUITimeline PUBLIC $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/extensions/Timeline/include> $<INSTALL_INTERFACE:include>)
    target_link_libraries(AmberUITimeline PUBLIC AmberUI)
    target_compile_features(AmberUITimeline PUBLIC cxx_std_17)
    set_target_properties(AmberUITimeline PROPERTIES POSITION_INDEPENDENT_CODE ON EXPORT_NAME Timeline)
endif()

# -----------------------------------------------------------------------------
# Examples
# -----------------------------------------------------------------------------
if (AMBERUI_BUILD_EXAMPLES)
    find_package(glfw3 3.3 QUIET)
    if (NOT glfw3_FOUND)
        include(FetchContent)
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
        FetchContent_Declare(glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.4
            GIT_SHALLOW TRUE)
        FetchContent_MakeAvailable(glfw)
    endif()

    find_package(OpenGL REQUIRED)

    add_library(AmberUIDemoBackend STATIC
        examples/common/backend.cpp
    )
    target_include_directories(AmberUIDemoBackend PUBLIC examples/common)
    target_link_libraries(AmberUIDemoBackend PUBLIC glfw OpenGL::GL ${AMBERUI_IMGUI_TARGET})
    target_sources(AmberUIDemoBackend PRIVATE
        "${AMBERUI_IMGUI_DIR}/backends/imgui_impl_glfw.cpp"
        "${AMBERUI_IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
    )
    target_include_directories(AmberUIDemoBackend PRIVATE "${AMBERUI_IMGUI_DIR}/backends")

    add_executable(AmberUIBasic examples/Basic/main.cpp)
    target_link_libraries(AmberUIBasic PRIVATE AmberUIDemoBackend AmberUI)

    add_executable(AmberUIGallery examples/WidgetGallery/main.cpp examples/WidgetGallery/Panels.cpp)
    target_link_libraries(AmberUIGallery PRIVATE AmberUIDemoBackend AmberUI)
    if (AMBERUI_BUILD_EXTENSIONS)
        target_link_libraries(AmberUIGallery PRIVATE AmberUI::NodeGraph AmberUI::Timeline)
        target_compile_definitions(AmberUIGallery PRIVATE AMBERUI_WITH_EXTENSIONS=1)
    endif()
endif()

# -----------------------------------------------------------------------------
# Tests
# -----------------------------------------------------------------------------
if (AMBERUI_BUILD_TESTS)
    enable_testing()
    add_executable(AmberUITests tests/test_main.cpp)
    target_link_libraries(AmberUITests PRIVATE AmberUI)
    target_compile_definitions(AmberUITests PRIVATE AMBERUI_TEST_IMGUI_DIR="${AMBERUI_IMGUI_DIR}")
    if (AMBERUI_BUILD_EXTENSIONS)
        target_link_libraries(AmberUITests PRIVATE AmberUI::NodeGraph AmberUI::Timeline)
        target_compile_definitions(AmberUITests PRIVATE AMBERUI_WITH_EXTENSIONS=1)
    endif()
    add_test(NAME AmberUITests COMMAND AmberUITests)
endif()

# -----------------------------------------------------------------------------
# Install / package
# -----------------------------------------------------------------------------
if (AMBERUI_INSTALL)
    include(GNUInstallDirs)
    include(CMakePackageConfigHelpers)

    install(TARGETS AmberUI
        EXPORT AmberUITargets
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
        INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
    )
    if (AMBERUI_BUILD_EXTENSIONS)
        install(TARGETS AmberUINodeGraph AmberUITimeline
            EXPORT AmberUITargets
            ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
            INCLUDES DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}
        )
    endif()
    install(DIRECTORY include/AmberUI DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    if (AMBERUI_BUILD_EXTENSIONS)
        install(DIRECTORY extensions/NodeGraph/include/AmberUI DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
        install(DIRECTORY extensions/Timeline/include/AmberUI DESTINATION ${CMAKE_INSTALL_INCLUDEDIR})
    endif()
    install(DIRECTORY assets/ DESTINATION ${CMAKE_INSTALL_DATADIR}/AmberUI)
    install(EXPORT AmberUITargets
        FILE AmberUITargets.cmake
        NAMESPACE AmberUI::
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AmberUI)

    configure_package_config_file(cmake/AmberUIConfig.cmake.in
        "${CMAKE_CURRENT_BINARY_DIR}/AmberUIConfig.cmake"
        INSTALL_DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AmberUI)
    write_basic_package_version_file(
        "${CMAKE_CURRENT_BINARY_DIR}/AmberUIConfigVersion.cmake"
        VERSION ${PROJECT_VERSION}
        COMPATIBILITY SameMajorVersion)
    install(FILES
        "${CMAKE_CURRENT_BINARY_DIR}/AmberUIConfig.cmake"
        "${CMAKE_CURRENT_BINARY_DIR}/AmberUIConfigVersion.cmake"
        DESTINATION ${CMAKE_INSTALL_LIBDIR}/cmake/AmberUI)
endif()
