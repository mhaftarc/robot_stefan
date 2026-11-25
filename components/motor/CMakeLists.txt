idf_component_register(SRCS "motor.c"
                    INCLUDE_DIRS "include"
                    REQUIRES common
                    PRIV_REQUIRES esp_driver_gpio esp_driver_ledc
                    )

