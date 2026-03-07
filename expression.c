#include "cbsh.h"

// Math function implementations
double evalSqr(double value) {
    if (value < 0) {
        printf("Error: SQR of negative number\n");
        return 0;
    }
    return sqrt(value);
}

double evalRnd(double value) {
    // RND(0) returns a random number between 0 and 1
    // RND(n) where n>0 returns random number between 0 and n-1 (integer)
    // RND(n) where n<0 reseeds the generator
    static bool seeded = false;
    
    if (!seeded) {
        srand(time(NULL));
        seeded = true;
    }
    
    if (value < 0) {
        srand((unsigned int)(-value));
        return 0;
    } else if (value == 0) {
        return (double)rand() / RAND_MAX;
    } else {
        return (double)(rand() % (int)value);
    }
}

double evalSin(double value) {
    return sin(value);
}

double evalAbs(double value) {
    return fabs(value);
}

double evalInt(double value) {
    return floor(value);
}

int evalUsr(double address) {
    // USR is a machine language call - not implemented in this interpreter
    // In real BASIC, this would call a machine language subroutine at address
    printf("USR function not implemented (would call address %g)\n", address);
    return 0;
}

// Evaluate an expression with support for functions and operators
double evaluateExpression(Token *tokens, int numTokens) {
    if (numTokens == 0) {
        return 0;
    }
    
    // Handle single value
    if (numTokens == 1) {
        return getNumericValue(&tokens[0]);
    }
    
    // Handle function calls: FUNC(arg)
    if (numTokens >= 4 && tokens[0].type == TOKEN_KEYWORD && 
        tokens[1].type == TOKEN_OPERATOR && strcmp(tokens[1].value, "(") == 0) {
        
        // Find matching closing parenthesis
        int parenDepth = 1;
        int closeParenIndex = -1;
        for (int i = 2; i < numTokens; i++) {
            if (tokens[i].type == TOKEN_OPERATOR) {
                if (strcmp(tokens[i].value, "(") == 0) {
                    parenDepth++;
                } else if (strcmp(tokens[i].value, ")") == 0) {
                    parenDepth--;
                    if (parenDepth == 0) {
                        closeParenIndex = i;
                        break;
                    }
                }
            }
        }
        
        if (closeParenIndex > 2) {
            double arg = evaluateExpression(&tokens[2], closeParenIndex - 2);
            
            switch (tokens[0].keyword) {
                case KW_SQR:
                    return evalSqr(arg);
                case KW_RND:
                    return evalRnd(arg);
                case KW_SIN:
                    return evalSin(arg);
                case KW_ABS:
                    return evalAbs(arg);
                case KW_INT:
                    return evalInt(arg);
                case KW_USR:
                    return evalUsr(arg);
                default:
                    printf("Unknown function\n");
                    return 0;
            }
        }
    }
    
    // Handle parenthesized expressions: (expr)
    if (numTokens >= 3 && tokens[0].type == TOKEN_OPERATOR && strcmp(tokens[0].value, "(") == 0) {
        int parenDepth = 1;
        int closeParenIndex = -1;
        for (int i = 1; i < numTokens; i++) {
            if (tokens[i].type == TOKEN_OPERATOR) {
                if (strcmp(tokens[i].value, "(") == 0) {
                    parenDepth++;
                } else if (strcmp(tokens[i].value, ")") == 0) {
                    parenDepth--;
                    if (parenDepth == 0) {
                        closeParenIndex = i;
                        break;
                    }
                }
            }
        }
        
        if (closeParenIndex == numTokens - 1) {
            // Entire expression is in parentheses
            return evaluateExpression(&tokens[1], closeParenIndex - 1);
        }
    }
    
    // Handle binary operations with proper precedence
    // First pass: Handle + and - (lowest precedence)
    for (int i = numTokens - 1; i >= 0; i--) {
        if (tokens[i].type == TOKEN_OPERATOR) {
            if (strcmp(tokens[i].value, "+") == 0) {
                double left = evaluateExpression(tokens, i);
                double right = evaluateExpression(&tokens[i + 1], numTokens - i - 1);
                return left + right;
            } else if (strcmp(tokens[i].value, "-") == 0 && i > 0) {
                double left = evaluateExpression(tokens, i);
                double right = evaluateExpression(&tokens[i + 1], numTokens - i - 1);
                return left - right;
            }
        }
    }
    
    // Second pass: Handle * and / (higher precedence)
    for (int i = numTokens - 1; i >= 0; i--) {
        if (tokens[i].type == TOKEN_OPERATOR) {
            if (strcmp(tokens[i].value, "*") == 0) {
                double left = evaluateExpression(tokens, i);
                double right = evaluateExpression(&tokens[i + 1], numTokens - i - 1);
                return left * right;
            } else if (strcmp(tokens[i].value, "/") == 0) {
                double left = evaluateExpression(tokens, i);
                double right = evaluateExpression(&tokens[i + 1], numTokens - i - 1);
                if (right == 0) {
                    printf("Division by zero\n");
                    return 0;
                }
                return left / right;
            }
        }
    }
    
    // Handle comparison operators: =, <, >, <=, >=, <>
    for (int i = 0; i < numTokens; i++) {
        if (tokens[i].type == TOKEN_OPERATOR) {
            double left = evaluateExpression(tokens, i);
            double right = evaluateExpression(&tokens[i + 1], numTokens - i - 1);
            
            if (strcmp(tokens[i].value, "=") == 0) {
                return (left == right) ? 1 : 0;
            } else if (strcmp(tokens[i].value, "<") == 0) {
                if (i + 1 < numTokens && tokens[i + 1].type == TOKEN_OPERATOR && 
                    strcmp(tokens[i + 1].value, ">") == 0) {
                    // <> (not equal)
                    right = evaluateExpression(&tokens[i + 2], numTokens - i - 2);
                    return (left != right) ? 1 : 0;
                } else if (i + 1 < numTokens && tokens[i + 1].type == TOKEN_OPERATOR && 
                    strcmp(tokens[i + 1].value, "=") == 0) {
                    // <= (less than or equal)
                    right = evaluateExpression(&tokens[i + 2], numTokens - i - 2);
                    return (left <= right) ? 1 : 0;
                } else {
                    return (left < right) ? 1 : 0;
                }
            } else if (strcmp(tokens[i].value, ">") == 0) {
                if (i + 1 < numTokens && tokens[i + 1].type == TOKEN_OPERATOR && 
                    strcmp(tokens[i + 1].value, "=") == 0) {
                    // >= (greater than or equal)
                    right = evaluateExpression(&tokens[i + 2], numTokens - i - 2);
                    return (left >= right) ? 1 : 0;
                } else {
                    return (left > right) ? 1 : 0;
                }
            }
        }
    }
    
    // Handle simple 3-token expression: value op value
    if (numTokens == 3) {
        double val1 = getNumericValue(&tokens[0]);
        double val2 = getNumericValue(&tokens[2]);
        
        if (strcmp(tokens[1].value, "+") == 0) return val1 + val2;
        if (strcmp(tokens[1].value, "-") == 0) return val1 - val2;
        if (strcmp(tokens[1].value, "*") == 0) return val1 * val2;
        if (strcmp(tokens[1].value, "/") == 0) {
            if (val2 == 0) {
                printf("Division by zero\n");
                return 0;
            }
            return val1 / val2;
        }
    }
    
    printf("Invalid expression\n");
    return 0;
}
