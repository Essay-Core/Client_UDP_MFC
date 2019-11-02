
// Client_UDP_MFCDlg.h : ͷ�ļ�
//

#pragma once
#include "C_udp_client.h"
#include "C_SingleCircuList.h"
#include "st_MSG.h"
#include "F_cjson.h"

#include "resource.h"
#include <thread>
#include <windows.h>
#include <stdio.h>
#include "afxcmn.h"
#include <math.h>  
#include "afxwin.h"

#include <vector>

using namespace std;

// CClient_UDP_MFCDlg �Ի���
class CClient_UDP_MFCDlg : public CDialogEx
{
// ����
public:
	CClient_UDP_MFCDlg(CWnd* pParent = NULL);	// ��׼���캯��
	~CClient_UDP_MFCDlg();
// �Ի�������
	enum { IDD = IDD_CLIENT_UDP_MFC_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV ֧��

// ʵ��  
protected:
	HICON m_hIcon;

	// ���ɵ���Ϣӳ�亯��
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()

public:
	afx_msg void OnBnClickedOpenfile();
	afx_msg void OnBnClickedButtonStart();
	afx_msg void OnBnClickedButtonEnd();

	afx_msg void OnBnClickedButtonBind();

	afx_msg void OnBnClickedButtonAdcWrite();
	afx_msg void OnBnClickedButtonAdcRead();

	afx_msg void OnBnClickedButtonFpgaWrite();
	afx_msg void OnBnClickedButtonFpgaRead();

	afx_msg void OnBnClickedButtonListClear();
	
	
public:
	//�༭��
	CEdit *m_url;
	CEdit *m_ip; //msg_ip
	CEdit *m_EDIT_ipLocal; //����ip
	CEdit *m_port_Edit_Ctrl;
	CEdit *m_port_Edit_Data;

	CEdit *m_CEdit_ip_adc;  //����IP config_ip
	CEdit *m_CEdit_port_adc;

	//fpga
	CEdit *m_CEdit_fpga_reg;
	CEdit *m_CEdit_fpga_val;

	//adc
	CEdit *m_CEdit_adc_channel;
	CEdit *m_CEdit_adc_size;
	CEdit *m_CEdit_adc_addr;
	CEdit *m_CEdit_adc_data;
	//CIP   *m_Ctrl_ip;

	//list
	CListBox *m_listbox;

	//��Ϣ�ṹ��
	stHeadMsg m_msg;
	stAdcConfig m_adcConfig;
	stFpgaConfig m_fpgaConfig;
	stHeadFpgaConfig m_headFpgaConfig;
	
	stInitMsg m_initMsg;

	char m_filename[260];
	char m_IP[16];
	char m_IPLocal[16];
	char m_port_ctrl[12];
	char m_port_data[12];

	char m_ip_adc[16];
	char m_port_adc[12];

	char m_adc_channel[12];
	char m_adc_size[12];
	char m_adc_addr[12];
	char m_adc_data[12];

	char m_fpga_reg[32];
	char m_fpga_val[32];

	//�߳�
	CWinThread *m_pThreadSend;
	CWinThread *m_pThreadCtrl;
	CWinThread *m_pThreadWrite;

	thread m_th_data; //��������
	thread m_th_ctrl;	//���Ϳ��ƣ���ʼ����ָ��
	thread m_th_write;	//д�ļ�

	//����getlasterror()
	DWORD m_dwErr;

	//sock
	SOCKET m_sock_sig;
	SOCKET m_sock_adc;
	SOCKET sock;

	sockaddr_in m_sAddr_adc;
	sockaddr_in m_servAddr_sig;
	sockaddr_in m_servAddr_data;

	//���Ʒ��͵�״̬
	int m_status;	

	//��ر�־
	bool m_th_flags;  //�Ƿ񴴽��߳�
	bool start_status; //��ʼ�շ���־  
	bool m_click_Stop_th;  //����˳��̱߳�־

	//����
	PQUEUE m_Q;

	//�ļ�
	FILE *m_fd;

	int m_NumOfAccPack;
	UINT32 m_NumOfWritePack;

	CEdit *m_CEdit_write_count;
	UINT32 m_writeFile_countCtrl;
	UINT32 m_dequeue_count; //������Ӱ���
	UINT32 m_zeroBuf[340]; //�ļ���0
	
};

