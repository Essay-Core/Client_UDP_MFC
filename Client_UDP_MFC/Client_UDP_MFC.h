
// Client_UDP_MFC.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#pragma once

#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

#include "resource.h"		// ������


// CClient_UDP_MFCApp: 
// �йش����ʵ�֣������ Client_UDP_MFC.cpp
//

class CClient_UDP_MFCApp : public CWinApp
{
public:
	CClient_UDP_MFCApp();

// ��д
public:
	virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CClient_UDP_MFCApp theApp;