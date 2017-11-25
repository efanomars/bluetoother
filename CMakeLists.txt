# File: bluetoother/CMakeLists.txt

cmake_minimum_required(VERSION 3.0)

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

pkg_check_modules(GTKMM       REQUIRED  gtkmm-3.0>=${GTKMM_VERSION_REQ})
pkg_check_modules(BLUETOOTH   REQUIRED  bluez>=${BLUETOOTH_VERSION_REQ})

list(APPEND STMMI_BTER_EXTRA_INCLUDE_DIRS   ${GTKMM_INCLUDE_DIRS} ${BLUETOOTH_INCLUDE_DIRS} )
list(APPEND STMMI_BTER_EXTRA_LIBS           ${GTKMM_LIBRARIES}    ${BLUETOOTH_LIBRARIES}    )

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
        ${PROJECT_SOURCE_DIR}/src/main.cc
        ${PROJECT_SOURCE_DIR}/src/tootherwindow.h
        ${PROJECT_SOURCE_DIR}/src/tootherwindow.cc
        )
set(STMMI_BTER_DATA_FILES
#         ${PROJECT_SOURCE_DIR}/data/some.png
        )

add_executable(bluetoother ${STMMI_BTER_SOURCES} ${PROJECT_BINARY_DIR}/config.cc)

target_include_directories(bluetoother SYSTEM PUBLIC ${STMMI_BTER_EXTRA_INCLUDE_DIRS})

DefineTargetPublicCompileOptions(bluetoother)

include(GNUInstallDirs)
set(BLUETOOTHER_PKG_DATA_DIR     "${CMAKE_INSTALL_FULL_DATADIR}/bluetoother")
set(BLUETOOTHER_PKG_REL_DATA_DIR "${CMAKE_INSTALL_DATADIR}/bluetoother")

# Create config file for executable
configure_file("${PROJECT_SOURCE_DIR}/src/config.cc.in"
               "${PROJECT_BINARY_DIR}/config.cc" @ONLY)
# This allows config.cc to find the config.h include
target_include_directories(bluetoother PUBLIC ${PROJECT_SOURCE_DIR}/src)

if ($ENV{STMM_CMAKE_COMMENTS})
message(STATUS "")
message(STATUS "bluetoother was configured with the following options:")
message(STATUS " STMMI_BTER_SOURCES:            ${STMMI_BTER_SOURCES}")
#message(STATUS " STMMI_BTER_EXTRA_INCLUDE_DIRS: ${STMMI_BTER_EXTRA_INCLUDE_DIRS}")
message(STATUS " STMMI_BTER_EXTRA_LIBS:         ${STMMI_BTER_EXTRA_LIBS}")
message(STATUS " STMMI_BTER_DATA_FILES          ${STMMI_BTER_DATA_FILES}")
message(STATUS " CMAKE_BUILD_TYPE:              ${CMAKE_BUILD_TYPE}")
message(STATUS " CMAKE_CXX_COMPILER_ID:         ${CMAKE_CXX_COMPILER_ID}")
message(STATUS " CMAKE_CXX_COMPILER_VERSION:    ${CMAKE_CXX_COMPILER_VERSION}")
#message(STATUS " CMAKE_CXX_FLAGS:               $<TARGET_PROPERTY:bluetoother,COMPILE_OPTIONS>")
message(STATUS " install prefix:                ${CMAKE_INSTALL_PREFIX}")
endif()

target_link_libraries(bluetoother ${STMMI_BTER_EXTRA_LIBS}) # ${STMMI_BTER_EXTRA_LDFLAGS}

install(TARGETS bluetoother RUNTIME DESTINATION "bin")

install(FILES ${STMMI_BTER_DATA_FILES}  DESTINATION   ${BLUETOOTHER_PKG_REL_DATA_DIR})

