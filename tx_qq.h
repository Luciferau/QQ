#pragma once
#include <iostream>
#include <graphics.h>
using namespace std; 
#define QI_PAO_RIGHT_COLOR  RGB(18,183,245)
#define QI_PAO_LEFT_COLOR   RGB(229, 229, 229)
#define WINDOW_WIDTH	578//578
#define WINDOW_HEIGHT	825//825
#define MSG_WIN_WIDTH    428
#define LINE_HEIGHT 30
#define msgHeighMin   81
#define msgHeighMax   618
#define headHeight   51
 

struct Button {
	IMAGE imgNormal;
	IMAGE imgPress;
	int width, highth;
	int x, y;
	int flag; // 按钮的int类型标记
	bool pressed;
};
struct size {
	int width;
	int height;
};
typedef enum {
	LEFT_DIR,
	RIGHT_DIR
} direct_t;
typedef enum {
	RECEIVE,
	SEND
} msg_type;
typedef struct msg {
	char msg[1024];
	msg_type type;
	int height;
} msg_t;
DWORD char2LPCTSTR(const char ch[], wchar_t wth[]);
void initButton(Button* btn,
	const char* normalFile, const char* pressFile,
	int width, int highth, int flag);
bool checkButtonSelect(Button* btn, MOUSEMSG* msg);
void drawButton(Button* btn);
void drawPNG(int  picture_x, int picture_y, IMAGE* picture);
// 文本框输入
void textBox(int startX, int startY, int width, int height, int lineHeight,
COLORREF bgColor, COLORREF fontColor, char* msg, int* msgLen);
string wstring2string(wstring wstr);
bool isHZ(char str[], int index);//判断指定位置的字符是否是汉字
wstring GetIMEString();
void drawText(int x1, int y1, int width, int lineHeight, char* msg);
int calcQiPaoHeight(int width, int line_height, const char* msg);
struct size calQiPaoSize(const char* msg, int winWidth, int line_height);
void drawOneMsg(msg_t* msg, int* msg_height, IMAGE arrows[], IMAGE imgHeads[]);
void moveWindow(HWND hwnd, int offX, int offY);
void drawMsg(msg_t* msgAll, int currentMsgIndex, int* msgHeight, IMAGE arrows[], IMAGE imgHeads[]);
void GBKToUTF8(string& strGBK);
std::string UTF8ToGBK(const char* strUTF8);
bool TCPInit(SOCKET* sock, sockaddr_in* addr, const char* serverIP, int port);