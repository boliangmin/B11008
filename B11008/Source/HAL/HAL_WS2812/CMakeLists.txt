idf_component_register(SRCS "hal_ws2812.c"
                    INCLUDE_DIRS "./"
                    REQUIRES "espressif__led_strip" "SYS")