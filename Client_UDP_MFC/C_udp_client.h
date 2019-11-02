#pragma once
#include <WinSock2.h>
#include <stdio.h>
#include "st_MSG.h"

#include <thread>
#include "Client_UDP_MFCDlg.h"
#include "F_cjson.h"

#include <time.h> 

#pragma comment(lib, "ws2_32.lib")

#define IP_CMD "192.168.1.12"
#define PORT_CMD 1001

#define IP_DATA "192.168.1.8"
#define PORT_DATA  1003

#define IP_ADC_FPGA "192.168.1.12"
#define PORT_ADC_FPGA 1004

//adc
#define  ADC_CAHNNEL "0x00"
#define  ADC_SIZE "0x00"
#define  ADC_ADDR "0x00"
#define  ADC_DATA "0x00"

//fpga
#define FPGA_REG "0x00"
#define FPGA_VAL "0x00"

#define MAX_NODE 10*10000
#define MAX_WRITE_FILE_PACK 00

typedef struct stConfigMsg
{
	char fileName[128];
	char ip_cmd[32];
	char ip_data[32];
	u_short port_cmd;
	u_short port_dadta;

}stConfigMsg;

typedef struct stMsg
{
	int msg_id;
	char data[1400];

}stMsg,pstMsg;

int th_send_commend_new(void* th);
int th_recv_data_new(void* th);
int th_write_data_new(void* th);

int get_file_size(FILE *fd);
char *get_file_name(char *fileName);

//socket
bool ifSocketTrue(SOCKET *sock);
bool init_sock(SOCKET *sock, sockaddr_in *sAddr, char*ip, char* port, bool bind_flas);

//设置非阻塞
void setnonblocking(SOCKET *sockfd);
void setTimeOut(SOCKET *sockfd);

//2019年6月19日11:00:31 获取Debug或Release所在的路径
void GetModuleDir(char fileName[260]);
void TcharToChar(const TCHAR * tchar, char * _char);


//输出状态信息到listBox,并且显示最新插入一行   
void showCurLineChar(CListBox *m_listbox, char* cStr);
void showCurLine(CListBox *m_listbox, LPCTSTR conStr);

//十六进制
//字符串装换 
CString intToLpctstrHex(int hexInt);

//判断是否为十六进制数  2
int DetermWhIsHEX(char *strHexIn);

//16 str to 10 int   3
bool ConvertStrToHex(char *strHexIn);

//过滤清除 0x   1
bool cutSubStr(char *str, const char* subStr);

bool getHex(char* m_fpga_reg, UINT32 *retInt);

bool saveInitArg(void* puser);
bool setInit(CListBox *m_listbox, stInitMsg *init, void* puser);

//时间
void No_Cursor();
void gotoxy(short x, short y);
bool getCurTime(char *timeBuf);