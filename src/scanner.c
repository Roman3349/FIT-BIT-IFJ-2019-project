/*
 * Copyright (C) 2019 Pavel Raur <xraurp00@stud.fit.vutbr.cz>
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
#include "scanner.h"



/*
 * Scans source code in file and creates token representation
 * @param file    source file
 * @returns token that represents current keyword
 * @pre file is opened in read mode
 */
token_t scan(FILE* file){

    // create output token and set it to no value
    token_t output_token;
    output_token.data.value = NULL;
    output_token.type = T_NONE;

    // return error if no file is no file is supplied
    if(!file){
        return output_token;
    }

    // first char of the new token
    char tmp = '\0';
    // beginning of the line
    static int line_beginning = TRUE;

    // process code offset (number of tabs/spaces)
    // at the beginning of the line
    if(line_beginning) {
        if((output_token.data.size = getCodeOffset(file)) == -1){
            // read failed
            output_token.data.value = NULL;
            return output_token;
        }
        output_token.type = T_OFFSET;
        line_beginning = FALSE;
        return output_token;
    }

    // read first char
    if(fread(&tmp, 1, 1, file) != 1){
        free(output_token.data.value);
        output_token.data.value = NULL;
        return output_token;
    }

    switch (tmp){
        case ':' :
            output_token.type = T_NEWBLOCK;
            break;
        case ')' :
            output_token.type = T_RPAR;
            break;
        case '(' :
            output_token.type = T_LPAR;
            break;
        case '\'':
        case '\"':
            output_token.type = T_COMMENT;
            // TODO
            // process comment / string
            break;
        case '#' :
            output_token.type = T_COMMENT;
            // TODO
            // process comment to the end of the line
        case '=' :
        case '+' :
        case '-' :
        case '*' :
        case '/' :
        case '%' : // mod
            output_token.type = T_OPERATOR;
            break;
        case '\n':
            line_beginning = TRUE;
            output_token.type = T_EOL;
            break;
        case '\0':
            line_beginning = TRUE;
            output_token.type = T_EOF;
            break;
        case '\t':
        case ' ' :
            output_token.type = T_WHITESPACE;
            break;
        case '1' :
        case '2' :
        case '3' :
        case '4' :
        case '5' :
        case '6' :
        case '7' :
        case '8' :
        case '9' :
        case '0' :
            // if operation fails return empty token
            if(!process_number(file, &output_token, tmp)) {
                output_token.data.value = NULL;
                output_token.type = T_NONE;
            }
            break;
        default:
            output_token.type = T_CONTROLWORD;
            // TODO
            // output_token.data.value = process_controlword(...);
            break;
    }


    return output_token;

}

/*
 * Counts tabs or spaces in offset at the beginning of the line
 * @param file    source file
 * @returns number of whitespaces in padding or -1 if error
 */
int getCodeOffset(FILE* file){

    // reference character
    char ref = '\0';
    // currently processed character
    char tmp = '\0';
    // counts whitespaces
    int counter = 0;

    // read first char
    if(fread(&tmp, 1, 1, file) != 1){
        return -1;
    }

    // check if there is offset
    if(tmp == ' '
       || tmp == '\t'){
        counter++;
        // set reference character to check
        // if there are not mixed tabs and spaces
        ref = tmp;
    } else {
        // if there is no offset, seek back
        fseek(file, -1, SEEK_CUR);
        return 0;
    }

    while (fread(&tmp, 1, 1, file) == 1){

        // check if character is still offset
        if(tmp == ref){
            counter++;
        }
        else {
            // seek back if character doesnt match the reference
            fseek(file, -1, SEEK_CUR);
            return counter;
        }
    }
    return -1; // read failure
}

/*
 * Checks if number is decimal
 * @param num  number to check
 * @returns TRUE if number is decimal, FALSE if not
 */
int is_dec(char num){
    return ((num >= '0') && (num <= '9'));
}

/*
 * Checks if number is octal
 * @param num  number to check
 * @returns TRUE if number is octal, FALSE if not
 */
int is_oct(char num){
    return ((num >= '0') && (num <= '7'));
}

/*
 * Checks if number is hexadecimal
 * @param num  number to check
 * @returns TRUE if number is hexadecimal, FALSE if not
 */
