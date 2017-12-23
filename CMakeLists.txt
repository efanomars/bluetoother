# File: bluetoother/CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

option(STMM_INSTALL_LAUNCHER "Install launcher in share/applications/ (implies STMM_INSTALL_ICONS=ON)" ON)
option(STMM_INSTALL_ICONS "Install icons in share/icons/hicolor/(size)/apps/" ON)
# The following is needed because PolKit doesn't check /usr/local (just /usr)
set(STMM_POLKIT_POLICY_INSTALL_PREFIX "/usr" CACHE PATH "PolKit policy installation prefix (absolute path)")


project(bluetoother CXX)

set(RUNTIME_OUTPUT_DIRECTORY "build")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} ${CMAKE_CURRENT_SOURCE_DIR}/share/cmake) 

include(CommonUtil)

CheckBinaryNotSourceTree()
CheckBuildType()
DefineCommonCompileOptions()

include(FindPkgConfig)
if (NOT PKG_CONFIG_FOUND)
     message(FATAL_ERROR "Mandatory 'pkg-config' not found!")
endif()

#
set(BLUETOOTHER_MAJOR_VERSION 0)
set(BLUETOOTHER_MINOR_VERSION 1)
set(BLUETOOTHER_VERSION "${BLUETOOTHER_MAJOR_VERSION}.${BLUETOOTHER_MINOR_VERSION}.0")

set(GTKMM_VERSION_REQ "3.14.0")
set(BLUETOOTH_VERSION_REQ "5.23")
set(JSONCPP_VERSION_REQ "1.7.4")

pkg_check_modules(GTKMM       REQUIRED  gtkmm-3.0>=${GTKMM_VERSION_REQ})
pkg_check_modules(BLUETOOTH   REQUIRED  bluez>=${BLUETOOTH_VERSION_REQ})

list(APPEND STMMI_BTER_EXTRA_INCLUDE_DIRS  ${GTKMM_INCLUDE_DIRS} ${BLUETOOTH_INCLUDE_DIRS})
list(APPEND STMMI_BTER_EXTRA_LIBS          ${GTKMM_LIBRARIES}    ${BLUETOOTH_LIBRARIES})

string(STRIP "${STMMI_BTER_EXTRA_LIBS}"         STMMI_BTER_EXTRA_LIBS)
string(STRIP "${STMMI_BTER_EXTRA_INCLUDE_DIRS}" STMMI_BTER_EXTRA_INCLUDE_DIRS)

# Source files (and headers only used for building)
set(STMMI_BTER_SOURCES
        ${PROJECT_SOURCE_DIR}/src/btservice.h
        ${PROJECT_SOURCE_DIR}/src/btservice.cc
        ${PROJECT_SOURCE_DIR}/src/config.h
        ${PROJECT_SOURCE_DIR}/src/hciadapter.h
        ${PROJECT_SOURCE_DIR}/src/hciadapter.cc
        ${PROJECT_SOURCE_DIR}/src/hcisocket.h
        ${PROJECT_SOURCE_DIR}/src/hcisocket.cc
        ${PROJECT_SOURCE_DIR}/src/jsoncommon.h
        ${PROJECT_SOURCE_DIR}/src/jsoncommon.cc
        ${PROJECT_SOURCE_DIR}/src/json.hpp
        ${PROJECT_SOURCE_DIR}/src/main.cc
        ${PROJECT_SOURCE_DIR}/src/namedialog.h
        ${PROJECT_SOURCE_DIR}/src/namedialog.cc
        ${PROJECT_SOURCE_DIR}/src/tootherserver.h
        ${PROJECT_SOURCE_DIR}/src/tootherserver.cc
        ${PROJECT_SOURCE_DIR}/src/toothersources.h
        ${PROJECT_SOURCE_DIR}/src/toothersources.cc
        ${PROJECT_SOURCE_DIR}/src/tootherwindow.h
        ${PROJECT_SOURCE_DIR}/src/tootherwindow.cc
        )

