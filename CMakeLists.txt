cmake_minimum_required(VERSION 3.5)

idf_component_register(SRCS "metric_tracker.cpp"
                    INCLUDE_DIRS "include"
                    REQUIRES esp_http_client)
