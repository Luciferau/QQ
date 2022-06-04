#include "tx_qq.h"
#include <conio.h>
#include <iostream>
using namespace std;

#pragma comment(lib,"Imm32.lib")


DWORD char2LPCTSTR(const char ch[], wchar_t wth[])
{
	int i;
	for (i = 0; ch[i]; i++)
		wth[i] = ch[i];
	wth[i] = '\0';
	return i;
}

// 载入PNG图并去透明部分
void drawPNG(int  picture_x, int picture_y, IMAGE* picture) //x为载入图片的X坐标，y为Y坐标
{

	// 变量初始化
	DWORD* dst = GetImageBuffer();    // GetImageBuffer()函数，用于获取绘图设备的显存指针，EASYX自带
	DWORD* draw = GetImageBuffer();
	DWORD* src = GetImageBuffer(picture); //获取picture的显存指针
	int picture_width = picture->getwidth(); //获取picture的宽度，EASYX自带
	int picture_height = picture->getheight(); //获取picture的高度，EASYX自带
	int graphWidth = getwidth();       //获取绘图区的宽度，EASYX自带
	int graphHeight = getheight();     //获取绘图区的高度，EASYX自带
	int dstX = 0;    //在显存里像素的角标

	// 实现透明贴图 公式： Cp=αp*FP+(1-αp)*BP ， 贝叶斯定理来进行点颜色的概率计算
	for (int iy = 0; iy < picture_height; iy++)
	{
		for (int ix = 0; ix < picture_width; ix++)
		{
			int srcX = ix + iy * picture_width; //在显存里像素的角标
			int sa = ((src[srcX] & 0xff000000) >> 24); //0xAArrggbb;AA是透明度
			int sr = ((src[srcX] & 0xff0000) >> 16); //获取RGB里的R
			int sg = ((src[srcX] & 0xff00) >> 8);   //G
			int sb = src[srcX] & 0xff;              //B
			if (ix >= 0 && ix <= graphWidth && iy >= 0 && iy <= graphHeight && dstX <= graphWidth * graphHeight)
			{
				dstX = (ix + picture_x) + (iy + picture_y) * graphWidth; //在显存里像素的角标
				int dr = ((dst[dstX] & 0xff0000) >> 16);
				int dg = ((dst[dstX] & 0xff00) >> 8);
				int db = dst[dstX] & 0xff;
				draw[dstX] = ((sr * sa / 255 + dr * (255 - sa) / 255) << 16)  //公式： Cp=αp*FP+(1-αp)*BP  ； αp=sa/255 , FP=sr , BP=dr
					| ((sg * sa / 255 + dg * (255 - sa) / 255) << 8)         //αp=sa/255 , FP=sg , BP=dg
					| (sb * sa / 255 + db * (255 - sa) / 255);              //αp=sa/255 , FP=sb , BP=db
			}
		}
	}
}

void initButton(Button* btn, const char* normalFile, const char* pressFile,
	int width, int highth, int flag) {
	if (!btn) return;
	loadimage(&btn->imgNormal, normalFile, width, highth, true);
	loadimage(&btn->imgPress, pressFile, width, highth, true);

	btn->width = width;
	btn->highth = highth;

	btn->pressed = false;
	btn->flag = flag;
}

bool checkButtonSelect(Button* btn, MOUSEMSG* msg) {
	float margin = 0.01;
	if (msg->x >= btn->x + btn->width * margin &&
		msg->x <= btn->x + btn->width * (1 - margin) &&
		msg->y >= btn->y + btn->highth * margin &&
		msg->y <= btn->y + btn->highth * (1 - margin)) {
		return true;
	}
	else {
		return false;
	}
}



void drawButton(Button* btn) {
	if (!btn) return;
	if (btn->pressed) {
		drawPNG(btn->x, btn->y, &btn->imgPress);
	}
	else {
		drawPNG(btn->x, btn->y, &btn->imgNormal);
	}
}

