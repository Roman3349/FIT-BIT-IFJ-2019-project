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
            if(!process_string(file, &output_token, tmp)) {
                //output_token.data.value = NULL;
                output_token.type = T_NONE;
            }
            break;
        case '#' :
            if(!process_comment(file, &output_token)) {
                //output_token.data.value = NULL;
                output_token.type = T_NONE;
            }
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
                //output_token.data.value = NULL;
                output_token.type = T_NONE;
            }
            break;
        default: // keyword
            if(is_letter(tmp)) {
                if(!process_keyword(file, &output_token, tmp)){
                    //output_token.data.value = NULL;
                    output_token.type = T_NONE;
                }
            }
            else {
                output_token.type = T_UNKNOWN;
                output_token.data.value = dynStrInit();
                dynStrAppendChar(output_token.data.value, tmp);
                dynStrAppendChar(output_token.data.value, '\0');
            }
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

    token->data.value = dynStrInit();
    dynStrAppendChar(token->data.value, first_number);
    // temporary char buffer
    char tmp;

    if(fread(&tmp, 1, 1, file) != 1){
        return -1; // read fails
    }

    // if first number is 0 value can be binary, octal or hexadecimal
    if(first_number == 0) {
        // check second value
        // is number between <0,7> = octal
        if(is_oct(tmp)) {
            token->type = T_NUM_OCTA;
            dynStrAppendChar(token->data.value, tmp);
        }
        else { // is code
            switch (tmp) {
                // binary
                case 'B' :
                case 'b' :
                    token->type = T_NUM_BIN;
                    dynStrAppendChar(token->data.value, tmp);
                    break;
                    // hexadecimal
                case 'X' :
                case 'x' :
                    token->type = T_NUM_HEX;
                    dynStrAppendChar(token->data.value, tmp);
                    break;
                    // octal
                case 'O' :
                case 'o' :
                    token->type = T_NUM_OCTA;
                    dynStrAppendChar(token->data.value, tmp);
                    break;
                case 'e' :
                case 'E' :
                case '.' :
                    token->type = T_NUM_FLOAT;
                    dynStrAppendChar(token->data.value, tmp);
                    break;
                    // none of these
                    // value will be checked in first iteration of
                    // while cycle
                default:
                    break;
            }
        }
    }

    // read rest of the number
    while(TRUE) {

        // if value was not set, set it to int
        if (token->type == T_NONE) {
            token->type = T_NUM_INT;
        }
        else {
            if (fread(&tmp, 1, 1, file) != 1) {
                return -1; // read fails
            }
        }

        // check if value is number of given type
        // hexadecimal
        if((token->type == T_NUM_HEX  && is_hex(tmp))
           // octal
        || (token->type == T_NUM_OCTA && is_oct(tmp))
           // binary
        || (token->type == T_NUM_BIN  && is_bin(tmp))
           // int of float
        ||((token->type == T_NUM_INT || token->type == T_NUM_FLOAT)
                                      && is_dec(tmp)))
        {
            dynStrAppendChar(token->data.value, tmp);
        }
            // float detection
        else if(tmp == '.'
             || tmp == 'e'
             || tmp == 'E')
        {
            // set type float and continue
            // after floating point / exponent
            token->type = T_NUM_FLOAT;
            dynStrAppendChar(token->data.value, tmp);
        }
            // not a number
        else {
            // seek one char back to process
            // it later
            fseek(file, -1, SEEK_CUR);
            // add end of string
            dynStrAppendChar(token->data.value, '\0');

            return 0;
        }
    } // while(TRUE)
} // process_number()


/*
 * Scans keyword to a token
 * @param file          source file
 * @param token         pointer to a token where data will be stored
 * @param first_number  first char of the keyword
 * @returns status: 0 = success
 *                 -1 = file error
 *                 -2 = token error / memory allocation
 * @pre token must be empty - initialized to type T_NONE and value NULL
 */
