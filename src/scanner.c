/*
 * Copyright (C) 2019 Pavel Raur <xraurp00@stud.fit.vutbr.cz>
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

token_t scan(FILE* file, intStack_t* stack) {
    // create output token and set it to no value
    token_t output_token;
    output_token.data.strval = NULL;
    output_token.type = T_ERROR;

    // return error if no file is no file is supplied or
    // stack is NULL or stack is not initialized (doesn't have 0 on top)
    if (!file || !stack || stackIsEmpty(stack)) {
        return output_token;
    }

    // first char of the new token
    int tmp = 0;
    // beginning of the line
    static bool line_beginning = true;
    // code offset in current indent
    int offset = 0;
    // number of processed offsets
    // init 0 is on stack
    int offsetCount = 1;
    // tels how many dedens have to be send if
    // return from multiple indents is processed
    static int returnDedent = 0;
    // return dedent if returning from multiple indentation
    if(returnDedent) {
        if(!stackPop(stack, &tmp)) {
            return output_token; // failed to pop
        }
        returnDedent--;
        output_token.type = T_DEDENT;
        return output_token;
    }

    // read char
    while ((tmp = fgetc(file)) != EOF) {
        // process code offset (number of spaces/tabs)
        // at the beginning of the line
        if (line_beginning) {

            // number of indent offsets on stack
            int referenceCount;
            if (!stackCount(stack, &referenceCount)) {
                return output_token;
            }

            // currently processed offset if already on stack
            int referenceOffset = 0;
            if (referenceCount >= offsetCount) {
                if (!stackGetIndent(stack, &referenceOffset, offsetCount)) {
                    return output_token;
                }
            }

        	switch (tmp) {
		        case '\n':
			        // remove empty line and continue with next one
			        offset = 0;
			        offsetCount = 1;
			        continue;
        		case ' ':
        		    // new block starting with space
        		    if (referenceCount >= offsetCount && referenceOffset == offset) {
                        offsetCount++;
                        offset = 1;
                    } else if (offset >= 0) { // part of this block
        		        offset++;
        		    } else { // error indentation
        		        output_token.type = T_UNKNOWN;
        		        return output_token;
        		    }
                    continue;
        		case '\t':
        		    // new block starting with tab
        		    if (referenceCount >= offsetCount && referenceOffset == offset) {
                        offsetCount++;
                        offset = -1;
                    } else if (offset <= 0) { // part of block
				        offset--;
			        } else { // error indentation
        		        output_token.type = T_UNKNOWN;
                        return output_token;
        		    }
                    continue;
        		case '#':
			        if (remove_line_comment(file)) {
				        return output_token; // failed to read from file
			        }
			        continue;
		        default:
			        // all chars of the offset were read
			        // put back last char to be processed later (its not offset char)
			        ungetc(tmp, file);
			        line_beginning = false;

			        // bad indent
			        if(referenceCount >= offsetCount) {
                        if (referenceOffset != offset) {
                            output_token.type = T_UNKNOWN;
                            return output_token;
                        }
                    }
			        // end of block
			        if(referenceCount > offsetCount) {
			            // number of dedents that have to be returned
			            returnDedent = referenceCount - offsetCount - 1; // -1 current token
			            int temp;
			            if(!stackPop(stack, &temp)) {
			                return output_token;
			            }
			            output_token.type = T_DEDENT;
			            return output_token;
			        }
			        // begining of block
                    else if(referenceCount < offsetCount) {
                        stackPush(stack, offset);
                        output_token.type = T_INDENT;
                        return output_token;
                    }
                    // still same block
                    else {
                        continue; // process content
                    }
        	}// switch
        }// if (line_beginning)

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
                } else if (return_status) {
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
                if ((tmp = fgetc(file)) == '=') {
                    output_token.type = T_OP_EQ;
                } else {
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
                if ((tmp = fgetc(file)) == '/') {
                    output_token.type = T_OP_IDIV;
                } else {
                    ungetc(tmp, file);
                }
                break;
            case '>' :
                output_token.type = T_OP_GREATER;
                if ((tmp = fgetc(file)) == '=') {
                    output_token.type = T_OP_GREATER_EQ;
                } else {
                    ungetc(tmp, file);
                }
                break;
            case '<' :
                output_token.type = T_OP_LESS;
                if ((tmp = fgetc(file)) == '=') {
                    output_token.type = T_OP_LESS_EQ;
                } else {
                    ungetc(tmp, file);
                }
                break;
            case '!' :
                if ((tmp = fgetc(file)) == '=') {
                    output_token.type = T_OP_NOT_EQ;
                } else {
					output_token.type = T_UNKNOWN;
                    ungetc(tmp, file);
                }
                break;
            case ',' :
                output_token.type = T_COMMA;
                break;
			case '\r':
				// Skip CR
				continue;
            case '\n':
                line_beginning = true;
                output_token.type = T_EOL;
                break;
            case ' ' :
            case '\t':
                continue; // skip whitespace
            default: // keyword
                if (isdigit(tmp)) {
	                return_status = process_number(file, &output_token, tmp);
	                if (return_status == ANALYSIS_FAILED) {
		                output_token.type = T_UNKNOWN;
	                } else if (return_status == EXECUTION_ERROR) {
		                output_token.type = T_ERROR;
	                }
	                break;
                } else if (isalpha(tmp) || tmp == '_') {
                    if (process_keyword(file, &output_token, tmp)) {
                        output_token.type = T_ERROR;
                    }
                } else { // unknown value
                    output_token.type = T_UNKNOWN;
                }
                break;
        }

        return output_token;
    } // while(...)

    // EOF reached
    // return all indentation from stack
    int tmpvar; // temporary stores offset
    if (!stackTop(stack, &tmpvar)) {
        return output_token;
    } else if (tmpvar) {
        if (!stackPop(stack, &tmpvar)) {
            return output_token;
        }
        output_token.type = T_DEDENT;
    } else {
        line_beginning = true;
        output_token.type = T_EOF;
    }
    return output_token;

}

bool is_oct(int num) {
    return ((num >= '0') && (num <= '7'));
}


bool is_bin(int num) {
    return ((num == '0') || (num == '1'));
}

int process_number(FILE* file, token_t* token, int first_number) {
    // check file and if no token is supplied and
    // token already have some value - exit
    if (!file || !token || token->data.strval) {
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
    int tmp = fgetc(file);

    // read next char, check for eof
    if (tmp == EOF) {
        token->data.intval = strtol(str_number->string,NULL, 10);
        dynStrFree(str_number);
        return SUCCESS;
    }

    // if first number is 0 value can be binary, octal or hexadecimal
    if (first_number == '0') {
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
            }
    }

    // read rest of the number
    while(true) {
        // if value was not set, set it to int
        if (type == N_UNDEF) {
            type = N_INT;
        } else {
	        tmp = fgetc(file);
        }

        // check if value is number of given type
        // hexadecimal
        if ((type == N_HEX  && isxdigit(tmp))
           // octal
        || (type == N_OCT && is_oct(tmp))
           // binary
        || (type == N_BIN  && is_bin(tmp))
           // int of float
        || ((type == N_INT || type == N_FLO) && isdigit(tmp)))
        {
            dynStrAppendChar(str_number, (char)tmp);
        } else if ((type == N_INT) && (tmp == '.')) { // float detection
            // set type float and continue
            // after decimal point
            type = N_FLO;
            dynStrAppendChar(str_number, (char)tmp);
            decimalPoint = true;
            continue;
        } else if ((type == N_INT || type == N_FLO) // float with exponent - base * 10 ^ exp
             && (tmp == 'e' || tmp == 'E')) {

            if (exponent) { // exponent is parsed allready
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
        } else if (sig && exponent && (tmp == '+' || tmp == '-')) { // process exponent sign
            dynStrAppendChar(str_number, (char)tmp);
        } else if (tmp == '_') { // underscore can be used for number separation, for better readability
            if (!sig) { // cannot be on first position in exponent
                continue; // skip the underscore and read the next number
            } else {
                dynStrFree(str_number);
                return ANALYSIS_FAILED;
            }
        } else if (isalnum(tmp)) { // not a number or not a number in given range (octal / binary)
            dynStrFree(str_number);
            return ANALYSIS_FAILED;
        } else {
            // put the char back
            if (!feof(file)) {
                ungetc(tmp, file);
            }
            // number after decimal point is missing
            if (decimalPoint) {
                dynStrFree(str_number);
                return ANALYSIS_FAILED;
            }
            // empty string means there was only number code, not the value
            // or exponent number in baseEexp format is missing
            if (dynStrIsEmpty(str_number)) {
                dynStrFree(str_number);
                return ANALYSIS_FAILED;
            } else if (dynStrEqualString(str_number, "+")
                 || dynStrEqualString(str_number, "-")) {
	            // only sign, but no exponent value
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
                    if (!exponent) { // number with decimal point
                        token->data.floatval = strtod(str_number->string,
                                                      NULL);
                    } else { // base * 10 ^ exp
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
}


int process_keyword(FILE* file, token_t* token, int first_char) {
    // check file and if token is initialized and empty
    if (!file || !token || token->data.strval) {
        return EXECUTION_ERROR;
    }

    dynStr_t* tmp_string = dynStrInit();
    // append first character
    dynStrAppendChar(tmp_string, (char) first_char);
    // temporary variable for currently processed char
    int tmp;

    while ((tmp = fgetc(file)) != EOF) {
        // char is part of keyword
        if (isalnum(tmp) || tmp == '_') {
            dynStrAppendChar(tmp_string, (char)tmp);
        } else {
            ungetc(tmp, file);
            token->type = getKeywordType(tmp_string->string);

            if (token->type == T_ID) {
                token->data.strval = tmp_string;
            } else {
                dynStrFree(tmp_string);
            }
            return SUCCESS;
        }
    }

    token->type = getKeywordType(tmp_string->string);

    if (token->type == T_ID) {
        token->data.strval = tmp_string;
    } else {
        dynStrFree(tmp_string);
    }
    return SUCCESS;
}


enum token_type getKeywordType(char *string) {
    for(int i = 0; i < 12; i++) { // 12 types
        if (strcmp(string, KEYWORDS[i]) == 0) {
	        return  i + T_KW_DEF;
        }
    }
    return T_ID; // none of these
}

// TODO
// - check if there can be unescaped quotation marks at the middle
// of the string
int process_string(FILE* file, token_t* token, int qmark) {
    // token must be initialized and empty
    if (!file || !token || token->data.strval) {
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
        if (qmark_end == qmark_beginning) {
            ungetc(tmp, file);
            if ((qmark_beginning == 1) && (qmark_end == 1)) {
                token->type = T_STRING;
            } else {
                token->type = T_STRING_ML;
            }
            return SUCCESS;
        } else if (esc) { // process escaped char
            if (process_escape_seq(file, token, tmp) == ANALYSIS_FAILED) {
	            return ANALYSIS_FAILED;
            }
            esc = false;
        } else if (tmp == qmark) { // is same as opening quotation mark
            if (beginning && qmark_beginning < 3) {
                qmark_beginning++;
            } else {
                beginning = false;
                qmark_end++;
            }
        } else { // char inside the string
            beginning = false;
            // unescaped quotation mark that doesn't close
            // the string / comment
            qmark_end = 0;

            // empty string
            if (qmark_beginning == 2) {
                // return char
                ungetc(tmp, file);
                // return empty string token
                token->type = T_STRING;
                return SUCCESS;
            } else if (tmp == '\\') { // escaped char
                esc = true;
            } else if (tmp == '\n' && qmark_beginning == 1) {
                // fail if there is newline, and string is not multiline
                dynStrFree(token->data.strval);
                token->data.strval = NULL;
                return ANALYSIS_FAILED;
            } else {
                // add char to string data
                dynStrAppendChar(token->data.strval, (char) tmp);
            }
        }
    }

    // string is complete (was completed in last iteration)
    if (qmark_end == qmark_beginning) {
        ungetc(tmp, file);
        if (qmark_beginning == 1) {
            token->type = T_STRING;
        } else {
            token->type = T_STRING_ML;
        }
        return SUCCESS;
    } else { // eof is in the middle of the string
        dynStrFree(token->data.strval);
        token->data.strval = NULL;
        return ANALYSIS_FAILED;
    }

}

int process_escape_seq(FILE* file, token_t *token, int c) {
	char charCode[4] = "";
	switch (c) {
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
				if (is_oct(c)) {
					charCode[i] = (char)c;
					c = fgetc(file);
				} else { // bad octal value of character
					dynStrFree(token->data.strval);
					token->data.strval = NULL;
					return ANALYSIS_FAILED;
				}
			}
			dynStrAppendChar(token->data.strval, (char) strtol(charCode, NULL, 8));
			ungetc(c, file);
			break;
		case 'x' : // \xhh... ASCII character with hex value hh...
			c = fgetc(file);
			for (int i = 0; i < 2; i++) {
				if (isxdigit(c)) {
					charCode[i] = (char)c;
					c = fgetc(file);
				} else { // bad hexadecimal value of character
					dynStrFree(token->data.strval);
					token->data.strval = NULL;
					return ANALYSIS_FAILED;
				}
			}
			dynStrAppendChar(token->data.strval, (char) strtol(charCode, NULL, 16));
			ungetc(c, file);
			break;
		default :
			dynStrAppendChar(token->data.strval, '\\');
			dynStrAppendChar(token->data.strval, (char)c);
			break;
	}
	return SUCCESS;
}

// TODO
// test escaped eol! Eol in python can't be escaped!
int remove_line_comment(FILE* file) {
    if (!file) {
        return EXECUTION_ERROR;
    }

    // stores currently processed char
    int tmp;

    while((tmp = fgetc(file)) != EOF) {
        if (tmp == '\n') {
            ungetc(tmp, file);
            return SUCCESS;
        }
    }
    return SUCCESS;
}

char* tokenToString (enum token_type type) {
	char *arr[] = {
		"EOL",
		"EOF",
		"'+'",
		"'-'",
		"'*'",
		"'/'",
		"'//'",
		"'=='",
		"'>'",
		"'<'",
		"'>='",
		"'<='",
		"'!='",
		"NUMBER",
		"FLOAT",
		"STRING",
		"STRING_MULTILINE",
		"IDENTIFIER",
		"'def'",
		"'if'",
		"'else'",
		"'while'",
		"'pass'",
		"'return'",
		"'None'",
		"'and'",
		"'or'",
		"'not'",
		"'True'",
		"'False'",
		"'='",
		"':'",
		"','",
		"'('",
		"')'",
		"INDENTATION",
		"DEDENTATION",
		"UNKNOWN",
		"ERROR"
	};
	if (type >= T_EOL && type <= T_ERROR) {
		return arr[type];
	} else {
		return "";
	}
}
