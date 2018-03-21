#
# This file is part of R600-disass a program to analyse R600
# binary shaders.
#
# R600-disass is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation; either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, see <http://www.gnu.org/licenses/>.
#

PROJECT("mia")

cmake_minimum_required(VERSION 3.7)

SET(VENDOR "Gert Wollny")
SET(PACKAGE_NAME "r600-disass")
SET(MAJOR_VERSION 0)
SET(MINOR_VERSION 0)
SET(MICRO_VERSION 1)
SET(INTERFACE_AGE 0)
SET(BINARY_AGE    0)

SET(PACKAGE_VERSION "${MAJOR_VERSION}.${MINOR_VERSION}.${MICRO_VERSION}")

MATH(EXPR CURRENT "${MICRO_VERSION} - ${BINARY_AGE}")
SET(REVISION ${INTERFACE_AGE})
MATH(EXPR AGE "${BINARY_AGE} - ${INTERFACE_AGE}")

SET(LIBRARY_SOVERSION_INFO "${CURRENT}")
SET(LIBRARY_VERSION_INFO "${CURRENT}.${AGE}.${REVISION}")

ENABLE_TESTING()

include(CheckCXXCompilerFlag)

set(CMAKE_CXX_STANDARD_DEFAULT "")
IF (((${CMAKE_CXX_COMPILER_ID} STREQUAL "GNU") AND
      (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 5.3.0)) OR
    ((${CMAKE_CXX_COMPILER_ID} STREQUAL "Clang") AND
      (CMAKE_CXX_COMPILER_VERSION VERSION_GREATER 3.4.0)))
  MESSAGE(STATUS "Compiler has full c++14 support")

  # g++ >= 6.0 sets std=c++14 by default
  IF (NOT ( ${CMAKE_CXX_COMPILER_ID}  STREQUAL "GNU") OR
      (CMAKE_CXX_COMPILER_VERSION VERSION_LESS 6.0.0))
    set(CMAKE_CXX_STANDARD 14)
    set(CMAKE_CXX_STANDARD_REQUIRED ON)
  ENDIF()

ELSE()
  CHECK_CXX_COMPILER_FLAG("-std=c++14" HAS_GNU_CXX11_FLAG)
  IF(HAS_GNU_CXX11_FLAG)
    SET(CXX_14_FLAG "-std=c++14")
  ELSE(HAS_GNU_CXX11_FLAG)
    CHECK_CXX_COMPILER_FLAG("-std=c++0y" HAS_GNU_CXX0Y_FLAG)
    IF(HAS_GNU_CXX0Y_FLAG)
      SET(CXX_14_FLAG "-std=c++0y")
    ELSE(HAS_GNU_CXX0Y_FLAG)
      MESSAGE(WARNING "Don't know how to enable C++14, if you are lucky, they are enabled by default")
    ENDIF(HAS_GNU_CXX0Y_FLAG)
  ENDIF(HAS_GNU_CXX0Y_FLAG)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}  ${CXX_14_FLAG}")
ENDIF()


find_package(Qt5 COMPONENTS Core Gui Widgets REQUIRED)

find_package(GTest)

if (NOT GTEST_FOUND)
        SET(BUILD_SHARED_LIBS TRUE)
        ADD_SUBDIRECTORY(/usr/src/gtest gtest)
        SET(GTEST_LIBRARY gtest)
        SET(GTEST_MAIN_LIBRARY gtest_main)
endif()

include_directories(${GTEST_INCLUDE_DIR})

set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_INCLUDE_CURRENT_DIR ON)

include_directories (${PROJECT_SOURCE_DIR})
include_directories (${PROJECT_BINARY_DIR})

ADD_SUBDIRECTORY(r600)

SET(EXE_SRC
  main.cpp
  mainwindow.cpp)

ADD_EXECUTABLE(r600-disass-qt ${EXE_SRC})
TARGET_LINK_LIBRARIES(r600-disass-qt
  r600-disass Qt5::Gui Qt5::Widgets Qt5::Core)
