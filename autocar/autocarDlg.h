#pragma comment(lib, "vfw32")
#include <iostream>
#include <string>

#include "cv.h"
#include "highgui.h"
#include "CvvImage.h"
#include "opencv.hpp"
#include "debugrob.h"
#include "./serial/ComPort.h"
#include "resource.h"

using namespace std;
using namespace cv;
using namespace Debug;

// �Զ���MFC��Ϣ���� �������ݽ�����Ϣ @PostMessageFunc OnReceiveData()
#define WM_RECV_SERIAL_DATA WM_USER + 101
using Contors_t = vector<vector<Point>>;
using Contor_t = vector<Point>;

class CautocarDlg : virtual DebugLabComm::CComPort, public CDialog , public DebugRob
{
public:
  explicit CautocarDlg(CWnd* pParent = NULL);
  virtual BOOL OnInitDialog();
  afx_msg void OnClose();

  IplImage* m_Frame;
  CvvImage m_CvvImage;
  CString m_linedegree;
  /**
   * @func: DoDataExchange - DDX/DDV֧��
   *      : OnPaint - ��Ի��������С����ť
   *      : OnQueryDragIcon - ���û��϶���С������ʱϵͳ���ô˺���ȡ�ù����ʾ
   * @Message function mapping
   *      DECLARE_MESSAGE_MAP()
   *      @see autocarDlg.cpp BEGIN_MESSAGE_MAP
   */
  virtual void DoDataExchange(CDataExchange* pDX);
  afx_msg void OnPaint();
  afx_msg HCURSOR OnQueryDragIcon();
  DECLARE_MESSAGE_MAP()

  /* ������غ��� ********************************************/
  afx_msg void OnBnClickedBt_OpenSerial();
  afx_msg void OnBnClickedBt_CloseSerial();
  afx_msg void OnBnClickedBt_SendToSerial();
  void PrintlnToSerial(const string& message);
  char HexToChar(char bChar);
 // void PrintToSerial(const string& message);
  afx_msg LONG OnRecvSerialData(WPARAM wParam, LPARAM lParam);
  void SendData(const char arrays[], int lenth);

