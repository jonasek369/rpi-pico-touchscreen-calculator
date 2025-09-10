#ifndef FIG_AST_H
#define FIG_AST_H 1

#include "tokenizer.h"

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

typedef struct ExprNode{
	Token op;
	struct ExprNode* operand1;
	struct ExprNode* operand2;
} ExprNode;


void free_ast(ExprNode* node){
    if (node == NULL) {
        return; // nothing to free
    }
    free_ast(node->operand1);
    free_ast(node->operand2);
    free(node);
}

void print_ast(ExprNode* node, int depth) {
    if (node == NULL) return;

    // Indent based on depth
    for (int i = 0; i < depth; i++) {
        printf("  ");
    }

    if (node->op.type != 0) {
        printf("%.*s\n", node->op.size, node->op.value);
    } else {
        printf("(null)\n");
    }

    // Recursively print operands
    print_ast(node->operand1, depth + 1);
    print_ast(node->operand2, depth + 1);
}


ExprNode* new_exprnode(Token token, ExprNode* op1, ExprNode* op2){
	ExprNode* node = malloc(sizeof(ExprNode));
	if(!node){
		fprintf(stderr, "Could not allocated memory for exprnode!\n");
	}
	node->op = token;
	node->operand1 = op1;
	node->operand2 = op2;
	return node;
}

void pop_push_operator(Token* opStack, ExprNode** exprStack){
	Token op = arrpop(opStack);
	ExprNode* right = arrpop(exprStack);
	ExprNode* left  = arrpop(exprStack);
	ExprNode* node = new_exprnode(op, left, right);
	arrput(exprStack, node);
}

ExprNode* generate_ast(Token* tokens){
	Token* opStack = NULL;
	ExprNode** exprStack = NULL;
	for(size_t i = 0; i < arrlenu(tokens); i++){
		Token token = tokens[i];
		if(token.type == NUMBER){
			ExprNode* node = new_exprnode(token, NULL, NULL);
			arrput(exprStack, node);
		}
		else if(token.type == IDENTIFIER){
			arrput(opStack, token);
		}
		else if(token.type == OPERATOR){
			while(
				arrlenu(opStack) > 0 && 
				!(opStack[arrlenu(opStack)-1].type == SEPARATOR && opStack[arrlenu(opStack)-1].token_info == '(') && // is left parenthesis
				(
					operator_precedence(opStack[arrlenu(opStack)-1].token_info) < operator_precedence(token.token_info) || (
						operator_precedence(opStack[arrlenu(opStack)-1].token_info) == operator_precedence(token.token_info) && 
						operator_left_to_right(token.token_info)
					)
				) // Using C precedence which is flipped (smaller higher precedence)
				){
				pop_push_operator(opStack, exprStack);
			}
			arrput(opStack, token);
		}else if(token.type == SEPARATOR && token.token_info == ','){
			while(!(opStack[arrlenu(opStack)-1].type == SEPARATOR && opStack[arrlenu(opStack)-1].token_info == '(')){
				pop_push_operator(opStack, exprStack);
			}
		}else if((token.type == SEPARATOR && token.token_info == '(')){
			arrput(opStack, token);
		}else if((token.type == SEPARATOR && token.token_info == ')')){
			while(arrlenu(opStack) > 0 && !(opStack[arrlenu(opStack)-1].type == SEPARATOR && opStack[arrlenu(opStack)-1].token_info == '(')){
				assert(arrlenu(opStack) != 0 && "Operator stack should not be empty here!");
				pop_push_operator(opStack, exprStack);
			}
			if(arrlenu(opStack)>0){
				assert((opStack[arrlenu(opStack)-1].type == SEPARATOR && opStack[arrlenu(opStack)-1].token_info == '(') && "Top operator should be left parenthesis");
			}
			

			if (arrlenu(opStack) == 0) {
			    fprintf(stderr, "Mismatched parentheses: No '(' found for ')'\n");
			    return NULL;
			}

			arrpop(opStack); // discard ')'

			if(arrlenu(opStack) > 0 && opStack[arrlenu(opStack)-1].type == IDENTIFIER){
				pop_push_operator(opStack, exprStack);
			}
		}
	}
	while(arrlenu(opStack) > 0){
		assert(!(opStack[arrlenu(opStack)-1].type == SEPARATOR && (opStack[arrlenu(opStack)-1].token_info == '(' || opStack[arrlenu(opStack)-1].token_info == ')')) && "Parenthesis mismatch!");
		pop_push_operator(opStack, exprStack);
	}
	ExprNode* root = NULL;
	if(arrlenu(exprStack) > 0){
		root = exprStack[0]; // Should be root now
	}

	arrfree(opStack);
	arrfree(exprStack);

	return root; // Show be root now
}

int str_to_int(char* str, size_t size) {
    int result = 0;
    bool negative = false;
    size_t i = 0;

    if (size == 0 || str == NULL) return 0; // handle empty string or NULL

    // Handle optional leading minus
    if (str[0] == '-') {
        negative = true;
        i = 1;
    }

    for (; i < size; i++) {
        if (str[i] < '0' || str[i] > '9') {
            // Invalid character found, stop conversion
            break;
        }
        result = result * 10 + (str[i] - '0');
    }

    return negative ? -result : result;
}

float compute_ast(ExprNode* node){
    if(!node) {
        fprintf(stderr, "Invalid node\n");
        return 0;
    }

    switch(node->op.type){
        case NUMBER:
            return (float) str_to_int(node->op.value, node->op.size);

        case OPERATOR: {
            float left = compute_ast(node->operand1);
            float right = compute_ast(node->operand2);

            switch(node->op.token_info) {
                case PLUS:  return left + right;
                case MINUS: return left - right;
                case MULT:  return left * right;
                case DIV:   return left / right;
                default:
                    fprintf(stderr, "Unknown operator: %d\n", node->op.token_info);
                    return 0;
            }
        }

        default:
            fprintf(stderr, "Unknown node type: %d\n", node->op.type);
            return 0;
    }
}
#endif