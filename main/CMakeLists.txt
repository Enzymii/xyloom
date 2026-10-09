idf_component_register(
    SRCS "main.c" "passport/input.c" "passport/world.c" "passport/view.c"
         "passport/storage.c" "passport/watering_record.c"
         "passport/growth.c" "passport/growth_record.c" "passport/life.c" "passport/network.c" "passport/network_form.c"
         "../assets/fonts/passport/passport_font_18.c"
         "../assets/images/passport/passport_images.c"
    INCLUDE_DIRS "."
    REQUIRES bsp esp_timer nvs_flash esp_wifi esp_netif esp_event esp_http_server
)
