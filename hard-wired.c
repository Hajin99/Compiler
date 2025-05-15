#include <stdio.h>
#include <string.h>
#include <stdlib.h> // malloc, free 함수를 사용하기 위해 추가

// Define States
typedef enum {
    S_START, S_IN_ID, S_ACC_ID,
    S_IN_DECIMAL, S_ACC_DECIMAL,
    S_ZERO, S_IN_OCTAL, S_ACC_OCTAL,
    S_BEFORE_HEX, S_IN_HEX, S_ACC_HEX,
    // Special Symbols 
    S_PLUS, S_MINUS, S_MULTIPLY, S_DIVIDE, S_MODULO,
    S_IN_ASSIGN, S_ACC_ASSIGN, // 할당
    S_EQUAL, S_NOT_EQUAL,
    S_LESS, S_GREATER, S_LESS_EQUAL, S_GREATER_EQUAL,
    S_AND, S_OR, S_NOT,
    S_SEMICOLON,
    S_LPAREN, S_RPAREN, 
    S_LBRACE, S_RBRACE,
    S_LBRACKET, S_RBRACKET,
    S_COMMA, S_DOT,
    S_ERROR
} State;

// 파일 읽기 함수
char *read_file(const char *filename) {
    FILE *file;
    char *buffer;
    long file_size;
    int i;

    // 파일 열기(읽기)
    file = fopen(filename, "r");
    if(file == NULL) {
        printf("파일을 열 수 없습니다.");
        return NULL;
    }

    // 파일 크기 확인
    // 파일 끝으로 이동해서 0바이트 이동(끝에 그대로 위치)
    fseek(file, 0 , SEEK_END);
    // 파일의 처음부터 현재 위치까지의 바이트(파일의 현재 위치를 바이트로 반환, 파일이 유효하지 않다면 -1L을 반환)
    file_size = ftell(file);
    rewind(file); // 파일 포인터를 파일의 시작 위치로

    // 파일 내용 저장할 배열 동적 할당
    buffer = (char *)malloc(sizeof(char) * (file_size + 1));
    if (buffer == NULL) {
        printf("메모리 할당 실패\n");
        fclose(file);
        return NULL;
    }

    // 파일 내용 배열에 저장
    fread(buffer, sizeof(char), file_size, file);
    buffer[file_size] = '\0'; // 버퍼 끝에 종료 문자 추가

    // 파일 닫기
    fclose(file);
    return buffer;
}

// 파일 쓰기(읽기) 함수
void write_file(const char *filename, const char *content) {
    FILE *file = fopen(filename, "w");
    if(file == NULL){
        printf("%s 파일을 열 수 없습니다.\n", filename);
        return;
    }

    fprintf(file, "%s", content);
    fclose(file);
    printf("파일에 기록 완료.");
}

// 상태 처리 함수
void process_content(const char *content, char *buff) {
    State state = S_START;
    int i;

    for (i = 0; i < strlen(content); i++) {
        char ch = content[i];
        buff[i] = ch;

        switch (state) {
            case S_START:
                if ((ch >= 'a' && ch <= 'z') || (ch >= 'A' && ch <= 'Z'))
                    state = S_IN_ID;
                else if (ch >= '1' && ch <= '9')
                    state = S_IN_DECIMAL;
                else if (ch == '0')
                    state = S_ZERO;
                else if (ch == '+')
                    state = S_PLUS;
                else if (ch == '-')
                    state = S_MINUS;
                else if (ch == '*')
                    state = S_MULTIPLY;
                else if (ch == '/')
                    state = S_DIVIDE;
                else if (ch == ' ' || ch == '\n' || ch == '\t')
                    state = S_START; // 무시 (공백)
                else
                    state = S_ERROR;
                break;

            case S_ZERO:
                if (ch >= '0' && ch <= '7')
                    state = S_ZERO; // 8진수 (0으로 시작)
                else if (ch == 'x' || ch == 'X')
                    state = S_BEFORE_HEX; // 16진수 되기 전
                else if (ch == ' ')
                    state = S_START;
                else
                    state = S_ERROR;
                break;

            case S_IN_ID:
                if ((ch >= 'a' && ch <= 'z') || 
                    (ch >= 'A' && ch <= 'Z') || 
                    (ch >= '0' && ch <= '9'))
                    state = S_IN_ID;
                else if(ch == ' ')
                    state = S_ACC_ID;
                else {
                    state = S_ACC_ID;
                    i--; // 문자 다시 처리
                }
                break;

            case S_IN_DECIMAL:
                if (ch >= '0' && ch <= '9')
                    state = S_IN_DECIMAL;
                else {
                    state = S_ACC_DECIMAL;
                    i--; // 숫자 이후 문자 다시 처리
                }
                break;

            default:
                state = S_ERROR;
                break;
            
        }

        if (state == S_ERROR) {
            printf("오류: 인식할 수 없는 문자 '%c'\n", ch);
            break;
        }
    }
}

int main() {
    char *filename = "input.txt";
    char *output_file = "output.txt";
    char *content;
    char *buff;
    
    // 현재 상태
    State state = S_START;

    content = read_file(filename);
    if (content == NULL) {
        printf("파일 읽기 오류\n");
        return 1;
    }


    // 파일 쓰기
    write_file(output_file, content);

    // 동적 할당 해제
    free(content);

    return 0;
}

// 버퍼에 이때까지 문자를 저장해 놓고 어디로 옮겨서,
// 1 | int, 2| main 이런식으로 토큰 단위로 구분짓기
// 심볼 테이블, 리터럴 테이블 만들기