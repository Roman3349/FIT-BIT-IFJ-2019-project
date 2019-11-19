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

const char* KEYWORDS[] = {
        "def",
        "if",
        "else",
        "while",
        "pass",
        "return"
};

enum number_type {
    N_INT,
    N_HEX,
    N_OCT,
    N_BIN,
    N_FLO,
    N_UNDEF
};

// detect if function have found end of file
static bool eof_reached = false;

token_t scan(FILE* file, intStack_t* stack){

    // create output token and set it to no value
    token_t output_token;
    output_token.data.strval = NULL;
    output_token.type = T_NONE;

    // return error if no file is no file is supplied
    if(!file){
        return output_token;
    }
    // return error if stack is not initialized
    if(!stack){
        return output_token;
    }

    // first char of the new token
    int tmp = 0;
    // beginning of the line
    static bool line_beginning = true;
    // code offset
    int offset = 0;

    // read char
    while ((!eof_reached) && ((tmp = fgetc(file)) != EOF)) {

        // process code offset (number of spaces)
        // at the beginning of the line
        if (line_beginning) {
            if(tmp == ' ') {
                offset++;
                continue;
            }
            // remove empty line and continue with next one
            else if(tmp == '\n') {
                offset = 0;
                continue;
            }
            // remove line with comment (without any code)
            else if(tmp == '#') {
                if(remove_line_comment(file)) {
                    return output_token; // failed to read from file
                }
                continue;
            }
            else { // all chars of the offset were read
                // put back last char to be processed later (its not offset char)
                ungetc(tmp, file);
                line_beginning = false;
                int lastOffset; // offset of current block
                if (stackIsEmpty(stack)) {
                    lastOffset = 0; // main block
                }
                else if (!stackTop(stack, &lastOffset)) {
                    return output_token; // failed to read from stack
                }
                // end of code block
                if(lastOffset > offset) {
                    // check offset of previous block:

                    // pop current offset
                    if (!stackPop(stack, &lastOffset)) {
                        return output_token; // pop failed
                    }

                    int previousBlockOffset;
                    if (stackIsEmpty(stack)) {
                        previousBlockOffset = 0; // main block
                    }
                    else if (!stackTop(stack, &previousBlockOffset)){
                        return output_token; // top failed
                    }

                    // offset doesn't match the previous block offset
                    if (offset != previousBlockOffset) {
                        return output_token;
                    }

                    // offset match
                    output_token.type = T_DEDENT;
                    return output_token;
                }
                // start of new block
                else if (lastOffset < offset) {
                    stackPush(stack, offset);
                    output_token.type = T_INDENT;
                    return output_token;
                }
                else { // if(lastOffset == offset)
                    continue; // still same block, continue with content
                }
            }
        } // if (line_beginning)

        switch (tmp) {
            case ':' :
                output_token.type = T_COLON;
                break;
            case ')' :
                output_token.type = T_RPAR;
                break;
            case '(' :
                output_token.type = T_LPAR;
                break;
            case '\'':
            case '\"':
                if (process_string(file, &output_token, tmp)) {
                    //output_token.data.strval = NULL;
                    output_token.type = T_NONE;
                }
                break;
            case '#' :
                if (remove_line_comment(file)) {
                    output_token.type = T_NONE;
                }
            case '=' :
            case '+' :
            case '-' :
            case '*' :
            case '/' :
                output_token.type = T_OPERATOR;
                break;
            case '\n':
                line_beginning = true;
                output_token.type = T_EOL;
                break;
            case ' ' :
            case '\t':
                continue; // continue with next char
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
                if (process_number(file, &output_token, tmp)) {
                    output_token.type = T_NONE;
                }
                break;
            default: // keyword
                if (is_letter(tmp) || tmp == '_') {
                    if (process_keyword(file, &output_token, tmp)) {
                        //output_token.data.strval = NULL;
                        output_token.type = T_NONE;
                    }
                } else { // unknown value
                    output_token.type = T_UNKNOWN;
                    output_token.data.strval = dynStrInit();
                    dynStrAppendChar(output_token.data.strval, (char) tmp);
                }
                break;
        }// switch (tmp)


        return output_token;
    } // while(...)

    // eof reached
    line_beginning = true;
    eof_reached = false;
    output_token.type = T_EOF;
    return output_token;

}

