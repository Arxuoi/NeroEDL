cmake_minimum_required(VERSION 3.20)
project(NeroEDL VERSION 0.1.0 LANGUAGES CXX)

option(NEROEDL_SANITIZERS "Enable address and undefined-behaviour sanitizers" OFF)
option(NEROEDL_WARNINGS_AS_ERRORS "Treat compiler warnings as errors" OFF)
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

find_package(PkgConfig QUIET)
if(PkgConfig_FOUND)
  pkg_check_modules(LIBUSB IMPORTED_TARGET libusb-1.0)
endif()

add_library(neroedl_core
  core/usb/usb.cpp core/detector/classifier.cpp core/monitor/monitor.cpp
  core/logging/log.cpp entry/entry.cpp protocols/sahara/sahara.cpp
  devices/database.cpp)
target_include_directories(neroedl_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_compile_options(neroedl_core PRIVATE -Wall -Wextra -Wpedantic)
if(NEROEDL_WARNINGS_AS_ERRORS)
  target_compile_options(neroedl_core PRIVATE -Werror)
endif()
if(LIBUSB_FOUND)
  target_link_libraries(neroedl_core PRIVATE PkgConfig::LIBUSB)
  target_compile_definitions(neroedl_core PRIVATE NEROEDL_HAS_LIBUSB=1)
else()
  message(WARNING "libusb-1.0 development files not found: building the explicit unavailable backend")
endif()
if(NEROEDL_SANITIZERS AND CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
  target_compile_options(neroedl_core PUBLIC -fsanitize=address,undefined -fno-omit-frame-pointer)
  target_link_options(neroedl_core PUBLIC -fsanitize=address,undefined)
endif()

add_executable(neroedl cli/main.cpp)
target_link_libraries(neroedl PRIVATE neroedl_core)
target_compile_options(neroedl PRIVATE -Wall -Wextra -Wpedantic)

include(CTest)
if(BUILD_TESTING)
  add_executable(neroedl_tests tests/tests.cpp)
  target_link_libraries(neroedl_tests PRIVATE neroedl_core)
  add_test(NAME unit COMMAND neroedl_tests ${CMAKE_CURRENT_SOURCE_DIR}/devices/database.json)
endif()

install(TARGETS neroedl RUNTIME DESTINATION bin)
install(FILES devices/database.json DESTINATION share/neroedl)
