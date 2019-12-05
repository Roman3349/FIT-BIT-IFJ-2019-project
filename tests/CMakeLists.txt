# Copyright (C) 2019 Roman Ondráček <xondra58@stud.fit.vutbr.cz>
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.

cmake_minimum_required(VERSION 3.0)

enable_testing()
find_package(GTest QUIET)

if (NOT GTest_FOUND)
    add_subdirectory(${CMAKE_SOURCE_DIR}/external/googletest googletest EXCLUDE_FROM_ALL)
    set(GTEST_BOTH_LIBRARIES gtest gtest_main)
else (NOT GTest_FOUND)
    include_directories(${GTEST_INCLUDE_DIRS})
endif (NOT GTest_FOUND)

include_directories(${CMAKE_SOURCE_DIR}/src)

file(GLOB_RECURSE _HDRFILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE _SRCFILES ${CMAKE_CURRENT_SOURCE_DIR}/*.cpp)

add_executable(tests ${_HDRFILES} ${_SRCFILES})
target_link_libraries(tests ${GTEST_BOTH_LIBRARIES} scanner parser)
