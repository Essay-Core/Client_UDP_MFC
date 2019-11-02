
// Client_UDP_MFCDlg.cpp : 实现文件
//

#include "stdafx.h"
#include "Client_UDP_MFC.h"
#include "Client_UDP_MFCDlg.h"
#include "afxdialogex.h"

using namespace std;

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
public:
	CAboutDlg();

// 对话框数据
	enum { IDD = IDD_ABOUTBOX };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
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


// CClient_UDP_MFCDlg 对话框
CClient_UDP_MFCDlg::CClient_UDP_MFCDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CClient_UDP_MFCDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);

	m_Q = new QUEUE;

	WSADATA wsd = { 0 };
	WSAStartup(0x0202, &wsd);

	start_status = false; //初始状态为假，未开始
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

	//退出线程
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


// CClient_UDP_MFCDlg 消息处理程序

BOOL CClient_UDP_MFCDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	// 将“关于...”菜单项添加到系统菜单中。

	// IDM_ABOUTBOX 必须在系统命令范围内。
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

	// 设置此对话框的图标。  当应用程序主窗口不是对话框时，框架将自动
	//  执行此操作
	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// TODO:  在此添加额外的初始化代码

	//获取可执行文件路径，显示 2019年6月19日13:53:08
	memset(m_filename, 0, 260);
	GetModuleDir(m_filename);



	//add 2019年7月5日10:58:52
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

	//==2019年7月5日10:58:52

	//fpga默认显示
	SetDlgItemText(IDC_EDIT_FPGA_REG, _T(FPGA_REG));	
	SetDlgItemText(IDC_EDIT_FPGA_VAL, _T(FPGA_VAL));

	//ADC默认显示
	SetDlgItemText(IDC_EDIT_ADC_CHANNEL, _T(ADC_CAHNNEL));	//ADC
	SetDlgItemText(IDC_EDIT_ADC_SIZE, _T(ADC_SIZE));	//ADC
	SetDlgItemText(IDC_EDIT_ADC_ADDR, _T(ADC_ADDR));	//ADC
	SetDlgItemText(IDC_EDIT_ADC_DATA, _T(ADC_DATA));	//ADC

	//列表
	m_listbox = (CListBox*)GetDlgItem(IDC_LIST_STATUS);

	//adc 
	m_CEdit_adc_channel = (CEdit*)GetDlgItem(IDC_EDIT_ADC_CHANNEL);
	m_CEdit_adc_size = (CEdit*)GetDlgItem(IDC_EDIT_ADC_SIZE);
	m_CEdit_adc_addr = (CEdit*)GetDlgItem(IDC_EDIT_ADC_ADDR);
	m_CEdit_adc_data = (CEdit*)GetDlgItem(IDC_EDIT_ADC_DATA);

	//fpga
	m_CEdit_fpga_reg = (CEdit*)GetDlgItem(IDC_EDIT_FPGA_REG);
	m_CEdit_fpga_val = (CEdit*)GetDlgItem(IDC_EDIT_FPGA_VAL);

	//创建队列
	CreateQueue(m_Q, MAX_NODE);
	
	//使线程可创建
	m_th_flags = false;

	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
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

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CClient_UDP_MFCDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // 用于绘制的设备上下文

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CClient_UDP_MFCDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}


void CClient_UDP_MFCDlg::OnBnClickedOpenfile()
{
	// TODO:  在此添加控件通知处理程序代码
	CString m_strFilePath = _T("./");
	CEdit* pBoxOne;

	CString filepath;
	CFileDialog dlg(TRUE, NULL, NULL, NULL);
	//打开图像
	if (dlg.DoModal() == IDOK)
	{
		//读取图像文件名
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
		AfxMessageBox(_T("已开始，请点击停止按钮"));
		return;
	}

	//清空队列
	Clequeue(m_Q);
	
	// TODO:  在此添加控件通知处理程序代码
	//获取控件的操作句柄
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

	//点击开始时，重新初始化套接字
	if (sock)
	{
		closesocket(sock); //先关闭后重新创建套接字
	}

	if (!init_sock(&sock, &m_servAddr_data, m_IPLocal, m_port_data, true)) //绑定本地IP 端口，接收数据
	{
		return;
	}

	//2019年6月25日11:49:54
	m_CEdit_write_count = (CEdit *)GetDlgItem(IDC_EDIT_WRITE_COUNT);
	m_writeFile_countCtrl = GetDlgItemInt(IDC_EDIT_WRITE_COUNT);
// 	if (m_writeFile_countCtrl <=0)
// 	{
// 		AfxMessageBox(_T("请填写大于0的数作为写入文件包数"));
// 		return;
// 	}

	setTimeOut(&sock); //设置接收超时，时间为3秒

	m_msg.msg_id = SOCK_MSG_START_GET_DATA;  //控制的， 发送开始控制信号
	start_status = true;	//已开始， 等待点击结束按钮是才可以重置	
	m_click_Stop_th = false; //即将创建线程，置为假，可进入线程循环
	m_NumOfAccPack = 0; //重新点击开始，重新计数
	m_NumOfWritePack = 0; //初始化写入文件包数
	m_dequeue_count = 0; //初始化出队数

	//线程只需要创建一次就好了，不需要销毁或重新创建，等到程序退出才退出线程
	if (!m_th_flags)
	{
		m_th_ctrl = thread(th_send_commend_new, this);
		m_th_data = thread(th_recv_data_new, this);
		m_th_write = thread(th_write_data_new, this);

		m_th_flags = TRUE; //创建线程成功，不在创建，只在程序退出时退出
	}
}


