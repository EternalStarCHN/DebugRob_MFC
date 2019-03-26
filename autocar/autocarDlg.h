#pragma comment(lib, "vfw32")
#include <iostream>
#include <string>

#include "cv.h"
#include "CvvImage.h"
#include "opencv.hpp"
#include "debugrob.h"
#include "./serial/ComPort.h"
#include "resource.h"
using namespace std;
using namespace cv;
using namespace Debug;
// 自定义MFC消息常量 串口数据接收消息 @PostMessageFunc OnReceiveData()
#define WM_RECV_SERIAL_DATA WM_USER + 101
using Contors_t = vector<vector<Point>>;
using Contor_t = vector<Point>;

class CautocarDlg : virtual DebugLabComm::CComPort, public CDialog , public DebugRob
{
public:
  explicit CautocarDlg(CWnd* pParent = NULL);
  virtual BOOL OnInitDialog();
  afx_msg void OnClose();

  /**
   * @func: DoDataExchange - DDX/DDV支持
   *      : OnPaint - 向对话框添加最小化按钮
   *      : OnQueryDragIcon - 当用户拖动最小化窗口时系统调用此函数取得光标显示
   * @Message function mapping
   *      DECLARE_MESSAGE_MAP()
   *      @see autocarDlg.cpp BEGIN_MESSAGE_MAP
   */
  virtual void DoDataExchange(CDataExchange* pDX);
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  DECLARE_MESSAGE_MAP()

  /* 串口相关函数 ********************************************/
  afx_msg void OnBnClickedBt_OpenSerial();
  afx_msg void OnBnClickedBt_CloseSerial();
  afx_msg void OnBnClickedBt_SendToSerial();
  void PrintlnToSerial(const string& message);
  void PrintToSerial(const string& message);
  afx_msg LONG OnRecvSerialData(WPARAM wParam, LPARAM lParam);
  //SendData arrays从unsigned char 改为了 const char
  void SendData(const char arrays[], int lenth);

  /* OpenCV相关函数 *****************************************/
  afx_msg void OnBnClickedBt_OpenCamera();
  afx_msg void OnBnClickedBt_CloseCamera();
  afx_msg void OnBnClickedBt_AutoDrive();
  afx_msg void OnBnClickedBt_ImageIdentification();
  afx_msg void OnBnClickedBt_Test();
  void ImageRecognition(Mat src);

  /* 路线相关函数********************************************/
  afx_msg void OnBnClickedPatern12();
  afx_msg void OnBnClickedBtauto12();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  int _1To2(void);
  int _2To3(void);
  int _2To4(void);
  int _3To5(void);
  int _3To6(void);
  int _4To5(void);
  int _4To6(void);
  int _5To7(void);
  int _5To8(void);
  int _6To7(void);
  int _6To8(void);
  CString m_locationstart;
  CString m_locationnext;
  CString m_locationgold;
  int sign;//现态
  int next;//次态
  int step;//步骤
  
  /* 两种模式相关函数********************************************/
  void PointMode(PointMode_t, int16_t angle = 0, uint8_t lineSpeed = 10);//点模式
  /* 0xF0 0x04 0xhh 0xhh 0xhh 0x0A */
  void LineMode(uint8_t point, uint8_t delay = 0, uint8_t transSpeed = 40);//线模式
  /* 0xF0 0x03 0xhh 0xhh 0x0A */






protected:
  HICON appIcon_;

private:
  void _SerialOpen(int commNum = 2, int baudRate = 115200);
  void _OnCommReceive(LPVOID pSender, void* pBuf, DWORD InBufferCount) override;
  void _OnCommBreak(LPVOID pSender, DWORD dwMask, COMSTAT stat) override;

  //TAG:这里ImageBox应该是一个枚举类型，避免错误
  void _ShowImageOnImageBox(int ImageBox, Mat& frame);
  void _StretchBlt(int ImageBox, CDC& cdcSrc,
    int x = 0, int y = 0, int w = 48, int h = 48);

  /* 图像识别算法 ********************************************/
  /**
   * @func: _Binaryzation - 对传入的Mat进行二值化处理
   *        _FindContour  - 对传入的Mat进行处理并获取最大内轮廓
   * @Message 算法一
   *    摄像头         -(读取图片)->       OpenCV::Mat
   *    OpenCV::Mat   -(二值化处理)->     黑白的Mat
   *    黑白的Mat      -(获取轮廓)->       轮廓组
   *    轮廓组         -(查找最大内轮廓)->  最大内轮廓
   *    内轮廓         -(计算周长、面积)->  特征值（识别量）
   * 
   * @Message 算法二
   *    OpenCV 模板匹配算法（Template matching）
   */
  void _Binaryzation(const Mat & inputMat, Mat & outputMat);
  Mat _Binaryzation(const Mat & inputMat);
  void _FindContour(Mat & binaryMat, Contor_t &maximumInterContor);
  Contor_t _FindContour(Mat & binaryMat);
  const Contor_t & _FindContour();
  int _TemplateMatching(Mat & srcMat);

  /* 私有数据区 *********************************************/
  CString _msgSerialSend;
  CString _msgSerialReceive;

  CvvImage _cvvImage;

  /* 图形识别用 *********************************************/
  const vector<pair<Mat, int>> _TARGET_IMAGE_LIST;

  VideoCapture _cameraForPic;
  VideoCapture _cameraForPath;

  Mat _binaryMat;
  Contors_t _contours_all;
  Contor_t _maximumInterContor;
  //TAG: 特征值应该唯一，将两个double值变成一个struct会好
  double _conLength;
  double _conArea;

  //TAG: _mode的类型应该设置为一个 枚举类
  int _mode;

public:
	afx_msg void OnBnClickedBt_ImageTest();//图像识别测试
};
