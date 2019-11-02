
// Client_UDP_MFCDlg.cpp : ʵ���ļ�
//

#include "stdafx.h"
#include "Client_UDP_MFC.h"
#include "Client_UDP_MFCDlg.h"
#include "afxdialogex.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// ����Ӧ�ó��򡰹��ڡ��˵���� CAboutDlg �Ի���

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// �Ի�������
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV ֧��

// ʵ��
protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(CAboutDlg::IDD)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CClient_UDP_MFCDlg �Ի���
CClient_UDP_MFCDlg::CClient_UDP_MFCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClient_UDP_MFCDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_Q = new QUEUE;

	WSADATA wsd = { 0 };
	WSAStartup(0x0202, &wsd);

	start_status = false; //��ʼ״̬Ϊ�٣�δ��ʼ
}

CClient_UDP_MFCDlg::~CClient_UDP_MFCDlg()
{
	WSACleanup();
	if (m_Q->pBase)
	{
		free(m_Q->pBase);
	}

	if (m_Q)
	{
		delete m_Q;
	}

	//�˳��߳�
	if (m_click_Stop_th == true)
	{
		m_click_Stop_th = false;
		if (m_th_write.joinable())
		{
			m_th_write.join();
		}

		if (m_th_ctrl.joinable())
		{
			m_th_ctrl.join();
		}

		if (m_th_data.joinable())
		{
			m_th_data.join();
		}
	}

}

void CClient_UDP_MFCDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	if (sock)
	{
		closesocket(sock);
	}
	if (m_sock_sig)
	{
		closesocket(m_sock_sig);
	}
	
}

BEGIN_MESSAGE_MAP(CClient_UDP_MFCDlg, CDialogEx)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_OPENFILE, &CClient_UDP_MFCDlg::OnBnClickedOpenfile)
	ON_BN_CLICKED(IDC_BUTTON_START, &CClient_UDP_MFCDlg::OnBnClickedButtonStart)
	ON_BN_CLICKED(IDC_BUTTON_END, &CClient_UDP_MFCDlg::OnBnClickedButtonEnd)

	//ON_BN_CLICKED(IDC_BUTTON_ADC_BIND, &CClient_UDP_MFCDlg::OnBnClickedButtonAdcBind)
	ON_BN_CLICKED(IDC_BUTTON_LIST_CLEAR, &CClient_UDP_MFCDlg::OnBnClickedButtonListClear)

	ON_BN_CLICKED(IDC_BUTTON_FPGA_WRITE, &CClient_UDP_MFCDlg::OnBnClickedButtonFpgaWrite)
	ON_BN_CLICKED(IDC_BUTTON_FPGA_READ, &CClient_UDP_MFCDlg::OnBnClickedButtonFpgaRead)

	ON_BN_CLICKED(IDC_BUTTON_ADC_WRITE, &CClient_UDP_MFCDlg::OnBnClickedButtonAdcWrite)
	ON_BN_CLICKED(IDC_BUTTON_ADC_READ, &CClient_UDP_MFCDlg::OnBnClickedButtonAdcRead)
	ON_BN_CLICKED(IDC_BUTTON_BIND, &CClient_UDP_MFCDlg::OnBnClickedButtonBind)
END_MESSAGE_MAP()


// CClient_UDP_MFCDlg ��Ϣ�������

