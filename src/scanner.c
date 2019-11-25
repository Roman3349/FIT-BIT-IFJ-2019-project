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
        "return",
        "None",
        "and",
        "or",
        "not",
        "True",
        "False"
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
    output_token.type = T_ERROR;

    // return error if no file is no file is supplied
    if(!file){
        return output_token;
    }
    // return error if stack is NULL
    if(!stack){
        return output_token;
    }
    // stack is not initialized (doesn't have 0 on top)
    if (stackIsEmpty(stack)) {
        return output_token;
    }

    // first char of the new token
    int tmp = 0;
    // beginning of the line
    static bool line_beginning = true;
    // char used for indentation
    static int offset_char = '\0';
    // code offset
    int offset = 0;

    // read char
    while ((!eof_reached) && ((tmp = fgetc(file)) != EOF)) {

        // process code offset (number of spaces)
        // at the beginning of the line
        // TODO
        // add handling for tabs!
        if (line_beginning) {
            if(tmp == ' ' || tmp == '\t') {
                // set char used in indentation on first occurrence
                if(offset_char == '\0'){
                    offset_char = tmp;
                    offset++;
                    continue;
                }
                else if(tmp == offset_char) {
                    offset++;
                    continue;
                }
                else {
                    // error - there are mixed tabs and spaces in indentation
                    output_token.type = T_UNKNOWN;
                    return output_token;
                }
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
                int lastOffset; // offset of current block (last on stack)
                if (!stackTop(stack, &lastOffset)) {
                    return output_token; // failed to read from stack
                }
                // end of code block
                if(lastOffset > offset) {
                    // check offset of previous block:
                    // pop current offset
                    if (!stackPop(stack, &lastOffset)) {
                        return output_token; // pop failed
                    }
                    // read previous offset
                    int previousBlockOffset;
                    if (!stackTop(stack, &previousBlockOffset)){
                        return output_token; // top failed
                    }

                    // offset is higher than previous block offset
                    // but lower than current block offset
                    // error of indentation
                    if (offset > previousBlockOffset) {
                        output_token.type = T_UNKNOWN;
                        return output_token;
                    }
                    // offset is lower (return from multiple indent)
                    else if (offset < previousBlockOffset) {
                        // force evaluation of indentation next time
                        line_beginning = true;
                        // stack value is popped already
                        for ( ; offset > 0; offset--) {
                            ungetc(offset_char, file); // undo all offset chars
                        } // return dedent token
                    } // else - offset match
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

        // return status of auxiliary functions
        int return_status;

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
                return_status = process_string(file, &output_token, tmp);
                if (return_status == ANALYSIS_FAILED) {
                    output_token.type = T_UNKNOWN;
                }
                else if (return_status) {
                    output_token.type = T_ERROR;
                }
                break;
            case '#' :
                if (remove_line_comment(file)) {
                    output_token.type = T_ERROR;
                    break;
                }
                continue; // process eol after comment removal
            case '=' :
                output_token.type = T_ASSIGN;
                if((tmp = fgetc(file)) == EOF) {
                    eof_reached = true;
                }
                else if(tmp == '=') {
                    output_token.type = T_OP_EQ;
                }
                else {
                    ungetc(tmp, file);
                }
                break;
            case '+' :
                output_token.type = T_OP_ADD;
                break;
            case '-' :
                output_token.type = T_OP_SUB;
                break;
            case '*' :
                output_token.type = T_OP_MUL;
                break;
            case '/' :
                output_token.type = T_OP_DIV;
                if((tmp = fgetc(file)) == EOF) {
                    eof_reached = true;
                }
                else if(tmp == '/') {
                    output_token.type = T_OP_IDIV;
                }
                else {
                    ungetc(tmp, file);
                }
                break;
            case '>' :
                output_token.type = T_OP_GREATER;
                if((tmp = fgetc(file)) == EOF) {
                    eof_reached = true;
                }
                else if(tmp == '=') {
                    output_token.type = T_OP_GREATER_EQ;
                }
                else {
                    ungetc(tmp, file);
                }
                break;
            case '<' :
                output_token.type = T_OP_LESS;
                if((tmp = fgetc(file)) == EOF) {
                    eof_reached = true;
                }
                else if(tmp == '=') {
                    output_token.type = T_OP_LESS_EQ;
                }
                else {
                    ungetc(tmp, file);
                }
                break;
            case '!' :
                output_token.type = T_OP_NEG;
                if((tmp = fgetc(file)) == EOF) {
                    eof_reached = true;
                }
                else if(tmp == '=') {
                    output_token.type = T_OP_NOT_EQ;
                }
                else {
                    ungetc(tmp, file);
                }
                break;
            case ',' :
                output_token.type = T_COMMA;
                break;
            case '\n':
                line_beginning = true;
                output_token.type = T_EOL;
                break;
            case ' ' :
            case '\t':
                continue; // skip whitespace
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
                return_status = process_number(file, &output_token, tmp);
                if (return_status == ANALYSIS_FAILED) {
                    output_token.type = T_UNKNOWN;
                }
                else if (return_status == EXECUTION_ERROR) {
                    output_token.type = T_ERROR;
                }
                break;
            default: // keyword
                if (isalpha(tmp) || tmp == '_') {
                    if (process_keyword(file, &output_token, tmp)) {
                        output_token.type = T_ERROR;
                    }
                }
                else { // unknown value
                    output_token.type = T_UNKNOWN;
                }
                break;
        }// switch (tmp)

        return output_token;
    } // while(...)

    // eof reached

    // return all indentation from stack
    int tmpvar; // temporary stores offset
    if(!stackTop(stack, &tmpvar)) {
        return output_token;
    }
    else if(tmpvar) {
        if(!stackPop(stack, &tmpvar)) {
            return output_token;
        }
        output_token.type = T_DEDENT;
    }
    else {
        line_beginning = true;
        eof_reached = false;
        output_token.type = T_EOF;
    }
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
        return EXECUTION_ERROR;
    }
    // if no token is supplied
    if(!token){
        return EXECUTION_ERROR;
    }
    // if token already have some value - exit
    if(token->data.strval){
        return EXECUTION_ERROR;
    }

    // set token type to number
    token->type = T_NUMBER;

    // number type
    enum number_type type = N_UNDEF;

    // tels if last processed char was a decimal point
    bool decimalPoint = false;
    // is set to true if exponent is found
    bool exponent = false;
    // set to true if next char can be exponent sign
    bool sig = false;
    // temporary variable for storing base if number have exponent
    double base = 0;

    // temporary number string buffer
    dynStr_t* str_number = dynStrInit();

    dynStrAppendChar(str_number, (char) first_number);
    // temporary char buffer
    int tmp;

    // read next char, check for eof
    if((tmp = fgetc(file)) == EOF){
        eof_reached = true;
        token->data.intval = strtol(str_number->string,
                                    NULL, 10);
        dynStrFree(str_number);
        return SUCCESS;
    }

    // if first number is 0 value can be binary, octal or hexadecimal
    if(first_number == '0') {
        // check second value
        switch (tolower(tmp)) {
            // binary
            case 'b' :
                type = N_BIN;
                dynStrClear(str_number); // remove number type code from string
                break;
            // hexadecimal
            case 'x' :
                type = N_HEX;
                dynStrClear(str_number); // remove number type code from string
                break;
            // octal
            case 'o' :
                type = N_OCT;
                dynStrClear(str_number); // remove number type code from string
                break;
            // none of these
            // value will be checked in first iteration of while cycle
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
        else if((type == N_INT) && (tmp == '.'))
        {
            // set type float and continue
            // after decimal point
            type = N_FLO;
            dynStrAppendChar(str_number, (char)tmp);
            decimalPoint = true;
            continue;
        }
        // float with exponent - base * 10 ^ exp
        else if((type == N_INT || type == N_FLO)
             && (tmp == 'e' || tmp == 'E')) {

            if(exponent) { // exponent is parsed allready
                dynStrFree(str_number);
                return ANALYSIS_FAILED;
            }
            type = N_FLO;
            // base is complete
            base = strtod(str_number->string, NULL);
            // clear number to store exponent
            dynStrClear(str_number);
            exponent = true;
            // next char can be exponent sign (+ or -)
            sig = true;
            continue;
        }
        // process exponent sign
        else if(sig && exponent && (tmp == '+' || tmp == '-')) {
            dynStrAppendChar(str_number, (char)tmp);
        }
        // underscore can be used for number separation, for better readability
        else if(tmp == '_') {
            if(!sig) { // cannot be on first position in exponent
                continue; // skip the underscore and read the next number
            }
            else {
                dynStrFree(str_number);
                return ANALYSIS_FAILED;
            }
        }
        // not a number or not a number in given range (octal / binary)
        else if(isalnum(tmp)) {
            dynStrFree(str_number);
            return ANALYSIS_FAILED;
        }
        else {
            // put the char back
            if(!eof_reached) {
                ungetc(tmp, file);
            }
            // number after decimal point is missing
            if(decimalPoint) {
                dynStrFree(str_number);
                return ANALYSIS_FAILED;
            }
            // empty string means there was only number code, not the value
            // or exponent number in baseEexp format is missing
            if(dynStrIsEmpty(str_number)) {
                dynStrFree(str_number);
                return ANALYSIS_FAILED;
            }
            // only sign, but no exponent value
            else if(dynStrEqualString(str_number, "+")
                 || dynStrEqualString(str_number, "-")) {
                dynStrFree(str_number);
                return ANALYSIS_FAILED;
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
                    if(!exponent) { // number with decimal point
                        token->data.floatval = strtod(str_number->string,
                                                      NULL);
                    }
                    else { // base * 10 ^ exp
                        double exp = strtod(str_number->string, NULL);
                        token->data.floatval = base * pow(10, exp);
                    }
                    break;
                default:
                    token->type = T_UNKNOWN;
                    token->data.strval = NULL;
                    break;
            }
            // deallocate the memory
            dynStrFree(str_number);
            return SUCCESS;
        }

        // resets state variables:
        decimalPoint = false; // tels if last processed char was a decimal pint
        sig = false; // tels if next char can be sign

    } // while(TRUE)
} // process_number()


int process_keyword(FILE* file, token_t* token, int first_char) {
    // check file
    if (!file) {
        return EXECUTION_ERROR;
    }
    // check if token is initialized and empty
    if (!token || token->data.strval) {
        return EXECUTION_ERROR;
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
            }
            else {
                dynStrFree(tmp_string);
            }
            return SUCCESS;
        }
    }

    token->type = getKeywordType(tmp_string->string);

    if(token->type == T_ID) {
        token->data.strval = tmp_string;
    }
    else {
        dynStrFree(tmp_string);
    }
    eof_reached = true;
    return SUCCESS;
}


