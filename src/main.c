/*
 * Copyright (C) 2019 Roman Ondráček <xondra58@stud.fit.vutbr.cz>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdio.h>
#include "parser.h"

/**
 * Main function
 * @param argc Argument count
 * @param argv Arguments
 * @return Execution status
 */
int main(int argc, char *argv[]) {
	FILE* file = fopen("/home/fjerabek/Downloads/example1.src", "r");
    treeElement_t tree = syntaxParse(file);
	printTree(tree, 0);
	treeFree(tree);
    fclose(file);
	return 0;
}