BOOL CClient_UDP_MFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// ��������...���˵�����ӵ�ϵͳ�˵��С�

	// IDM_ABOUTBOX ������ϵͳ���Χ�ڡ�
	ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
	ASSERT(IDM_ABOUTBOX < 0xF000);

	CMenu* pSysMenu = GetSystemMenu(FALSE);
	if (pSysMenu != NULL)
	{
		BOOL bNameValid;
		CString strAboutMenu;
		bNameValid = strAboutMenu.LoadString(IDS_ABOUTBOX);
		ASSERT(bNameValid);
		if (!strAboutMenu.IsEmpty())
		{
			pSysMenu->AppendMenu(MF_SEPARATOR);
			pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
		}
	}

	// ���ô˶Ի����ͼ�ꡣ  ��Ӧ�ó��������ڲ��ǶԻ���ʱ����ܽ��Զ�
	//  ִ�д˲���
	SetIcon(m_hIcon, TRUE);			// ���ô�ͼ��
	SetIcon(m_hIcon, FALSE);		// ����Сͼ��

	// TODO:  �ڴ���Ӷ���ĳ�ʼ������

	//��ȡ��ִ���ļ�·������ʾ 2019��6��19��13:53:08
	memset(m_filename, 0, 260);
	GetModuleDir(m_filename);



	//add 2019��7��5��10:58:52
	FILE *f = fopen("config.ini", "r");
	char freadStr[256] = { 0 };
	fread(freadStr, 1, 256, f);
	fclose(f);
	stIpPortInitMsg initmsg;
	getJsStr(freadStr, "ip_romote", initmsg.ip_romote);
	getJsStr(freadStr, "ip_local", initmsg.ip_local);

	getJsInt(freadStr, "port_ctrl", &initmsg.port_ctrl);
	getJsInt(freadStr, "port_data", &initmsg.port_data);
	getJsInt(freadStr, "port_config", &initmsg.port_config);

	USES_CONVERSION;
	SetDlgItemText(IDC_FILENAME, A2CW(W2A(CString(m_filename))));
	SetDlgItemInt(IDC_EDIT_WRITE_COUNT, MAX_WRITE_FILE_PACK);

	SetDlgItemText(IDC_EDIT_IP, A2CW(W2A(CString(initmsg.ip_romote))));
	SetDlgItemText(IDC_EDIT_IP2, A2CW(W2A(CString(initmsg.ip_local))));

	SetDlgItemInt(IDC_EDIT_PORT, initmsg.port_ctrl);
	SetDlgItemInt(IDC_EDIT_PORT2, initmsg.port_data);
	SetDlgItemInt(IDC_EDIT_PORT_ADC, initmsg.port_config);

	//==2019��7��5��10:58:52

	//fpgaĬ����ʾ
	SetDlgItemText(IDC_EDIT_FPGA_REG, _T(FPGA_REG));	
	SetDlgItemText(IDC_EDIT_FPGA_VAL, _T(FPGA_VAL));

	//ADCĬ����ʾ
	SetDlgItemText(IDC_EDIT_ADC_CHANNEL, _T(ADC_CAHNNEL));	//ADC
	SetDlgItemText(IDC_EDIT_ADC_SIZE, _T(ADC_SIZE));	//ADC
	SetDlgItemText(IDC_EDIT_ADC_ADDR, _T(ADC_ADDR));	//ADC
	SetDlgItemText(IDC_EDIT_ADC_DATA, _T(ADC_DATA));	//ADC

	//�б�
	m_listbox = (CListBox*)GetDlgItem(IDC_LIST_STATUS);

	//adc 
	m_CEdit_adc_channel = (CEdit*)GetDlgItem(IDC_EDIT_ADC_CHANNEL);
	m_CEdit_adc_size = (CEdit*)GetDlgItem(IDC_EDIT_ADC_SIZE);
	m_CEdit_adc_addr = (CEdit*)GetDlgItem(IDC_EDIT_ADC_ADDR);
	m_CEdit_adc_data = (CEdit*)GetDlgItem(IDC_EDIT_ADC_DATA);

	//fpga
	m_CEdit_fpga_reg = (CEdit*)GetDlgItem(IDC_EDIT_FPGA_REG);
	m_CEdit_fpga_val = (CEdit*)GetDlgItem(IDC_EDIT_FPGA_VAL);

	//��������
	CreateQueue(m_Q, MAX_NODE);
	
	//ʹ�߳̿ɴ���
	m_th_flags = false;

	return TRUE;  // ���ǽ��������õ��ؼ������򷵻� TRUE
}

void CClient_UDP_MFCDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialogEx::OnSysCommand(nID, lParam);
	}
}

// �����Ի��������С����ť������Ҫ����Ĵ���
//  �����Ƹ�ͼ�ꡣ  ����ʹ���ĵ�/��ͼģ�͵� MFC Ӧ�ó���
//  �⽫�ɿ���Զ���ɡ�

