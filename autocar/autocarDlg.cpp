#include "stdafx.h"
#include "autocarDlg.h"
#include "strngs.h"

#include <math.h>
#include <stdio.h>
#include "resource.h"
#include "DebugRob.h"
#pragma comment(lib,"libtesseract302.lib")

CvCapture *capture;
CRect rect;
HDC hDC;

/*老工程图像识别**************************/
CvCapture *capture1;
CRect rect1;
HDC hDC1;
CRect rect2;
HDC hDC2;
CRect rect3;
HDC hDC3;
/*********************************************/

CautocarDlg::CautocarDlg(CWnd* pParent /*=NULL*/)
  : CDialog(IDD_AUTOCAR_DIALOG, pParent)
  , CComPort(this)
  , _msgSerialSend()
  , _msgSerialReceive()
  //TAG:避免这样的魔法值
  , _mode(0)
  , _cameraForPic(0)
  , appIcon_(AfxGetApp()->LoadIcon(IDR_MAINFRAME))
  , _TARGET_IMAGE_LIST1{
  {imread("G:\\Desktop\\DebugRob_MFC\\autocar\\pic\\目标3.png",CV_LOAD_IMAGE_COLOR), 3},
  {imread("G:\\Desktop\\DebugRob_MFC\\autocar\\pic\\目标4.png",CV_LOAD_IMAGE_COLOR), 4},
  {imread("G:\\Desktop\\DebugRob_MFC\\autocar\\pic\\目标5.png",CV_LOAD_IMAGE_COLOR), 5},
  {imread("G:\\Desktop\\DebugRob_MFC\\autocar\\pic\\目标6.png",CV_LOAD_IMAGE_COLOR), 6},
  {imread("G:\\Desktop\\DebugRob_MFC\\autocar\\pic\\目标7.png",CV_LOAD_IMAGE_COLOR), 7},
  {imread("G:\\Desktop\\DebugRob_MFC\\autocar\\pic\\目标8.png",CV_LOAD_IMAGE_COLOR), 8} }
  , _TARGET_IMAGE_LIST{
    {imread("G:\\Desktop\\DebugRob_MFC\\autocar\\pic\\目标3.png"), 3},
    {imread("G:\\Desktop\\DebugRob_MFC\\autocar\\pic\\目标4.png"), 4},
    {imread("G:\\Desktop\\DebugRob_MFC\\autocar\\pic\\目标5.png"), 5},
    {imread("G:\\Desktop\\DebugRob_MFC\\autocar\\pic\\目标6.png"), 6},
    {imread("G:\\Desktop\\DebugRob_MFC\\autocar\\pic\\目标7.png"), 7},
    {imread("G:\\Desktop\\DebugRob_MFC\\autocar\\pic\\目标8.png"), 8} } {}
BOOL CautocarDlg::OnInitDialog()
{
  CDialog::OnInitDialog();

  /* IDM_ABOUTBOX 必须在系统命令范围内 *******************************/
  ASSERT((IDM_ABOUTBOX & 0xFFF0) == IDM_ABOUTBOX);
  ASSERT(IDM_ABOUTBOX < 0xF000);

  CMenu* pSysMenu = GetSystemMenu(FALSE);
  if (pSysMenu != nullptr)
  {
    CString strAboutMenu;
    strAboutMenu.LoadString(IDS_ABOUTBOX);
    if (!strAboutMenu.IsEmpty())
    {
      pSysMenu->AppendMenu(MF_SEPARATOR);
      pSysMenu->AppendMenu(MF_STRING, IDM_ABOUTBOX, strAboutMenu);
    }
  }

  /* 设置此对话框的图标。当应用程序主窗口不是对话框时，框架将自动 **********/
  SetIcon(appIcon_, FALSE);

  OnBnClickedBt_OpenSerial();
  OnBnClickedBt_OpenCamera();
  OnBnClickedBt_AutoDrive();

  return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CautocarDlg::OnClose()
{
  // TODO: Add your message handler code here and/or call default
  OnBnClickedBt_CloseSerial();
  OnBnClickedBt_CloseCamera();
  for (auto imagePair : _TARGET_IMAGE_LIST)
  {
    imagePair.first.release();
  }
  CDialog::OnClose();
}

/* Windows功能函数 -----------------------------------------*/
void CautocarDlg::DoDataExchange(CDataExchange* pDX)
{
  CDialog::DoDataExchange(pDX);
  DDX_Text(pDX, IDC_EDITSEND, _msgSerialSend);//在发送框内显示_msgSerialSend
  DDX_Text(pDX, IDC_EDITREV, _msgSerialReceive);//在接收框内显示_msgSerialReceive
  DDX_Text(pDX, IDC_EDITSHOW, _datashow);
}

void CautocarDlg::OnPaint()
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
    dc.DrawIcon(x, y, appIcon_);
  }
  else
  {
    CDialog::OnPaint();
  }
}

HCURSOR CautocarDlg::OnQueryDragIcon()
{
  return static_cast<HCURSOR>(appIcon_);
}

BEGIN_MESSAGE_MAP(CautocarDlg, CDialog)
  ON_WM_SYSCOMMAND()
  ON_WM_PAINT()
  ON_WM_QUERYDRAGICON()
  ON_WM_TIMER()
  ON_WM_CLOSE()

  ON_MESSAGE(WM_RECV_SERIAL_DATA, OnRecvSerialData)

    ON_BN_CLICKED(IDC_BTOPEN      , OnBnClickedBt_OpenSerial)              //打开串口按钮
    ON_BN_CLICKED(IDC_BTCLOSE     , OnBnClickedBt_CloseSerial)             //关闭串口按钮
    ON_BN_CLICKED(IDC_BTSEND      , OnBnClickedBt_SendToSerial)            //发送数据按钮
    ON_BN_CLICKED(IDC_BTAUTODRIVE , OnBnClickedBt_AutoDrive)               //自动驾驶按钮
    ON_BN_CLICKED(IDC_BTOPENVIDEO , OnBnClickedBt_OpenCamera)              //打开摄像头按钮
    ON_BN_CLICKED(IDC_BTCLOSEVIDEO, OnBnClickedBt_CloseCamera)             //关闭摄像头按钮
    ON_BN_CLICKED(IDC_BTSCANNUMBER, OnBnClickedBt_ImageIdentification)     //识别数字按钮
    ON_BN_CLICKED(IDC_BTTEST      , OnBnClickedBt_Test)                    //模版匹配测试按钮
    ON_BN_CLICKED(IDC_BTAUTO12, &CautocarDlg::OnBnClickedBtauto12)         //1号位到2号位脚本开始
    ON_BN_CLICKED(IDC_BTImageTest, &CautocarDlg::OnBnClickedBt_ImageTest)  //图像识别测试
    ON_BN_CLICKED(IDC_BTTakePhoto, &CautocarDlg::OnBnClickedBttakephoto)   //拍照按钮
	ON_BN_CLICKED(IDC_BTAUTO23, &CautocarDlg::OnBnClickedBtauto23)
	ON_BN_CLICKED(IDC_BTAUTO24, &CautocarDlg::OnBnClickedBtauto24)
	ON_BN_CLICKED(IDC_BTAUTO35, &CautocarDlg::OnBnClickedBtauto35)
	ON_BN_CLICKED(IDC_BTAUTO36, &CautocarDlg::OnBnClickedBtauto36)
	ON_BN_CLICKED(IDC_BTAUTO45, &CautocarDlg::OnBnClickedBtauto45)
	ON_BN_CLICKED(IDC_BTAUTO46, &CautocarDlg::OnBnClickedBtauto46)
	ON_BN_CLICKED(IDC_BTAUTO57, &CautocarDlg::OnBnClickedBtauto57)
	ON_BN_CLICKED(IDC_BTAUTO58, &CautocarDlg::OnBnClickedBtauto58)
	ON_BN_CLICKED(IDC_BTAUTO67, &CautocarDlg::OnBnClickedBtauto67)
	ON_BN_CLICKED(IDC_BTAUTO68, &CautocarDlg::OnBnClickedBtauto68)
	ON_BN_CLICKED(IDC_BTAUTO78, &CautocarDlg::OnBnClickedBtauto78)
	ON_BN_CLICKED(IDC_BTAUTO87, &CautocarDlg::OnBnClickedBtauto87)
	ON_BN_CLICKED(IDC_BTAUTO71, &CautocarDlg::OnBnClickedBtauto71)
	ON_BN_CLICKED(IDC_BTAUTO81, &CautocarDlg::OnBnClickedBtauto81)
	ON_BN_CLICKED(IDC_BTHU_LAR, &CautocarDlg::OnBnClickedBtHU_LAR)
	ON_BN_CLICKED(IDC_BTTURN_BACK, &CautocarDlg::OnBnClickedBtTURN_BACK)
	ON_BN_CLICKED(IDC_BTHEAD_MOVE, &CautocarDlg::OnBnClickedBtHEAD_MOVE)
	ON_BN_CLICKED(IDC_BTCARREST, &CautocarDlg::OnBnClickedBtRESET)
	ON_BN_CLICKED(IDC_BTID34, &CautocarDlg::BTImageRecognition_3or4)
	ON_BN_CLICKED(IDC_BTID56, &CautocarDlg::BTImageRecognition_5or6)
	ON_BN_CLICKED(IDC_BTID78, &CautocarDlg::BTImageRecognition_7or8)
	ON_BN_CLICKED(IDC_BTStop, &CautocarDlg::OnBnClickedBtStop)
	ON_BN_CLICKED(IDC_BTRST, &CautocarDlg::OnBnClickedBtrst)
	ON_BN_CLICKED(IDC_BTCLEAR, &CautocarDlg::OnBnClickedB_ClearEdit)
	ON_BN_CLICKED(IDC_BT_StopAutoDrive, &CautocarDlg::OnBnClickedBtStopautodrive)
	ON_BN_CLICKED(IDC_BTCLEARSHOW, &CautocarDlg::OnBnClickedBtclearshow)
