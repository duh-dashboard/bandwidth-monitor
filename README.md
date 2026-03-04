# Bandwidth Monitor Widget

Dashboard widget plugin for real-time network bandwidth monitoring.

## Features

- Displays **download** and **upload** throughput
- Samples counters every second
- Interface selector (e.g. `eth0`, `wlan0`)
- Persists selected interface across restarts (`serialize` / `deserialize`)
- Graceful fallback:
  - Non-Linux: shows an informative unsupported-platform message
  - Linux with no active interfaces: shows `N/A`

## Repository Layout

```text
.
├── CMakeLists.txt
├── BandwidthMonitorWidget.h
├── BandwidthMonitorWidget.cpp
├── bandwidth-monitor.json
├── README.md
├── LICENSE
└── .github/
```

## Build

This plugin expects the Dashboard widget SDK target (`dashboard-sdk`).

### Option A: Build as part of Dashboard (recommended)

Add this repo to your Dashboard tree and include it from parent CMake, ensuring `dashboard-sdk` is already available.

### Option B: Standalone build against local SDK

```bash
cmake -S . -B build \
  -DDASHBOARD_WIDGET_SDK_DIR=/path/to/dashboard/widget-sdk
cmake --build build --parallel
```

The plugin output is written to:

```text
build/dashboard/plugins/
```

## Install / Usage

1. Build the plugin.
2. Copy the resulting shared library into Dashboard's runtime plugin directory (next to the Dashboard binary):
   - Linux: `.../dashboard/plugins/libbandwidth-monitor-widget.so`
3. Launch Dashboard.
4. Open **Add Widget** → choose **Bandwidth Monitor**.
5. Select a network interface from the dropdown.

## Notes

- Throughput is computed from deltas of `/proc/net/dev` counters on Linux.
- Rates are shown as `B/s`, `KiB/s`, `MiB/s`, or `GiB/s`.
- The first sample after selecting an interface is a baseline and displays `--`.