void CClient_UDP_MFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // ���ڻ��Ƶ��豸������

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// ʹͼ���ڹ����������о���
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// ����ͼ��
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//���û��϶���С������ʱϵͳ���ô˺���ȡ�ù��
//��ʾ��
HCURSOR CClient_UDP_MFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CClient_UDP_MFCDlg::OnBnClickedOpenfile()
{
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	CString m_strFilePath = _T("./");
	CEdit* pBoxOne;

	CString filepath;
	CFileDialog dlg(TRUE, NULL, NULL, NULL);
	//��ͼ��
	if (dlg.DoModal() == IDOK)
	{
		//��ȡͼ���ļ���
		m_strFilePath = dlg.GetPathName();

		pBoxOne = (CEdit*)GetDlgItem(IDC_FILENAME);
		pBoxOne->SetWindowText(m_strFilePath);

		showCurLine(m_listbox, _T("Modified file save path successfully!"));
		showCurLine(m_listbox, m_strFilePath);
	}
	else
	{
		return;
	}

}


void CClient_UDP_MFCDlg::OnBnClickedButtonStart()
{
	if (start_status)
	{
		AfxMessageBox(_T("�ѿ�ʼ������ֹͣ��ť"));
		return;
	}

	//��ն���
	Clequeue(m_Q);
	
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	//��ȡ�ؼ��Ĳ������
	m_url = (CEdit*)GetDlgItem(IDC_FILENAME);
	m_ip = (CEdit*)GetDlgItem(IDC_EDIT_IP);
	m_EDIT_ipLocal = (CEdit*)GetDlgItem(IDC_EDIT_IP2);
	m_port_Edit_Ctrl = (CEdit *)GetDlgItem(IDC_EDIT_PORT);
	m_port_Edit_Data = (CEdit *)GetDlgItem(IDC_EDIT_PORT2);

	GetWindowTextA(*m_url, (LPSTR)m_filename, 128);

	GetWindowTextA(*m_ip, (LPSTR)m_IP, 16);
	GetWindowTextA(*m_EDIT_ipLocal, (LPSTR)m_IPLocal, 16);

	GetWindowTextA(*m_port_Edit_Ctrl, (LPSTR)m_port_ctrl, 12);
	GetWindowTextA(*m_port_Edit_Data, (LPSTR)m_port_data, 12);

	//�����ʼʱ�����³�ʼ���׽���
	if (sock)
	{
		closesocket(sock); //�ȹرպ����´����׽���
	}

	if (!init_sock(&sock, &m_servAddr_data, m_IPLocal, m_port_data, true)) //�󶨱���IP �˿ڣ���������
	{
		return;
	}

	//2019��6��25��11:49:54
	m_CEdit_write_count = (CEdit *)GetDlgItem(IDC_EDIT_WRITE_COUNT);
	m_writeFile_countCtrl = GetDlgItemInt(IDC_EDIT_WRITE_COUNT);
// 	if (m_writeFile_countCtrl <=0)
// 	{
// 		AfxMessageBox(_T("����д����0������Ϊд���ļ�����"));
// 		return;
// 	}

	setTimeOut(&sock); //���ý��ճ�ʱ��ʱ��Ϊ3��

	m_msg.msg_id = SOCK_MSG_START_GET_DATA;  //���Ƶģ� ���Ϳ�ʼ�����ź�
	start_status = true;	//�ѿ�ʼ�� �ȴ����������ť�ǲſ�������	
	m_click_Stop_th = false; //���������̣߳���Ϊ�٣��ɽ����߳�ѭ��
	m_NumOfAccPack = 0; //���µ����ʼ�����¼���
	m_NumOfWritePack = 0; //��ʼ��д���ļ�����
	m_dequeue_count = 0; //��ʼ��������

	//�߳�ֻ��Ҫ����һ�ξͺ��ˣ�����Ҫ���ٻ����´������ȵ������˳����˳��߳�
	if (!m_th_flags)
	{
		m_th_ctrl = thread(th_send_commend_new, this);
		m_th_data = thread(th_recv_data_new, this);
		m_th_write = thread(th_write_data_new, this);

		m_th_flags = TRUE; //�����̳߳ɹ������ڴ�����ֻ�ڳ����˳�ʱ�˳�
	}
}