int process_keyword(FILE* file, token_t* token, char first_char) {
    // check file
    if (!file) {
        return -1;
    }
    // check if token is initialized and empty
    if (!token || token->data.value) {
        return -2;
    }

    token->type = T_KEYWORD;

    // allocate output string
    token->data.value = dynStrInit();
    // append first character
    dynStrAppendChar(token->data.value, first_char);
    // temporary variable for currently processed char
    char tmp;

    while (fread(&tmp, 1, 1, file) == 1) {
        // char is part of keyword
        if(is_letter(tmp) || is_dec(tmp)) {
            dynStrAppendChar(token->data.value, tmp);
        }
        else {
            fseek(file, -1, SEEK_CUR);
            dynStrAppendChar(token->data.value, '\0');
            return 0;
        }
    }

    return -1; // read failure
}

/*
 * Checks if given string is lowercase of uppercase letter
 * @param c  string to check
 * @returns TRUE if c is letter in given range, FALSE otherwise
 */
int is_letter(char c) {
    return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')));
}


/*
 * Scans string or multiline comment to a token
 * @param file          source file
 * @param token         pointer to a token where data will be stored
 * @param qmark         first quotation mark, to determine string end
 * @returns status: 0 = success
 *                 -1 = file error
 *                 -2 = token error / memory allocation
 * @pre token must be empty - initialized to type T_NONE and value NULL
 */
// TODO
// - check if there can be unescaped quotation marks at the middle
// of the string
int process_string(FILE* file, token_t* token, char qmark) {
    if(!file) {
        return -1;
    }
    // token must be initialized and empty
    if(!token || token->data.value) {
        return -2;
    }

    token->data.value = dynStrInit();

    // is set to true if character is escaped
    int esc = FALSE;
    // if set to false after beginning of the string is processed
    int beginning = TRUE;

    // counter of quotation marks of the string / comment
    int qmark_beginning = 1;
    int qmark_end = 0;

    // stores currently processed char
    char tmp;

    // read to the end of string
    while(fread(&tmp, 1, 1, file) == 1) {
        if(qmark_end == qmark_beginning) {
            if(qmark_beginning == qmark_end == 1) {
                token->type = T_STRING;
            }
            else {
                token->type = T_STRING_ML;
            }
        }
        else if(esc) { // process escaped char
            dynStrAppendChar(token->data.value, tmp);
            esc = FALSE;
        }
        // is same as opening quotation mark
        else if(tmp == qmark) {
            if(beginning && qmark_beginning < 3) {
                qmark_beginning++;
            }
            else {
                beginning = FALSE;
                qmark_end++;
            }
        }
        else { // char inside the string
            beginning = FALSE;
            // unescaped quotation mark that doesn't close
            // the string / comment
            qmark_end = 0;

            // empty string
            if(qmark_beginning == 2) {
                // seek back 1 char
                fseek(file, -1, SEEK_CUR);
                // return empty string token
                token->type = T_STRING;
                return 0; // success
            }
            else {
                // add char to string data
                dynStrAppendChar(token->data.value, tmp);
                if(tmp == '\\') { // escaped char
                    esc = TRUE;
                }
            }
        }
    }// while()

    return -1; // read failure

}

/*
 * Scans line comment to a token (everything to the end of the line)
 * @param file          source file
 * @param token         pointer to a token where data will be stored
 * @returns status: 0 = success
 *                 -1 = file error
 *                 -2 = token error / memory allocation
 * @pre token must be empty - initialized to type T_NONE and value NULL
 */
// TODO
// check if eol can be escaped!
int process_comment(FILE* file, token_t* token){
    if(!file) {
        return -1;
    }
    // token must be initialized and empty
    if(!token || token->data.value) {
        return -2;
    }

    token->type = T_COMMENT;
    token->data.value = dynStrInit();
    // stores currently processed char
    char tmp;

    while(fread(&tmp, 1, 1, file) == 1) {
        // end of line/file = end of 1 line comment
        if(tmp == '\n' || tmp == '\0') {
            dynStrAppendChar(token->data.value, '\0');
            // seek back 1 char
            fseek(file, -1, SEEK_CUR);
            return 0; // success
        }
        else {
            // add char to the token data
            dynStrAppendChar(token->data.value, tmp);
        }
    } // while ()

    return -1; // read failure
}