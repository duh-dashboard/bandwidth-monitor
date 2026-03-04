cmake_minimum_required(VERSION 3.21)
project(bandwidth-monitor-widget LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_AUTOMOC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets Network)
include(GNUInstallDirs)

if(NOT TARGET widget-sdk)
    find_package(widget-sdk REQUIRED)
endif()

add_library(bandwidth-monitor-widget MODULE
    BandwidthMonitorWidget.cpp
    BandwidthMonitorWidget.h
)

target_link_libraries(bandwidth-monitor-widget PRIVATE Qt6::Widgets Qt6::Network widget-sdk)
target_compile_definitions(bandwidth-monitor-widget PRIVATE DASHBOARD_WIDGET_LIBRARY)

set_target_properties(bandwidth-monitor-widget PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/plugins
)

install(TARGETS bandwidth-monitor-widget
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}/dashboard/plugins
)
install(FILES bandwidth-monitor.json
    DESTINATION ${CMAKE_INSTALL_LIBDIR}/dashboard/plugins
)