void CClient_UDP_MFCDlg::OnBnClickedButtonEnd()
{

	m_msg.msg_id = SOCK_MSG_STOP_GET_DATA;  //ֹͣ��Ϣ
	start_status = false; //ֹͣ��������

	Clequeue(m_Q);  

	if (sock > 0)
	{
		closesocket(sock);
	}

}


//adc write  //FPGA_OPERA_ADC_WRITE
void CClient_UDP_MFCDlg::OnBnClickedButtonAdcWrite()
{
	if (!ifSocketTrue(&m_sock_adc))
	{
		m_CEdit_ip_adc = (CEdit *)GetDlgItem(IDC_EDIT_IP);
		m_CEdit_port_adc = (CEdit *)GetDlgItem(IDC_EDIT_PORT_ADC);

		GetWindowTextA(*m_CEdit_ip_adc, (LPSTR)m_ip_adc, 16);
		GetWindowTextA(*m_CEdit_port_adc, (LPSTR)m_port_adc, 12);

		//��ʼ��socket������
		if (!init_sock(&m_sock_adc, &m_sAddr_adc, m_ip_adc, m_port_adc, false))
		{
			showCurLine(m_listbox, _T("bind IP��PORT false!!"));

		}

		setTimeOut(&m_sock_adc);

// 		char ip_port_buf[64] = { 0 };
// 		sprintf(ip_port_buf, "bind ip:%s, port:%s", m_ip_adc, m_port_adc);
// 		showCurLineChar(m_listbox, ip_port_buf);
		
	}

	memset((char*)&m_adcConfig, 0, sizeof(stAdcConfig));
	
	////2019��6��20��16:41:17
	GetWindowTextA(*m_CEdit_adc_channel, (LPSTR)m_adc_channel, 12);
	GetWindowTextA(*m_CEdit_adc_size, (LPSTR)m_adc_size, 12);
	GetWindowTextA(*m_CEdit_adc_addr, (LPSTR)m_adc_addr, 12);
	GetWindowTextA(*m_CEdit_adc_data, (LPSTR)m_adc_data, 12);

	if (!getHex(m_adc_channel, &m_adcConfig.channel))
	{
		return;
	}
	if (!getHex(m_adc_size, &m_adcConfig.size))
	{
		return;
	}
	if (!getHex(m_adc_addr, &m_adcConfig.addr))
	{
		return;
	}
	if (!getHex(m_adc_data, &m_adcConfig.data))
	{
		return;
	}

	memset((char*)&m_headFpgaConfig, 0, sizeof(stHeadFpgaConfig));
	m_headFpgaConfig.opera = FPGA_OPERA_ADC_WRITE;
	memcpy((char*)&m_headFpgaConfig.dada, (char*)&m_adcConfig, sizeof(stAdcConfig));

	int send_count = sendto(m_sock_adc, (char*)&m_headFpgaConfig, sizeof(stHeadFpgaConfig), 0, (sockaddr*)&m_sAddr_adc, sizeof(sockaddr));
	if (send_count < 0)
	{
		showCurLine(m_listbox, _T("send to m_sock_adc false!"));

		return;
	}
	stHeadFpgaConfig cfg = { 0 };
	int saLen = sizeof(sockaddr);
	int recv_count = recvfrom(m_sock_adc, (char*)&cfg, sizeof(stHeadFpgaConfig), 0, (sockaddr*)&m_sAddr_adc, &saLen);
	if (recv_count < 0)
	{
		showCurLine(m_listbox, _T("recvfrom to m_sock_adc false!"));
		m_dwErr = GetLastError();
		return;
	}

	if (cfg.opera == FPGA_OPERA_ADC_WRITE)
	{
		showCurLine(m_listbox, _T("adc write send success!"));

	}

}