wstring GetIMEString()
{
	// 术语说明：以输入“中国”为例
	// 切换到中文输入法后，输入“zhongguo”，这个字符串称作“IME 组成字符串”
	// 而在输入法列表中选择的字符串“中国”则称作“IME 结果字符串”

	wstring str;	// 返回的字符串

	HWND hWnd = GetHWnd();
	HIMC hIMC = ImmGetContext(hWnd);	// 获取 IMC 句柄
	if (hIMC == NULL)
		return str;

	static bool flag = false;		// 输入完成标记
									// 在输入中时，IME 组成字符串不为空，置 true
									// 输入完成后，IME 组成字符串为空，置 false

	DWORD dwSize = ImmGetCompositionStringW(hIMC, GCS_COMPSTR, NULL, 0);		// 获取“IME 组成字符串”的长度

	if (dwSize > 0)		// 如果 IME 组成字符串不为空，且没有错误（此时 dwSize 为负值），则置输入完成标记为 true
	{
		if (flag == false)
			flag = true;
	}
	else if (dwSize == 0 && flag)	// 如果 IME 组成字符串为空，并且标记为 true，则获取 IME 结果字符串
	{
		dwSize = ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, NULL, 0);		// 获取 IME 结果字符串的大小
		if (dwSize > 0)				// 如果 IME 结果字符串不为空，且没有错误
		{
			// 为获取 IME 结果字符串分配空间
			wchar_t* lpWideStr = new WCHAR[dwSize / sizeof(wchar_t) + 1];
			ImmGetCompositionStringW(hIMC, GCS_RESULTSTR, lpWideStr, dwSize);	// 获取 IME 结果字符串
			lpWideStr[dwSize / sizeof(wchar_t)] = '\0';		// 补充字符串结尾 0 字符
			str = lpWideStr;		// 添加到 string 中
			delete[]lpWideStr;		// 释放空间
		}
		flag = false;
	}
	ImmReleaseContext(hWnd, hIMC);	// 释放 HIMC

	return str;
}

bool isHZ(char str[], int index) { //判断指定位置的字符是否是汉字
	int i = 0;
	while (i < index) {
		if (str[i] > 0) {
			i++;
		}
		else {
			i += 2;
		}
	}

	if (i == index) {
		return false;
	}
	else {
		return true;
	}
}

string wstring2string(wstring wstr)
{
	string result;
	//获取缓冲区大小，并申请空间，缓冲区大小事按字节计算的  
	int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), NULL, 0, NULL, NULL);
	char* buffer = new char[len + 1];
	//宽字节编码转换成多字节编码  
	WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), wstr.size(), buffer, len, NULL, NULL);
	buffer[len] = '\0';
	//删除缓冲区并返回值  
	result.append(buffer);
	delete[] buffer;
	return result;
}