int is_hex(char num){
    return (((num >= '0') && (num <= '7'))
         || ((num >= 'a') && (num <= 'f'))
         || ((num >= 'A') && (num <= 'F')));
}

/*
 * Checks if number is binary
 * @param num  number to check
 * @returns TRUE if number is binary, FALSE otherwise
 */
int is_bin(char num) {
    return ((num == '0') || (num == '1'));
}

/*
 * Scans number to a token
 * @param file          source file
 * @param token         pointer to a token where data will be stored
 * @param first_number  first digit of the number
 * @returns status: 0 = success
 *                 -1 = file error
 *                 -2 = token error / memory allocation
 * @pre token must be empty - initialized to type T_NONE and value NULL
 *
 * @TODO
 * check if number starting with zero and isn't octa is an error
 */
int process_number(FILE* file, token_t* token ,char first_number){

    // check file
    if(!file) {
        return -1;
    }
    // if no token is supplied
    if(!token){
        return -2;
    }
    // if token already have some value - exit
    if(token->data.value){
        return -2;
    }

    // allocate some space for number
    if((token->data.value = malloc(10)) == NULL){
        return -2;
    }
    token->data.value[0] = first_number;
    // next free position in the token
    unsigned position = 1;

    if(fread(&token->data.value[position], 1, 1, file) != 1){
        return -1; // read fails
    }

    // if first number is 0 value can be binary, octal or hexadecimal
    if(first_number == 0) {
        // check second value
        // is number between <0,7> = octal
        if(is_oct(token->data.value[1])) {
            token->type = T_NUM_OCTA;
            position++;
        }
        else { // is code
            switch (token->data.value[1]) {
                // binary
                case 'B' :
                case 'b' :
                    token->type = T_NUM_BIN;
                    position++;
                    break;
                    // hexadecimal
                case 'X' :
                case 'x' :
                    token->type = T_NUM_HEX;
                    position++;
                    break;
                    // octal
                case 'O' :
                case 'o' :
                    token->type = T_NUM_OCTA;
                    position++;
                    break;
                case 'e' :
                case 'E' :
                case '.' :
                    token->type = T_NUM_FLOAT;
                    position++;
                    break;
                    // none of these
                    // value will be checked in first iteration of
                    // while cycle
                default:
                    break;
            }
        }
    }
    // if value was not set, set it to int
    if (token->type == T_NONE) {
        token->type = T_NUM_INT;
    }

    // read rest of the number
    while(TRUE) {

        // skip first 2 positions - first is already checked
        // second is assigned and needs to be checked
        if(position > 1) {
            if (fread(&token->data.value[position], 1, 1, file) != 1) {
                return -1; // read fails
            }
        }

        // check if value is number of given type
        // hexadecimal
        if((token->type == T_NUM_HEX && is_hex(token->data.value[position]))
           // octal
           || (token->type == T_NUM_OCTA && is_oct(token->data.value[position]))
           // binary
           || (token->type == T_NUM_BIN && is_bin(token->data.value[position]))
           // int of float
           || ((token->type == T_NUM_INT || token->type == T_NUM_FLOAT)
               && is_dec(token->data.value[position])))
        {
            position++;
        }
            // float detection
        else if(token->data.value[position] == '.'
                || token->data.value[position] == 'e'
                || token->data.value[position] == 'E')
        {
            // set type float and continue
            // after floating point / exponent
            token->type = T_NUM_FLOAT;
            position++;
        }
            // not a number
        else {
            // seek one char back to process
            // it later
            fseek(file, -1, SEEK_CUR);
            // add end of string
            token->data.value[position] = '\0';
            // if type haven't been set yet
            // means there are no signs of
            // different type than int
            if (token->type == T_NONE) {
                token->type = T_NUM_INT;
            }
            return 0;
        }

        // reallocate memory if position have reached the last field
        if ((position%10) == 0){
            char* temp_ptr = realloc(token->data.value, position + 10);
            // exit if reallocation fails
            if(temp_ptr == NULL) {
                return -2;
            }
            token->data.value = temp_ptr;
        }
    } // while(TRUE)
} // process_number()