//adc read
void CClient_UDP_MFCDlg::OnBnClickedButtonAdcRead()
{
	if (!ifSocketTrue(&m_sock_adc))
	{
//		m_CEdit_ip_adc = (CEdit *)GetDlgItem(IDC_EDIT_IP_ADC);
		m_CEdit_port_adc = (CEdit *)GetDlgItem(IDC_EDIT_PORT_ADC);

		GetWindowTextA(*m_CEdit_ip_adc, (LPSTR)m_ip_adc, 16);
		GetWindowTextA(*m_CEdit_port_adc, (LPSTR)m_port_adc, 12);

		//��ʼ��socket������
		if (!init_sock(&m_sock_adc, &m_sAddr_adc, m_ip_adc, m_port_adc, false))
		{
			showCurLine(m_listbox, _T("bind IP��PORT false!!"));

		}
		setTimeOut(&m_sock_adc);

// 		char ip_port_buf[64] = { 0 };
// 		sprintf(ip_port_buf, "bind ip:%s, port:%s", m_ip_adc, m_port_adc);
		//showCurLineChar(m_listbox, ip_port_buf);
	}

	GetWindowTextA(*m_CEdit_adc_channel, (LPSTR)m_adc_channel, 12);
	GetWindowTextA(*m_CEdit_adc_size, (LPSTR)m_adc_size, 12);
	GetWindowTextA(*m_CEdit_adc_addr, (LPSTR)m_adc_addr, 12);
	GetWindowTextA(*m_CEdit_adc_data, (LPSTR)m_adc_data, 12);

	memset(&m_adcConfig, 0, sizeof(stAdcConfig));
	if (!getHex(m_adc_channel, &m_adcConfig.channel))
	{
		return;
	}
	if (!getHex(m_adc_size, &m_adcConfig.size))
	{
		return;
	}
	if (!getHex(m_adc_addr, &m_adcConfig.addr))
	{
		return;
	}
	if (!getHex(m_adc_data, &m_adcConfig.data))
	{
		return;
	}

	memset((char*)&m_headFpgaConfig, 0, sizeof(stHeadFpgaConfig));
	memcpy((char*)m_headFpgaConfig.dada, (char*)&m_adcConfig, sizeof(stAdcConfig));
	//��������־
	m_headFpgaConfig.opera = FPGA_OPERA_ADC_READ;


	int send_count = sendto(m_sock_adc, (char*)&m_headFpgaConfig, sizeof(stHeadFpgaConfig), 0, (sockaddr*)&m_sAddr_adc, sizeof(sockaddr));
	if (send_count < 0)
	{
		showCurLine(m_listbox, _T("send to adc read false!"));

		return;
	}

	stHeadFpgaConfig cfg = { 0 };
	stAdcConfig *adc_cfg = NULL;
	int saLen = sizeof(sockaddr);
	int recv_count = recvfrom(m_sock_adc, (char*)&cfg, sizeof(stHeadFpgaConfig), 0, (sockaddr*)&m_sAddr_adc, &saLen);
	if (recv_count == SOCKET_ERROR)
	{
		showCurLine(m_listbox, _T("recvfrom to m_sock_adc false!"));
		m_dwErr = GetLastError();
		return;
	}

	if (cfg.opera == FPGA_OPERA_ADC_READ)
	{	
		adc_cfg = (stAdcConfig *)cfg.dada;

		USES_CONVERSION;
		SetDlgItemText(IDC_EDIT_ADC_CHANNEL, A2CW(W2A(intToLpctstrHex(adc_cfg->channel))));
		SetDlgItemText(IDC_EDIT_ADC_SIZE, A2CW(W2A(intToLpctstrHex(adc_cfg->size))));
		SetDlgItemText(IDC_EDIT_ADC_ADDR, A2CW(W2A(intToLpctstrHex(adc_cfg->addr))));
		SetDlgItemText(IDC_EDIT_ADC_DATA, A2CW(W2A(intToLpctstrHex(adc_cfg->data))));

		showCurLine(m_listbox, _T("adc read ok!"));

	}
	else
	{
		showCurLine(m_listbox, _T("adc read recv error!!"));
	}

}