END_MESSAGE_MAP()

/* 串口相关函数 ---------------------------------------------*/

//打开串口
void CautocarDlg::OnBnClickedBt_OpenSerial()
{
  if (!this->IsOpen())
  {
    _SerialOpen(5);
  }
}

//关闭串口
void CautocarDlg::OnBnClickedBt_CloseSerial()
{
  //if (this->_serialPort->IsOpen())
  //{
  //  PrintlnToSerial("Serial is closing.");
  //}
  Close();
}

//串口发送数据
void CautocarDlg::OnBnClickedBt_SendToSerial()
{
  UpdateData(true);
  char in[15] = {0};
  CString InputString;
  GetDlgItem(IDC_EDITSEND)->GetWindowText(in,15);
  char op[6];

  op[0] = HexToChar(in[0]) * 16 + HexToChar(in[1]);
  op[1] = HexToChar(in[3]) * 16 + HexToChar(in[4]);
  op[2] = HexToChar(in[6]) * 16 + HexToChar(in[7]);
  op[3] = HexToChar(in[9]) * 16 + HexToChar(in[10]);
  op[4] = HexToChar(in[12]) * 16 + HexToChar(in[13]);
  Output(op,5);
  UpdateData(false);
}

//发送内容ascii转hex
char CautocarDlg::HexToChar(char bChar) {
	if ((bChar >= 0x30) && (bChar <= 0x39))
		bChar -= 0x30;
	else if ((bChar >= 0x41) && (bChar <= 0x46))//大写字母 
		bChar -= 0x37;
	else if ((bChar >= 0x61) && (bChar <= 0x66))//小写字母 
		bChar -= 0x57;
	else bChar = 0xff;
	return bChar;
}

//发送函数1(未使用)
void CautocarDlg::PrintlnToSerial(const string& message)
{
  if (IsOpen())
  {
    Output(message.c_str(), message.length());
    Output("\r\n", 2);
  }
}

//串口命令接收线程
void CautocarDlg::_OnCommReceive(LPVOID pSender, void* pBuf, DWORD InBufferCount)
{
  BYTE *pRecvBuf = new BYTE[InBufferCount]; //delete at OnRecvSerialData();
  CautocarDlg* pThis = (CautocarDlg*)pSender;

  CopyMemory(pRecvBuf, pBuf, InBufferCount);

  pThis->PostMessage(WM_RECV_SERIAL_DATA, WPARAM(pRecvBuf), InBufferCount);

}

//串口数据接收线程
LONG CautocarDlg::OnRecvSerialData(WPARAM wParam, LPARAM lParam)
{
  UpdateData(true);
  
  CHAR *pBuf = (CHAR*)wParam;
  DWORD dwBufLen = lParam;
  
  _msgSerialReceive.Format(_T("%s\r\n")+ _msgSerialReceive, wParam);//edit框内不覆盖换行输出

	char rev[15] = { 0 };
	for (int i = 0; i < 15 && pBuf[i] >= '\0'; i++) {
		rev[i] = *(pBuf + i);
	}
	if (_msgSerialReceive.IsEmpty()) 
	{	
		_msgSerialReceive.Format(_T("...\r\n") + _msgSerialReceive);
	}
	else {
		if (Findrst(rev)) {
			OnBnClickedBtrst();
			SetTimer(2, 50, NULL);
		}
		else {
			Debug::DebugRob::UpdateReceiveTag();
		}

	}
  delete[] pBuf; //new at OnCommReceive();
  UpdateData(false);
  return 0;
}

//命令中断
void CautocarDlg::_OnCommBreak(LPVOID pSender, DWORD dwMask, COMSTAT stat)
{
  //deal with the break of com here
  switch (dwMask)
  {
  case  EV_BREAK:
  {
    break;
  }
  case EV_CTS:
  {
    break;
  }
  case EV_DSR:
  {
    break;
  }
  case EV_ERR:
  {
    break;
  }
  case EV_RING:
  {
    break;
  }
  case EV_RLSD:
  {
    break;
  }
  case EV_RXCHAR:
  {
    break;
  }
  case EV_RXFLAG:
  {
    break;
  }
  case EV_TXEMPTY:
  {
    break;
  }
  default:
  {
  }
  }
}

/* OpenCV相关函数 ------------------------------------------*/

//打开摄像头
void CautocarDlg::OnBnClickedBt_OpenCamera()
{
  //TAG: 开启逻辑有些奇怪，没能以最简单的方式查询是否有可用摄像机
  //Tips: .open()函数，会先release已打开的摄像头
  _cameraForPic.open(0);
  if (!_cameraForPic.isOpened())
  {
    //TAG:应该抛出异常
    AfxMessageBox("无法打开摄像头，Win10请确认摄像头隐私设置是否开启");
  }
  else
  {
    _cameraForPic >> ontimer_frame;
    _ShowImageOnImageBox(IDC_ImageBox1, ontimer_frame);
    SetTimer(1, 50, NULL);
  }
}

//关闭摄像头
void CautocarDlg::OnBnClickedBt_CloseCamera()
{
  CDC deviceContext;
  CBitmap bitmap;
  bitmap.LoadBitmap(IDB_BITMAP1);  
  deviceContext.CreateCompatibleDC(nullptr);
  deviceContext.SelectObject(&bitmap);

  KillTimer(1);
  _cameraForPic.release();

  _StretchBlt(IDC_ImageBox1, deviceContext);
  _StretchBlt(IDC_ImageBox2, deviceContext);
  _StretchBlt(IDC_ImageBox3, deviceContext);
  _StretchBlt(IDC_ImageBox4, deviceContext);
}

//图像识别
void CautocarDlg::OnBnClickedBt_ImageIdentification()
{
  if (_cameraForPic.isOpened()) {
    Mat inputMat;
    _cameraForPic >> inputMat;
    ImageRecognition(inputMat);

    UpdateData(true);
    UpdateData(false);
  }
  else
  {
    //TAG:抛出异常
  }
}

