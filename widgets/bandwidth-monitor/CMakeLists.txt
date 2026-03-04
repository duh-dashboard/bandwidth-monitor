add_library(bandwidth-monitor-widget MODULE
    BandwidthMonitorWidget.cpp
    BandwidthMonitorWidget.h
)

target_link_libraries(bandwidth-monitor-widget PRIVATE Qt6::Widgets Qt6::Network dashboard-sdk)
target_compile_definitions(bandwidth-monitor-widget PRIVATE DASHBOARD_WIDGET_LIBRARY)

set_target_properties(bandwidth-monitor-widget PROPERTIES
    LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/dashboard/plugins
)
