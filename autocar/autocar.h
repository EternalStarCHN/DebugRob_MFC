// autocar.h : PROJECT_NAME Ӧ�ó������ͷ�ļ�
//

#include "stdafx.h"
#ifndef __AFXWIN_H__
	#error "�ڰ������ļ�֮ǰ������stdafx.h�������� PCH �ļ�"
#endif

// CautocarApp:
// �йش����ʵ�֣������ autocar.cpp
//

class CautocarApp : public CWinApp
{
public:
	CautocarApp();

// ��д
  virtual BOOL InitInstance();

// ʵ��

	DECLARE_MESSAGE_MAP()
};

extern CautocarApp theApp;