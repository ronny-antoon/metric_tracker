#include "metric_tracker.hpp"

#include <esp_log.h>
#include <esp_http_client.h>
#include <arch/sys_arch.h>

#include <on_error.hpp>

static const char *TAG = "metric_tracker";

typedef struct
{
    char *url_path;
    char *device_id;
    uint16_t max_buffer_size;
    char *json_buffer;
} metric_tracker_config_t;

static metric_tracker_config_t *config_post;

esp_err_t metric_tracker_init(const char *url_path, const char *device_id, uint16_t max_buffer_size)
{
    // Initialize the metric tracker with provided parameters
    ESP_LOGI(TAG, "Initializing metrics server with URL: %s, Device ID: %s, Max Buffer Size: %d", url_path, device_id,
             max_buffer_size);

    // Allocate memory for the configuration
    config_post = (metric_tracker_config_t *)malloc(sizeof(metric_tracker_config_t));
    ON_NULL_PRINT_RETURN(config_post, ESP_ERR_NO_MEM, "Memory allocation failed for config_post");

    // Allocate memory and copy URL path
    config_post->url_path = new char[strlen(url_path) + 1];
    strcpy(config_post->url_path, url_path);

    // Allocate memory and copy device ID
    config_post->device_id = new char[strlen(device_id) + 1];
    strcpy(config_post->device_id, device_id);

    // Set max buffer size and allocate memory for JSON buffer
    config_post->max_buffer_size = max_buffer_size;
    config_post->json_buffer = (char *)calloc(max_buffer_size, sizeof(char));

    ESP_LOGI(TAG, "Metrics server initialized successfully");
    return ESP_OK;
}

esp_err_t metric_tracker_add_metric(const char *name, float value)
{
    // Add a metric to the JSON buffer
    ESP_LOGI(TAG, "Adding metric: %s with value: %f", name, value);
    ON_NULL_PRINT_RETURN(config_post, ESP_ERR_INVALID_STATE, "Config post is NULL, invalid state");

    // Check for buffer overflow risk
    if (strlen(config_post->json_buffer) + strlen(name) + 50 > config_post->max_buffer_size)
    {
        ON_ERR_PRINT_RETURN(ESP_ERR_NO_MEM, ESP_ERR_NO_MEM, "Buffer overflow risk, metric not added");
    }

    // If JSON buffer is empty, initialize it
    if (strlen(config_post->json_buffer) == 0)
    {
        sprintf(config_post->json_buffer, "{\"device_id\": \"%s\", \"metrics\": [{\"name\": \"%s\", \"value\": %f}",
                config_post->device_id, name, value);
    }
    else
    {
        // Append metric to existing JSON buffer
        sprintf(config_post->json_buffer + strlen(config_post->json_buffer), ", {\"name\": \"%s\", \"value\": %f}",
                name, value);
    }

    ESP_LOGI(TAG, "Metric added successfully");
    return ESP_OK;
}

esp_err_t metric_tracker_add_heap()
{
    // Add heap metrics to the JSON buffer
    ESP_LOGI(TAG, "Adding heap metrics");
    ON_NULL_PRINT_RETURN(config_post, ESP_ERR_INVALID_STATE, "Config post is NULL, invalid state");

    // If JSON buffer is empty, initialize it
    if (strlen(config_post->json_buffer) == 0)
    {
        sprintf(config_post->json_buffer,
                "{\"device_id\": \"%s\", \"heap\": [{\"name\": \"heap_free\", \"value\": %d}, {\"name\": "
                "\"heap_min_free\", \"value\": %d}]",
                config_post->device_id, (int)esp_get_free_heap_size(), (int)esp_get_minimum_free_heap_size());
    }
    else
    {
        // Append heap metrics to existing JSON buffer
        sprintf(config_post->json_buffer + strlen(config_post->json_buffer),
                ", \"heap\": [{\"name\": \"heap_free\", \"value\": %d}, {\"name\": \"heap_min_free\", \"value\": %d}]",
                (int)esp_get_free_heap_size(), (int)esp_get_minimum_free_heap_size());
    }

    ESP_LOGI(TAG, "Heap metrics added successfully");
    return ESP_OK;
}

esp_err_t metric_tracker_add_tasks()
{
    // Add task metrics to the JSON buffer
    ESP_LOGI(TAG, "Adding task metrics");
    ON_NULL_PRINT_RETURN(config_post, ESP_ERR_INVALID_STATE, "Config post is NULL, invalid state");

    TaskStatus_t *pxTaskStatusArray;
    volatile UBaseType_t uxArraySize, x;

    // Get the number of tasks
    uxArraySize = uxTaskGetNumberOfTasks();

    // Allocate memory for the task status array
    pxTaskStatusArray = (TaskStatus_t *)malloc(uxArraySize * sizeof(TaskStatus_t));
    ON_NULL_PRINT_RETURN(pxTaskStatusArray, ESP_ERR_NO_MEM, "Memory allocation failed for pxTaskStatusArray");

    // Get the system state
    uxArraySize = uxTaskGetSystemState(pxTaskStatusArray, uxArraySize, NULL);

    if (strlen(config_post->json_buffer) == 0)
    {
        // If JSON buffer is empty, initialize it
        sprintf(config_post->json_buffer, "{\"device_id\": \"%s\", \"tasks\": [{\"name\": \"%s\", \"stack_free\": %d}",
                config_post->device_id, pxTaskStatusArray[0].pcTaskName,
                (int)pxTaskStatusArray[0].usStackHighWaterMark);
    }
    else
    {
        // Append task metrics to existing JSON buffer
        sprintf(config_post->json_buffer + strlen(config_post->json_buffer),
                ", \"tasks\": [{\"name\": \"%s\", \"stack_free\": %d}", pxTaskStatusArray[0].pcTaskName,
                (int)pxTaskStatusArray[0].usStackHighWaterMark);
    }

    for (x = 1; x < uxArraySize - 1; x++)
    {
        // Add remaining task metrics to JSON buffer
        sprintf(config_post->json_buffer + strlen(config_post->json_buffer), ", {\"name\": \"%s\", \"stack_free\": %d}",
                pxTaskStatusArray[x].pcTaskName, (int)pxTaskStatusArray[x].usStackHighWaterMark);
    }

    // Add the last task
    sprintf(config_post->json_buffer + strlen(config_post->json_buffer), ", {\"name\": \"%s\", \"stack_free\": %d}]",
            pxTaskStatusArray[uxArraySize - 1].pcTaskName,
            (int)pxTaskStatusArray[uxArraySize - 1].usStackHighWaterMark);

    // Free allocated memory
    free(pxTaskStatusArray);

    ESP_LOGI(TAG, "Task metrics added successfully");
    return ESP_OK;
}

