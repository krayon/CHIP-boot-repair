CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(chip-boot-repair)

INCLUDE ( "${CMAKE_TOOLCHAIN_FILE}" )

SET( PROJECT_VERSION_MINOR "1" )
SET( PROJECT_VERSION_MAJOR "0" )
SET( PROJECT_VERSION_PATCH "1" )
SET( PROJECT_VERSION "${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH}" )

FIND_PACKAGE(PkgConfig)

PKG_SEARCH_MODULE(GTK REQUIRED gtk+-2.0)
PKG_SEARCH_MODULE(LIBUSB REQUIRED libusb-1.0)

SET( SOURCE_FILES
  src/GtkRepairView.cpp
  src/RepairTool.cpp
  src/fel.c
  src/libsunxi.cpp
)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
ADD_DEFINITIONS(-DLIBSUNXI)

INCLUDE_DIRECTORIES(
  include
  ${GTK_INCLUDE_DIRS}
  ${LIBUSB_INCLUDE_DIRS}
)

MESSAGE( STATUS "GTK_INCLUDE_DIRS: " ${GTK_INCLUDE_DIRS} )

ADD_EXECUTABLE( chip-boot-repair ${SOURCE_FILES})
LINK_DIRECTORIES(
  ${GTK_LIBRARY_DIRS}
  ${LIBUSB_LIBRARY_DIRS}
)
TARGET_LINK_LIBRARIES( chip-boot-repair ${GTK_LIBRARIES} ${LIBUSB_LIBRARIES} )

INSTALL( FILES "payload/padded-uboot" DESTINATION "share/chip-boot-repair" )
INSTALL( FILES "payload/sunxi-spl-with-ecc.bin" DESTINATION "share/chip-boot-repair" )
INSTALL( FILES "payload/sunxi-spl.bin" DESTINATION "share/chip-boot-repair" )
INSTALL( FILES "payload/uboot.scr" DESTINATION "share/chip-boot-repair" )

INSTALL( TARGETS "chip-boot-repair" DESTINATION sbin )
ADD_CUSTOM_TARGET(create_gz ALL COMMAND gzip "-9" "-fc" "${CMAKE_CURRENT_SOURCE_DIR}/assets/changelog" > "changelog.gz")
ADD_DEPENDENCIES( chip-boot-repair create_gz )

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -static-libgcc -static-libstdc++")

SET( CPACK_GENERATOR "DEB")
SET( CPACK_PACKAGE_FILE_NAME "chip-boot-repair" )
SET( CPACK_STRIP_FILES "bin/chip-boot-repair" )
SET( CPACK_DEBIAN_PACKAGE_VERSION "${PROJECT_VERSION}" )
SET( CPACK_DEBIAN_PACKAGE_MAINTAINER "Next Thing Co. <ahoyahoy@nextthing.co>" )
SET( CPACK_DEBIAN_PACKAGE_DEPENDS "libusb-1.0-0,libgtk2.0-0" )
SET( CPACK_DEBIAN_PACKAGE_DESCRIPTION "A boot repair tool for C.H.I.P.! Recovers C.H.I.P.s that suffer from too many bit flips in the boot area of the NAND \\\\(•◡•)/" )
SET( CPACK_DEBIAN_PACKAGE_SHLIBDEPS  ON )

INSTALL(FILES ${CMAKE_CURRENT_SOURCE_DIR}/assets/license.txt
      DESTINATION share/doc/chip-boot-repair
      COMPONENT applications
      RENAME copyright)

INSTALL(FILES "${CMAKE_CURRENT_BINARY_DIR}/changelog.gz"
      DESTINATION share/doc/chip-boot-repair
      COMPONENT applications
      RENAME changelog.gz)

INSTALL( FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/chip-boot-repair.desktop" DESTINATION share/applications )
INSTALL( PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/assets/chip-boot-repair-pkexec" DESTINATION bin )
INSTALL( FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/com.ubuntu.pkexec.chip-boot-repair.policy" DESTINATION share/polkit-1/actions )
INSTALL( FILES "${CMAKE_CURRENT_SOURCE_DIR}/assets/chip-boot-repair.svg" DESTINATION share/icons/hicolor/scalable/apps )

INCLUDE( CPack )
