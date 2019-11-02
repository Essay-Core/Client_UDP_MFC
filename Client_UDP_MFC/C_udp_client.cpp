#include "stdafx.h"
#include "C_udp_client.h"
#include "st_MSG.h"
#include <iostream>
#include "Client_UDP_MFCDlg.h"

using namespace std;
#define BUF_SIZE 1024*10
#define MAX_BUFFER_SIZE 64*1000

int get_file_size(FILE *fd)
{
	if (!fd)
	{
		return -1;
	}

	//把文件的位置指针移到文件尾
	fseek(fd, 0, SEEK_END);

	//获取文件长度;
	int data_len = ftell(fd);

	//获取长度之后，需要重新将文件指针移到最开始的位置，然后在开始读取文件
	fseek(fd, 0, SEEK_SET);

	return data_len;
}

char *get_file_name(char *fileName)
{
	return  strrchr(fileName, '\\');
}


int th_send_commend_new(void* th)
{
	CClient_UDP_MFCDlg *pp = (CClient_UDP_MFCDlg*)th;

	int saddrLen = sizeof(sockaddr);

	if (!init_sock(&pp->m_sock_sig, &pp->m_servAddr_sig, pp->m_IP, pp->m_port_ctrl, false))
	{
		return -1;
	}
	while (1)
	{
		switch (pp->m_msg.msg_id)
		{
			//无动作
		case SOCK_MSG_NULL:
			Sleep(1000);

			if (pp->m_click_Stop_th == true)
			{
				break; //退出线程
			}

			break;

			//发送控制消息
		case SOCK_MSG_START_GET_DATA:
			sendto(pp->m_sock_sig, (char*)&pp->m_msg, sizeof(stHeadMsg), 0, (struct sockaddr*)&pp->m_servAddr_sig, sizeof(pp->m_servAddr_sig));
			showCurLine(pp->m_listbox, _T("Send the collection START command"));

			pp->m_msg.msg_id = SOCK_MSG_NULL;

			break;

		case SOCK_MSG_STOP_GET_DATA:
			sendto(pp->m_sock_sig, (char*)&pp->m_msg, sizeof(stHeadMsg), 0, (struct sockaddr*)&pp->m_servAddr_sig, sizeof(pp->m_servAddr_sig));
			showCurLine(pp->m_listbox, _T("Send the collection STOP command"));

			pp->m_msg.msg_id = SOCK_MSG_NULL;

			break;//收到停止信号，即可退出线程

		default:
			pp->m_msg.msg_id = SOCK_MSG_NULL;
			break;
		}
		
		//判断是否退出线程
		if (pp->m_click_Stop_th == true)
		{
			break; //退出线程
		}
	}
	

	return 0;
}

int th_recv_data_new(void* th)
{
	CClient_UDP_MFCDlg *pp = (CClient_UDP_MFCDlg*)th;

	if (pp->sock == SOCKET_ERROR)
	{
		if (!init_sock(&pp->sock, &pp->m_servAddr_data, pp->m_IPLocal, pp->m_port_data, true))
		{
			return -1;
		}
	}

	int count = 0;
	int addrLen = sizeof(sockaddr);
	int headLen = sizeof(_MSG);

	FILE *fd1 = { 0 };
	pMSG t_msg = NULL;
	_MSG recvBuf = { 0 };

	memset(&recvBuf, 0, sizeof(_MSG));

	while (true)
	{
		if (pp->start_status) ////开始收发标志 
		{
			if (pp->m_NumOfAccPack == 1)
			{
				showCurLine(pp->m_listbox, _T("Receiving data..."));
			}

			//清空相应的内存块
			memset(&recvBuf, 0, sizeof(_MSG));

			count = recvfrom(pp->sock, (char*)&recvBuf, headLen, 0, (sockaddr*)&pp->m_servAddr_data, &addrLen);
			if (count == SOCKET_ERROR)
			{
				DWORD err = GetLastError();
				if (err == 10060)
				{
					showCurLine(pp->m_listbox, _T("Receiving time OUT"));
					continue;
				}
				continue;
			}
			if (count > 0)
			{
				pp->m_NumOfAccPack++;
			}
			t_msg = (pMSG)&recvBuf;

			//根据接收到的头部进行入队操作
			switch (t_msg->msg_id)
			{
			case SOCK_MSG_NULL:
				continue;

				//开始，打开文件
			case SOCK_MSG_START_SEND_DATA:
				continue;

			case SOCK_MSG_SENDING_DATA:
				//入队操作
				Enqueue(pp->m_Q, (_DATA*)t_msg->data);
				continue;

				//停止发送，并保存文件
			case SOCK_MSG_STOP_SEND_DATA:
				continue;

			case SOCK_MSG_START_GET_DATA:
				continue;

			case SOCK_MSG_STOP_GET_DATA:
				continue;

			default:
				break;
			}
		}
		else
		{
				
			//判断是否退出线程
			if (pp->m_click_Stop_th == true)
			{
				break; //退出线程
			}

			continue; //如果一直没有退出，那当整个程序退出的时候就无法正常退出线程
		}

		
	}

	return 0;
}