//图像模版匹配测试
void CautocarDlg::OnBnClickedBt_Test()
{
  //TAG:图像输入到时候是用摄像头输入的，不过现在测试下来，理想情况下能够识别
  //    但是只要有一些干扰，那么神奇的第6幅图，如何识别，第6幅图都是最佳匹配
  //    所以算法打算到场地上去调试再写了。
  Mat imageTest = imread("G:\\Desktop\\DebugRob_MFC\\autocar\\pic\\测试图片13.jpg");
  int res = _TemplateMatching(imageTest);
  imshow("识别原图像", imageTest);
  imshow("识别结果图像", _TARGET_IMAGE_LIST[res].first);
  imageTest.release();
}

//图像识别函数
void CautocarDlg::ImageRecognition(Mat src)//图像识别处理
{
  _binaryMat = _Binaryzation(src);
  _maximumInterContor = _FindContour(_binaryMat.clone()/*!!!*/);
  
  _conLength = arcLength(_maximumInterContor, true);
  _conArea = contourArea(_maximumInterContor, true);

  /* 显示图片 */
  Mat drawMat = Mat::zeros(_binaryMat.size(), CV_8UC3);
  for (int i = 0; i < _contours_all.size(); i++)
  {
    drawContours(drawMat, _contours_all, i, Scalar(0, 0, 255), 2);
  }
  drawContours(drawMat, Contors_t{ _maximumInterContor }, 0, Scalar(0, 255, 0), 2);

  _ShowImageOnImageBox(IDC_ImageBox3, _binaryMat);
  _ShowImageOnImageBox(IDC_ImageBox2, drawMat);
}

//自动驾驶
void CautocarDlg::OnBnClickedBt_AutoDrive()
{
  UpdateData(true);
  _mode = 10;//大模式自动开机
  SetTimer(2, 50, NULL);
  UpdateData(false);
}

//重置自动驾驶
void CautocarDlg::OnBnClickedBtrst()
{
	KillTimer(2);
	sign = 1;
	next = 0;
	back = 0;
	step = 0;
	go = 0;
	_datashow.Format(_T("自动驾驶状态重置成功\r\n") + _datashow, tex);
}

//停止自动驾驶
void CautocarDlg::OnBnClickedBtStopautodrive()
{
	KillTimer(2);
}

//判断串口收到的是不是重置信号的字符串"reset"
int CautocarDlg::Findrst(char a[]) {
	auto length = strlen(a);
	if (length ==5 && a[0] == 'r' && a[1] == 'e' && a[2] == 's' && a[3] == 'e' && a[4] == 't')
		return 1;
	else 
	{
		return 0;
	}
}

//传输协议
void CautocarDlg::Mode(PointMode_t pointMode, int8_t command )
{
	UpdateData(true);
	srand((unsigned)time(NULL));
	int8_t rand_1, rand_2, count;//产生随机数并求和到count
	rand_1 = 1 + rand() % (100 - 1 + 1);
	rand_2 = 1 + rand() % (100 - 1 + 1);
	count = pointMode + command + rand_1 + rand_2;
	const char serialSendDataArray[5] = {  pointMode, command, rand_1, rand_2, count };
	SendData(serialSendDataArray, 5);
	UpdateData(false);
}

//串口数据发送函数
void CautocarDlg::SendData(const char arrays[], int lenth)
{
	if (this->IsOpen())
	{
		this->Output(arrays, lenth);
	}
}

/*路线测试脚本------------------------------------------------------------*/

