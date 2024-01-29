# Install script for directory: /home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/sources/ippcp

# Set the install prefix
if(NOT DEFINED CMAKE_INSTALL_PREFIX)
  set(CMAKE_INSTALL_PREFIX "/home/jxzhan/intel/ippcp_2021.4.0")
endif()
string(REGEX REPLACE "/$" "" CMAKE_INSTALL_PREFIX "${CMAKE_INSTALL_PREFIX}")

# Set the install configuration name.
if(NOT DEFINED CMAKE_INSTALL_CONFIG_NAME)
  if(BUILD_TYPE)
    string(REGEX REPLACE "^[^A-Za-z0-9_]+" ""
           CMAKE_INSTALL_CONFIG_NAME "${BUILD_TYPE}")
  else()
    set(CMAKE_INSTALL_CONFIG_NAME "Release")
  endif()
  message(STATUS "Install configuration: \"${CMAKE_INSTALL_CONFIG_NAME}\"")
endif()

# Set the component getting installed.
if(NOT CMAKE_INSTALL_COMPONENT)
  if(COMPONENT)
    message(STATUS "Install component: \"${COMPONENT}\"")
    set(CMAKE_INSTALL_COMPONENT "${COMPONENT}")
  else()
    set(CMAKE_INSTALL_COMPONENT)
  endif()
endif()

# Install shared libraries without execute permission?
if(NOT DEFINED CMAKE_INSTALL_SO_NO_EXE)
  set(CMAKE_INSTALL_SO_NO_EXE "1")
endif()

# Is this installation the result of a crosscompile?
if(NOT DEFINED CMAKE_CROSSCOMPILING)
  set(CMAKE_CROSSCOMPILING "FALSE")
endif()

# Set default install directory permissions.
if(NOT DEFINED CMAKE_OBJDUMP)
  set(CMAKE_OBJDUMP "/usr/bin/objdump")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/intel64//libippcpy8.so.11.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/intel64//libippcpy8.so.11"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      file(RPATH_CHECK
           FILE "${file}"
           RPATH "")
    endif()
  endforeach()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/intel64/" TYPE SHARED_LIBRARY FILES
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/.build/RELEASE/lib/libippcpy8.so.11.2"
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/.build/RELEASE/lib/libippcpy8.so.11"
    )
  foreach(file
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/intel64//libippcpy8.so.11.2"
      "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/intel64//libippcpy8.so.11"
      )
    if(EXISTS "${file}" AND
       NOT IS_SYMLINK "${file}")
      if(CMAKE_INSTALL_DO_STRIP)
        execute_process(COMMAND "/usr/bin/strip" "${file}")
      endif()
    endif()
  endforeach()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/intel64//libippcpy8.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/intel64//libippcpy8.so")
    file(RPATH_CHECK
         FILE "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/intel64//libippcpy8.so"
         RPATH "")
  endif()
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/intel64/" TYPE SHARED_LIBRARY FILES "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/.build/RELEASE/lib/libippcpy8.so")
  if(EXISTS "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/intel64//libippcpy8.so" AND
     NOT IS_SYMLINK "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/intel64//libippcpy8.so")
    if(CMAKE_INSTALL_DO_STRIP)
      execute_process(COMMAND "/usr/bin/strip" "$ENV{DESTDIR}${CMAKE_INSTALL_PREFIX}/lib/intel64//libippcpy8.so")
    endif()
  endif()
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/include/ippcp.h"
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/include/ippcpdefs.h"
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/include/ippversion.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/intel64/" TYPE STATIC_LIBRARY FILES "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/.build/RELEASE/lib/libippcp_s_y8.a")
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/include" TYPE FILE FILES
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/include/ippcp.h"
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/include/ippcpdefs.h"
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/include/ippversion.h"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/pkgconfig" TYPE FILE OPTIONAL FILES
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/ippcp-static-intel64.pc"
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/ippcp-dynamic-intel64.pc"
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/crypto_mb-static-intel64.pc"
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/crypto_mb-dynamic-intel64.pc"
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/ippcp-static-ia32.pc"
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/ippcp-dynamic-ia32.pc"
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/ippcp-static-intel64-nonpic.pc"
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/ippcp-static-ia32-nonpic.pc"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/lib/cmake/ippcp" TYPE FILE FILES
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/ippcp-config.cmake"
    "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/ippcp-config-version.cmake"
    )
endif()

if("x${CMAKE_INSTALL_COMPONENT}x" STREQUAL "xUnspecifiedx" OR NOT CMAKE_INSTALL_COMPONENT)
  file(INSTALL DESTINATION "${CMAKE_INSTALL_PREFIX}/tools/custom_library_tool_python" TYPE DIRECTORY FILES "/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/tools/ipp_custom_library_tool_python/")
endif()

if(NOT CMAKE_INSTALL_LOCAL_ONLY)
  # Include the install script for each subdirectory.
  include("/home/jxzhan/tdx-loader-v1.0.01.01/seam-loader-main/seam-loader-main/p-seam-loader/libs/ipp/ipp-crypto-ippcp_2021.4/_build/sources/ippcp/crypto_mb/cmake_install.cmake")

endif()

