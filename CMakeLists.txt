cmake_minimum_required(VERSION 2.8)
project(headphones-plug-detector C)

find_package (PulseAudio REQUIRED)

add_executable(${PROJECT_NAME}
  src/main.c
)

target_include_directories(${PROJECT_NAME} PUBLIC
  ${PULSEAUDIO_INCLUDE_DIR}
)
target_link_libraries(${PROJECT_NAME}
  ${PULSEAUDIO_LIBRARY}
  ${PULSEAUDIO_MAINLOOP_LIBRARY}
)
