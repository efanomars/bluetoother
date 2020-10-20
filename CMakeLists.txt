# Copyright © 2019-2020  Stefano Marsili, <stemars@gmx.ch>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public
# License along with this program; if not, see <http://www.gnu.org/licenses/>

# File:   CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

option(STMM_INSTALL_MAN_PAGE "Install man page in debian.orig/" ON)
option(STMM_INSTALL_LAUNCHER "Install launcher in share/applications/ (implies STMM_INSTALL_ICONS=ON)" ON)
option(STMM_INSTALL_ICONS "Install icons in share/icons/hicolor/(size)/apps/" ON)
# The following is needed because PolKit doesn't check /usr/local (just /usr)
set(STMM_POLKIT_POLICY_INSTALL_PREFIX "/usr" CACHE PATH "PolKit policy installation prefix (absolute path)")


project(bluetoother CXX)

set(RUNTIME_OUTPUT_DIRECTORY "build")

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/share/cmake")

include(CommonUtil)

CheckBinaryNotSourceTree()
CheckBuildType()
DefineCommonCompileOptions("c++14")

# Source files (and headers only used for building)
set(STMMI_BTER_SOURCES
        "${PROJECT_SOURCE_DIR}/src/btservice.h"
        "${PROJECT_SOURCE_DIR}/src/btservice.cc"
        "${PROJECT_SOURCE_DIR}/src/config.h"
        "${PROJECT_SOURCE_DIR}/src/hciadapter.h"
        "${PROJECT_SOURCE_DIR}/src/hciadapter.cc"
        "${PROJECT_SOURCE_DIR}/src/hcisocket.h"
        "${PROJECT_SOURCE_DIR}/src/hcisocket.cc"
        "${PROJECT_SOURCE_DIR}/src/jsoncommon.h"
        "${PROJECT_SOURCE_DIR}/src/jsoncommon.cc"
        "${PROJECT_SOURCE_DIR}/src/main.cc"
        "${PROJECT_SOURCE_DIR}/src/namedialog.h"
        "${PROJECT_SOURCE_DIR}/src/namedialog.cc"
        "${PROJECT_SOURCE_DIR}/src/tootherserver.h"
        "${PROJECT_SOURCE_DIR}/src/tootherserver.cc"
        "${PROJECT_SOURCE_DIR}/src/toothersources.h"
        "${PROJECT_SOURCE_DIR}/src/toothersources.cc"
        "${PROJECT_SOURCE_DIR}/src/tootherwindow.h"
        "${PROJECT_SOURCE_DIR}/src/tootherwindow.cc"
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
         message(FATAL_ERROR "Couldn't find the pkexec command. Install the 'policykit-1' package first!")
    endif()
endif()

set(STMMI_BTER_DATA_DIR ${PROJECT_SOURCE_DIR}/data)

add_executable(bluetoother ${STMMI_BTER_SOURCES} "${PROJECT_BINARY_DIR}/config.cc")

include("bluetoother-defs.cmake")

target_include_directories(bluetoother SYSTEM PUBLIC ${BLUETOOTHER_EXTRA_INCLUDE_DIRS})
# This allows config.cc to find the config.h include
target_include_directories(bluetoother        PUBLIC ${PROJECT_SOURCE_DIR}/src)
target_include_directories(bluetoother        PUBLIC "share/thirdparty")

target_link_libraries(bluetoother ${BLUETOOTHER_EXTRA_LIBRARIES})

DefineTargetPublicCompileOptions(bluetoother)

include(GNUInstallDirs)
set(BLUETOOTHER_PKG_DATA_DIR "${CMAKE_INSTALL_FULL_DATADIR}/bluetoother")
set(BLUETOOTHER_PKG_REL_DATA_DIR  "${CMAKE_INSTALL_DATADIR}/bluetoother")

# Create config file for executable
configure_file("${PROJECT_SOURCE_DIR}/src/config.cc.in"
               "${PROJECT_BINARY_DIR}/config.cc" @ONLY)
if (STMM_INSTALL_LAUNCHER)
    set(BLUETOOTHER_INSTALLED_PATH "${CMAKE_INSTALL_FULL_BINDIR}/bluetoother")
    configure_file("${STMMI_BTER_DATA_DIR}/polkit-1/com.efanomars.bluetoother.policy.in"
                   "${PROJECT_BINARY_DIR}/com.efanomars.bluetoother.policy" @ONLY)
endif()
if (STMM_INSTALL_MAN_PAGE)
    configure_file("${PROJECT_SOURCE_DIR}/scripts/priv/debian.orig/bluetoother.1"
                   "${PROJECT_BINARY_DIR}/bluetoother.1" @ONLY)
    CreateManGz(bluetoother "bluetoother.1")
endif()

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "bluetoother was configured with the following options:")
message(STATUS " STMMI_BTER_SOURCES:            ${STMMI_BTER_SOURCES}")
message(STATUS " BLUETOOTHER_EXTRA_LIBRARIES:   ${BLUETOOTHER_EXTRA_LIBRARIES}")
message(STATUS " CMAKE_BUILD_TYPE:              ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:         ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:    ${CMAKE_CXX_COMPILER_VERSION}")
message(STATUS " install prefix:                ${CMAKE_INSTALL_PREFIX}")
endif()

install(TARGETS bluetoother RUNTIME DESTINATION "bin")

if (STMM_INSTALL_LAUNCHER)
    install(FILES          "${STMMI_BTER_DATA_DIR}/applications/com.efanomars.bluetoother.desktop"
            DESTINATION  "${CMAKE_INSTALL_DATADIR}/applications/")
    install(FILES                        "${PROJECT_BINARY_DIR}/com.efanomars.bluetoother.policy"
            DESTINATION "${STMM_POLKIT_POLICY_INSTALL_PREFIX}/share/polkit-1/actions/")
endif()
if (STMM_INSTALL_ICONS OR STMM_INSTALL_LAUNCHER)
    install(FILES            "${STMMI_BTER_DATA_DIR}/icons/hicolor/24x24/apps/bluetoother.png"
            DESTINATION    "${CMAKE_INSTALL_DATADIR}/icons/hicolor/24x24/apps/")
    install(FILES            "${STMMI_BTER_DATA_DIR}/icons/hicolor/32x32/apps/bluetoother.png"
            DESTINATION    "${CMAKE_INSTALL_DATADIR}/icons/hicolor/32x32/apps/")
    install(FILES            "${STMMI_BTER_DATA_DIR}/icons/hicolor/48x48/apps/bluetoother.png"
            DESTINATION    "${CMAKE_INSTALL_DATADIR}/icons/hicolor/48x48/apps/")
    install(FILES         "${STMMI_BTER_DATA_DIR}/icons/hicolor/scalable/apps/bluetoother.svg"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/icons/hicolor/scalable/apps/")
endif()
if (STMM_INSTALL_MAN_PAGE)
    install(FILES                   "${PROJECT_BINARY_DIR}/bluetoother.1.gz"
            DESTINATION "${CMAKE_INSTALL_DATADIR}/man/man1/")
endif()
 