void textBox(int startX, int startY, int width, int height, int lineHeight,
	COLORREF bgColor, COLORREF fontColor, char* msg, int* msgLen)
{
	// 画文本框
	//rectangle(10, 18, 630, 42);
	//rectangle(10, 18, 630, 126);

	int x = startX;				// 文字输出的位置。每输出一个，坐标相应增加
	int y = startY;
	int ncursor = 0;		// 实现光标闪烁的计数器
	//char msg[1024] = "";
	*msgLen = 0;
	int lineWidth[10] = { 0 };
	int lineIndex = 0;
	int lineMax = (height) / lineHeight;

	while (1) {
		if (*msgLen == 0) {
			x = startX;
			y = startY;
			memset(lineWidth, 0, sizeof(lineWidth));
			lineIndex = 0;
			setbkcolor(bgColor);
			clearrectangle(startX, startY, startX + width, startY + height);
		}

		if (_kbhit()) {
			setbkcolor(bgColor);
			setbkmode(OPAQUE);

			// ASCII 字符输入
			wchar_t c = _getwch();
			if (c == '\r') {
				if (lineIndex >= lineMax - 1) continue;

				lineWidth[lineIndex++] = x;
				setlinecolor(bgColor); //抹掉原来的光标线
				//line(x + 2, 23, x + 2, 37);
				line(x + 2, y + 1, x + 2, y + 15);

				x = startX;
				y += lineHeight;

				msg[*msgLen] = '\n';
				*msgLen += 1;
			}
			else if (c == 8) {  //退格键
				if (x == startX) { //到了最左边
					if (lineIndex > 0) {
						//抹掉原来的光标线
						setlinecolor(bgColor);
						line(x + 2, y + 1, x + 2, y + 15);

						lineIndex--;
						x = lineWidth[lineIndex];
						y -= lineHeight;
						if (msg[*msgLen - 1] == '\n') {
							*msgLen--;
						}
					}
				}
				else {
					if (isHZ(msg, *msgLen - 1)) { //中文字符
						setbkcolor(bgColor);
						int w = textwidth('8') * 2;
						clearrectangle(x - w, y, x + 2, y + 25);
						x -= w;
						*msgLen -= 2;
					}
					else {
						setbkcolor(bgColor);
						int w = textwidth(msg[*msgLen - 1]); //'1'和'w'不等宽
						clearrectangle(x - w, y, x + 2, y + 25);
						x -= w;
						*msgLen -= 1;
					}
				}
			}
			else {
				//outtextxy(x, 22, c);
				int maxW = startX + width - 2 * textwidth('a');

				if (lineIndex >= lineMax - 1 && x >= maxW) {
					continue;
				}
				setbkcolor(bgColor);
				setbkmode(OPAQUE);
				outtextxy(x, y, c);
				x += textwidth(c);

				msg[*msgLen] = c;
				*msgLen += 1;

				if (x >= startX + width - 2 * textwidth('a')) {
					if (lineIndex >= lineMax - 1 && x >= maxW) {
						continue;
					}

					lineWidth[lineIndex++] = x;
					x = startX;
					y += lineHeight;
				}
			}
		}
		else // 除此之外，检测是否有 IME 输入，如果有，输出
		{
			wstring s = GetIMEString();
			string s2 = wstring2string(s);
			if (lineIndex >= lineMax - 1) {
				if (x + textwidth(s2.c_str()) >= startX + width - 2 * textwidth('a'))
					continue;
			}

			if (s.size() > 0) {
				setbkcolor(bgColor);
				setbkmode(OPAQUE);

				int rightWidth = startX + width - x;
				int rightCount = rightWidth / (textwidth('8') * 2);

				setbkcolor(bgColor);
				setbkmode(OPAQUE);

				if (rightCount == 0) {
					lineWidth[lineIndex++] = x;
					x = startX;
					y += lineHeight;
					outtextxy(x, y, s2.c_str());
					x += textwidth(s2.c_str());
				}
				else if (rightCount < s.size()) {
					wstring tmp1 = s.substr(0, rightCount);
					wstring tmp2 = s.substr(rightCount);
					string tmp1s = wstring2string(tmp1);
					string tmp2s = wstring2string(tmp2);
					outtextxy(x, y, tmp1s.c_str());
					x += textwidth(tmp1s.c_str());
					lineWidth[lineIndex++] = x;
					outtextxy(startX, y + lineHeight, tmp2s.c_str());
					x = startX + textwidth(tmp2s.c_str());
					y += lineHeight;
				}
				else {
					outtextxy(x, y, s2.c_str());
					x += textwidth(s2.c_str());
				}

				string tmp = wstring2string(s);
				strcpy(&msg[*msgLen], tmp.c_str());
				*msgLen += strlen(tmp.c_str());
			}
		}

		setlinecolor((ncursor++ / 10 % 2 == 0) ? fontColor : bgColor); // 绘制光标
		line(x + 2, y + 1, x + 2, y + 15);

		Sleep(20); // 延时
	}
}

