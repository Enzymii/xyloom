cmake_minimum_required(VERSION 3.16)
project(passport_render C CXX)
if(NOT LVGL_SOURCE)
    message(FATAL_ERROR "Set LVGL_SOURCE to a LVGL v9.5.0 source checkout")
endif()
set(LV_BUILD_CONF_PATH "${CMAKE_CURRENT_SOURCE_DIR}/lv_conf.h" CACHE PATH "" FORCE)
set(CONFIG_LV_BUILD_DEMOS OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(CONFIG_LV_USE_THORVG_INTERNAL OFF CACHE BOOL "" FORCE)
add_subdirectory(${LVGL_SOURCE} lvgl)
set(ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../..")
add_executable(passport_render render.c
    ${ROOT}/main/passport/view.c ${ROOT}/main/passport/world.c ${ROOT}/main/passport/growth.c
    ${ROOT}/assets/fonts/passport/passport_font_18.c
    ${ROOT}/assets/images/passport/passport_images.c)
target_include_directories(passport_render PRIVATE ${ROOT}/main)
target_link_libraries(passport_render PRIVATE lvgl m)
