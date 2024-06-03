#ifndef METRIC_TRACKER_HPP
#define METRIC_TRACKER_HPP

// #define configUSE_TRACE_FACILITY (1)

#include <cstdint>
#include <esp_err.h>

/**
 * @brief Initialize the metric tracker
 *
 * @param url_path The URL path to send the metrics to
 * @param device_id The device ID to send the metrics with
 * @param max_buffer_size The maximum buffer size for the metrics
 *
 * @return ESP_OK on success, ESP_ERR_NO_MEM if memory allocation failed
 */
esp_err_t metric_tracker_init(const char *url_path, const char *device_id, uint16_t max_buffer_size);

esp_err_t metric_tracker_add_metric(const char *name, float value);

esp_err_t metric_tracker_send(bool send_heap = true, bool send_tasks = true);

esp_err_t metric_tracker_auto_send(uint16_t interval_seconds, bool send_heap = true, bool send_tasks = true);

esp_err_t metric_tracker_deinit();

#endif // METRIC_TRACKER_HPP
