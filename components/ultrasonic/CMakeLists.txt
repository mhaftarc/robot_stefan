idf_component_register(SRCS "ultrasonic.c"
                       INCLUDE_DIRS "include"
                       PRIV_REQUIRES common esp_driver_gpio esp_timer)
