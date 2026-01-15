cmake_minimum_required(VERSION 3.16)

project(HourlyChime VERSION 1.0.0 LANGUAGES CXX)

# Version and Date configuration
if(NOT DEFINED HOURLY_CHIME_VERSION)
    set(HOURLY_CHIME_VERSION "${PROJECT_VERSION}")
endif()

if(NOT DEFINED HOURLY_CHIME_BUILD_DATE)
    string(TIMESTAMP HOURLY_CHIME_BUILD_DATE "%Y-%m-%d")
endif()

add_compile_definitions(HOURLY_CHIME_VERSION_STR="${HOURLY_CHIME_VERSION}")
add_compile_definitions(HOURLY_CHIME_DATE_STR="${HOURLY_CHIME_BUILD_DATE}")

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia Network)

set(PROJECT_SOURCES
    src/main.cpp
    src/HourlyChime.cpp
    src/SettingsDialog.cpp
    src/Config.cpp
    src/SynthGenerator.cpp
    resources.qrc
)

set(PROJECT_HEADERS
    src/HourlyChime.h
    src/SettingsDialog.h
    src/Config.h
    src/SynthGenerator.h
)

qt_standard_project_setup()

qt_add_executable(HourlyChime
    MANUAL_FINALIZATION
    ${PROJECT_SOURCES}
    ${PROJECT_HEADERS}
)

target_link_libraries(HourlyChime PRIVATE
    Qt6::Core
    Qt6::Gui
    Qt6::Widgets
    Qt6::Multimedia
    Qt6::Network
)

qt_finalize_executable(HourlyChime)

# Handle assets
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

install(TARGETS HourlyChime
    BUNDLE DESTINATION .
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

if(WIN32)
    set_target_properties(HourlyChime PROPERTIES WIN32_EXECUTABLE ON)
    qt_generate_deploy_app_script(
        TARGET HourlyChime
        OUTPUT_SCRIPT deploy_script
        NO_UNSUPPORTED_PLATFORM_ERROR
    )
    install(SCRIPT ${deploy_script})
endif()

if(UNIX AND NOT APPLE)
    install(FILES packaging/linux/hourly-chime.desktop DESTINATION share/applications)
    install(FILES assets/images/icon.png DESTINATION share/pixmaps RENAME hourly-chime.png)
endif()


set(CPACK_PACKAGE_NAME "HourlyChime")
set(CPACK_PACKAGE_VENDOR "Eddie Dover")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A simple hourly chime application")

# Strip 'v' prefix if present for CPack version
if(HOURLY_CHIME_VERSION MATCHES "^v")
    string(SUBSTRING "${HOURLY_CHIME_VERSION}" 1 -1 CPACK_PACKAGE_VERSION)
else()
    set(CPACK_PACKAGE_VERSION "${HOURLY_CHIME_VERSION}")
endif()

set(CPACK_PACKAGE_CONTACT "Eddie Dover")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")

if(WIN32)
    set(CPACK_GENERATOR "NSIS")
    set(CPACK_NSIS_MODIFY_PATH ON)
    set(CPACK_NSIS_DISPLAY_NAME "HourlyChime")
else()
    set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
    set(CPACK_RPM_PACKAGE_GROUP "Amusements/Sound")
    set(CPACK_GENERATOR "RPM")
endif()

include(CPack)