esp_err_t metric_tracker_send(bool send_heap, bool send_tasks)
{
    // Send tracked metrics to the server
    ESP_LOGI(TAG, "Sending metrics with send_heap: %d, send_tasks: %d", send_heap, send_tasks);

    // Check for NULL configuration
    if (config_post == NULL)
    {
        memset(config_post->json_buffer, '\0', config_post->max_buffer_size);
        ON_ERR_PRINT_RETURN(ESP_ERR_INVALID_STATE, ESP_ERR_INVALID_STATE, "Config post is NULL, invalid state");
    }
    if (strlen(config_post->json_buffer) != 0)
    {
        sprintf(config_post->json_buffer + strlen(config_post->json_buffer), "]");
    }

    // If sending heap or task metrics is requested
    if (send_heap || send_tasks)
    {
        if (send_heap)
        {
            metric_tracker_add_heap();
        }
        if (send_tasks)
        {
            metric_tracker_add_tasks();
        }
        sprintf(config_post->json_buffer + strlen(config_post->json_buffer), "}"); // Close the JSON
    }
    else
    {
        // If no metrics to send, ensure JSON buffer is not empty
        if (strlen(config_post->json_buffer) == 0)
        {
            memset(config_post->json_buffer, '\0', config_post->max_buffer_size);
            ON_ERR_PRINT_RETURN(ESP_ERR_INVALID_STATE, ESP_ERR_INVALID_STATE, "No metrics to send, invalid state");
        }

        sprintf(config_post->json_buffer + strlen(config_post->json_buffer), "]}"); // Close the JSON
    }

    // Configure HTTP client
    esp_http_client_config_t config = {.url = config_post->url_path, .method = HTTP_METHOD_POST};

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == NULL)
    {
        memset(config_post->json_buffer, '\0', config_post->max_buffer_size);
        ON_ERR_PRINT_RETURN(ESP_ERR_NO_MEM, ESP_ERR_NO_MEM, "HTTP client initialization failed");
    }

    // Set POST field
    esp_err_t err = esp_http_client_set_post_field(client, config_post->json_buffer, strlen(config_post->json_buffer));
    if (err != ESP_OK)
    {
        esp_http_client_cleanup(client);
        memset(config_post->json_buffer, '\0', config_post->max_buffer_size);
        ON_ERR_PRINT_RETURN(err, err, "Setting post field failed: %s", esp_err_to_name(err));
    }

    // Set header
    err = esp_http_client_set_header(client, "Content-Type", "application/json");
    if (err != ESP_OK)
    {
        esp_http_client_cleanup(client);
        memset(config_post->json_buffer, '\0', config_post->max_buffer_size);
        ON_ERR_PRINT_RETURN(err, err, "Setting header failed: %s", esp_err_to_name(err));
    }

    // Perform HTTP request
    err = esp_http_client_perform(client);
    if (err != ESP_OK)
    {
        esp_http_client_cleanup(client);
        memset(config_post->json_buffer, '\0', config_post->max_buffer_size);
        ON_ERR_PRINT_RETURN(err, err, "HTTP client perform failed: %s", esp_err_to_name(err));
    }

    err = esp_http_client_cleanup(client);
    ON_ERR_PRINT(err, "HTTP client cleanup failed: %s", esp_err_to_name(err));

    // Clear the buffer
    memset(config_post->json_buffer, '\0', config_post->max_buffer_size);

    ESP_LOGI(TAG, "Metrics sent successfully");
    return err;
}

esp_err_t metric_tracker_deinit()
{
    // Deinitialize the metric tracker
    ESP_LOGI(TAG, "Deinitializing metrics server");
    ON_NULL_PRINT_RETURN(config_post, ESP_ERR_INVALID_STATE, "Config post is NULL, invalid state");

    // Free allocated memory
    free(config_post->url_path);
    free(config_post->device_id);
    free(config_post->json_buffer);
    free(config_post);
    config_post = NULL;

    ESP_LOGI(TAG, "Metrics server deinitialized successfully");
    return ESP_OK;
}

esp_err_t metric_tracker_auto_send(uint16_t interval_seconds, bool send_heap, bool send_tasks)
{
    // Start automatic sending of tracked metrics
    ESP_LOGI(TAG, "Starting auto-send with interval: %d seconds, send_heap: %d, send_tasks: %d", interval_seconds,
             send_heap, send_tasks);
    ON_NULL_PRINT_RETURN(config_post, ESP_ERR_INVALID_STATE, "Config post is NULL, invalid state");

    while (true)
    {
        // Send metrics and delay
        metric_tracker_send(send_heap, send_tasks);
        vTaskDelay(interval_seconds * 1000 / portTICK_PERIOD_MS);
    }

    // This line will never be reached
    return ESP_OK;
}
