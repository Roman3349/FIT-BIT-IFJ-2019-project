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

add_library(dynamic_string dynamic_string.c dynamic_string.h)
add_library(parser parser.c parser.h)
add_library(scanner scanner.c scanner.h)
add_library(stack stack.c stack.h)
add_library(symtable symtable.c symtable.h)
add_library(token_stack token_stack.c token_stack.h)
add_library(parse_tree parse_tree.c parse_tree.h)
add_library(tree_element_stack tree_element_stack.c tree_element_stack.h)
target_link_libraries(scanner dynamic_string stack m)
target_link_libraries(token_stack scanner)
target_link_libraries(symtable dynamic_string)
target_link_libraries(parse_tree scanner)
target_link_libraries(parser parse_tree token_stack symtable tree_element_stack)
target_link_libraries(tree_element_stack parse_tree)

add_executable(ic19 main.c)
target_link_libraries(ic19 scanner parser)