int th_write_data_new(void* th)
{
	CClient_UDP_MFCDlg *pp = (CClient_UDP_MFCDlg*)th;
	vector<int> coll;

	_DATA *t_msg = NULL;
	_DATA t_data;
	int ret_close = 0;
	int write_count = 0;
	char recvBufConut[64] = { 0 };
	char strBuf[64] = {0};

	while (true)
	{
		if (pp->start_status) 
		{
			if (!pp->m_fd)
			{
				pp->m_fd = fopen(pp->m_filename, "ab+");
				if (!pp->m_fd)
				{
					return -1;
				}
				pp->m_NumOfAccPack = 0;
				showCurLine(pp->m_listbox, _T("File opened successfully"));

			}

			/********************************************************************
			2019年6月25日10:40:50
			添加变量，记录写入文件的数量
			当前写入数与包序号对比，不一样则写入0
			写入文件达到某个值时停止写入

			是否自动发送停止消息？	不，还是手动停止，比较简单，不需要改太多
			是否入队出队操作？		可以出入对，只是不写入文件

			2019年6月27日10:53:08
			记录下缺少的包序号
			当重新开始采集数据的时候，需要添加当前采集的时间，丢包序号
			*********************************************************************/

			//出队操作，写文件
			if (Dequeue(pp->m_Q, &t_data))
			{
				//add 2019年6月25日15:48:01
				//当出队数小于包序时，写0
				pp->m_dequeue_count++;
			
				while (pp->m_dequeue_count < t_data.sequNum)
				{
					//memset(strBuf, 0, sizeof(strBuf));
					//sprintf_s(strBuf, "recv count error number is :%d",pp->m_dequeue_count);
					//showCurLineChar(pp->m_listbox, strBuf);

					//记录所丢的包序号
					coll.push_back(pp->m_dequeue_count);

					memset(pp->m_zeroBuf, 0, 340 * 4);
					write_count = fwrite(pp->m_zeroBuf, 4, 340, pp->m_fd);
					if (write_count > 0)
					{
						//showCurLine(pp->m_listbox, _T("fwrite success"));
						pp->m_NumOfWritePack++;//写文件计数，达到某个数值时，关闭文件不再写入
					}

					pp->m_dequeue_count++;
				}

				//达到某个数值时，不再写入,等待退出，或重新开始
				//如果当输入的的是0是，不限制文件的大小  add 2019年6月30日18:36:13
				if (pp->m_NumOfWritePack >= pp->m_writeFile_countCtrl && 0 != pp->m_writeFile_countCtrl)
				{
					//add 2019年6月27日11:38:48
					pp->m_msg.msg_id = SOCK_MSG_STOP_GET_DATA;  //停止消息
					pp->start_status = false; //停止接收数据

					Clequeue(pp->m_Q);

					if (pp->sock > 0)
					{
						closesocket(pp->sock);
					}
					//==2019年6月27日11:39:39

					//停止写入文件
					continue;
				}
				
				write_count = fwrite(t_data.data, 4, t_data.length, pp->m_fd);
				if (write_count > 0)
				{
					//showCurLine(pp->m_listbox, _T("fwrite success"));
					pp->m_NumOfWritePack++;//写文件计数，达到某个数值时，关闭文件不再写入
				}

			}
		}
		else
		{
			if (pp->m_fd > 0)
			{
				ret_close = fclose(pp->m_fd);
				pp->m_fd = NULL;

				showCurLine(pp->m_listbox, _T("File Closed!"));
				sprintf(recvBufConut, "The total number of received is %d", pp->m_NumOfAccPack);				

				Clequeue(pp->m_Q);
				
				//2019年6月27日11:04:46
				//将记录的丢包的序号输出日写入文件
				FILE *f = fopen("losePackNum.txt", "ab+");
				char loserNu[16] = { 0 };
				char timeBuf[16] = { 0 };

				//获取当前时间字符串
				getCurTime(timeBuf);

				memset(loserNu, 0, sizeof(loserNu));
				sprintf(loserNu, "\r\ntime: %s\r\n", timeBuf);
				fwrite(loserNu, 1, strlen(loserNu), f);

				for (int i = 0; i < coll.size(); ++i)//size(),返回容器中的元素个数
				{
					memset(loserNu, 0, sizeof(loserNu));
					sprintf(loserNu, "%d,", coll[i]);
					fwrite(loserNu, 1, strlen(loserNu), f);
				}

				fclose(f);
				continue;

			}
		}

		//判断是否退出线程
		if (pp->m_click_Stop_th == true)
		{
			break; //退出线程
		}
	}
}

