cmake_minimum_required (VERSION 2.8)
project (headphones-plug-detector C)

set (CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake")
include (gdbus_codegen)

option (ENABLE_SYSTEMD "Install systemd unit file" OFF)

set (MPRIS_GENERATED_DIR "${PROJECT_BINARY_DIR}/mpris-generated")
if (NOT EXISTS "${MPRIS_GENERATED_DIR}")
  file (MAKE_DIRECTORY "${MPRIS_GENERATED_DIR}")
endif ()
set (MPRIS_SPEC_DIR "${PROJECT_SOURCE_DIR}/third-party/mpris-spec")

generate_gdbus_code (
  OUTPUT "${MPRIS_GENERATED_DIR}/mpris-core"
  INTERFACE "org.mpris"
  INPUT "${MPRIS_SPEC_DIR}/org.mpris.MediaPlayer2.xml"
)

generate_gdbus_code (
  OUTPUT "${MPRIS_GENERATED_DIR}/mpris-player"
  INTERFACE "org.mpris"
  INPUT "${MPRIS_SPEC_DIR}/org.mpris.MediaPlayer2.Player.xml"
)

find_package (PulseAudio REQUIRED)
find_package (PkgConfig REQUIRED)
pkg_check_modules (GLIB REQUIRED glib-2.0)
pkg_check_modules (GIO_UNIX REQUIRED gio-unix-2.0)

add_executable (${PROJECT_NAME}
  "${MPRIS_GENERATED_DIR}/mpris-core.c"
  "${MPRIS_GENERATED_DIR}/mpris-player.c"
  src/main.c
  src/mpris.c
)

target_include_directories (${PROJECT_NAME} PUBLIC
  include
  ${MPRIS_GENERATED_DIR}
  ${PULSEAUDIO_INCLUDE_DIR}
  ${GLIB_INCLIDE_DIRS}
  ${GIO_UNIX_INCLUDE_DIRS}
)
target_link_libraries (${PROJECT_NAME}
  ${PULSEAUDIO_LIBRARY}
  ${PULSEAUDIO_MAINLOOP_LIBRARY}
  ${GLIB_LIBRARIES}
  ${GIO_UNIX_LIBRARIES}
)

set (EXECUTABLE_PATH "${CMAKE_INSTALL_PREFIX}/bin/${PROJECT_NAME}")

if (ENABLE_SYSTEMD)
  configure_file (
    headphones-plug-detector.service
    headphones-plug-detector.service
    ESCAPE_QUOTES
  )
  install (
    FILES "${CMAKE_BINARY_DIR}/headphones-plug-detector.service"
    DESTINATION "/usr/lib/systemd/user"
  )
endif ()

install (TARGETS ${PROJECT_NAME}
  DESTINATION bin
)
