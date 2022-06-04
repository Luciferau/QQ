#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mmsystem.h>
#include "tx_qq.h"
#include <winsock.h>
//13 4 
#define SERVER_IP "118.126.117.125"//������IP �ַ���
#define MY_IP "127.0.0.1"
#define PORT 2022//���Ϊ
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "tx_qq.lib")
#pragma comment(lib, "winmm.lib")

char ip_addr[32];//�Է���IP �����
SOCKET serverSocket;//ͨ������׽��ֽ���ͨ��
sockaddr_in sockAddr;//�Զ������ַ��IP�Ͷ˿ںţ�
HWND hwnd;//���촰�ھ��
int screenwidth;//��Ļ�Ŀ��
int screenhight;//��Ļ�ĸ߶�
int msgHight;//���ݵĸ߶Ⱦ��붥�˵ľ���
int msgHeight; // �����ݵ� y ����

IMAGE imgBg;//���촰�ڱ���ͼƬ
IMAGE imageArrows[2];//����ļ�ͷ
IMAGE imageHeads[2];//����ͷ��
Button btnClose;
Button btnTitle;

Button btnSend;
char msgEdit[1024];//�༭��������ַ��������λ��
//�ı���������ַ�
int msgLen;



msg_t msgAll[100];//��Ϣ������ʷ��¼����Ϣ���飩 ѭ���洢
int msgCount = 1;//һ�����ٸ���Ϣ
int currentMsgIndex = -1;//��Ϣ������ һ��ʼһ����Ϣû������Ϊ-1


void initUI()
{
	//�������촰��
	
	initgraph(WINDOW_WIDTH  ,WINDOW_HEIGHT  ,EW_SHOWCONSOLE);
	setbkmode(TRANSPARENT);//���ñ�����˼Ϊ͸�� ʵ���������͸��
	//���ر�����
	hwnd = GetHWnd();//��ȡ��ǰ���ڵľ��
	SetWindowLong(hwnd, GWL_STYLE/*������ʽ*/,GetWindowLong(hwnd,GWL_STYLE) - WS_CAPTION/*ȥ��������*/);

	//�ƶ�����λ��
	screenhight = GetSystemMetrics(SM_CYSCREEN);
	screenwidth = GetSystemMetrics(SM_CXSCREEN);
 
	MoveWindow(hwnd, screenwidth / 4, screenhight /10    , WINDOW_WIDTH  , WINDOW_HEIGHT   , false/*�Ƿ����»���*/);
	 
	//���Ʊ���ͼƬ 
	loadimage(&imgBg,"./res/bg.png", WINDOW_WIDTH  , WINDOW_HEIGHT,true);
	putimage(0, 0, &imgBg);
	 
	//��������β��
	loadimage(&imageArrows[0]/*���*/,"./res/left_arrow,jpg",6,8,true/*�ȱ�������*/);
	loadimage(&imageArrows[0]/*�ұ�*/,"./res/right	_arrow,jpg",6,8,true/*�ȱ�������*/);

	//��������ͷ��
	loadimage(&imageHeads[0],"./res/rock.jpg",55,51,true);//���
	loadimage(&imageHeads[1],"./res/niu.jpg",44,51,true);//right

	//���ذ�ť 3��
	initButton(&btnClose, "./res/close_normal.jpg","./res/close_press.jpg",32/*��ť���*/,33,0);

	//������
	btnClose.x = WINDOW_WIDTH - 32;
	btnClose.y = 0;

	initButton(&btnSend, "./res/send_normal.jpg", "./res/send_press.jpg", 88/*��ť���*/, 28, 0);
	btnSend.y = 337;
	btnSend.y = 784;

	initButton(&btnTitle, "./res/title.jpg", "./res/titel.jpg", 460/*titel���*/, 39, 0);

	btnTitle.x = 0;
	btnTitle.y = 0;
	 msgHight = 120;//���ݵĸ߶Ⱦ��붥�˵ľ���
	 //�༭��Ϣ����ʾ��Ϣ���ı���ɫ
	 setcolor(BLACK);
}