bool is_oct(int num){
    return ((num >= '0') && (num <= '7'));
}


bool is_bin(int num) {
    return ((num == '0') || (num == '1'));
}

int process_number(FILE* file, token_t* token, int first_number) {

    // check file
    if(!file) {
        return -1;
    }
    // if no token is supplied
    if(!token){
        return -2;
    }
    // if token already have some value - exit
    if(token->data.strval){
        return -2;
    }

    // set token type to number
    token->type = T_NUMBER;

    // number type
    enum number_type type = N_UNDEF;

    // temporary number string buffer
    dynStr_t* str_number;
    str_number = dynStrInit();

    dynStrAppendChar(str_number, (char) first_number);
    // temporary char buffer
    int tmp;

    // read next char, check for eof
    if((tmp = fgetc(file)) == EOF){
        eof_reached = true;
        token->data.intval = strtol(str_number->string,
                                    NULL, 10);
        dynStrFree(str_number);
        return 0;
    }

    // if first number is 0 value can be binary, octal or hexadecimal
    if(first_number == '0') {
        // check second value
        switch (tmp) {
            // binary
            case 'B' :
            case 'b' :
                type = N_BIN;
                break;
            // hexadecimal
            case 'X' :
            case 'x' :
                type = N_HEX;
                break;
            // octal
            case 'O' :
            case 'o' :
                type = N_OCT;
                break;
            case 'e' :
            case 'E' :
            case '.' :
                type = N_FLO;
                dynStrAppendChar(str_number, (char)tmp);
                break;
            // none of these
            // value will be checked in first iteration of while cycle
            // TODO: send error token
            default:
                type = N_UNDEF;
                break;
            } // switch (tmp)
    } // if(first_number == '0')

    // read rest of the number
    while(true) {

        // if value was not set, set it to int
        if (type == N_UNDEF) {
            type = N_INT;
        }
        else if ((tmp = fgetc(file)) == EOF) {
            eof_reached = true;
        }

        // check if value is number of given type
        // hexadecimal
        if((type == N_HEX  && isxdigit(tmp))
           // octal
        || (type == N_OCT && is_oct(tmp))
           // binary
        || (type == N_BIN  && is_bin(tmp))
           // int of float
        ||((type == N_INT || type == N_FLO)
                                      && isdigit(tmp)))
        {
            dynStrAppendChar(str_number, (char)tmp);
        }
        // float detection
        else if((type == N_INT) && (
                tmp == '.'
             || tmp == 'e'
             || tmp == 'E'))
        {
            // set type float and continue
            // after floating point / exponent
            type = N_FLO;
            dynStrAppendChar(str_number, (char)tmp);
        }
        // not a number
        else {
            // return the char
            if(!eof_reached) {
                ungetc(tmp, file);
            }
            // convert string to number
            switch (type) {
                case N_INT:
                    token->data.intval = strtol(str_number->string,
                                                NULL, 10);
                    break;
                case N_BIN:
                    token->data.intval = strtol(str_number->string,
                                                NULL, 2);
                    break;
                case N_OCT:
                    token->data.intval = strtol(str_number->string,
                                                NULL, 8);
                    break;
                case N_HEX:
                    token->data.intval = strtol(str_number->string,
                                                NULL, 16);
                    break;
                case N_FLO:
                    token->type = T_FLOAT;
                    token->data.floatval = strtod(str_number->string,
                                                  NULL);
                    break;
                default:
                    token->type = T_NONE;
                    token->data.strval = NULL;
                    break;
            }
            // deallocate the memory
            dynStrFree(str_number);
            return 0;
        }
    } // while(TRUE)
} // process_number()


