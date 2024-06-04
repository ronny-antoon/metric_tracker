# metric_tracker Component Framework for ESP-IDF

## Overview

The `metric_tracker` component framework for ESP-IDF provides a simple interface for tracking and sending metrics data to a server. It is designed to be easy to integrate into ESP32 and ESP32-S2 projects using the ESP-IDF framework.

## Usage

### Installation

To use the `metric_tracker` component framework in your ESP-IDF project, follow these steps:

1. Clone or download the `metric_tracker` repository.
2. Copy the `metric_tracker` directory into your ESP-IDF project's `components` directory.
3. Configure the component in your `CMakeLists.txt` file.

OR

- Add dependencies to idf_component.yml in your project/main directory
```
dependencies:
  ronny-antoon/metric_tracker:
    version: "^0.0.9"
```


### Configuration

Before using the `metric_tracker` component, make sure to configure the following parameters in your project's `sdkconfig.defaults` file:

- `CONFIG_FREERTOS_USE_TRACE_FACILITY=y`

### API

The `metric_tracker` component provides the following API functions:

- `esp_err_t metric_tracker_init(const char *url_path, const char *device_id, uint16_t max_buffer_size)`: Initializes the metric tracker.
- `esp_err_t metric_tracker_add_metric(const char *name, float value)`: Adds a metric to be tracked.
- `esp_err_t metric_tracker_send(bool send_heap = true, bool send_tasks = true)`: Sends the tracked metrics to the server.
- `esp_err_t metric_tracker_auto_send(uint16_t interval_seconds, bool send_heap = true, bool send_tasks = true)`: Starts automatic sending of tracked metrics at a specified interval.
- `esp_err_t metric_tracker_deinit()`: Deinitializes the metric tracker.

## License

The `metric_tracker` component framework is licensed under the MIT License. See the [LICENSE](LICENSE) file for details.

## Links

- [GitHub Repository](https://github.com/your/repository)
- [Espressif IDF Documentation](https://docs.espressif.com/projects/esp-idf/)