enum token_type getKeywordType(char *string) {
    for(int i = 0; i < 12; i++) { // 12 types
        if(strcmp(string, KEYWORDS[i]) == 0) {
	        return  i + T_KW_DEF;
        }
    }
    return T_ID; // none of these
}

// TODO
// - check if there can be unescaped quotation marks at the middle
// of the string
int process_string(FILE* file, token_t* token, int qmark) {
    if(!file) {
        return EXECUTION_ERROR;
    }
    // token must be initialized and empty
    if(!token || token->data.strval) {
        return EXECUTION_ERROR;
    }

    token->data.strval = dynStrInit();

    // is set to true if character is escaped
    bool esc = false;
    // if set to false after beginning of the string is processed
    bool beginning = true;

    // counter of quotation marks of the string / comment
    int qmark_beginning = 1;
    int qmark_end = 0;

    // stores currently processed char
    int tmp;

    // read to the end of string
    while((tmp = fgetc(file)) != EOF) {
        if(qmark_end == qmark_beginning) {
            ungetc(tmp, file);
            if((qmark_beginning == 1) && (qmark_end == 1)) {
                token->type = T_STRING;
            }
            else {
                token->type = T_STRING_ML;
            }
            return SUCCESS;
        }
        else if(esc) { // process escaped char

            char charCode[] = "\0\0\0";
            long intCharVal;

            switch (tmp) {
                case '\\':
                    dynStrAppendChar(token->data.strval, '\\');
                    break;
                case '\'':
                    dynStrAppendChar(token->data.strval, '\'');
                    break;
                case '\"':
                    dynStrAppendChar(token->data.strval, '\"');
                    break;
                case 'a' :
                    dynStrAppendChar(token->data.strval, '\a');
                    break;
                case 'b' :
                    dynStrAppendChar(token->data.strval, '\b');
                    break;
                case 'f' :
                    dynStrAppendChar(token->data.strval, '\f');
                    break;
                case 'n' :
                    dynStrAppendChar(token->data.strval, '\n');
                    break;
                case 'r' :
                    dynStrAppendChar(token->data.strval, '\r');
                    break;
                case 't' :
                    dynStrAppendChar(token->data.strval, '\t');
                    break;
                case 'v' :
                    dynStrAppendChar(token->data.strval, '\v');
                    break;
                case '3' : // \ooo ASCII character with octal value ooo
                case '2' :
                case '1' :
                case '0' :
                    for (int i = 0; i < 3; i++) {
                        if (is_oct(tmp)) {
                            charCode[i] = (char)tmp;
                            tmp = fgetc(file);
                        }
                        else { // bad octal value of character
                            dynStrFree(token->data.strval);
                            token->data.strval = NULL;
                            return ANALYSIS_FAILED;
                        }
                    }
                    intCharVal = strtol(charCode, NULL, 8);
                    dynStrAppendChar(token->data.strval, (char)intCharVal);
                    ungetc(tmp, file);
                    break;
                case 'x' : // \xhh... ASCII character with hex value hh...
                    charCode[0] = '0';
                    tmp = fgetc(file);
                    for (int i = 1; i < 3; i++) {
                        if (isxdigit(tmp)) {
                            charCode[i] = (char)tmp;
                            tmp = fgetc(file);
                        }
                        else { // bad hexa value of character
                            dynStrFree(token->data.strval);
                            token->data.strval = NULL;
                            return ANALYSIS_FAILED;
                        }
                    }
                    intCharVal = strtol(charCode, NULL, 16);
                    dynStrAppendChar(token->data.strval, (char)intCharVal);
                    ungetc(tmp, file);
                    break;
                default :
                    dynStrAppendChar(token->data.strval, '\\');
                    dynStrAppendChar(token->data.strval, (char)tmp);
                    break;
            }
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
                return SUCCESS;
            }
            else if(tmp == '\\') { // escaped char
                esc = true;
            }
            else if (tmp == '\n' && qmark_beginning == 1) {
                // fail if there is newline, and string is not multiline
                dynStrFree(token->data.strval);
                token->data.strval = NULL;
                return ANALYSIS_FAILED;
            }
            else {
                // add char to string data
                dynStrAppendChar(token->data.strval, (char) tmp);
            }
        }
    }// while()

    eof_reached = true;

    // string is complete (was completed in last iteration)
    if(qmark_end == qmark_beginning) {
        ungetc(tmp, file);
        if(qmark_beginning == 1) {
            token->type = T_STRING;
        }
        else {
            token->type = T_STRING_ML;
        }
        return SUCCESS;
    }
    else { // eof is in the middle of the string
        dynStrFree(token->data.strval);
        token->data.strval = NULL;
        return ANALYSIS_FAILED;
    }

}

// TODO
// test escaped eol! Eol in python can't be escaped!
int remove_line_comment(FILE* file){
    if(!file) {
        return EXECUTION_ERROR;
    }

    // stores currently processed char
    int tmp;

    while((tmp = fgetc(file)) != EOF) {
        if(tmp == '\n') {
            ungetc(tmp, file);
            return SUCCESS;
        }
    } // while ()
    eof_reached = true;
    return SUCCESS;
}