int process_keyword(FILE* file, token_t* token, int first_char) {
    // check file
    if (!file) {
        return -1;
    }
    // check if token is initialized and empty
    if (!token || token->data.strval) {
        return -2;
    }

    dynStr_t* tmp_string = dynStrInit();
    // append first character
    dynStrAppendChar(tmp_string, (char) first_char);
    // temporary variable for currently processed char
    int tmp;

    while ((tmp = fgetc(file)) != EOF) {
        // char is part of keyword
        if(isalnum(tmp) || tmp == '_') {
            dynStrAppendChar(tmp_string, (char)tmp);
        }
        else {
            ungetc(tmp, file);
            token->type = getKeywordType(tmp_string->string);

            if(token->type == T_ID) {
                token->data.strval = tmp_string;
            } else {
                dynStrFree(tmp_string);
            }
            return 0;
        }
    }

    eof_reached = true;
    return -1;
}


enum token_type getKeywordType(char *string) {
    int i;
    for(i = 0; i < 7; i++) { // 7 number of types
        if(strcmp(string, KEYWORDS[i]) == 0){
            break;
        }
    }
    switch (i) {
        case 0:
            return T_ID_DEF;
        case 1:
            return T_ID_IF;
        case 2:
            return T_ID_ELSE;
        case 3:
            return T_ID_WHILE;
        case 4:
            return T_ID_PASS;
        case 5:
            return T_ID_RETURN;
        default:
            return T_ID;
    }
}

/*
 * Checks if given string is lowercase of uppercase letter
 * @param c  string to check
 * @returns TRUE if c is letter in given range, FALSE otherwise
 */
int is_letter(int c) {
    return (((c >= 'a') && (c <= 'z')) || ((c >= 'A') && (c <= 'Z')));
}


// TODO
// - check if there can be unescaped quotation marks at the middle
// of the string
int process_string(FILE* file, token_t* token, int qmark) {
    if(!file) {
        return -1;
    }
    // token must be initialized and empty
    if(!token || token->data.strval) {
        return -2;
    }

    token->data.strval = dynStrInit();

    // is set to true if character is escaped
    int esc = false;
    // if set to false after beginning of the string is processed
    int beginning = true;

    // counter of quotation marks of the string / comment
    int qmark_beginning = 1;
    int qmark_end = 0;

    // stores currently processed char
    int tmp;

    // read to the end of string
    while((tmp = fgetc(file)) != EOF) {
        if(qmark_end == qmark_beginning) {
            if((qmark_beginning == qmark_end) == 1) {
                token->type = T_STRING;
            }
            else {
                token->type = T_STRING_ML;
            }
        }
        else if(esc) { // process escaped char
            dynStrAppendChar(token->data.strval, (char)tmp);
            esc = false;
        }
        // is same as opening quotation mark
        else if(tmp == qmark) {
            if(beginning && qmark_beginning < 3) {
                qmark_beginning++;
            }
            else {
                beginning = false;
                qmark_end++;
            }
        }
        else { // char inside the string
            beginning = false;
            // unescaped quotation mark that doesn't close
            // the string / comment
            qmark_end = 0;

            // empty string
            if(qmark_beginning == 2) {
                // return char
                ungetc(tmp, file);
                // return empty string token
                token->type = T_STRING;
                return 0; // success
            }
            else {
                // add char to string data
                dynStrAppendChar(token->data.strval, (char)tmp);
                if(tmp == '\\') { // escaped char
                    esc = true;
                }
            }
        }
    }// while()
    eof_reached = true;
    return -1;

}

// TODO
// test escaped eol! Eol in python can't be escaped!
int remove_line_comment(FILE* file){
    if(!file) {
        return -1;
    }

    // stores currently processed char
    int tmp;

    while((tmp = fgetc(file)) != EOF) {
        if(tmp == '\n') {
            ungetc(tmp, file);
            return 0; // success
        }
    } // while ()
    eof_reached = true;
    return 0;
}