  /* OpenCV��غ��� *****************************************/
  afx_msg void OnBnClickedBt_OpenCamera();
  afx_msg void OnBnClickedBt_CloseCamera();
  afx_msg void OnBnClickedBt_AutoDrive();
  afx_msg void OnBnClickedBt_ImageIdentification();
  afx_msg void OnBnClickedBt_Test();
  afx_msg void OnBnClickedBt_ImageTest();
  afx_msg void OnBnClickedBttakephoto();
  void ImageRecognition(Mat src);
  afx_msg void BTImageRecognition_3or4();  //ͼ��ʶ�����
  afx_msg void BTImageRecognition_5or6();  //ͼ��ʶ�����
  afx_msg void BTImageRecognition_7or8();  //ͼ��ʶ�����
  /* ·����غ���********************************************/
  afx_msg void OnBnClickedBtauto12();//·�߲��԰�ť
  afx_msg void OnBnClickedBtauto23();
  afx_msg void OnBnClickedBtauto24();
  afx_msg void OnBnClickedBtauto35();
  afx_msg void OnBnClickedBtauto36();
  afx_msg void OnBnClickedBtauto45();
  afx_msg void OnBnClickedBtauto46();
  afx_msg void OnBnClickedBtauto57();
  afx_msg void OnBnClickedBtauto58();
  afx_msg void OnBnClickedBtauto67();
  afx_msg void OnBnClickedBtauto68();
  afx_msg void OnBnClickedBtauto78();
  afx_msg void OnBnClickedBtauto87();
  afx_msg void OnBnClickedBtauto75();
  afx_msg void OnBnClickedBtauto76();
  afx_msg void OnBnClickedBtauto85();
  afx_msg void OnBnClickedBtauto86();
  afx_msg void OnBnClickedBtauto53();
  afx_msg void OnBnClickedBtauto54();
  afx_msg void OnBnClickedBtauto63();
  afx_msg void OnBnClickedBtauto64();
  afx_msg void OnBnClickedBtauto32();
  afx_msg void OnBnClickedBtauto42();
  afx_msg void OnBnClickedBtauto21();
  afx_msg void OnBnClickedBtHU_LAR();
  afx_msg void OnBnClickedBtTURN_BACK();
  afx_msg void OnBnClickedPatern12();//�Զ���ʻ��
  afx_msg void OnBnClickedBtStop();//��ͣ
  afx_msg void OnBnClickedBtrst();//�����Զ���ʻ
  afx_msg void OnBnClickedBtStopautodrive();//ֹͣ�Զ���ʻ
  afx_msg void OnBnClickedB_ClearEdit();
  afx_msg void OnTimer(UINT_PTR nIDEvent);
  void TEST12(void);
  void TEST23(void);
  void TEST24(void);
  void TEST35(void);
  void TEST36(void);
  void TEST45(void);
  void TEST46(void);
  void TEST57(void);
  void TEST58(void);
  void TEST67(void);
  void TEST68(void);
  void TEST78(void);
  void TEST87(void);
  void TEST75(void);
  void TEST76(void);
  void TEST85(void);
  void TEST86(void);
  void TEST53(void);
  void TEST54(void);
  void TEST63(void);
  void TEST64(void);
  void TEST32(void);
  void TEST42(void);
  void TEST21(void);
  int _1To2(void);// 0x01
  int _2To3(void);// 0x02
  int _2To4(void);// 0x03
  int _3To5(void);// 0x04
  int _3To6(void);// 0x05
  int _4To5(void);// 0x06
  int _4To6(void);// 0x07
  int _5To7(void);// 0x08
  int _5To8(void);// 0x09
  int _6To7(void);// 0x0A
  int _6To8(void);// 0x0B
  int _7To8(void);// 0x0C
  int _8To7(void);// 0x0D
  int _7To5(void);// 0x0E
  int _7To6(void);// 0x0F
  int _8To5(void);// 0x10
  int _8To6(void);// 0x11
  int _5To3(void);// 0x12
  int _5To4(void);// 0x13
  int _6To3(void);// 0x14
  int _6To4(void);// 0x15
  int _3To2(void);// 0x16
  int _4To2(void);// 0x17
  int _2To1(void);// 0x18
  CString m_locationstart;
  CString m_locationnext;
  CString m_locationgold;
  int Findrst(char a[]);//�ж��Ƿ����ñ�־
  int sign;//��̬
  int next;//��̬
  int step;//����
  int go = 0;//�ж��Ƿ�Ϊ�����ã�0Ϊ����ָ��
  int chose = 0;//�жϽ����ĸ��ű����Ժ���
  int exclude1 = 0;//�ų���־1
  int exclude2 = 0;//�ų���־2
  int back = 0;//���ر�־����ֵΪ0��1��ʾ�Ƿ��ؽű���
  /* ����ģʽ��غ���********************************************/
  void Mode(PointMode_t pointMode, int8_t command);//����Э��
protected:
  HICON appIcon_;
private:
  void _SerialOpen(int commNum = 2, int baudRate = 115200);
  void _OnCommReceive(LPVOID pSender, void* pBuf, DWORD InBufferCount) override;
  void _OnCommBreak(LPVOID pSender, DWORD dwMask, COMSTAT stat) override;
  //TAG:����ImageBoxӦ����һ��ö�����ͣ��������
  void _ShowImageOnImageBox(int ImageBox, Mat& frame);
  void _StretchBlt(int ImageBox, CDC& cdcSrc,
    int x = 0, int y = 0, int w = 48, int h = 48);

  /* ͼ��ʶ���㷨 ********************************************/
  /**
   * @func: _Binaryzation - �Դ����Mat���ж�ֵ������
   *        _FindContour  - �Դ����Mat���д�����ȡ���������
   * @Message �㷨һ
   *    ����ͷ         -(��ȡͼƬ)->       OpenCV::Mat
   *    OpenCV::Mat   -(��ֵ������)->     �ڰ׵�Mat
   *    �ڰ׵�Mat      -(��ȡ����)->       ������
   *    ������         -(�������������)->  ���������
   *    ������         -(�����ܳ������)->  ����ֵ��ʶ������
   * 
   * @Message �㷨��
   *    OpenCV ģ��ƥ���㷨��Template matching��
   */
  void _Binaryzation(const Mat & inputMat, Mat & outputMat);
  Mat _Binaryzation(const Mat & inputMat);
  void _FindContour(Mat & binaryMat, Contor_t &maximumInterContor);
  Contor_t _FindContour(Mat & binaryMat);
  const Contor_t & _FindContour();
  int _TemplateMatching(Mat & srcMat);
  int _HashMatching(Mat & srcMat);
  void CautocarDlg::_OldalgorithmMatching();

  /* ˽�������� *********************************************/
  CString _msgSerialSend;
  CString _msgSerialReceive;
  CString rst="resetreset";
  CString reset;
  CvvImage _cvvImage;

  /* ͼ��ʶ���� *********************************************/
  const vector<pair<Mat, int>> _TARGET_IMAGE_LIST;
  const vector<pair<Mat, int>> _TARGET_IMAGE_LIST1;

  VideoCapture _cameraForPic;
  VideoCapture _cameraForPath;

  Mat _binaryMat;
  Contors_t _contours_all;
  Contor_t _maximumInterContor;
  //TAG: ����ֵӦ��Ψһ��������doubleֵ���һ��struct���
  double _conLength;
  double _conArea;
  //TAG: _mode������Ӧ������Ϊһ�� ö����
  int _mode;
};
