#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mmsystem.h>
#include "tx_qq.h"
#include <winsock.h>
//13 4 
#define SERVER_IP "118.126.117.125"//服务器IP 字符串
#define MY_IP "127.0.0.1"
#define PORT 2022//最大为
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "tx_qq.lib")
#pragma comment(lib, "winmm.lib")

char ip_addr[32];//对方的IP 输入的
SOCKET serverSocket;//通过这个套接字进行通信
sockaddr_in sockAddr;//对端网络地址（IP和端口号）
HWND hwnd;//聊天窗口句柄
int screenwidth;//屏幕的宽度
int screenhight;//屏幕的高度
int msgHight;//气泡的高度距离顶端的距离
int msgHeight; // 新气泡的 y 坐标

IMAGE imgBg;//聊天窗口背景图片
IMAGE imageArrows[2];//聊天的箭头
IMAGE imageHeads[2];//两个头像
Button btnClose;
Button btnTitle;

Button btnSend;
char msgEdit[1024];//编辑区输入的字符串保存的位置
//文本框输入的字符
int msgLen;



msg_t msgAll[100];//消息的了历史记录（消息数组） 循环存储
int msgCount = 1;//一共多少个消息
int currentMsgIndex = -1;//消息的索引 一开始一条消息没有所以为-1


void initUI()
{
	//创建聊天窗口
	
	initgraph(WINDOW_WIDTH  ,WINDOW_HEIGHT  ,EW_SHOWCONSOLE);
	setbkmode(TRANSPARENT);//设置背景意思为透明 实现字体输出透明
	//隐藏标题栏
	hwnd = GetHWnd();//获取当前窗口的句柄
	SetWindowLong(hwnd, GWL_STYLE/*设置样式*/,GetWindowLong(hwnd,GWL_STYLE) - WS_CAPTION/*去掉标题烂*/);

	//移动窗口位置
	screenhight = GetSystemMetrics(SM_CYSCREEN);
	screenwidth = GetSystemMetrics(SM_CXSCREEN);
 
	MoveWindow(hwnd, screenwidth / 4, screenhight /10    , WINDOW_WIDTH  , WINDOW_HEIGHT   , false/*是否重新绘制*/);
	 
	//绘制背景图片 
	loadimage(&imgBg,"./res/bg.png", WINDOW_WIDTH  , WINDOW_HEIGHT,true);
	putimage(0, 0, &imgBg);
	 
	//加载气泡尾巴
	loadimage(&imageArrows[0]/*左边*/,"./res/left_arrow,jpg",6,8,true/*等比例缩放*/);
	loadimage(&imageArrows[0]/*右边*/,"./res/right	_arrow,jpg",6,8,true/*等比例缩放*/);

	//加载聊天头像
	loadimage(&imageHeads[0],"./res/rock.jpg",55,51,true);//左边
	loadimage(&imageHeads[1],"./res/niu.jpg",44,51,true);//right

	//加载按钮 3个
	initButton(&btnClose, "./res/close_normal.jpg","./res/close_press.jpg",32/*按钮宽度*/,33,0);

	//放在哪
	btnClose.x = WINDOW_WIDTH - 32;
	btnClose.y = 0;

	initButton(&btnSend, "./res/send_normal.jpg", "./res/send_press.jpg", 88/*按钮宽度*/, 28, 0);
	btnSend.y = 337;
	btnSend.y = 784;

	initButton(&btnTitle, "./res/title.jpg", "./res/titel.jpg", 460/*titel宽度*/, 39, 0);

	btnTitle.x = 0;
	btnTitle.y = 0;
	 msgHight = 120;//气泡的高度距离顶端的距离
	 //编辑信息的显示信息的文本颜色
	 setcolor(BLACK);
}

void connect()
{
	// 连接服务器端（发起网络连接请求）
	//阻塞式链接 没有连接一直卡着
	connect(serverSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr));
	printf("已经接入！\n");
}

//编辑信息
DWORD WINAPIV msgEditHandle(LPVOID param)
{
	textBox(10,663,420,110,LINE_HEIGHT/*行距*/, WHITE/*背景颜色*/ ,BLACK/* 字体颜色*/,msgEdit/*信息保存的位置 */,&msgLen);

	return NULL;
}

