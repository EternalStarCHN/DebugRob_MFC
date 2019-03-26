// autocar.h : PROJECT_NAME 应用程序的主头文件
//

#include "stdafx.h"
#ifndef __AFXWIN_H__
	#error "在包含此文件之前包含“stdafx.h”以生成 PCH 文件"
#endif

// CautocarApp:
// 有关此类的实现，请参阅 autocar.cpp
//

class CautocarApp : public CWinApp
{
public:
	CautocarApp();

// 重写
  virtual BOOL InitInstance();

// 实现

	DECLARE_MESSAGE_MAP()
};

extern CautocarApp theApp;