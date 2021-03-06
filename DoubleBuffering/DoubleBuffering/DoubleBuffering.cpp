#include <stdio.h>
#include <stdlib.h>
#include <Windows.h>
#include <string.h>

/*
더블 버퍼링 예제 by Jaeguins
비 ASCII 문자를 대비하여 문자도 전부 4byte int에 저장합니다.
ASCII문자는 대부분 반각이니 띄워쓰기를 뒤에 붙여 칸 크기를 맞췄습니다.

더블 버퍼링 ->버퍼를 두개를 두고 화면에 출력하기(시스템 콘솔 출력,Frame배열)

Double Buffering example by Jaeguins
All characters are stored by int because of non-ASCII characters.
all ASCII characters contains space after it because of character size.
*/
#define WINDOW_WIDTH 40 //창 가로길이
#define WINDOW_HEIGHT 20 //창 세로길이
#define FPS 20 //초당 화면갱신량
#define MAX_COMPONENT 10 //최대 요소갯수


typedef struct component {
    int width,  //너비
        height, //높이
        top,    //y위치(위쪽 기준)
        left;   //x위치(왼쪽 기준)
    int* data;
}Component;//화면에 표시하는 요소

Component* InitComponent(int top, int left, int width, int height);//Component 초기화
void FreeComponent(Component** target);//Component 메모리해제
int GetData(Component* target, int x, int y);//특정 위치 값 부르기
void InputData(Component* target, int x, int y, int value);//해당 위치에 값 집어넣기
void InsertComponentToFrame(Component* target);//요소를 다음 그릴 화면에 넣기

const int NaN = '　';//요소가 없는 부분
const int frameInterval = 1000 / FPS;//프레임당 대기시간
const int frameSize = WINDOW_WIDTH * WINDOW_HEIGHT;//한 프레임의 크기
int Frame[frameSize];//프레임
char* buffer;//출력할때 쓸 버퍼
Component* components[MAX_COMPONENT];//요소들 (인덱스 번호가 높을수록 겹쳤을때 위에 보임)

int* ConvertToInt(const char* input,int* length);//문자열을 여기서 쓰는 자료구조형으로 변환
int CheckASCII(int in);//ASCII문자인지 확인하는 함수(1이면 아님)
void GoToXY(int x, int y);//해당 좌표로 커서 이동
void InputDataToFrame(int x, int y, int value);//프레임에 글자 넣기
void CleanFrame();//프레임 청소
void PrintBuffer();//프레임 출력

int main() {
    for (int i = 0; i < MAX_COMPONENT; i++) {
        components[i] = NULL;
    }
    //프레임들 초기화

    //예제들

    components[0] = InitComponent(5, 5, 3, 3);
    int val = '■';
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 3; j++) {
            if (i == 1 || j == 1)InputData(components[0], i, j, val);
        }
    }
    //■로 그린 +모양(비 ASCII문자 애니메이션)

    components[1] = InitComponent(10, 10, 5, 5);
    val = 'X';
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (i == j || i == 4-j)InputData(components[1], i, j, val);
        }
    }
    //X로 그린 x모양(ASCII문자 애니메이션)

    int helloLength = 0;
    int* hello=ConvertToInt("text예제 ?!ㄱㄴㄷㄻ wow",&helloLength);
    components[3] = InitComponent(2, 8, helloLength, 1);
    for (int i = 0; i < helloLength; i++) {
        InputData(components[3], i, 0, hello[i]);
    }
    //문자 고정

    components[2] = InitComponent(10, 10, 5, 5);
    val = '곰';
    for (int i = 0; i < 5; i++) {
        for (int j = 0; j < 5; j++) {
            if (i == 2 || j == 2)InputData(components[2], i, j, val);
        }
    }
    //'곰'으로 그린 +모양(애니메이션 겹침)

    components[4] = InitComponent(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
    for (int i = 0; i < WINDOW_WIDTH; i++) {
        for (int j = 0; j < WINDOW_HEIGHT; j++) {
            int yBorder = i == 0 || i == WINDOW_WIDTH - 1;
            int xBorder = j == 0 || j == WINDOW_HEIGHT - 1;
            if (xBorder) {
                InputData(components[4], i, j, 'ㅡ');
            }
            if (yBorder) {
                InputData(components[4], i, j, '│');
            }
            if (xBorder&&yBorder) {
                InputData(components[4], i, j, '┼');
            }
        }
    }
    //테두리(비 ASCII문자 고정)

    /*
    여타 시작 전에 해야할 부분
    */

    while (1) {//그리는 부분
        components[0]->top = components[0]->top >= 20 ? -5 : components[0]->top + 1;
        components[1]->left = components[1]->left >= 40 ? -5 : components[1]->left + 1;
        components[2]->left = components[2]->left <= -5 ? 40 : components[2]->left - 1;
        //+,x,곰 모양 애니메이션
        /*
        여타 매 프레임마다 해야할 부분
        */
        CleanFrame();//이전그림 지우기(앞 순회에서 이미 그렸음)
        PrintBuffer();//그려서 출력하기
        Sleep(frameInterval);//다음 프레임으로
    }
}