void drawText(int x1, int y1, int width, int lineHeight, char* msg) {
	int len = strlen(msg);
	int i = 0;
	int x = x1 + 10;
	int y = y1 + 10;
	char tmp[3] = { 0 };

	setbkmode(TRANSPARENT);

	while (i < len) {
		if (msg[i] > 0) {
			if (msg[i] == '\n') {
				x = x1 + 10;
				y += lineHeight;
			}
			else {
				outtextxy(x, y, msg[i]);
				x += textwidth(msg[i]);
			}
			i++;
		}
		else {
			tmp[0] = msg[i++];
			tmp[1] = msg[i++];
			outtextxy(x, y, tmp);
			x += textwidth(tmp);
		}

		if (x >= x1 + width - 15) {
			x = x1 + 10;
			if (msg[i] == '\n') i++; //如果行满的时候准备换行时，对于的字符正好是回车符，就跳过回车符，否则会连跳两行
			y += lineHeight;
		}
	}
}


int calcQiPaoHeight(int width, int line_height, const char* msg) {
	int x1 = 10;
	int y1 = 10;
	int len = strlen(msg);
	int i = 0;
	int x = x1;
	int y = y1;
	char tmp[3] = { 0 };
	while (i < len) {
		if (msg[i] > 0) {
			if (msg[i] == '\n') {
				x = x1;
				y += line_height;
			}
			else {
				//outtextxy(x, y, msg[i]);
				x += textwidth(msg[i]);
			}
			i++;
		}
		else {
			tmp[0] = msg[i++];
			tmp[1] = msg[i++];
			//outtextxy(x, y, tmp);
			x += textwidth(tmp);
		}

		if (x > width - 15) {
			if (i >= len) break;
			x = x1;
			if (msg[i] == '\n') i++;
			y += line_height;
		}
	}

	int ret = y + textheight('8') + 10;
	return ret;
}

int  calLineWidthMax(const char* msg) {
	char buff[1024];
	strcpy(buff, msg);
	const char* p1 = buff;
	int max = 0;

	while (*p1) {
		char* p2 = (char*)strchr(p1, '\n');
		if (p2 == NULL) {
			int tmp = textwidth(p1);
			max = tmp > max ? tmp : max;
			break;
		}
		*p2 = 0;
		int tmp = textwidth(p1);
		max = tmp > max ? tmp : max;

		p1 = p2 + 1;
	}

	return max + 10 * 2;
}


struct size calQiPaoSize(const char* msg, int winWidth, int line_height) {
	struct size size;
	int qiPaoWithMax = winWidth * 0.7;
	int w = calLineWidthMax(msg);
	if (w > qiPaoWithMax) {
		w = qiPaoWithMax;
	}

	size.width = w;
	size.height = calcQiPaoHeight(w, line_height, msg);

	return size;
}

void drawOneMsg(msg_t* msg, int* msg_height, IMAGE arrows[], IMAGE imgHeads[]) {
	if (msg->type == SEND) {
		setfillcolor(QI_PAO_RIGHT_COLOR);
		setlinecolor(QI_PAO_RIGHT_COLOR);
		setcolor(QI_PAO_RIGHT_COLOR);
		struct size size = calQiPaoSize(msg->msg, MSG_WIN_WIDTH, LINE_HEIGHT);
		int touXiangWidth = 70;
		int qiPaoX1 = MSG_WIN_WIDTH - size.width - touXiangWidth;
		int qiPaoY1 = *msg_height;
		int qiPaoX2 = qiPaoX1 + size.width;
		int qiPaoY2 = *msg_height + size.height;
		fillroundrect(qiPaoX1, qiPaoY1, qiPaoX2, qiPaoY2, 10, 10);
		putimage(qiPaoX2, *msg_height + 15, &arrows[SEND]);
		setcolor(WHITE);
		drawText(qiPaoX1, qiPaoY1, size.width, LINE_HEIGHT, msg->msg);
		setcolor(BLACK);
		putimage(qiPaoX2 + 10, qiPaoY1, &imgHeads[SEND]);
		msg->height = size.height < headHeight ? headHeight : size.height;
		*msg_height += msg->height + 30;
	}
	else {
		setfillcolor(QI_PAO_LEFT_COLOR);
		setlinecolor(QI_PAO_LEFT_COLOR);
		setcolor(QI_PAO_LEFT_COLOR);

		struct size size = calQiPaoSize(msg->msg, MSG_WIN_WIDTH, LINE_HEIGHT);
		int touXiangWidth = 70;
		int qiPaoX1 = touXiangWidth;
		int qiPaoY1 = *msg_height;
		int qiPaoX2 = qiPaoX1 + size.width;
		int qiPaoY2 = *msg_height + size.height;
		fillroundrect(qiPaoX1, qiPaoY1, qiPaoX2, qiPaoY2, 10, 10);
		putimage(qiPaoX1 - 6, *msg_height + 15, &arrows[RECEIVE]);
		setcolor(BLACK);
		drawText(qiPaoX1, qiPaoY1, size.width, LINE_HEIGHT, msg->msg);
		putimage(10, qiPaoY1, &imgHeads[RECEIVE]);

		msg->height = size.height < headHeight ? headHeight : size.height;
		*msg_height += msg->height + 30;
	}
}