//1到2测试按钮
void CautocarDlg::OnBnClickedBtauto12()
{
	Mode(NO_ACTION, 0x01);//从1台走到2台
	UpdateData(true);
	chose = 1;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//2到3测试按钮
void CautocarDlg::OnBnClickedBtauto23()
{
	Mode(NO_ACTION, 0x02);//从2台走到3台
	UpdateData(true);
	chose = 2;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//2到4测试按钮
void CautocarDlg::OnBnClickedBtauto24()
{
	Mode(NO_ACTION, 0x03);//从2台走到4台
	UpdateData(true);
	chose = 3;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//3到5测试按钮
void CautocarDlg::OnBnClickedBtauto35()
{
	Mode(NO_ACTION, 0x04);//从3台走到5台
	UpdateData(true);
	chose = 4;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//3到6测试按钮
void CautocarDlg::OnBnClickedBtauto36()
{
	Mode(NO_ACTION, 0x05);//从3台走到6台
	UpdateData(true);
	chose = 5;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//4到5测试按钮
void CautocarDlg::OnBnClickedBtauto45()
{
	Mode(NO_ACTION, 0x06);//从4台走到5台
	UpdateData(true);
	chose = 6;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//4到6测试按钮
void CautocarDlg::OnBnClickedBtauto46()
{
	Mode(NO_ACTION, 0x07);//从4台走到6台
	UpdateData(true);
	chose = 7;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//5到7测试按钮
void CautocarDlg::OnBnClickedBtauto57()
{
	Mode(NO_ACTION, 0x08);//从5台走到7台
	UpdateData(true);
	chose = 8;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//5到8测试按钮
void CautocarDlg::OnBnClickedBtauto58()
{
	Mode(NO_ACTION, 0x09);//从5台走到8台
	UpdateData(true);
	chose = 9;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//6到7测试按钮
void CautocarDlg::OnBnClickedBtauto67()
{
	Mode(NO_ACTION, 0x0A);//从6台走到7台
	UpdateData(true);
	chose = 10;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//6到8测试按钮
void CautocarDlg::OnBnClickedBtauto68()
{
	Mode(NO_ACTION, 0x0B);//从6台走到8台
	UpdateData(true);
	chose = 11;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//7到8测试按钮
void CautocarDlg::OnBnClickedBtauto78()
{
	Mode(NO_ACTION, 0x0C);//从7台走到8台
	UpdateData(true);
	chose = 12;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//8到7测试按钮
void CautocarDlg::OnBnClickedBtauto87()
{
	Mode(NO_ACTION, 0x0D);//从8台走到7台
	UpdateData(true);
	chose = 13;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//7到1测试按钮
void CautocarDlg::OnBnClickedBtauto71()
{
	Mode(NO_ACTION, 0x0E);//从7台走到1台
	UpdateData(true);
	chose = 14;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//8到1测试按钮
void CautocarDlg::OnBnClickedBtauto81()
{
	Mode(NO_ACTION, 0x0F);//从8台走到1台
	UpdateData(true);
	chose = 15;
	SetTimer(4, 50, NULL);
	UpdateData(false);
}
//左右举手测试函数
void CautocarDlg::OnBnClickedBtHU_LAR()
{
	Mode(HU_LAR, 0xFF);
}
//转身测试按钮
void CautocarDlg::OnBnClickedBtTURN_BACK()
{
	Mode(TURN_BACK, 0xFF);
}
//摇头测试按钮
void CautocarDlg::OnBnClickedBtHEAD_MOVE()
{
	Mode(HEAD_MOVE, 0xFF);
}
//重置下位机测试按钮
void CautocarDlg::OnBnClickedBtRESET()
{
	Mode(RESET, 0xFF);
}
//急停按钮
void CautocarDlg::OnBnClickedBtStop()
{
	Mode(NO_ACTION, 0x00);//急停
}

/*路线脚本--------------------------------------------------------------*/

//1to2测试用
void CautocarDlg::TEST12(void) {
			this->_OldalgorithmMatching();
			float res = atof(m_locationgold);
			if (res < res34) {
				next = 3;
			}
			else {
				next = 4;
			}
			if (next) {
				CString str;
				str.Format(_T("图像识别结果：%d"), next);
				AfxMessageBox(str);
			}
			else {
				AfxMessageBox("存在异常");
			}
			KillTimer(4);
}
//2to3测试用
void CautocarDlg::TEST23(void) {
	this->_OldalgorithmMatching();
	float res = atof(m_locationgold);
	if (res < res56) {
		next = 5;
	}
	else {
		next = 6;
	}
	if (next) {
		CString str;
		str.Format(_T("图像识别结果：%d"), next);
		AfxMessageBox(str);
	}
	else {
		AfxMessageBox("存在异常");
	}
	KillTimer(4);
}
//2to4测试用
void CautocarDlg::TEST24(void) {
	this->_OldalgorithmMatching();
	float res = atof(m_locationgold);
	if (res < res56) {
		next = 5;
	}
	else {
		next = 6;
	}
	if (next) {
		CString str;
		str.Format(_T("图像识别结果：%d"), next);
		AfxMessageBox(str);
	}
	else {
		AfxMessageBox("存在异常");
	}
	KillTimer(4);
}
//3to5测试用
void CautocarDlg::TEST35(void) {
	this->_OldalgorithmMatching();
	float res = atof(m_locationgold);
	if (res < res78) {
		next = 8;
	}
	else {
		next = 7;
	}
	if (next) {
		CString str;
		str.Format(_T("图像识别结果：%d"), next);
		AfxMessageBox(str);
	}
	else {
		AfxMessageBox("存在异常");
	}
	KillTimer(4);
}
//3to6测试用
void CautocarDlg::TEST36(void) {
	this->_OldalgorithmMatching();
	float res = atof(m_locationgold);
	if (res < res78) {
		next = 8;
	}
	else {
		next = 7;
	}
	if (next) {
		CString str;
		str.Format(_T("图像识别结果：%d"), next);
		AfxMessageBox(str);
	}
	else {
		AfxMessageBox("存在异常");
	}
	KillTimer(4);
}
//4to5测试用
void CautocarDlg::TEST45(void) {
	this->_OldalgorithmMatching();
	float res = atof(m_locationgold);
	if (res < res78) {
		next = 8;
	}
	else {
		next = 7;
	}
	if (next) {
		CString str;
		str.Format(_T("图像识别结果：%d"), next);
		AfxMessageBox(str);
	}
	else {
		AfxMessageBox("存在异常");
	}
	KillTimer(4);
}
//4to6测试用
void CautocarDlg::TEST46(void) {
	this->_OldalgorithmMatching();
	float res = atof(m_locationgold);
	if (res < res78) {
		next = 8;
	}
	else {
		next = 7;
	}
	if (next) {
		CString str;
		str.Format(_T("图像识别结果：%d"), next);
		AfxMessageBox(str);
	}
	else {
		AfxMessageBox("存在异常");
	}
	KillTimer(4);
}
//5to7测试用(到台后无图像识别)
void CautocarDlg::TEST57(void) {
	KillTimer(4);
}
//5to8测试用(到台后无图像识别)
void CautocarDlg::TEST58(void) {
	KillTimer(4);
}
//6to7测试用(到台后无图像识别)
void CautocarDlg::TEST67(void) {
	KillTimer(4);
}
//6to8测试用(到台后无图像识别)
void CautocarDlg::TEST68(void) {
	KillTimer(4);
}
//7to8测试用(到台后无图像识别)
void CautocarDlg::TEST78(void) {
	KillTimer(4);
}
//8to7测试用(到台后无图像识别)
void CautocarDlg::TEST87(void) {
	KillTimer(4);
}
//7to1测试用(到台后无图像识别)
void CautocarDlg::TEST71(void) {
	KillTimer(4);
}
//8to1测试用(到台后无图像识别)
void CautocarDlg::TEST81(void) {
	KillTimer(4);
}

//脚本起点(在此处判断)--------------------------------------------------*/

void CautocarDlg::OnBnClickedPatern12()
{
	if (sign == -1 || sign == 1000)
	{
		return;
	}
	else if (sign == 1)
	{
		_1To2();
	}
	else if (sign == 2)
	{
		if ((back == 0) && (next == 3))
		{
			_2To3();
		}
		else if ((back == 0) && (next == 4))
		{
			_2To4();
		}
	}
	else if (sign == 3)
	{
		if ((back == 0) && (next == 5))
		{
			_3To5();
		}
		else if ((back == 0) && (next == 6))
		{
			_3To6();
		}
	}
	else if (sign == 4)
	{
		if ((back == 0) && (next == 5))
		{
			_4To5();
		}
		else if ((back == 0) && (next == 6))
		{
			_4To6();
		}
	}
	else if (sign == 5)
	{
		if ((back == 0) && (next == 7))
		{
			_5To7();
		}
		else if ((back == 0) && (next == 8))
		{
			_5To8();
		}
	}
	else if (sign == 6)
	{
		if ((back == 0) && (next == 7))
		{
			_6To7();
		}
		else if ((back == 0) && (next == 8))
		{
			_6To8();
		}
	}
	else if (sign == 7) {
			_7To1();
	}
	else if (sign == 8) {
			_8To1();
	}
}//运行脚本

/**自动驾驶脚本---------------------------------------------------------*/

//自动驾驶脚本1to2
int CautocarDlg::_1To2(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x01);//从1台走到2台
		break;
	case 1:
		this->_OldalgorithmMatching();
		Mode(HEAD_MOVE, 0xFF);
		break;
	case 2:
		float res = atof(m_locationgold);
		if (res < res34) {
			UpdateData(true);
			next = 3;
			_datashow.Format(_T("识别结果：3号图，特征值：%f\r\n") + _datashow,res);
			UpdateData(false);
		}
		else {
			UpdateData(true);
			next = 4;
			_datashow.Format(_T("识别结果：4号图，特征值：%f\r\n") + _datashow, res);
			UpdateData(false);
		}
		sign = 2;
		step = 0;
		back = 0;
		if (next)
			Mode(TURN_BACK, 0xFF);
		break;
	}
	return next;
}
//自动驾驶脚本2to3
int CautocarDlg::_2To3(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x02);//从2台走到3台
		break;
	case 1:
		this->_OldalgorithmMatching();
		Mode(TURN_BACK, 0xFF);
		break;
	case 2:
		float res = atof(m_locationgold);
		if (res < res56) {
			UpdateData(true);
			next = 5;
			//exclude = 1;
			_datashow.Format(_T("识别结果：5号图，特征值：%f\r\n") + _datashow, res);
			UpdateData(false);
		}
		else {
			UpdateData(true);
			next = 6;
			//exclude = 0;
			_datashow.Format(_T("识别结果：6号图，特征值：%f\r\n") + _datashow, res);
			UpdateData(false);
		}
		sign = 3;
		step = 0;
		back = 0;
		if (next)
			Mode(HU_LAR,0xFF);
		break;
	}
	return next;
}
//自动驾驶脚本2to4
int CautocarDlg::_2To4(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x03);//从2台走到4台
		break;
	case 1:
		this->_OldalgorithmMatching();
		Mode(TURN_BACK,0xFF);
		break;
	case 2:
		float res = atof(m_locationgold);
		if (res < res56) {
			UpdateData(true);
			next = 5;
			//exclude = 1;
			_datashow.Format(_T("识别结果：5号图，特征值：%f\r\n") + _datashow, res);
			UpdateData(false);
		}
		else {
			UpdateData(true);
			next = 6;
			//exclude = 0;
			_datashow.Format(_T("识别结果：6号图，特征值：%f\r\n") + _datashow, res);
			UpdateData(false);
		}
		sign = 4;
		step = 0;
		back = 0;
		if (next)
			Mode(HU_LAR, 0xFF);

		break;
	}
	return next;
}
//自动驾驶脚本3to5
int CautocarDlg::_3To5(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x04);//从3台走到5台
		break;
	case 1:
		this->_OldalgorithmMatching();
		Mode(TURN_BACK,0xFF);
		break;
	case 2:
		float res = atof(m_locationgold);
		if (res < res78) {
			UpdateData(true);
			next = 8;
			_datashow.Format(_T("识别结果：8号图，特征值：%f\r\n") + _datashow, res);
			UpdateData(false);
		}
		else {
			UpdateData(true);
			next = 7;
			_datashow.Format(_T("识别结果：7号图，特征值：%f\r\n") + _datashow, res);
			UpdateData(false);
		}
		sign = 5;
		step = 0;
		back = 0;
		if (next)
			Mode(HU_LAR, 0xFF);
		break;
	}
	return next;
}
//自动驾驶脚本3to6
int CautocarDlg::_3To6(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x05);//从3台走到6台
		break;
	case 1:
		this->_OldalgorithmMatching();
		Mode(TURN_BACK,0xFF);
		break;
	case 2:
		float res = atof(m_locationgold);
		if (res < res78) {
			UpdateData(true);
			next = 8;
			_datashow.Format(_T("识别结果：8号图，特征值：%f\r\n") + _datashow, res);
			UpdateData(false);
		}
		else {
			UpdateData(true);
			next = 7;
			UpdateData(false);
		}
		sign = 6;
		step = 0;
		back = 0;
		if (next)
			Mode(HU_LAR, 0xFF);
		break;
	}
	return next;
}
//自动驾驶脚本4to5
int CautocarDlg::_4To5(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x06);//从4台走到5台
		break;
	case 1:
		this->_OldalgorithmMatching();
		Mode(TURN_BACK,0xFF);
		break;
	case 2:
		float res = atof(m_locationgold);
		if (res < res78) {
			UpdateData(true);
			next = 8;
			_datashow.Format(_T("识别结果：8号图，特征值：%f\r\n") + _datashow, res);
			UpdateData(false);
		}
		else {
			UpdateData(true);
			next = 7;
			_datashow.Format(_T("识别结果：8号图，特征值：%f\r\n") + _datashow, res);
			UpdateData(false);
		}
		sign = 5;
		step = 0;
		back = 0;
		if (next)
			Mode(HU_LAR, 0xFF);
		break;
	}
	return next;
}
//自动驾驶脚本4to6
int CautocarDlg::_4To6(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x07);//从4台走到6台
		break;
	case 1:
		this->_OldalgorithmMatching();
		Mode(TURN_BACK,0xFF);
		break;
	case 2:
		float res = atof(m_locationgold);
		if (res < res78) {
			UpdateData(true);
			next = 8;
			_datashow.Format(_T("识别结果：8号图，特征值：%f\r\n") + _datashow, res);
			UpdateData(false);
		}
		else {
			UpdateData(true);
			next = 7;
			_datashow.Format(_T("识别结果：7号图，特征值：%f\r\n") + _datashow, res);
			UpdateData(false);
		}
		sign = 6;
		step = 0;
		back = 0;
		if (next)
			Mode(HU_LAR, 0xFF);
		break;
	}
	return next;
}
//自动驾驶脚本5to7
int CautocarDlg::_5To7(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x08);//从5台走到7台
		break;
	case 1:
		Mode(TURN_BACK,0xFF);
		break;
	case 2:
		sign = 7;
		step = 0;
		back = 1;
		Mode(HU_LAR, 0xFF);
		break;
	}
	return next;
}
//自动驾驶脚本5to8
int CautocarDlg::_5To8(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x09);//从5台走到8台
		break;
	case 1:
		Mode(TURN_BACK, 0xFF);
		break;
	case 2:
		sign = 8;
		step = 0;
		back = 1;
		Mode(HU_LAR, 0xFF);
		break;
	}
	return next;
}
//自动驾驶脚本6to7
int CautocarDlg::_6To7(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x0A);//从6台走到7台
		break;
	case 1:
		Mode(TURN_BACK, 0xFF);
		break;
	case 2:
		sign = 7;
		step = 0;
		back = 1;
		Mode(HU_LAR, 0xFF);
		break;
	}
	return next;
}
//自动驾驶脚本6to8
int CautocarDlg::_6To8(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x0B);//从6台走到8台
		break;
	case 1:
		Mode(TURN_BACK, 0xFF);
		break;
	case 2:
		sign = 8;
		step = 0;
		back = 1;
		Mode(HU_LAR, 0xFF);
		break;
	}
	return next;
}
//自动驾驶脚本7to8
int CautocarDlg::_7To8(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x0C);//从7台走到8台
		break;
	case 1:
		sign = 8;
		step = 0;
		next = 0;
		back = 1;
		//exclude = 0;//清除去对台标志
		Mode(TURN_BACK, 0xFF);
		break;
	}
	return next;
}
//自动驾驶脚本8to7
int CautocarDlg::_8To7(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x0D);//从8台走到7台
		break;
	case 1:
		sign = 7;
		step = 0;
		next = 0;
		back = 1;
		//exclude = 0;//清除去对台标志
		Mode(TURN_BACK, 0xFF);
		break;
	}
	return next;
}
//自动驾驶脚本7to1
int CautocarDlg::_7To1(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x0E);//从7台走到1台
		break;
	case 1:
		Mode(TURN_BACK, 0xFF);
		break;
	case 2:
		sign = 1;
		step = 0;
		next = 2;
		back = 0;//回到起点返回标志清零
		Mode(RESET, 0xFF);
		break;
	}
	return next;
}
//自动驾驶脚本8to1
int CautocarDlg::_8To1(void)
{
	switch (step++) {
	case 0:
		Mode(NO_ACTION, 0x0F);//从8台走到1台
		break;
	case 1:
		Mode(TURN_BACK, 0xFF);
		break;
	case 2:
		sign = 1;
		step = 0;
		next = 2;
		back = 0;//回到起点返回标志清零
		Mode(RESET, 0xFF);
		break;
	}
	return next;
}


