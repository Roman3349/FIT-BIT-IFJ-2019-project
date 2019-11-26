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

file(GLOB_RECURSE _HDRFILES ${CMAKE_CURRENT_SOURCE_DIR}/*.h)
file(GLOB_RECURSE _SRCFILES ${CMAKE_CURRENT_SOURCE_DIR}/*.c)

add_library(dynamic_string dynamic_string.c dynamic_string.h)
add_library(parser parser.c parser.h)
add_library(scanner scanner.c scanner.h)
add_library(stack stack.c stack.h)
add_library(token_stack token_stack.c token_stack.h)
add_library(parse_tree parse_tree.c parse_tree.h)
target_link_libraries(scanner dynamic_string stack m)
target_link_libraries(token_stack scanner)
target_link_libraries(parser token_stack)
target_link_libraries(parser parse_tree token_stack)

add_executable(ic19int main.c)
target_link_libraries(ic19int scanner parser)