void CClient_UDP_MFCDlg::OnBnClickedButtonEnd()
{

	m_msg.msg_id = SOCK_MSG_STOP_GET_DATA;  //停止消息
	start_status = false; //停止接收数据

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

		//初始化socket，不绑定
		if (!init_sock(&m_sock_adc, &m_sAddr_adc, m_ip_adc, m_port_adc, false))
		{
			showCurLine(m_listbox, _T("bind IP、PORT false!!"));

		}

		setTimeOut(&m_sock_adc);

// 		char ip_port_buf[64] = { 0 };
// 		sprintf(ip_port_buf, "bind ip:%s, port:%s", m_ip_adc, m_port_adc);
// 		showCurLineChar(m_listbox, ip_port_buf);
		
	}

	memset((char*)&m_adcConfig, 0, sizeof(stAdcConfig));
	
	////2019年6月20日16:41:17
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

		//初始化socket，不绑定
		if (!init_sock(&m_sock_adc, &m_sAddr_adc, m_ip_adc, m_port_adc, false))
		{
			showCurLine(m_listbox, _T("bind IP、PORT false!!"));

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
	//读操作标志
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
	//2019年6月26日09:43:37
	//判断套接字是否为真
	//当为假的时候需要获取控件的ip和端口号
	if (!ifSocketTrue(&m_sock_adc))
	{
//		m_CEdit_ip_adc = (CEdit *)GetDlgItem(IDC_EDIT_IP_ADC);
		m_CEdit_port_adc = (CEdit *)GetDlgItem(IDC_EDIT_PORT_ADC);

		GetWindowTextA(*m_CEdit_ip_adc, (LPSTR)m_ip_adc, 16);
		GetWindowTextA(*m_CEdit_port_adc, (LPSTR)m_port_adc, 12);

		//初始化socket，不绑定
		if (!init_sock(&m_sock_adc, &m_sAddr_adc, m_ip_adc, m_port_adc, false))
		{
			showCurLine(m_listbox, _T("bind IP、PORT false!!"));

		}
		setTimeOut(&m_sock_adc);//设置超时

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

	//判断输入十六进制数值
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

	//判断收到的与发送的相同，表示成功
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

		//初始化socket，不绑定
		if (!init_sock(&m_sock_adc, &m_sAddr_adc, m_ip_adc, m_port_adc, false))
		{
			showCurLine(m_listbox, _T("bind IP、PORT false!!"));

		}
		setTimeOut(&m_sock_adc);

// 		char ip_port_buf[64] = { 0 };
// 		sprintf(ip_port_buf, "bind ip:%s, port:%s", m_ip_adc, m_port_adc);
// 		showCurLineChar(m_listbox, ip_port_buf);
// 	
	}
  
	// TODO:  在此添加控件通知处理程序代码
	memset((char*)&m_headFpgaConfig, 0, sizeof(stHeadFpgaConfig));
	memset((char*)&m_fpgaConfig, 0, sizeof(stFpgaConfig));

	//读操作标志
	m_CEdit_fpga_reg = (CEdit *)GetDlgItem(IDC_EDIT_FPGA_REG);

	GetWindowTextA(*m_CEdit_fpga_reg, (LPSTR)m_fpga_reg, 32);

	//判断输入十六进制数值
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

//清空list
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
 	SetDlgItemText(IDC_BUTTON_BIND, _T("重新绑定"));

	// TODO:  在此添加控件通知处理程序代码
	m_CEdit_ip_adc = (CEdit *)GetDlgItem(IDC_EDIT_IP);//删除控件 IDC_EDIT_IP_ADC
	m_CEdit_port_adc = (CEdit *)GetDlgItem(IDC_EDIT_PORT_ADC);

	GetWindowTextA(*m_CEdit_ip_adc, (LPSTR)m_ip_adc, 16);
	GetWindowTextA(*m_CEdit_port_adc, (LPSTR)m_port_adc, 12);

	//初始化socket，不绑定
	if (!init_sock(&m_sock_adc, &m_sAddr_adc, m_ip_adc, m_port_adc, false))
	{
		showCurLine(m_listbox, _T("bind IP、PORT false!!"));

	}
	//showCurLine(m_listbox, _T("bind IP、PORT success!!"));

	//2019年7月2日10:28:12
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

	//点击开始时，重新初始化套接字
	if (sock)
	{
		closesocket(sock); //先关闭后重新创建套接字
	}

	if (!init_sock(&sock, &m_servAddr_data, m_IPLocal, m_port_data, true)) //绑定本地IP 端口，接收数据
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
	//==2019年7月2日10:28:25

	//setnonblocking(&m_sock_adc);
	setTimeOut(&sock);
	setTimeOut(&m_sock_sig);
	setTimeOut(&m_sock_adc);

}