/* 定时器相关函数 ----------------------------------------------------*/

//定时器函数(定时器1：窗体显示用；定时器2：自动驾驶用；定时器3：未使用；定时器4：测试按钮用)
void CautocarDlg::OnTimer(UINT_PTR nIDEvent)
{
	UpdateData(true);
	CDialog::OnTimer(nIDEvent);

	if (nIDEvent == 1)
	{
		if (_cameraForPic.isOpened())
		{
			ontimer_frame.release();
			_cameraForPic >> ontimer_frame;
			//_ShowImageOnImageBox(IDC_ImageBox1, ontimer_frame);
		}
		else
		{
			// TAG:抛出异常
			KillTimer(1);
		}
	}
	if (nIDEvent == 2)
	{
		if (ReceiveCommand())//如果收到下位机发来的信息
		{
			if (go == 0) {//第一次收到下位机传来的信号起跑运行1to2脚本
				sign = 1;//现在在1台
				step = 0;//步骤置0
				go++;//跳出起跑
				OnBnClickedPatern12();
			}
			else {
				OnBnClickedPatern12();
			}

		}
	}
	if (nIDEvent == 4) {
		if (ReceiveCommand())//如果收到下位机发来的信息
		{
			switch (chose) {
			case 1:TEST12();
				break;
			case 2:TEST23();
				break;
			case 3:TEST24();
				break;
			case 4:TEST35();
				break;
			case 5:TEST36();
				break;
			case 6:TEST45();
				break;
			case 7:TEST46();
				break;
			case 8:TEST57();
				break;
			case 9:TEST58();
				break;
			case 10:TEST67();
				break;
			case 11:TEST68();
				break;
			case 12:TEST78();
				break;
			case 13:TEST87();
				break;
			case 14:TEST71();
				break;
			case 15:TEST81();
				break;
			default:;
				break;
			}
		}
	}
	UpdateData(false);
}
//打开串口
void CautocarDlg::_SerialOpen(int commNum /*=2*/, int baudRate /*=115200*/)
{
  DCB portSettingsDCB;
  if (!Open(commNum, CComPort::AutoReceiveBySignal))
  {
    // TAG:遇到错误应该抛出异常
    CString sMsg;
    sMsg.Format("提示:不能打开串口%d!", commNum);
    AfxMessageBox(sMsg, MB_ICONINFORMATION | MB_OK);
  }
  else
  {
    GetSerialPort()->GetState(portSettingsDCB);
    portSettingsDCB.BaudRate = 115200;
    GetSerialPort()->SetState(portSettingsDCB);
  }
}
//图像显示
void CautocarDlg::_ShowImageOnImageBox(int ImageBox, Mat frame)
{
  CRect rect;
  GetDlgItem(ImageBox)->GetClientRect(&rect);

  _cvvImage.CopyOf(&static_cast<IplImage>(frame), 1);
  _cvvImage.DrawToHDC(GetDlgItem(ImageBox)->GetDC()->GetSafeHdc(), &rect);
}

