#ifndef FIG_TOKENIZER_H
#define FIG_TOKENIZER_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {IDENTIFIER=1, KEYWORD, OPERATOR, NUMBER, SEPARATOR} TokenType;

typedef struct {
	TokenType type;
	uint8_t token_info;
	char* value;
	size_t size;
} Token;


typedef struct {
	size_t index;
	char* source;
} TokenizerState;


typedef struct{
	Token* tokens;
	char* tokensValues;
} TokenizerOutput;

static const bool is_whitespace[256] = {
  [' '] = 1, ['\t'] = 1, ['\n'] = 1, ['\r'] = 1
};

bool can_be_in_number(char c){
	if(c >= '0' && c <= '9'){
		return true;
	}
	return false;
}

#define OPERATOR_LENGTH 11
const char* operator_characters = "><=-+*/&|!,";

#define OPERATOR_COUNT 18


#define GT_EQ     0  // >=
#define LT_EQ     1  // <=
#define NEQ       2  // !=
#define EQ        3  // ==
#define PLS_EQ    4  // +=
#define MIN_EQ    5  // -=
#define MUL_EQ    6  // *=
#define DIV_EQ    7  // /=
#define AND_OP    8  // &&
#define OR_OP     9  // ||
#define NOT_OP   10  // !
#define PLUS     11  // +
#define MINUS    12  // -
#define MULT     13  // *
#define DIV      14  // /
#define ASSIGN   15  // =
#define GT       16  // >
#define LT       17  // <

char* OPERATORS[OPERATOR_COUNT] = {">=", "<=", "!=", "==", "+=", "-=", "*=", "/=", "&&", "||", "!", "+", "-", "*", "/", "=", ">", "<"};


bool operator_left_to_right(size_t index) {
    switch (index) {
        // Right-to-left associativity
        case PLS_EQ:
        case MIN_EQ:
        case MUL_EQ:
        case DIV_EQ:
        case NOT_OP:
        case ASSIGN:
            return false;

        // Left-to-right associativity
        case GT_EQ:
        case LT_EQ:
        case NEQ:
        case EQ:
        case AND_OP:
        case OR_OP:
        case PLUS:
        case MINUS:
        case MULT:
        case DIV:
        case GT:
        case LT:
            return true;
    }
    return true; // Default to left-to-right
}

size_t operator_precedence(size_t index) {
	// https://en.cppreference.com/w/c/language/operator_precedence.html
    switch (index) {
        case GT_EQ:
        case LT_EQ:
        case GT:
        case LT:
            return 6;

        case NEQ:
        case EQ:
            return 7;

        case PLS_EQ:
        case MIN_EQ:
        case MUL_EQ:
        case DIV_EQ:
        case ASSIGN:
            return 14;

        case AND_OP:
            return 11;

        case OR_OP:
            return 12;

        case NOT_OP:
            return 1;

        case PLUS:
        case MINUS:
            return 4;

        case MULT:
            return 3;

        case DIV:
            return 2;
    }
    return 0; // Unknown or invalid index
}

#define SEPARATOR_LENGTH 8
const char* separator_characters = "()[]{};,";

bool can_be_in_separator(char c){
	for(size_t i = 0; i < SEPARATOR_LENGTH; i++){
		if(separator_characters[i] == c){
			return true;
		}
	}
	return false;
}


bool can_be_in_operator(char c){
	for(size_t i = 0; i < OPERATOR_LENGTH; i++){
		if(operator_characters[i] == c){
			return true;
		}
	}
	return false;
}



Token lex_keyword(TokenizerState* state, bool* isTokenValid){
	Token token = {0};

	(void) state;

	// FIXME: Not implemented not needed for calculator 
	*isTokenValid = false;

	return token;
}

Token lex_number(TokenizerState* state, bool* isTokenValid){
	Token token = {0};
	*isTokenValid = false;
	size_t index_end = state->index;

	while(state->source[index_end] != '\0' && can_be_in_number(state->source[index_end])){
		index_end++;
	}
	size_t substring_length = index_end - state->index;

	if(substring_length == 0){
		return token;
	}

	token.type = NUMBER;
	token.token_info = 0;
	token.value = state->source + state->index;
	token.size = substring_length;

	*isTokenValid = true;
	state->index += substring_length;

	return token;
}


Token lex_identifier(TokenizerState* state, bool* isTokenValid){
	Token token = {0};

	(void) state;

	// FIXME: Not implemented not needed for calculator 
	*isTokenValid = false;

	return token;
}


Token lex_operator(TokenizerState* state, bool* isTokenValid){
	Token token = {0};
	*isTokenValid = false;
	size_t index_end = state->index;
	while(state->source[index_end] != '\0' && can_be_in_operator(state->source[index_end])){
		index_end++;
	}
	size_t substring_length = index_end - state->index;

	if(substring_length == 0){
		return token;
	}

	token.type = OPERATOR;
	token.token_info = 0;
	token.value = state->source + state->index;
	token.size = substring_length;

	*isTokenValid = true;
	state->index += substring_length;

	for(size_t i = 0; i < OPERATOR_COUNT; i++){
	    size_t op_len = strlen(OPERATORS[i]);
	    if(op_len == token.size && strncmp(OPERATORS[i], token.value, token.size) == 0){
	        token.token_info = (uint8_t) i;
	        return token;
	    }
	}

	// token isnt any supported operator
	*isTokenValid = false;

	return token;
}

Token lex_separator(TokenizerState* state, bool* isTokenValid){
	Token token = {0};
	if(!can_be_in_separator(state->source[state->index])){
		*isTokenValid = false;
		return token;
	}

	token.type = SEPARATOR;
	token.token_info = (uint8_t) state->source[state->index];
	token.value = state->source + state->index;
	token.size = 1; // Separator can only be of length 1

	*isTokenValid = true;
	state->index++;

	return token;
}


typedef Token (*LexTokenFunction)(TokenizerState* state, bool* isTokenValid);


int tokenize(const char* source, TokenizerOutput* output){
	output->tokens = NULL;

	// copying the data so we dont need to manage the source the user does
	output->tokensValues = malloc(strlen(source)+1);
	if(!output->tokensValues){
		printf("Could not copy source!\n");
		return 1;
	}
	strcpy(output->tokensValues, source);



	TokenizerState state = {0};
	state.index = 0;
	state.source = output->tokensValues;

	LexTokenFunction lexing_functions[] = {
		lex_keyword,
		lex_number,
		lex_identifier,
		lex_operator,
		lex_separator
	};


	while(state.source[state.index] != '\0'){
		while (is_whitespace[(uint8_t)state.source[state.index]]) {
        	state.index++;
   		}
   		bool isTokenValid = false;
   		for(size_t i = 0; i < sizeof(lexing_functions) / sizeof(lexing_functions[0]); i++){
			Token tok = lexing_functions[i](&state, &isTokenValid);
			if(isTokenValid){
				arrput(output->tokens, tok);
				break;
			}
		}
		if(!isTokenValid){
			printf("WARN: Encountered unknown char: %c\n", state.source[state.index]);
			state.index++;
		}
	}
	return 0;
}



#endif