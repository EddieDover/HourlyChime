cmake_minimum_required(VERSION 3.16)

project(HourlyChime VERSION 1.0.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Core Gui Widgets Multimedia)

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
)

qt_finalize_executable(HourlyChime)

# Handle assets
file(COPY ${CMAKE_SOURCE_DIR}/assets DESTINATION ${CMAKE_BINARY_DIR})

install(TARGETS HourlyChime
    BUNDLE DESTINATION .
    LIBRARY DESTINATION lib
    RUNTIME DESTINATION bin
)

install(FILES packaging/linux/hourly-chime.desktop DESTINATION share/applications)
install(FILES assets/images/icon.png DESTINATION share/pixmaps RENAME hourly-chime.png)


set(CPACK_PACKAGE_NAME "HourlyChime")
set(CPACK_PACKAGE_VENDOR "Eddie Dover")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "A simple hourly chime application")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_CONTACT "Eddie Dover")
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE")
set(CPACK_RPM_PACKAGE_LICENSE "GPLv3")
set(CPACK_RPM_PACKAGE_GROUP "Amusements/Sound")
set(CPACK_GENERATOR "RPM")

include(CPack)