void CautocarDlg::_StretchBlt(int ImageBox, CDC & cdcSrc, int x, int y, int w, int h)
{
  CRect rect;
  GetDlgItem(ImageBox)->GetClientRect(&rect);
  GetDlgItem(ImageBox)->GetDC()->StretchBlt(rect.left, rect.top, rect.Width(), rect.Height(), &cdcSrc, x, y, w, h, SRCCOPY);
}

void CautocarDlg::_Binaryzation(const Mat & inputMat, Mat & outputMat)
{
  cvtColor(inputMat, outputMat, CV_BGR2GRAY);
  blur(outputMat, outputMat, Size(3, 3));
  dilate(outputMat, outputMat, getStructuringElement(MORPH_RECT, Size(1, 1)));
  threshold(outputMat, outputMat, 100, 255, CV_THRESH_OTSU);
}

Mat CautocarDlg::_Binaryzation(const Mat & inputMat)
{
  Mat outputMat;

  _Binaryzation(inputMat, outputMat);

  return outputMat;
}

void CautocarDlg::_FindContour(Mat & binaryMat, Contor_t &maximumInterContor)
{
  Contors_t contours_all, contours_external;
  maximumInterContor.empty();
  /** TAG: TM这个函数会改变binaryMat的数据内容，即使用const声明变量也会改变，
   *       暂时的解决办法是用.clone创建副本传入参数，但是这样一来就得用release()释放内存
   */
  findContours(binaryMat, contours_all, CV_RETR_TREE, CHAIN_APPROX_NONE);
  findContours(binaryMat, contours_external, CV_RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);
  binaryMat.release();
  _contours_all = contours_all;

  //TAG: Contors_t类型在大小判断时会出错，这里要显式大小比较
  if (contours_all.size() > contours_external.size())
  {
    for (Contor_t contor_external : contours_external)
    {
      auto pBegin = remove(contours_all.begin(), contours_all.end(), contor_external);
      contours_all.erase(pBegin, contours_all.end());
    }

    auto pBiggest = max_element(contours_all.begin(), contours_all.end(),
      [](Contor_t conA, Contor_t conB)
      {
        return conA.size() < conB.size();
      }
    );
    maximumInterContor = *pBiggest;
  }
}

Contor_t CautocarDlg::_FindContour(Mat & binaryMat)
{
  Contor_t maximumInterContor;

  _FindContour(binaryMat, maximumInterContor);

  return maximumInterContor;
}

const Contor_t & CautocarDlg::_FindContour()
{
  _FindContour(_binaryMat.clone(), _maximumInterContor);
  return _maximumInterContor;
}


/*图像识别算法-------------------------------------------------------*/