bool ifSocketTrue(SOCKET *sock)
{
	if (INVALID_SOCKET == *sock)
	{
		DWORD m_dwErr = GetLastError();
		//AfxMessageBox(_T(" socket is INVALID_SOCKET!"));
		return false;
	}

	if (*sock <= 0)
	{
		//AfxMessageBox(_T("pls click BIND bt!"));
		return false;
	}

	return true;
}

bool init_sock(SOCKET *sock, sockaddr_in *sAddr, char*ip, char* port,bool bind_flas)
{
	DWORD m_dwErr;
	*sock = socket(AF_INET, SOCK_DGRAM, 0);
	if (INVALID_SOCKET == *sock)
	{
		m_dwErr = GetLastError();
		AfxMessageBox(_T(" m_sock_adc is INVALID_SOCKET!"));
		return false;
	}

	memset((char*)sAddr, 0, sizeof(sockaddr_in));
	sAddr->sin_family = AF_INET;
	sAddr->sin_port = htons(atoi(port));
	sAddr->sin_addr.S_un.S_addr = inet_addr(ip);

	if (bind_flas)
	{
		int ret = ::bind(*sock, (sockaddr*)sAddr, sizeof(sockaddr));
		if (SOCKET_ERROR == ret)
		{
			m_dwErr = GetLastError();
			AfxMessageBox(_T(" bind false!"));
			return false;
		}
	}

	return true;
	
}

void setnonblocking(SOCKET *sockfd)
{
	if (*sockfd < 0)
	{
		return;
	}
	
	int iMode = 1; //0：阻塞
	int ret = ioctlsocket(*sockfd, FIONBIO, (u_long FAR*) &iMode);//非阻塞设置
	if (ret < 0)
	{
		return;
	}
	
}

void setTimeOut(SOCKET *sockfd)
{
	if (*sockfd < 0)
	{
		return;
	}

	// 设置超时
	struct timeval timeout;
	timeout.tv_sec = 1000*1;//秒
	timeout.tv_usec = 0;//微秒

	if (setsockopt(*sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout)) == -1) 
	{
		AfxMessageBox(_T("set time out false"));
		return;
	}
}

//获取Debug或Release所在的路径,更改文件名
void GetModuleDir(char fileName[260])
{
	TCHAR exeFullPath[MAX_PATH]; // MAX_PATH
	GetModuleFileName(NULL, exeFullPath, MAX_PATH);

	char buf[260] = { 0 };
	(_tcsrchr(exeFullPath, _T('\\')))[1] = 0;
	TcharToChar(exeFullPath, buf);

	sprintf(fileName, "%stext.bin", buf);

}

//*tchar是TCHAR类型指针，*_char是char类型指针   
void TcharToChar(const TCHAR * tchar, char * _char)
{
	if (!tchar)
	{
		return;
	}
	int iLength;
	//获取字节长度   
	iLength = WideCharToMultiByte(CP_ACP, 0, tchar, -1, NULL, 0, NULL, NULL);
	//将tchar值赋给_char    
	WideCharToMultiByte(CP_ACP, 0, tchar, -1, _char, iLength, NULL, NULL);
}

//int to cstring ,cstring to lpctstring
CString intToLpctstrHex(int hexInt)
{
	char szStr[16] = { 0 };
	sprintf_s(szStr, "0x%x", hexInt);

	return CString(szStr);
}