if (STMM_INSTALL_LAUNCHER)
    mark_as_advanced(STMMIFINDPKEXEC)
    find_program(
            STMMIFINDPKEXEC
            pkexec
            NO_CMAKE_PATH
            NO_CMAKE_ENVIRONMENT_PATH
            NO_CMAKE_SYSTEM_PATH
            )
    if ("${STMMIFINDPKEXEC}" STREQUAL "STMMIFINDPKEXEC-NOTFOUND")
         message(FATAL_ERROR "Couldn't find the pkexec command. Install the gksu package first!")
    endif()
endif()

set(STMMI_BTER_DATA_DIR ${PROJECT_SOURCE_DIR}/data)

add_executable(bluetoother ${STMMI_BTER_SOURCES} ${PROJECT_BINARY_DIR}/config.cc)

target_include_directories(bluetoother SYSTEM PUBLIC ${STMMI_BTER_EXTRA_INCLUDE_DIRS})

DefineTargetPublicCompileOptions(bluetoother)

include(GNUInstallDirs)
set(BLUETOOTHER_PKG_DATA_DIR     "${CMAKE_INSTALL_FULL_DATADIR}/bluetoother")
set(BLUETOOTHER_PKG_REL_DATA_DIR "${CMAKE_INSTALL_DATADIR}/bluetoother")

# Create config file for executable
configure_file("${PROJECT_SOURCE_DIR}/src/config.cc.in"
               "${PROJECT_BINARY_DIR}/config.cc" @ONLY)
if (STMM_INSTALL_LAUNCHER)
    set(BLUETOOTHER_INSTALLED_PATH ${CMAKE_INSTALL_FULL_BINDIR}/bluetoother)
    configure_file("${STMMI_BTER_DATA_DIR}/polkit-1/com.github.efanomars.bluetoother.policy.in"
                   "${PROJECT_BINARY_DIR}/com.github.efanomars.bluetoother.policy" @ONLY)
endif()
# This allows config.cc to find the config.h include
target_include_directories(bluetoother PUBLIC ${PROJECT_SOURCE_DIR}/src)

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "bluetoother was configured with the following options:")
message(STATUS " STMMI_BTER_SOURCES:            ${STMMI_BTER_SOURCES}")
message(STATUS " STMMI_BTER_EXTRA_LIBS:         ${STMMI_BTER_EXTRA_LIBS}")
message(STATUS " STMMI_BTER_ICON_FILES          ${STMMI_BTER_ICON_FILES}")
message(STATUS " CMAKE_BUILD_TYPE:              ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:         ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:    ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " install prefix:                ${CMAKE_INSTALL_PREFIX}")
endif()

target_link_libraries(bluetoother ${STMMI_BTER_EXTRA_LIBS}) # ${STMMI_BTER_EXTRA_LDFLAGS}

install(TARGETS bluetoother RUNTIME DESTINATION "bin")

if (STMM_INSTALL_LAUNCHER)
    install(FILES         "${STMMI_BTER_DATA_DIR}/applications/bluetoother.desktop"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/applications/")
    install(FILES       "${PROJECT_BINARY_DIR}/com.github.efanomars.bluetoother.policy"
            DESTINATION "${STMM_POLKIT_POLICY_INSTALL_PREFIX}/share/polkit-1/actions/")
endif()
if (STMM_INSTALL_ICONS OR STMM_INSTALL_LAUNCHER)
    install(FILES         "${STMMI_BTER_DATA_DIR}/icons/hicolor/24x24/apps/bluetoother.png"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/24x24/apps/")
    install(FILES         "${STMMI_BTER_DATA_DIR}/icons/hicolor/32x32/apps/bluetoother.png"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps/")
    install(FILES         "${STMMI_BTER_DATA_DIR}/icons/hicolor/48x48/apps/bluetoother.png"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps/")
    install(FILES         "${STMMI_BTER_DATA_DIR}/icons/hicolor/scalable/apps/bluetoother.svg"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/")
endif()
 
