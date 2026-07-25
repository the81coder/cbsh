#include "cbsh.h"

// Forward declarations
double executeSqr(double);
double executeRnd(double);
double executeSin(double);
double executeCos(double);
double executeTan(double);
double executeAtn(double);
double executeExp(double);
double executeLog(double);
double executeAbs(double);
double executeInt(double);

// Evaluate a function call (e.g., SQR(9) -> tokens are [KW_SQR, "(", num, ")"])
static double evaluateFunctionCall(Token *tokens, int numTokens) {
    if (numTokens < 4 || tokens[1].type != TOKEN_OPERATOR || tokens[1].value[0] != '(') {
        printf("Invalid function call\n");
        return 0;
    }

    int parenDepth = 1;
    int argStart = 2;
    int argCount = 0;
    for (int i = argStart; i < numTokens; i++) {
        if (tokens[i].type == TOKEN_OPERATOR) {
            if (tokens[i].value[0] == '(') parenDepth++;
            else if (tokens[i].value[0] == ')') {
                parenDepth--;
                if (parenDepth == 0) break;
            }
        }
        argCount++;
    }

    double arg;
    if (argCount == 0) {
        printf("Empty function argument\n");
        return 0;
    }
    arg = evaluateExpression(&tokens[argStart], argCount);

    switch (tokens[0].keyword) {
        case KW_SQR: return executeSqr(arg);
        case KW_RND: return executeRnd(arg);
        case KW_SIN: return executeSin(arg);
        case KW_COS: return executeCos(arg);
        case KW_TAN: return executeTan(arg);
        case KW_ATN: return executeAtn(arg);
        case KW_EXP: return executeExp(arg);
        case KW_LOG: return executeLog(arg);
        case KW_ABS: return executeAbs(arg);
        case KW_INT: return executeInt(arg);
        default:
            printf("Unknown function\n");
            return 0;
    }
}

// Evaluate a binary operation
static double applyOperator(double left, const char *op, double right) {
    if (strcmp(op, "+") == 0) return left + right;
    if (strcmp(op, "-") == 0) return left - right;
    if (strcmp(op, "*") == 0) return left * right;
    if (strcmp(op, "/") == 0) {
        if (right == 0) {
            printf("Division by zero\n");
            return 0;
        }
        return left / right;
    }
    if (strcmp(op, "=") == 0) return left == right;
    if (strcmp(op, "<") == 0) return left < right;
    if (strcmp(op, ">") == 0) return left > right;
    printf("Unknown operator: %s\n", op);
    return 0;
}

// Evaluate an expression (left-to-right, no operator precedence)
double evaluateExpression(Token *tokens, int numTokens) {
    if (numTokens == 0) return 0;

    // Function call: e.g., SQR(9) or SIN(X)
    if (numTokens >= 4 && tokens[0].type == TOKEN_KEYWORD &&
        tokens[1].type == TOKEN_OPERATOR && tokens[1].value[0] == '(') {
        return evaluateFunctionCall(tokens, numTokens);
    }

    // Unary minus: e.g., -5
    if (numTokens == 2 && tokens[0].type == TOKEN_OPERATOR &&
        strcmp(tokens[0].value, "-") == 0) {
        return -getNumericValue(&tokens[1]);
    }

    // General expression evaluator (left-to-right)
    double result;
    int i = 0;

    // Handle leading unary minus in a longer expression: -A + B
    if (tokens[0].type == TOKEN_OPERATOR && strcmp(tokens[0].value, "-") == 0) {
        result = -getNumericValue(&tokens[1]);
        i = 2;
    } else {
        result = getNumericValue(&tokens[0]);
        i = 1;
    }

    while (i + 1 < numTokens) {
        if (tokens[i].type == TOKEN_OPERATOR && strchr("+-*/=<>", tokens[i].value[0]) != NULL) {
            double right = getNumericValue(&tokens[i + 1]);
            result = applyOperator(result, tokens[i].value, right);
            i += 2;
        } else {
            break;
        }
    }

    return result;
}