void connect()
{
	// ���ӷ������ˣ�����������������
	//����ʽ���� û������һֱ����
	connect(serverSocket, (SOCKADDR*)&sockAddr, sizeof(sockAddr));
	printf("�Ѿ����룡\n");
}

//�༭��Ϣ
DWORD WINAPIV msgEditHandle(LPVOID param)
{
	textBox(10,663,420,110,LINE_HEIGHT/*�о�*/, WHITE/*������ɫ*/ ,BLACK/* ������ɫ*/,msgEdit/*��Ϣ�����λ�� */,&msgLen);

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
	 

		buff[ret] = 0;//����ַ���������

		currentMsgIndex = (currentMsgIndex + 1) % (sizeof(msgAll[0]));
		
		strcpy(msgAll[currentMsgIndex].msg, buff);

		//������Ϣ����
		msgAll[currentMsgIndex].type = RECEIVE;
		msgCount++;
		//������
		drawMsg(msgAll, currentMsgIndex, &msgHeight, imageArrows, imageHeads);

	}

	return NULL;
}

	void mainUI() 
	{
		bool titleDrag = false; //��ʾ�����������Ƿ񱻵����϶�
		int titleLastX; //���ڵ���һ��λ�ã�X ����λ�ã�
		int titleLastY; //���ڵ���һ��λ�ã�X ����λ�ã�
		while (1) {
			MOUSEMSG m = GetMouseMsg();
			FlushMouseMsgBuffer(); //�����٣���׺�����϶������ı��ⰴť�������������Ϣ̫�࣬
			//���ֻ��ң�
			switch (m.uMsg) {
			case WM_MOUSEMOVE:
				// ��껬��������
				if (checkButtonSelect(&btnTitle, &m)) {
					if (btnTitle.pressed == true) {
						if (titleDrag == false) { // ��ʱ�������Ѿ���������£���׼���϶�
							titleLastX = m.x; // ��¼��ʼ����

							titleLastY = m.y;
							titleDrag = true;
						}
						else { // ��ʱ�������Ѿ���������£������϶�
						// �����϶�ƫ����
							int offX = m.x - titleLastX;
							int offY = m.y - titleLastY;
							moveWindow(hwnd, offX, offY); // �����϶�ƫ�������ƶ�����
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
					// �������Ƿ�Ӱ�ť���ƶ�����ť֮��
					if (btnClose.pressed == true) { // ���ӹرհ�ť�Ƴ�
						btnClose.pressed = false;
						drawButton(&btnClose);
					}
					if (btnSend.pressed == true) { // ���ӷ��Ͱ�ť�Ƴ�
						btnSend.pressed = false;
						drawButton(&btnSend);
					}
				}
				break;
			case WM_LBUTTONDOWN:
				if (checkButtonSelect(&btnTitle, &m)) {
					btnTitle.pressed = true; // �������±�����
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
					printf(" �Ѿ�����d %d ���ַ�, \n", ret);
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
					// �ɿ���������ť�����̧��
					btnTitle.pressed = false;
					titleDrag = false;
				}
				break;
			}
		}
	}


int main()
{
		
		printf("������Է���IP��ַ :");
		scanf_s("%s",ip_addr, sizeof(ip_addr));

		//��ʼ������������ ��ʼ��
		if (!TCPInit(&serverSocket, &sockAddr, ip_addr, PORT))
		{
			printf("�����ʼ��ʧ��!\n");
			return -1;
		}

			printf("�������� \n");
			connect();
			
			initUI();
			 
			//�����߳� 


			DWORD threadID;//�߳�ID �༭��Ϣ
			

			CreateThread(NULL, NULL, msgEditHandle, NULL, NULL, &threadID/*�߳�ID*/);

			CreateThread(NULL, NULL, threadFuncRcv, NULL, NULL, &threadID/*�߳�ID*/);

			mainUI();

			system("pause");
			return 0;
}