//fpga write FPGA_OPERA_WRITE
void CClient_UDP_MFCDlg::OnBnClickedButtonFpgaWrite()
{
	//2019��6��26��09:43:37
	//�ж��׽����Ƿ�Ϊ��
	//��Ϊ�ٵ�ʱ����Ҫ��ȡ�ؼ���ip�Ͷ˿ں�
	if (!ifSocketTrue(&m_sock_adc))
	{
//		m_CEdit_ip_adc = (CEdit *)GetDlgItem(IDC_EDIT_IP_ADC);
		m_CEdit_port_adc = (CEdit *)GetDlgItem(IDC_EDIT_PORT_ADC);

		GetWindowTextA(*m_CEdit_ip_adc, (LPSTR)m_ip_adc, 16);
		GetWindowTextA(*m_CEdit_port_adc, (LPSTR)m_port_adc, 12);

		//��ʼ��socket������
		if (!init_sock(&m_sock_adc, &m_sAddr_adc, m_ip_adc, m_port_adc, false))
		{
			showCurLine(m_listbox, _T("bind IP��PORT false!!"));

		}
		setTimeOut(&m_sock_adc);//���ó�ʱ

// 		char ip_port_buf[64] = { 0 };
// 		sprintf(ip_port_buf, "bind ip:%s, port:%s", m_ip_adc, m_port_adc);
// 		showCurLineChar(m_listbox, ip_port_buf);
		
	}

	memset((char*)&m_fpgaConfig, 0, sizeof(stFpgaConfig));
	memset((char*)&m_headFpgaConfig, 0, sizeof(stHeadFpgaConfig));
	
	m_CEdit_fpga_reg = (CEdit *)GetDlgItem(IDC_EDIT_FPGA_REG);
	m_CEdit_fpga_val = (CEdit *)GetDlgItem(IDC_EDIT_FPGA_VAL);

	GetWindowTextA(*m_CEdit_fpga_reg, (LPSTR)m_fpga_reg, 32);
	GetWindowTextA(*m_CEdit_fpga_val, (LPSTR)m_fpga_val, 32);

	//�ж�����ʮ��������ֵ
	if (!getHex(m_fpga_reg, &m_fpgaConfig.reg))
	{
		return;
	}

	if (!getHex(m_fpga_val, &m_fpgaConfig.val))
	{
		return;
	}

	m_headFpgaConfig.opera = FPGA_OPERA_WRITE;
	memcpy(m_headFpgaConfig.dada, (char*)&m_fpgaConfig, sizeof(stFpgaConfig));

	stHeadFpgaConfig hcfg = { 0 };
	int saLen = sizeof(sockaddr);  

	int send_count = sendto(m_sock_adc, (char*)&m_headFpgaConfig, sizeof(stHeadFpgaConfig), 0, (sockaddr*)&m_sAddr_adc, sizeof(sockaddr));
	if (send_count < 0)
	{
		showCurLine(m_listbox, _T("send to FPGA write false!"));
		m_dwErr = GetLastError();
		return;
	}
	showCurLine(m_listbox, _T("fpga send success!"));

	int recv_count = recvfrom(m_sock_adc, (char*)&hcfg, sizeof(stHeadFpgaConfig), 0, (sockaddr*)&m_sAddr_adc, &saLen);
	if (recv_count == SOCKET_ERROR)
	{
		showCurLine(m_listbox, _T("recvfrom to m_sock_adc false!"));
		m_dwErr = GetLastError();
		return;
	}

	//�ж��յ����뷢�͵���ͬ����ʾ�ɹ�
	if (hcfg.opera == FPGA_OPERA_WRITE)
	{
		showCurLine(m_listbox, _T("fpga write sueecss!"));
	}

}