int CheckASCII(int in) {
    return ((in & 0x8000) == 0x8000) ? 1 : 0;
}

void GoToXY(int x, int y) {
    COORD pos = { x,y };
    SetConsoleCursorPosition(GetStdHandle(STD_OUTPUT_HANDLE), pos);
}

Component* InitComponent(int top, int left, int width, int height) {
    Component* ret = (Component*)malloc(sizeof(Component));
    ret->left = left;
    ret->top = top;
    ret->width = width;
    ret->height = height;
    ret->data = (int*)malloc(sizeof(int)*width*height);
    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            InputData(ret, i, j, NaN);
        }
    }
    return ret;
}

void FreeComponent(Component** target) {
    Component* disp = *target;
    *target = NULL;
    free(disp->data);
    free(disp);
}

int GetData(Component* target, int x, int y) {
    return target->data[target->width  * y + x];
}

void InputData(Component* target, int x, int y, int value) {
    if (x < 0 || y < 0 || x >= target->width || y >= target->height)return;
    target->data[target->width  * y + x] = value;
}

int* ConvertToInt(const char* input,int *length) {
    int* ret = (int*)malloc(sizeof(int)*strlen(input));
    int c = 0;
    for (int i = 0; input[i]!='\0'; i++) {
        if ((input[i]&0x80)==0x80) {
            ret[c++] = input[i] * 256 + input[1 + i++];
        }
        else {
            ret[c++] = input[i];
        }
    }
    *length = c;
    return ret;
}

void InsertComponentToFrame(Component* target) {
    int*frame = Frame;
    for (int i = 0; i < target->width; i++)
    {
        for (int j = 0; j < target->height; j++) {
            InputDataToFrame(target->left + i, target->top + j, GetData(target, i, j));
        }
    }
}

void InputDataToFrame(int x, int y, int value) {
    if (x < 0 || x >= WINDOW_WIDTH || y < 0 || y >= WINDOW_HEIGHT) return;
    if (value == NaN)return;
    int index = x + y * WINDOW_WIDTH;
    int* frame = Frame;
    frame[index] = value;
}

void CleanFrame() {
    for (int i = 0; i < frameSize; i++) {
        Frame[i] = NaN;
    }
}

void PrintBuffer() {
    for (int i = 0; i < MAX_COMPONENT; i++) {
        if (components[i] != NULL) {
            InsertComponentToFrame(components[i]);
        }
    }
    buffer = (char*)malloc(sizeof(int)*frameSize);
    int count = 0;
    for (int i = 0; i < frameSize; i++) {
        if (CheckASCII(Frame[i])) {
            buffer[count++] = Frame[i]/256;
            buffer[count++] = Frame[i]%256;
        }
        else {
            buffer[count++] = Frame[i];
            buffer[count++] = ' ';
        }
            
        if ((i % (WINDOW_WIDTH)==WINDOW_WIDTH-1))buffer[count++] = '\n';
    }
    buffer[count] = '\0';
    GoToXY(0, 0);
    printf("%s", buffer);
    free(buffer);
}
