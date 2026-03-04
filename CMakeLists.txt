cmake_minimum_required(VERSION 3.21)
project(bandwidth-monitor-widget LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)

# If built outside the Dashboard mono-repo, point this to widget-sdk.
set(DASHBOARD_WIDGET_SDK_DIR "" CACHE PATH "Path to dashboard/widget-sdk")

if(NOT TARGET dashboard-sdk)
    if(DASHBOARD_WIDGET_SDK_DIR)
        add_subdirectory(${DASHBOARD_WIDGET_SDK_DIR} dashboard-widget-sdk)
    else()
        message(FATAL_ERROR
            "dashboard-sdk target not found. Either build this repo as part of Dashboard, "
            "or set -DDASHBOARD_WIDGET_SDK_DIR=/path/to/dashboard/widget-sdk")
    endif()
endif()

add_subdirectory(widgets)