//图像模版匹配算法
int CautocarDlg::_TemplateMatching(Mat & srcMat)
{
  //TAG:未修改，场地上修改算法
  /** @TAG:
    * 这个算法和我原先构思的很想，就是既然比较一幅静态的图片，那么只需要将其进行
    * 相减，减法后得出来的差值越小，那么相似度就越高。
    * OpenCV内部的模板比较函数大致思想也就是两个图片比较，但是并不是这么简单的
    * 减法，它运用了概率论中的部分内容，其中注释掉的normalize部分，从网上获取
    * 的资料来看，类似于二重随机变量中的那个协方差变量。也就是说，这个函数的底层
    * 实现还需要深究。
    * 然而问题不是出在这个算法部分，主要的问题在，这个算法要求在一个巨大的Mat中
    * 匹配一个小的Mat，而Mat中最基本的元素单位是point，也就是说，如果拍摄到的
    * 图片中，我们的图案大小与待匹配图案的量级不同，那么识别效果差异就会很大，甚
    * 至不能识别，这是目前面临最大的问题，还没有解决。
    * 还有一个问题是，神奇的第六幅图，那一座山。考虑到竞赛官方也不傻，肯定对模板
    * 匹配算法进行了测试，所以给出了第六幅图干扰匹配结果，那么为什么第六幅图会干
    * 扰算法，如果消除干扰，也是这个部分需要研究的问题。
    * 
    * 最后，即使这部分可能效果没有我们老算法好，但是也具有可取的地方，等到了最终
    * 竞赛的时候，一切以效果为主，能凑就凑，不过与之对应的，在竞赛之前，兄弟们能
    * 多研究就多研究，电视剧网络上有存档，美女今天走了一位，明天会看见新的，多多
    * 学习吧。
    */
  vector<double> numberList(8, 2);
  int tag = 0;
  for(auto imageTemplate : _TARGET_IMAGE_LIST)
  {
    Mat dst;
    int match_method = TM_SQDIFF_NORMED;
    int width = srcMat.cols - imageTemplate.first.cols + 1;
    int height = srcMat.rows - imageTemplate.first.rows + 1;
    Mat result(width, height, CV_32FC1);//ERR: width 和 height 不能小于0

    matchTemplate(srcMat, imageTemplate.first, result, match_method);
    //normalize(result, result, 0, 1, NORM_MINMAX);

    Point minLoc;
    Point maxLoc;
    double min, max;
    srcMat.copyTo(dst);
    Point temLoc;
    minMaxLoc(result, &min, &max, &minLoc, &maxLoc, Mat());
    if (match_method == TM_SQDIFF || match_method == TM_SQDIFF_NORMED) {
      temLoc = minLoc;
      numberList[imageTemplate.second - 3] = min;
    }
    else {
      temLoc = maxLoc;
      numberList[imageTemplate.second - 3] = max;
    }
  }

  double minn = numberList[0];
  int _return = 0;
  for (int i = 1; i < 6; i++)
  {
    if (numberList[i] < minn)
    {
      minn = numberList[i];
      _return = i;
    }
  }
  return _return;
}
//哈希感知图像识别算法
int CautocarDlg::_HashMatching(Mat & srcMat)
{
	int retnum[6];
	for (auto imageTemplate : _TARGET_IMAGE_LIST1)
	{
		cv::Mat matSrc, matSrc0, matSrc1, matSrc2;
		CV_Assert(srcMat.channels() == 3);
		CV_Assert(imageTemplate.first.channels() == 3);
		cv::resize(srcMat, matSrc1, cv::Size(357, 419), 0, 0, cv::INTER_NEAREST);
		//cv::flip(matSrc1, matSrc1, 1);
		cv::resize(imageTemplate.first, matSrc2, cv::Size(2177, 3233), 0, 0, cv::INTER_LANCZOS4);

		cv::Mat matDst1, matDst2;

		cv::resize(matSrc1, matDst1, cv::Size(8, 8), 0, 0, cv::INTER_CUBIC);
		cv::resize(matSrc2, matDst2, cv::Size(8, 8), 0, 0, cv::INTER_CUBIC);
		//注意此处必须使用两个变量，官方例程中使用单变量会导致
		cv::Mat temp1 = matDst1;
		cv::Mat temp2 = matDst2;
		cv::cvtColor(temp1, matDst1, CV_BGR2GRAY);
		cv::cvtColor(temp2, matDst2, CV_BGR2GRAY);

		int iAvg1 = 0, iAvg2 = 0;
		int arr1[64], arr2[64];

		for (int i = 0; i < 8; i++)
		{
			uchar* data1 = matDst1.ptr<uchar>(i);
			uchar* data2 = matDst2.ptr<uchar>(i);

			int tmp = i * 8;

			for (int j = 0; j < 8; j++)
			{
				int tmp1 = tmp + j;

				arr1[tmp1] = data1[j] / 4 * 4;
				arr2[tmp1] = data2[j] / 4 * 4;

				iAvg1 += arr1[tmp1];
				iAvg2 += arr2[tmp1];
			}
		}

		iAvg1 /= 64;
		iAvg2 /= 64;

		for (int i = 0; i < 64; i++)
		{
			arr1[i] = (arr1[i] >= iAvg1) ? 1 : 0;
			arr2[i] = (arr2[i] >= iAvg2) ? 1 : 0;
		}

		int iDiffNum = 0;

		for (int i = 0; i < 64; i++)
		{
			if (arr1[i] != arr2[i])
				iDiffNum++;
		}
		retnum[imageTemplate.second - 3] = iDiffNum;

	}

	double minn = retnum[0];
	int _return = 0;
	for (int i = 0; i < 5; i++)
	{
		if (retnum[i] < minn)
		{
			minn = retnum[i];
			_return = i;
		}
	}
	return _return + 3;
}
//特征值识别算法
void CautocarDlg::_OldalgorithmMatching()
{
	//关闭摄像头
	if (!capture1)
	{
		capture1 = cvCaptureFromCAM(0);
	}
	if (!capture1)
	{
		AfxMessageBox("无法打开摄像头");
		return;
	}
	g_dConLength = 0;
	g_dConArea = 0;
	//显示到第2个图像
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);

	m_CvvImage.CopyOf(m_Frame, 1);
	m_CvvImage.DrawToHDC(hDC3, &rect3);

	cv::Mat src = m_Frame;
	// resize(src,src,Size(800,600));//标准大小
	cv::Mat src_gray;
	cv::Mat src_all = src.clone();
	cv::Mat threshold_output;
	std::vector<std::vector<cv::Point>> contours, contours2, contours_out, contours_all;
	std::vector<cv::Vec4i> hierarchy, hierarchy_out, hierarchy_all;
	//预处理
	cvtColor(src, src_gray, CV_BGR2GRAY);//灰度化处理
	blur(src_gray, src_gray, cv::Size(3, 3)); //模糊，去除毛刺
	cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));//第一个参数MORPH_RECT表示矩形的卷积核，当然还可以选择椭圆形的、交叉型的
	
	dilate(src_gray, src, element);//腐蚀操作(膨胀操作貌似注释错了)
	threshold(src, threshold_output, 100, 255, cv::THRESH_OTSU);//otsu最大类间方差法进行二值化

	m_Frame = &IplImage(threshold_output);
	m_CvvImage.CopyOf(m_Frame, 1);
	m_CvvImage.DrawToHDC(hDC2, &rect2);

	//寻找轮廓
	//第一个参数是输入图像 2值化的
	//第二个参数是内存存储器，FindContours找到的轮廓放到内存里面。
	//第三个参数是层级，**[Next, Previous, First_Child, Parent]** 的vector
	//第四个参数是类型，采用树结构
	//第五个参数是节点拟合模式，这里是全部寻找 findContours(image_contour_outside, contours_out, hierarchy_out, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	findContours(threshold_output, contours_out, hierarchy_out, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);//找外轮廓

	findContours(threshold_output, contours_all, hierarchy_all, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);//找所有轮廓
	cv::Mat Drawing = cv::Mat::zeros(threshold_output.size(), CV_8UC3);

	if (contours_all.size() == contours_out.size())
		return; //没有内轮廓，则提前返回

	for (int i = 0; i < contours_out.size(); i++)
	{
		int conloursize = contours_out[i].size();
		for (int j = 0; j < contours_all.size(); j++)
		{
			int tem_size = contours_all[j].size();
			if (conloursize == tem_size)
			{
				swap(contours_all[j], contours_all[contours_all.size() - 1]);
				contours_all.pop_back();
				break;
			}
		}
	}

	//contours_all中只剩下内轮廓
	//查找最大轮廓
	double maxarea = 0;
	int maxAreaIdx = 0;
	for (int index = contours_all.size() - 1; index >= 0; index--)
	{
		double tmparea = fabs(contourArea(contours_all[index]));
		if (tmparea > maxarea)
		{
			maxarea = tmparea;
			maxAreaIdx = index; //记录最大轮廓的索引号
		}
	}
	cv::Scalar color = cv::Scalar(0, 0, 255);
	//绘制最大内轮廓
	drawContours(Drawing, contours_all, maxAreaIdx, color, 2, 8, hierarchy_all, 0, cv::Point());
	//轮廓长度
	g_dConLength = arcLength(contours_all[maxAreaIdx], true);
	//轮廓面积
	g_dConArea = contourArea(contours_all[maxAreaIdx], true);

	//m_locationgold.Format("长度%f 面积%f 比例%f", g_dConLength, g_dConArea, g_dConArea / g_dConLength);
	m_locationgold.Format("%f", g_dConArea / g_dConLength);//返回轮廓面积与轮廓长度的比值

	m_Frame = &IplImage(Drawing);
	m_CvvImage.CopyOf(m_Frame, 1);
	m_CvvImage.DrawToHDC(hDC1, &rect1);

	//关闭摄像头
	//cvReleaseCapture(&capture1);

	// OnBnClickedBtopenvideo();

	UpdateData(false);

}
//轮廓识别算法
void CautocarDlg::_LKMatching()
{
	//关闭摄像头
	if (!capture1)
	{
		capture1 = cvCaptureFromCAM(0);
	}
	if (!capture1)
	{
		AfxMessageBox("无法打开摄像头");
		return;
	}
	g_dConLength = 0;
	//显示到第2个图像
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);

	m_CvvImage.CopyOf(m_Frame, 1);
	m_CvvImage.DrawToHDC(hDC3, &rect3);

	cv::Mat src = m_Frame;
	// resize(src,src,Size(800,600));//标准大小
	cv::Mat src_gray;
	cv::Mat src_all = src.clone();
	cv::Mat threshold_output;
	std::vector<std::vector<cv::Point>> contours, contours2, contours_out, contours_all;
	std::vector<cv::Vec4i> hierarchy, hierarchy_out, hierarchy_all;
	//预处理
	cvtColor(src, src_gray, CV_BGR2GRAY);//灰度化处理
	blur(src_gray, src_gray, cv::Size(3, 3)); //模糊，去除毛刺
	cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));//第一个参数MORPH_RECT表示矩形的卷积核，当然还可以选择椭圆形的、交叉型的

	dilate(src_gray, src, element);//腐蚀操作(膨胀操作貌似注释错了)
	threshold(src, threshold_output, 100, 255, cv::THRESH_OTSU);//otsu最大类间方差法进行二值化

	m_Frame = &IplImage(threshold_output);
	m_CvvImage.CopyOf(m_Frame, 1);
	m_CvvImage.DrawToHDC(hDC2, &rect2);

	//寻找轮廓
	//第一个参数是输入图像 2值化的
	//第二个参数是内存存储器，FindContours找到的轮廓放到内存里面。
	//第三个参数是层级，**[Next, Previous, First_Child, Parent]** 的vector
	//第四个参数是类型，采用树结构
	//第五个参数是节点拟合模式，这里是全部寻找 findContours(image_contour_outside, contours_out, hierarchy_out, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	findContours(threshold_output, contours_out, hierarchy_out, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);//找外轮廓

	findContours(threshold_output, contours_all, hierarchy_all, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);//找所有轮廓
	cv::Mat Drawing = cv::Mat::zeros(threshold_output.size(), CV_8UC3);

	if (contours_all.size() == contours_out.size())
		return; //没有内轮廓，则提前返回

	for (int i = 0; i < contours_out.size(); i++)
	{
		int conloursize = contours_out[i].size();
		for (int j = 0; j < contours_all.size(); j++)
		{
			int tem_size = contours_all[j].size();
			if (conloursize == tem_size)
			{
				swap(contours_all[j], contours_all[contours_all.size() - 1]);
				contours_all.pop_back();
				break;
			}
		}
	}
	//contours_all中只剩下内轮廓
	//查找最大轮廓
	double maxarea = 0;
	int maxAreaIdx = 0;
	for (int index = contours_all.size() - 1; index >= 0; index--)
	{
		double tmparea = fabs(contourArea(contours_all[index]));
		if (tmparea > maxarea)
		{
			maxarea = tmparea;
			maxAreaIdx = index; //记录最大轮廓的索引号
		}
	}
	cv::Scalar color = cv::Scalar(0, 0, 255);
	//绘制最大内轮廓
	drawContours(Drawing, contours_all, maxAreaIdx, color, 2, 8, hierarchy_all, 0, cv::Point());
	//轮廓长度
	g_dConLength = arcLength(contours_all[maxAreaIdx], true);
	UpdateData(false);
}
//面积识别算法
void CautocarDlg::_MJMatching()
{
	//关闭摄像头
	if (!capture1)
	{
		capture1 = cvCaptureFromCAM(0);
	}
	if (!capture1)
	{
		AfxMessageBox("无法打开摄像头");
		return;
	}
	g_dConArea = 0;
	//显示到第2个图像
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);
	::Sleep(20);
	m_Frame = cvQueryFrame(capture1);

	m_CvvImage.CopyOf(m_Frame, 1);
	m_CvvImage.DrawToHDC(hDC3, &rect3);

	cv::Mat src = m_Frame;
	// resize(src,src,Size(800,600));//标准大小
	cv::Mat src_gray;
	cv::Mat src_all = src.clone();
	cv::Mat threshold_output;
	std::vector<std::vector<cv::Point>> contours, contours2, contours_out, contours_all;
	std::vector<cv::Vec4i> hierarchy, hierarchy_out, hierarchy_all;
	//预处理
	cvtColor(src, src_gray, CV_BGR2GRAY);//灰度化处理
	blur(src_gray, src_gray, cv::Size(3, 3)); //模糊，去除毛刺
	cv::Mat element = getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));//第一个参数MORPH_RECT表示矩形的卷积核，当然还可以选择椭圆形的、交叉型的

	dilate(src_gray, src, element);//腐蚀操作(膨胀操作貌似注释错了)
	threshold(src, threshold_output, 100, 255, cv::THRESH_OTSU);//otsu最大类间方差法进行二值化

	m_Frame = &IplImage(threshold_output);
	m_CvvImage.CopyOf(m_Frame, 1);
	m_CvvImage.DrawToHDC(hDC2, &rect2);

	//寻找轮廓
	//第一个参数是输入图像 2值化的
	//第二个参数是内存存储器，FindContours找到的轮廓放到内存里面。
	//第三个参数是层级，**[Next, Previous, First_Child, Parent]** 的vector
	//第四个参数是类型，采用树结构
	//第五个参数是节点拟合模式，这里是全部寻找 findContours(image_contour_outside, contours_out, hierarchy_out, RETR_EXTERNAL, CHAIN_APPROX_NONE);

	findContours(threshold_output, contours_out, hierarchy_out, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_NONE);//找外轮廓

	findContours(threshold_output, contours_all, hierarchy_all, cv::RETR_TREE, cv::CHAIN_APPROX_NONE);//找所有轮廓
	cv::Mat Drawing = cv::Mat::zeros(threshold_output.size(), CV_8UC3);

	if (contours_all.size() == contours_out.size())
		return; //没有内轮廓，则提前返回

	for (int i = 0; i < contours_out.size(); i++)
	{
		int conloursize = contours_out[i].size();
		for (int j = 0; j < contours_all.size(); j++)
		{
			int tem_size = contours_all[j].size();
			if (conloursize == tem_size)
			{
				swap(contours_all[j], contours_all[contours_all.size() - 1]);
				contours_all.pop_back();
				break;
			}
		}
	}
	//contours_all中只剩下内轮廓
	//查找最大轮廓
	double maxarea = 0;
	int maxAreaIdx = 0;
	for (int index = contours_all.size() - 1; index >= 0; index--)
	{
		double tmparea = fabs(contourArea(contours_all[index]));
		if (tmparea > maxarea)
		{
			maxarea = tmparea;
			maxAreaIdx = index; //记录最大轮廓的索引号
		}
	}
	cv::Scalar color = cv::Scalar(0, 0, 255);
	//绘制最大内轮廓
	drawContours(Drawing, contours_all, maxAreaIdx, color, 2, 8, hierarchy_all, 0, cv::Point());
	//轮廓面积
	g_dConArea = contourArea(contours_all[maxAreaIdx], true);
	UpdateData(false);
}
//图像识别测试按钮
void CautocarDlg::OnBnClickedBt_ImageTest()
{
	if (_cameraForPic.isOpened()) {
		Mat inputMat;
		int output1=0,output2=0;
		_cameraForPic >> inputMat;
		output1 = _TemplateMatching(inputMat)+2;
		output2 = _HashMatching(inputMat);
		this->_OldalgorithmMatching();//老工程图像识别测试
		float res = atof(m_locationgold);//老工程图像识别测试
		this->_LKMatching();
		this->_MJMatching();
		CString str;
		str.Format(_T("模版匹配图像识别结果：%d\n哈希感知图像识别结果：%d\n特征值图像识别结果：%f\n轮廓图像识别结果：%f\n面积图像识别结果：%f\n"), output1,output2,res, g_dConLength,g_dConArea);
		AfxMessageBox(str);
		UpdateData(true);
		UpdateData(false);
	}
	else
	{
		AfxMessageBox("摄像头未连接");
	}
}
//照片拍摄按钮（图片会以‘随机数.png’的格式保存）
void CautocarDlg::OnBnClickedBttakephoto()
{
	if (_cameraForPic.isOpened()) {
	Mat frame;
	_cameraForPic >> frame;
	imshow("show", frame);
	char filename[20];
	int8_t randnum;
	randnum = 1 + rand() % (100 - 1 + 1);
	sprintf_s(filename, "拍照图片-%d.png", randnum);
	imwrite(filename, frame);
	}
	else
	{
		AfxMessageBox("摄像头未连接");
	}
}