//fpga read
void CClient_UDP_MFCDlg::OnBnClickedButtonFpgaRead()
{
	if (!ifSocketTrue(&m_sock_adc))
	{
//		m_CEdit_ip_adc = (CEdit *)GetDlgItem(IDC_EDIT_IP_ADC);
		m_CEdit_port_adc = (CEdit *)GetDlgItem(IDC_EDIT_PORT_ADC);

		GetWindowTextA(*m_CEdit_ip_adc, (LPSTR)m_ip_adc, 16);
		GetWindowTextA(*m_CEdit_port_adc, (LPSTR)m_port_adc, 12);

		//��ʼ��socket������
		if (!init_sock(&m_sock_adc, &m_sAddr_adc, m_ip_adc, m_port_adc, false))
		{
			showCurLine(m_listbox, _T("bind IP��PORT false!!"));

		}
		setTimeOut(&m_sock_adc);

// 		char ip_port_buf[64] = { 0 };
// 		sprintf(ip_port_buf, "bind ip:%s, port:%s", m_ip_adc, m_port_adc);
// 		showCurLineChar(m_listbox, ip_port_buf);
// 	
	}
  
	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	memset((char*)&m_headFpgaConfig, 0, sizeof(stHeadFpgaConfig));
	memset((char*)&m_fpgaConfig, 0, sizeof(stFpgaConfig));

	//��������־
	m_CEdit_fpga_reg = (CEdit *)GetDlgItem(IDC_EDIT_FPGA_REG);

	GetWindowTextA(*m_CEdit_fpga_reg, (LPSTR)m_fpga_reg, 32);

	//�ж�����ʮ��������ֵ
	if (!getHex(m_fpga_reg, &m_fpgaConfig.reg))
	{
		return;
	}

	m_headFpgaConfig.opera = FPGA_OPERA_READ;
	memcpy((char*)&m_headFpgaConfig.dada, (char*)&m_fpgaConfig, sizeof(stFpgaConfig));
	int send_count = sendto(m_sock_adc, (char*)&m_headFpgaConfig, sizeof(stHeadFpgaConfig), 0, (sockaddr*)&m_sAddr_adc, sizeof(sockaddr));
	if (send_count < 0)
	{
		showCurLine(m_listbox, _T("send to FPGA read false!"));
		return;
	}

	stHeadFpgaConfig cfg = { 0 };
	stFpgaConfig *fpga_cfg = NULL;
	int saLen = sizeof(sockaddr);

	int recv_count = recvfrom(m_sock_adc, (char*)&cfg, sizeof(stHeadFpgaConfig), 0, (sockaddr*)&m_sAddr_adc, &saLen);
	if (recv_count == SOCKET_ERROR)
	{
		showCurLine(m_listbox, _T("recvfrom to m_sock_adc false"));
		m_dwErr = GetLastError();
		return;
	}

	if (cfg.opera == FPGA_OPERA_READ)
	{

		fpga_cfg = (stFpgaConfig *)cfg.dada;
		
		USES_CONVERSION;
		SetDlgItemText(IDC_EDIT_FPGA_REG, A2CW(W2A(intToLpctstrHex(fpga_cfg->reg))));
		SetDlgItemText(IDC_EDIT_FPGA_VAL, A2CW(W2A(intToLpctstrHex(fpga_cfg->val))));
	
		showCurLine(m_listbox, _T("fpga read ok!"));
	}
}

//���list
void CClient_UDP_MFCDlg::OnBnClickedButtonListClear()
{	
	USES_CONVERSION;

	SetDlgItemText(IDC_EDIT_FPGA_REG, A2CW(W2A(intToLpctstrHex(0))));
	SetDlgItemText(IDC_EDIT_FPGA_VAL, A2CW(W2A(intToLpctstrHex(0))));

	SetDlgItemText(IDC_EDIT_ADC_CHANNEL, A2CW(W2A(intToLpctstrHex(0))));
	SetDlgItemText(IDC_EDIT_ADC_SIZE, A2CW(W2A(intToLpctstrHex(0))));
	SetDlgItemText(IDC_EDIT_ADC_ADDR, A2CW(W2A(intToLpctstrHex(0))));
	SetDlgItemText(IDC_EDIT_ADC_DATA, A2CW(W2A(intToLpctstrHex(0))));

	m_listbox->ResetContent();
}


