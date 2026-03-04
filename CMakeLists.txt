cmake_minimum_required(VERSION 3.21)
project(bandwidth-monitor-widget LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# If built outside the Dashboard mono-repo, point this to widget-sdk.
set(DASHBOARD_WIDGET_SDK_DIR "" CACHE PATH "Path to dashboard/widget-sdk")

if(NOT TARGET widget-sdk)
    if(DASHBOARD_WIDGET_SDK_DIR)
        add_subdirectory(${DASHBOARD_WIDGET_SDK_DIR} widget-sdk)
    endif()
endif()

if(NOT TARGET widget-sdk)
    message(FATAL_ERROR
        "widget-sdk target not found. Either build this repo as part of Dashboard, "
        "or set -DDASHBOARD_WIDGET_SDK_DIR=/path/to/dashboard/widget-sdk")
endif()

add_library(bandwidth-monitor-widget MODULE
    BandwidthMonitorWidget.cpp
    BandwidthMonitorWidget.h
)

target_link_libraries(bandwidth-monitor-widget PRIVATE Qt6::Widgets Qt6::Network widget-sdk)
target_compile_definitions(bandwidth-monitor-widget PRIVATE DASHBOARD_WIDGET_LIBRARY)

set_target_properties(bandwidth-monitor-widget PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dashboard/plugins
)