void showCurLineInt(CListBox *m_listbox, LPCTSTR conStr, char* cStr)
{

}

void showCurLineChar(CListBox *m_listbox,char* cStr)
{
	if (!m_listbox || !cStr)
	{
		return;
	}

	USES_CONVERSION;
	A2CW(W2A(CString(cStr)));
	m_listbox->AddString(A2CW(W2A(CString(cStr)))); //插入

	int count = 0;
	count = m_listbox->GetCount();//获取总行数

	m_listbox->SetCurSel(count - 1);//显示最后一行
}

void showCurLine(CListBox *m_listbox, LPCTSTR conStr)
{
	if (!m_listbox || !conStr)
	{
		return;
	}
	m_listbox->AddString(conStr); //插入

	int count = 0;
	count = m_listbox->GetCount();//获取总行数

	m_listbox->SetCurSel(count - 1);//显示最后一行

}

int DetermWhIsHEX(char *strHexIn)
{
	if (!strHexIn)
	{
		return -1;
	}

	char str[128] = { 0 };

	memcpy(str, strHexIn, strlen(strHexIn));

	for (int i = 0; i < strlen(str); i++)
	{
		char c = str[i];
		if (!(((c >= '0') && (c <= '9')) || ((c >= 'a') && (c <= 'f')) || ((c >= 'A') && (c <= 'F'))))
		{
			AfxMessageBox(_T("请输入十六进制的数"));
			return -1;
		}
	}
	
	return 0;
}

//过滤0x标志
bool ConvertStrToHex(char *strHexIn)
{	
	if (!strHexIn)
	{
		return -1;
	}

	char tmp[32] = { 0 };
	char str[32] = { 0 };
	if (strstr(strHexIn, "0x"))
	{
		strcpy(str, strstr(strHexIn, "0x") + 2);
	}

	
	char strHex[32] = { 0 };
	strcpy(strHex, strHexIn);

	int i = 0, len = 0, temp = 0;
	__int64 nDec = 0;

	gets(strHex);

	len = strlen(strHex);
	for (i = 0; strHex[i] != '\0'; ++i)
	{
		switch (strHex[i])
		{
		case 'A': temp = 10; break;
		case 'B': temp = 11; break;
		case 'C': temp = 12; break;
		case 'D': temp = 13; break;
		case 'E': temp = 14; break;
		case 'F': temp = 15; break;
		default: temp = strHex[i] - '0'; break;
		}
		nDec += (temp*pow(16, len - 1 - i));
	}

	char strTmp[32];
	sprintf(strTmp, "%I64d", nDec);

	return 0;
}

//
bool cutSubStr(char *str, const char* subStr)
{
	//过滤0x标志
	if (!str || !subStr)
	{
		return false;
	}

	char tmp[32] = { 0 };
	char strBuf[32] = { 0 };
	if (strstr(str, "0x"))
	{
		strcpy(strBuf, strstr(str, "0x") + 2);
		memset(str, 0, sizeof(str));
		strcpy(str, strBuf);
		return true;
	}

	return true;
}

bool getHex(char* m_fpga_reg,UINT32 *retInt)
{
	if (!m_fpga_reg)
	{
		return false;
	}
	if (cutSubStr(m_fpga_reg, "0x"))
	{
		//判断是否为十六进制格式数值
		if (DetermWhIsHEX(m_fpga_reg) == 0)
		{
			*retInt = _strtoui64(m_fpga_reg, NULL, 16);
			return TRUE;
		}
	}

	return FALSE;
}