/*图像识别测试-------------------------------------------------------*/

//3or4图像识别测试
void CautocarDlg::BTImageRecognition_3or4()
{
	if (_cameraForPic.isOpened()) {
		Mat input;
		this->_OldalgorithmMatching();
		float res = atof(m_locationgold);
		CString str;
		if (res < res34) {
			str.Format(_T("比较值：22\n比较结果：3\n特征值：%f\n"), res);
			AfxMessageBox(str);
		}
		else {
			str.Format(_T("比较值：22\n比较结果：4\n特征值：%f\n"), res);
			AfxMessageBox(str);
		}
	}
	else {
		AfxMessageBox("摄像头未打开");
	}
}
//5or6图像识别测试
void CautocarDlg::BTImageRecognition_5or6()
{
	if (_cameraForPic.isOpened()) {
		Mat input;
		this->_OldalgorithmMatching();
		float res = atof(m_locationgold);
		CString str;
		if (res < res56) {
			str.Format(_T("比较值：11\n比较结果：5\n特征值：%f\n"), res);
			AfxMessageBox(str);
		}
		else {
			str.Format(_T("比较值：11\n比较结果：6\n特征值：%f\n"), res);
			AfxMessageBox(str);
		}
	}
	else {
		AfxMessageBox("摄像头未打开");
	}
}
//7or8图像识别测试
void CautocarDlg::BTImageRecognition_7or8()
{
	if (_cameraForPic.isOpened()) {
		Mat input;
		this->_OldalgorithmMatching();
		float res = atof(m_locationgold);
		CString str;
		if (res < res78) {
			str.Format(_T("比较值：16\n比较结果：8\n特征值：%f\n"), res);
			AfxMessageBox(str);
		}
		else {
			str.Format(_T("比较值：16\n比较结果：7\n特征值：%f\n"), res);
			AfxMessageBox(str);
		}
	}
	else {
		AfxMessageBox("摄像头未打开");
	}
}

/*Edit框清空按钮-----------------------------------------------------*/

//清空接收框按钮
void CautocarDlg::OnBnClickedB_ClearEdit()
{
	SetDlgItemText(IDC_EDITREV, "");
}
//清空图像识别反馈框按钮
void CautocarDlg::OnBnClickedBtclearshow()
{
	SetDlgItemText(IDC_EDITSHOW, "");
}
