cmake_minimum_required(VERSION 3.13)

set(WISDOM_PROJECT_PATH "~/pico/wisdom_sensor_net")
get_filename_component(WISDOM_PROJECT_PATH "${WISDOM_PROJECT_PATH}" REALPATH BASE_DIR "${CMAKE_BINARY_DIR}")
set(WISDOM_PROJECT_PATH ${WISDOM_PROJECT_PATH} CACHE PATH "Root of Wisdom Repo" FORCE)

# Circle Buffer
message("wisdom_init: initializing circle buffer")
add_subdirectory(${WISDOM_PROJECT_PATH}/libs/circle_buffer libs/circle_buffer)

set(target "wisdom_server")
project(${target} C)

add_executable(${target} 
	src/server.c
	src/child.c
)

target_link_libraries(${target} PRIVATE
	circle_buffer_lib
)