void moveWindow(HWND hwnd, int offX, int offY) {
	RECT rect;
	GetWindowRect(hwnd, &rect);
	int w = rect.right - rect.left;
	int h = rect.bottom - rect.top;
	int newX = rect.left + offX;
	int newY = rect.top + offY;
	/*if (newX < 0) newX = 0;
	if (newX > screenWidth - 100)newX = screenWidth - 100;
	if (newY < 0) newY = 0;
	if (newY > screenHeight - 100) newY = screenHeight - 100;*/
	MoveWindow(hwnd, newX, newY, w, h, true);
}

void drawMsg(msg_t* msgAll, int currentMsgIndex, int* msgHeight, IMAGE arrows[], IMAGE imgHeads[]) {
	struct size size = calQiPaoSize(msgAll[currentMsgIndex].msg, MSG_WIN_WIDTH, LINE_HEIGHT);
	msgAll[currentMsgIndex].height = size.height < headHeight ? headHeight : size.height;
	if (*msgHeight + msgAll[currentMsgIndex].height > msgHeighMax) {
		setbkcolor(WHITE);
		clearrectangle(2, 81, 424, 623);
		*msgHeight = msgHeighMin;
	}

	msg_t* msg = &msgAll[currentMsgIndex];
	drawOneMsg(msg, msgHeight, arrows, imgHeads);

	setcolor(WHITE);
	outtextxy(212, 13, "网络聊天室");
	setcolor(BLACK);
}


std::string UTF8ToGBK(const char* strUTF8)
{
	int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, NULL, 0);
	wchar_t* wszGBK = new wchar_t[len + 1];
	memset(wszGBK, 0, len * 2 + 2);
	MultiByteToWideChar(CP_UTF8, 0, strUTF8, -1, wszGBK, len);
	len = WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, NULL, 0, NULL, NULL);
	char* szGBK = new char[len + 1];
	memset(szGBK, 0, len + 1);
	WideCharToMultiByte(CP_ACP, 0, wszGBK, -1, szGBK, len, NULL, NULL);
	std::string strTemp(szGBK);
	if (wszGBK) delete[] wszGBK;
	if (szGBK) delete[] szGBK;
	return strTemp;
}

void GBKToUTF8(string& strGBK)
{
	int len = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
	wchar_t* wszUtf8 = new wchar_t[len];
	memset(wszUtf8, 0, len);
	MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, wszUtf8, len);

	len = WideCharToMultiByte(CP_UTF8, 0, wszUtf8, -1, NULL, 0, NULL, NULL);
	char* szUtf8 = new char[len + 1];
	memset(szUtf8, 0, len + 1);
	WideCharToMultiByte(CP_UTF8, 0, wszUtf8, -1, szUtf8, len, NULL, NULL);

	strGBK = szUtf8;
	delete[] szUtf8;
	delete[] wszUtf8;
}

bool TCPInit(SOCKET* sock, sockaddr_in* addr, const char* serverIP, int port) {
	WSADATA wsaData;//初始化结果
	int err = WSAStartup(MAKEWORD(1, 1), &wsaData);
	if (err != 0) {
		return false;
	}
	//2.网络套接字socket
	*sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	addr->sin_family = PF_INET;
	addr->sin_addr.S_un.S_addr = inet_addr(serverIP);
	addr->sin_port = htons(port);

	return true;
}