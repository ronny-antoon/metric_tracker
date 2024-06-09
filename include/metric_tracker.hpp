#ifndef METRIC_TRACKER_HPP
#define METRIC_TRACKER_HPP

#include <esp_err.h>

#include <cstdint>

/**
 * @brief Initializes the metric tracker.
 *
 * @param url_path The URL path of the metrics server.
 * @param device_id The unique identifier of the device.
 * @param max_buffer_size The maximum size of the buffer for storing metrics.
 * @return esp_err_t Returns ESP_OK on success, otherwise an error code.
 */
esp_err_t metric_tracker_init(const char *url_path, const char *device_id, uint16_t max_buffer_size);

/**
 * @brief Adds a metric to be tracked.
 *
 * @param name The name of the metric.
 * @param value The value of the metric.
 * @return esp_err_t Returns ESP_OK on success, otherwise an error code.
 */
esp_err_t metric_tracker_add_metric(const char *name, float value);

/**
 * @brief Sends the tracked metrics to the server.
 *
 * @param send_heap Whether to send heap metrics.
 * @param send_tasks Whether to send task metrics.
 * @return esp_err_t Returns ESP_OK on success, otherwise an error code.
 */
esp_err_t metric_tracker_send(bool send_heap = true, bool send_tasks = true);

/**
 * @brief Starts automatic sending of tracked metrics at a specified interval.
 *
 * @param interval_seconds The interval (in seconds) at which to send metrics.
 * @param send_heap Whether to send heap metrics.
 * @param send_tasks Whether to send task metrics.
 * @return esp_err_t Returns ESP_OK on success, otherwise an error code.
 */
esp_err_t metric_tracker_auto_send(uint16_t interval_seconds, bool send_heap = true, bool send_tasks = true);

/**
 * @brief Deinitializes the metric tracker.
 *
 * @return esp_err_t Returns ESP_OK on success, otherwise an error code.
 */
esp_err_t metric_tracker_deinit();

#endif  // METRIC_TRACKER_HPP