DWORD WINAPIV threadFuncRcv(LPVOID param)
{
	char buff[4096];
	while (1)
	{

		int ret = recv(serverSocket, buff, sizeof(buff), 0);
		if (ret <= 0) {
			return false;
		}

		mciSendString("play res/msg.mp3", NULL, NULL, NULL);
	 

		buff[ret] = 0;//添加字符串结束符

		currentMsgIndex = (currentMsgIndex + 1) % (sizeof(msgAll[0]));
		
		strcpy(msgAll[currentMsgIndex].msg, buff);

		//配置消息类型
		msgAll[currentMsgIndex].type = RECEIVE;
		msgCount++;
		//画气泡
		drawMsg(msgAll, currentMsgIndex, &msgHeight, imageArrows, imageHeads);

	}

	return NULL;
}

	void mainUI() 
	{
		bool titleDrag = false; //表示“标题栏”是否被单击拖动
		int titleLastX; //窗口的上一次位置（X 坐标位置）
		int titleLastY; //窗口的上一次位置（X 坐标位置）
		while (1) {
			MOUSEMSG m = GetMouseMsg();
			FlushMouseMsgBuffer(); //不能少，后缀快速拖动顶部的标题按钮，讲导致鼠标消息太多，
			//出现混乱！
			switch (m.uMsg) {
			case WM_MOUSEMOVE:
				// 鼠标滑过标题栏
				if (checkButtonSelect(&btnTitle, &m)) {
					if (btnTitle.pressed == true) {
						if (titleDrag == false) { // 此时标题栏已经被点击按下，正准备拖动
							titleLastX = m.x; // 记录初始坐标

							titleLastY = m.y;
							titleDrag = true;
						}
						else { // 此时标题栏已经被点击按下，正在拖动
						// 计算拖动偏移量
							int offX = m.x - titleLastX;
							int offY = m.y - titleLastY;
							moveWindow(hwnd, offX, offY); // 根据拖动偏移量，移动窗口
						}
					}
				}
				else if (checkButtonSelect(&btnSend, &m)) {
					btnSend.pressed = true;
					drawButton(&btnSend);
				}
				else if (checkButtonSelect(&btnClose, &m)) {
					btnClose.pressed = true;
					drawButton(&btnClose);
				}
				else {
					// 检查鼠标是否从按钮内移动到按钮之外
					if (btnClose.pressed == true) { // 鼠标从关闭按钮移出
						btnClose.pressed = false;
						drawButton(&btnClose);
					}
					if (btnSend.pressed == true) { // 鼠标从发送按钮移出
						btnSend.pressed = false;
						drawButton(&btnSend);
					}
				}
				break;
			case WM_LBUTTONDOWN:
				if (checkButtonSelect(&btnTitle, &m)) {
					btnTitle.pressed = true; // 单击按下标题栏
				}
				else if (checkButtonSelect(&btnClose, &m)) {
					btnClose.pressed = true;
					drawButton(&btnClose);
				}
				else if (checkButtonSelect(&btnSend, &m)) {
					btnSend.pressed = true;
					drawButton(&btnSend);
				}
				break;
			case WM_LBUTTONUP:
				if (checkButtonSelect(&btnClose, &m)) {
					//btnClose.pressed = false;
					//drawButton(&btnClose);
					closegraph();
					exit(0);
				}
				else if (checkButtonSelect(&btnSend, &m)) {
					btnSend.pressed = false;
					drawButton(&btnSend);
					int  ret = send(serverSocket, msgEdit, msgLen, 0);
					printf(" 已经发送d %d 个字符, \n", ret);
					currentMsgIndex = (currentMsgIndex + 1) % (sizeof(msgAll[0]));
					msgEdit[msgLen] = 0;
					strcpy(msgAll[currentMsgIndex].msg, msgEdit);
					memset(msgEdit, 0, sizeof(msgEdit));
					msgLen = 0;
					msgAll[currentMsgIndex].type = SEND;
					msgCount++;
					//drawMsg();
					drawMsg(msgAll, currentMsgIndex, &msgHeight, imageArrows, imageHeads);
				}
				else if (checkButtonSelect(&btnTitle, &m)) {
					// 松开标题栏按钮（左键抬起）
					btnTitle.pressed = false;
					titleDrag = false;
				}
				break;
			}
		}
	}


int main()
{
		
		printf("请输入对方的IP地址 :");
		scanf_s("%s",ip_addr, sizeof(ip_addr));

		//开始发起网络连接 初始化
		if (!TCPInit(&serverSocket, &sockAddr, ip_addr, PORT))
		{
			printf("网络初始化失败!\n");
			return -1;
		}

			printf("正在连接 \n");
			connect();
			
			initUI();
			 
			//创建线程 


			DWORD threadID;//线程ID 编辑信息
			

			CreateThread(NULL, NULL, msgEditHandle, NULL, NULL, &threadID/*线程ID*/);

			CreateThread(NULL, NULL, threadFuncRcv, NULL, NULL, &threadID/*线程ID*/);

			mainUI();

			system("pause");
			return 0;
}




