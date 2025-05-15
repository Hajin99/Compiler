#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h> // malloc, free 함수를 사용하기 위해 추가

// Define States
typedef enum {
    S_START, S_IN_ID, S_ACC_ID,
    S_IN_DECIMAL, S_ACC_DECIMAL,
    S_ZERO, S_IN_OCTAL, S_ACC_OCTAL,
    S_BEFORE_HEX, S_IN_HEX, S_ACC_HEX,
    S_SPECIAL, S_ERROR
} State;

// State Transition Table
State transition_table[40][128];

// Symbol Table 구조체 정의
typedef struct {
    char identifier[50];
    char type[10];
} Symbol;

// Symbol Table
Symbol symbol_table[100];
int symbol_count = 0;

// Literal Table
char literal_table[100][50];
char literal_types[100][10];
int literal_count = 0;

// Special Symbols 배열 선언
char special_chars[] = "+-*/=<>();,{}[]";

// Initialize Transition Table
void initialize_transition_table() {
    for (int i = 0; i < 40; i++)
        for (int j = 0; j < 128; j++)
            transition_table[i][j] = S_ERROR;

    // Identifier (S_START -> S_IN_ID)
    for (int i = 'a'; i <= 'z'; i++)
        transition_table[S_START][i] = S_IN_ID;
    for (int i = 'A'; i <= 'Z'; i++)
        transition_table[S_START][i] = S_IN_ID;
    transition_table[S_START]['_'] = S_IN_ID;

    // In S_IN_ID, any letter, digit, or underscore continues the identifier
    for (int i = 'a'; i <= 'z'; i++)
        transition_table[S_IN_ID][i] = S_IN_ID;
    for (int i = 'A'; i <= 'Z'; i++)
        transition_table[S_IN_ID][i] = S_IN_ID;
    for (int i = '0'; i <= '9'; i++)
        transition_table[S_IN_ID][i] = S_IN_ID;
    transition_table[S_IN_ID]['_'] = S_IN_ID;

    // Decimal Number (S_START -> S_IN_DECIMAL)
    for (int i = '0'; i <= '9'; i++)
        transition_table[S_START][i] = S_IN_DECIMAL;
    for (int i = '0'; i <= '9'; i++)
        transition_table[S_IN_DECIMAL][i] = S_IN_DECIMAL;
    // S_IN_DECIMAL에서 숫자가 아닌 문자를 만나면 오류 상태로 전이 (영문자 추가)
    for (int i = 0; i < 128; i++) {
        if (!isdigit(i) && isalpha(i)) {
            transition_table[S_IN_DECIMAL][i] = S_ERROR;
        } else if (!isdigit(i) && !isspace(i) && strchr(special_chars, i) == NULL) {
            transition_table[S_IN_DECIMAL][i] = S_ERROR;
        }
    }
    // S_START에서 숫자가 아닌 문자를 만나면 오류 상태로 전이 (특수 문자 제외)
    for (int i = 0; i < 128; i++) {
        if (!isalnum(i) && !isspace(i) && strchr(special_chars, i) == NULL) {
            transition_table[S_START][i] = S_ERROR;
        }
    }

    // Special Symbols (Handled directly in S_START)
    for (int i = 0; special_chars[i] != '\0'; i++) {
        transition_table[S_START][(unsigned char)special_chars[i]] = S_SPECIAL;
    }

    // Whitespace Handling (공백 무시)
    for (int i = 0; i < 40; i++) {
        transition_table[i][' '] = S_START;
        transition_table[i]['\t'] = S_START;
        transition_table[i]['\n'] = S_START;
    }
}

// Add to Symbol Table
void add_symbol(const char *identifier, const char *type, int is_error) {
    if (!is_error) {
        strcpy(symbol_table[symbol_count].identifier, identifier);
        strcpy(symbol_table[symbol_count].type, type);
        symbol_count++;
    }
}

// Add to Literal Table with Type
void add_literal(const char *literal, const char *type) {
    strcpy(literal_table[literal_count], literal);
    strcpy(literal_types[literal_count], type);
    literal_count++;
}

// Main Lexical Analyzer Function
void lexical_analyze(const char *code) {
    State current_state = S_START;
    const char *p = code;
    char token[100];
    int token_index = 0;
    int token_number = 1;
    int error_occurred = 0; // 오류 발생 플래그

    printf("Token Number | Token Value\n");

    while (*p) {
        if (isspace(*p)) { p++; continue; }

        // Transition to next state
        State next_state = transition_table[current_state][(unsigned char)*p];

        if (next_state == S_ERROR) {
            printf("Error: Invalid character '%c' at position %ld\n", *p, (long)(p - code));
            error_occurred = 1;
            current_state = S_START;
            p++;
            token_index = 0;
            continue;
        }

        // Handle Token
        if (next_state == S_SPECIAL) {
            if (token_index > 0) {
                token[token_index] = '\0';
                printf("%d | %s\n", token_number++, token);
                add_symbol(token, "unknown", error_occurred);
                token_index = 0;
            }
            printf("%d | %c\n", token_number++, *p);
            current_state = S_START;
        } else {
            token[token_index++] = *p;
            current_state = next_state;
        }

        // Check for end of token and potential errors immediately after digits
        if (current_state == S_IN_DECIMAL) {
            if (!isdigit(*(p + 1)) && !isspace(*(p + 1)) && *(p + 1) != '\0' && strchr(special_chars, *(p + 1)) == NULL) {
                printf("Error: Invalid character '%c' after numeric literal '%s' at position %ld\n", *(p + 1), token, (long)(p - code) + 1);
                error_occurred = 1;
                current_state = S_START;
                p++;
                token_index = 0;
                continue;
            } else if (!isdigit(*(p + 1))) {
                token[token_index] = '\0';
                printf("%d | %s\n", token_number++, token);
                add_literal(token, "int");
                token_index = 0;
                current_state = S_START;
            }
        } else if (current_state == S_IN_ID && !isalnum(*(p + 1)) && *(p + 1) != '_') {
            token[token_index] = '\0';
            printf("%d | %s\n", token_number++, token);
            add_symbol(token, "unknown", error_occurred);
            token_index = 0;
            current_state = S_START;
        } else if (current_state != S_START && isspace(*(p + 1))) {
            token[token_index] = '\0';
            printf("%d | %s\n", token_number++, token);
            if (current_state == S_IN_DECIMAL) {
                add_literal(token, "int");
            } else if (current_state == S_IN_ID) {
                add_symbol(token, "unknown", error_occurred);
            }
            token_index = 0;
            current_state = S_START;
        }

        p++;
    }

    // Flush last token
    if (token_index > 0) {
        token[token_index] = '\0';
        printf("%d | %s\n", token_number++, token);
        add_symbol(token, "unknown", error_occurred);
    }

    // Print Symbol Table
    printf("\nSymbol Table\nIdentifier | Type\n");
    for (int i = 0; i < symbol_count; i++) {
        printf("%s | %s\n", symbol_table[i].identifier, symbol_table[i].type);
    }

    // Print Literal Table
    printf("\nLiteral Table\nLiteral | Type\n");
    for (int i = 0; i < literal_count; i++) {
        printf("%s | %s\n", literal_table[i], literal_types[i]);
    }
}

// Test Program
int main() {
    initialize_transition_table();

    const char *sample_code = "int main() { int a1 = 10; int b =5; }";
    lexical_analyze(sample_code);

    return 0;
}