bool saveInitArg(void* puser)
{
	//读取数据控件的数据，做成json数据，存储在配置文件中
	//CClient_UDP_MFCDlg *pp = (CClient_UDP_MFCDlg*)puser;

	// 	pp->m_url = (CEdit*)GetDlgItem(IDC_FILENAME);
	// 	pp->m_ip = (CEdit*)GetDlgItem(IDC_EDIT_IP);
	// 	pp->m_EDIT_ipLocal = (CEdit*)GetDlgItem(IDC_EDIT_IP2);
	// 	pp->m_port_Edit_Ctrl = (CEdit *)GetDlgItem(IDC_EDIT_PORT);
	// 	pp->m_port_Edit_Data = (CEdit *)GetDlgItem(IDC_EDIT_PORT2);

	//GetWindowTextA(*pp->m_url, (LPSTR)pp->m_filename, 128);

	// 	GetWindowTextA(*pp->m_ip, (LPSTR)pp->m_initMsg.local_ip, 16);
	// 	GetWindowTextA(*pp->m_EDIT_ipLocal, (LPSTR)pp->m_initMsg.local_ip, 16);
	// 
	// 	GetWindowTextA(*pp->m_port_Edit_Ctrl, (LPSTR)pp->m_initMsg., 12);
	// 	GetWindowTextA(*pp->m_port_Edit_Data, (LPSTR)pp->m_initMsg, 12);
	// 
	// 	GetWindowTextA(*pp->m_CEdit_ip_adc, (LPSTR)pp->m_initMsg, 12);
	// 	GetWindowTextA(*pp->m_CEdit_port_adc, (LPSTR)pp->m_initMsg, 12);
	// 
	// 	GetWindowTextA(*pp->m_CEdit_fpga_reg, (LPSTR)pp->m_initMsg, 12);
	// 	GetWindowTextA(*pp->m_CEdit_fpga_val, (LPSTR)pp->m_initMsg, 12);
	// 
	// 	GetWindowTextA(*pp->m_CEdit_adc_channel, (LPSTR)pp->m_initMsg, 12);
	// 	GetWindowTextA(*pp->m_CEdit_adc_size, (LPSTR)pp->m_initMsg, 12);
	// 	GetWindowTextA(*pp->m_CEdit_adc_addr, (LPSTR)pp->m_initMsg, 12);
	// 	GetWindowTextA(*pp->m_CEdit_adc_data, (LPSTR)pp->m_initMsg, 12);

	char str[1024] = { 0 };
	char ip[32] = "192.168.1.12";
	insertJsonStr(str, "local_ip", "192.168.1.18");
	insertJsonStr(str, "local_port", "1003");
	insertJsonStr(str, "msg_ip", "192.168.1.12");

	return true;
}


bool setInit(CListBox *m_listbox, stInitMsg *init, void* puser)
{
	//CClient_UDP_MFCDlg *pp = (CClient_UDP_MFCDlg*)puser;

	FILE *fd = fopen("test.json", "rb");

	char readBuf[1024] = { 0 };
	char retStr[32] = { 0 };
	int retInt = 0;
	int nu = fread(readBuf, 1, 1024, fd);

	getJsStr(readBuf, "local_ip", retStr);
	showCurLineChar(m_listbox, retStr);

	getJsStr(readBuf, "local_port", retStr);
	showCurLineChar(m_listbox, retStr);

	getJsStr(readBuf, "msg_ip", retStr);
	showCurLineChar(m_listbox, retStr);
	getJsStr(readBuf, "msg_port", retStr);
	showCurLineChar(m_listbox, retStr);
	getJsStr(readBuf, "config_ip", retStr);
	showCurLineChar(m_listbox, retStr);
	getJsStr(readBuf, "config_port", retStr);
	showCurLineChar(m_listbox, retStr);
	getJsStr(readBuf, "fpga_reg", retStr);
	showCurLineChar(m_listbox, retStr);
	getJsStr(readBuf, "adc_channel", retStr);
	showCurLineChar(m_listbox, retStr);
	getJsStr(readBuf, "adc_size", retStr);
	showCurLineChar(m_listbox, retStr);
	getJsStr(readBuf, "adc_addr", retStr);
	showCurLineChar(m_listbox, retStr);
	getJsStr(readBuf, "adc_data", retStr);
	showCurLineChar(m_listbox, retStr);


	return true;
}

//add 2019年6月27日14:09:59
void No_Cursor()
{
	HANDLE hOut;
	hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cci;
	cci.dwSize = 1;
	cci.bVisible = false;
	SetConsoleCursorInfo(hOut, &cci);
}

void gotoxy(short x, short y)
{
	COORD pos = { x, y };
	HANDLE hOut = GetStdHandle(STD_OUTPUT_HANDLE);
	SetConsoleCursorPosition(hOut, pos);
	No_Cursor();
}

bool getCurTime(char *timeBuf)
{
	time_t t;
	struct tm *data_time = new tm;

	gotoxy(0, 0);
	time(&t);
	localtime_s(data_time, &t);

	sprintf(timeBuf, "%d:%d:%d", data_time->tm_hour, data_time->tm_min, data_time->tm_sec);

	delete data_time;
	return true;
}
//==2019年6月27日14:10:24