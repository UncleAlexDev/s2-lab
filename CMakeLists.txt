cmake_minimum_required(VERSION 3.16)

project(bbi2-lab LANGUAGES CXX)
set(EXE ${PROJECT_NAME})
message("EXE: ${EXE}")
set(ENV{NO_COLOR} "1")
set(ENV{CLICOLOR} "0")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
#set(CMAKE_CXX_EXTENSIONS OFF)
#set(CMAKE_BUILD_PARALLEL_LEVEL 15)

# Включаем автоматическую обработку Qt-файлов
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(app_icon_resource "app_logo.rc" "images/images.qrc")
include(src.cmake)
add_executable(${PROJECT_NAME} ${PROJECT_SOURCES} ${app_icon_resource}
    src/network/bbi_points.h)

target_compile_options(${PROJECT_NAME} PRIVATE
        -Wall -Wextra -mno-ms-bitfields
        $<$<CONFIG:Debug>:-g -O0>
        $<$<CONFIG:Release>:-O3 -march=native> #  -fopt-info-vec-optimized
        # either -march=native or -msse4.2 or -mavx2
)

target_include_directories(${PROJECT_NAME} PRIVATE ${PROJECT_INCLUDES})


# Консоль будет скрыта только на Windows
set_target_properties(${PROJECT_NAME} PROPERTIES WIN32_EXECUTABLE TRUE)

#----------------------------------------------------------
# Попытка найти Qt6, если нет — поиск Qt5
find_package(Qt6 COMPONENTS Core Widgets Charts Network LinguistTools QUIET)
if(Qt6_FOUND)
    message(STATUS "Найдена версия: ${Qt6_VERSION} - ${Qt6_DIR}")
    set(QTVER 6)
else()
    find_package(Qt5 COMPONENTS Core Widgets Charts Network LinguistTools REQUIRED)
    message(STATUS "Найдена версия: ${Qt5_VERSION} - ${Qt5_DIR}")
    set(QTVER 5)
endif()
message("QTVER: ${QTVER}")
message("CMAKE_PREFIX_PATH: ${CMAKE_PREFIX_PATH}")

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QTVER}::Core Qt${QTVER}::Widgets Qt${QTVER}::Charts Qt${QTVER}::Network)
# Для GCC ниже 9-й версии нужно явно линковать stdc++fs
if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU" AND CMAKE_CXX_COMPILER_VERSION VERSION_LESS 9.0)
    target_link_libraries(${PROJECT_NAME} PRIVATE stdc++fs)
endif()

add_subdirectory(packages/fmt)
target_link_libraries(${PROJECT_NAME} PRIVATE fmt)

if (WIN32)
    add_compile_definitions(WIN32_LEAN_AND_MEAN)
    message("add libs - wsock32 ws2_32")
    target_link_libraries(${PROJECT_NAME} PRIVATE wsock32 ws2_32)
endif ()

#----------------------------------------------------------
set(TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/locale/${PROJECT_NAME}.ru_RU.ts")
list(APPEND TS_FILES "${CMAKE_CURRENT_SOURCE_DIR}/locale/bbi2-lab.en_US.ts")
include(qt_tr.cmake)
add_translations(${PROJECT_NAME} "${TS_FILES}")

#----------------------------------------------------------
#include(FetchContent)
#FetchContent_Declare(
#        fmt
#        GIT_REPOSITORY https://github.com/fmtlib/fmt
#        GIT_TAG        12.1.0 # Или нужная вам версия
#)
#FetchContent_MakeAvailable(fmt)
#target_link_libraries(${PROJECT_NAME} PRIVATE fmt::fmt)

#----------------------------------------------------------
set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/dist-${CMAKE_BUILD_TYPE}" CACHE PATH "Install path" FORCE)
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_CURRENT_SOURCE_DIR}/dist-${CMAKE_BUILD_TYPE}" CACHE PATH "Install path" FORCE)
endif()

include(qt_install.cmake)
make_dist(${PROJECT_NAME})
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.ru_RU.qm" DESTINATION "locale")
install(FILES "${CMAKE_CURRENT_BINARY_DIR}/${PROJECT_NAME}.ru_RU.qm" DESTINATION ".")

