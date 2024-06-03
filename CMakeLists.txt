# if CONFIG_METRIC_TRACKER_ENABLE is 1 then set configUSE_TRACE_FACILITY to 1
# if(CONFIG_METRIC_TRACKER_ENABLE)
# set(configUSE_TRACE_FACILITY 1)
# endif()
# add_compile_definitions(configUSE_TRACE_FACILITY=1)

idf_component_register(SRCS "metric_tracker.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_client)