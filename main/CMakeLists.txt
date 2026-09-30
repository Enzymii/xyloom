idf_component_register(
    SRCS "main.c" "passport/input.c" "passport/world.c" "passport/view.c"
         "../assets/fonts/passport/passport_font_18.c"
         "../assets/images/passport/passport_images.c"
    INCLUDE_DIRS "."
    REQUIRES bsp esp_timer
)