void CClient_UDP_MFCDlg::OnBnClickedButtonBind()
{
 	SetDlgItemText(IDC_BUTTON_BIND, _T("���°�"));

	// TODO:  �ڴ���ӿؼ�֪ͨ����������
	m_CEdit_ip_adc = (CEdit *)GetDlgItem(IDC_EDIT_IP);//ɾ���ؼ� IDC_EDIT_IP_ADC
	m_CEdit_port_adc = (CEdit *)GetDlgItem(IDC_EDIT_PORT_ADC);

	GetWindowTextA(*m_CEdit_ip_adc, (LPSTR)m_ip_adc, 16);
	GetWindowTextA(*m_CEdit_port_adc, (LPSTR)m_port_adc, 12);

	//��ʼ��socket������
	if (!init_sock(&m_sock_adc, &m_sAddr_adc, m_ip_adc, m_port_adc, false))
	{
		showCurLine(m_listbox, _T("bind IP��PORT false!!"));

	}
	//showCurLine(m_listbox, _T("bind IP��PORT success!!"));

	//2019��7��2��10:28:12
	char ip_port_buf[64] = { 0 };
	sprintf(ip_port_buf, "bind success ip:%s, port:%s", m_ip_adc, m_port_adc);
	//showCurLineChar(m_listbox, ip_port_buf);

	m_ip = (CEdit*)GetDlgItem(IDC_EDIT_IP);
	m_EDIT_ipLocal = (CEdit*)GetDlgItem(IDC_EDIT_IP2);
	m_port_Edit_Ctrl = (CEdit *)GetDlgItem(IDC_EDIT_PORT);
	m_port_Edit_Data = (CEdit *)GetDlgItem(IDC_EDIT_PORT2);

	GetWindowTextA(*m_url, (LPSTR)m_filename, 128);

	GetWindowTextA(*m_ip, (LPSTR)m_IP, 16);
	GetWindowTextA(*m_EDIT_ipLocal, (LPSTR)m_IPLocal, 16); //m_port_data

	GetWindowTextA(*m_port_Edit_Ctrl, (LPSTR)m_port_ctrl, 12);
	GetWindowTextA(*m_port_Edit_Data, (LPSTR)m_port_data, 12);

	//�����ʼʱ�����³�ʼ���׽���
	if (sock)
	{
		closesocket(sock); //�ȹرպ����´����׽���
	}

	if (!init_sock(&sock, &m_servAddr_data, m_IPLocal, m_port_data, true)) //�󶨱���IP �˿ڣ���������
	{
		memset(ip_port_buf, 0, sizeof(ip_port_buf));
		sprintf(ip_port_buf, "bind failure ip:%s, port:%s", m_IPLocal, m_port_data);
		showCurLineChar(m_listbox, ip_port_buf);
		return;
	}

	memset(ip_port_buf, 0, sizeof(ip_port_buf));
	sprintf(ip_port_buf, "bind success ip:%s, port:%s", m_IPLocal, m_port_data);
	showCurLineChar(m_listbox, ip_port_buf);

	m_ip = (CEdit*)GetDlgItem(IDC_EDIT_IP);
	m_port_Edit_Ctrl = (CEdit *)GetDlgItem(IDC_EDIT_PORT);

	GetWindowTextA(*m_ip, (LPSTR)m_IP, 16);
	GetWindowTextA(*m_port_Edit_Ctrl, (LPSTR)m_port_ctrl, 12);
	if (!init_sock(&m_sock_sig, &m_servAddr_sig, m_IP, m_port_ctrl, false))
	{
		memset(ip_port_buf, 0, sizeof(ip_port_buf));
		sprintf(ip_port_buf, "init failure ip:%s, port:%s", m_IP, m_port_ctrl);
		showCurLineChar(m_listbox, ip_port_buf);
		return ;
	}
	memset(ip_port_buf, 0, sizeof(ip_port_buf));
	sprintf(ip_port_buf, "init success ip:%s, port:%s", m_IP, m_port_ctrl);
	//showCurLineChar(m_listbox, ip_port_buf);
	//==2019��7��2��10:28:25

	//setnonblocking(&m_sock_adc);
	setTimeOut(&sock);
	setTimeOut(&m_sock_sig);
	setTimeOut(&m_sock_adc);

}
