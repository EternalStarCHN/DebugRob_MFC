/*************************************************************************************************
*
*        ģ������:���̴߳���ͨѶ��(MultiThread Com class)
*        ������(author): liu_sir
*        ��������(date): 2003.4.20 -4.30
*        �޸���ǰ      : 2004.6.01 -6.14
*        ��ǰ�汾(Version) :v1.2
*        ��Ҫ��˵��(Main class):
*             CComPort ������  �ھۺ�һ��CSerialPort�����ڴ��ڵĲ���
*                      ����ģʽ˵��(Receive Mode):
*   	                 1.ManualReceiveByQuery,  //�ֶ���ѯ����
*		                   2.ManualReceiveByConst,  //��������(����һ����������)
*		                   3.AutoReceiveBySignal,   //�ź��Զ�����
*                      4.AutoReceiveByBreak,	   //�Զ��жϽ���
*                     �Ƽ�ʹ��:1��3
*		     CReadComThread ���ڶ��߳���  ��CComPort�����������в���
*
*
***************************************************************************************************/
#include "serialport.h"

#ifndef COMPORT_H
#define COMPORT_H

typedef void(*FOnReceiveData)(LPVOID, void*, DWORD);
typedef void(*FOnComBreak)(LPVOID, DWORD, COMSTAT stat);

namespace DebugLabComm {

  class CReadComThread;

  class CComPort
  {
  public:
    enum ReceiveMode
    {
      ManualReceiveByQuery,  //�ֶ���ѯ����
      ManualReceiveByConst,  //��������
      AutoReceiveBySignal,   //�ź��Զ�����
      AutoReceiveByBreak,	   //�Զ��жϽ���
    };

    enum FlowControl
    {
      NoFlowControl,
      CtsRtsFlowControl,
      CtsDtrFlowControl,
      DsrRtsFlowControl,
      DsrDtrFlowControl,
      XonXoffFlowControl
    };

    enum Parity
    {
      EvenParity,
      MarkParity,
      NoParity,
      OddParity,
      SpaceParity
    };

    enum StopBits
    {
      OneStopBit,
      OnePointFiveStopBits,
      TwoStopBits
    };

    CComPort(LPVOID pSender);
    virtual ~CComPort();
    CComPort(CComPort& cComPort) = delete;

    //1.��,�رմ��ں���
    bool Open(int nPort, ReceiveMode mode = AutoReceiveBySignal, DWORD dwBaud = 9600, Parity parity = NoParity, BYTE DataBits = 8,
      StopBits stopbits = OneStopBit, FlowControl fc = NoFlowControl);
    bool IsOpen(void);
    void Close();

    //2.���ý��պ���,�жϴ����� 
    void SetReceiveFunc(FOnReceiveData pfnOnReceiveData, LPVOID pSender);
    void SetBreakHandleFunc(FOnComBreak pfnOnComBreak);
    //3.��ȡ�������
    int GetCurPortNum() { return this->m_CurPortNum; }
    CSerialPort* GetSerialPort();
    HANDLE GetCloseHandle();
    ReceiveMode GetReceiveMode();

    //4.(�߳���)֪ͨ���մ�����     
    void ReceiveData(void* pBuf, DWORD InBufferCount); //�̵߳��õĽ��պ���
    void ComBreak(DWORD dwMask);

    //6.����,�������--����ʵ�ʸ���
    DWORD GetInBufferCount();
    DWORD GetInput(void* pBuf, DWORD Count, DWORD dwMilliseconds = 1000);
    DWORD Output(const char* pBuf, DWORD Count);
    bool IsOverlapped() { return m_IsOverlapped; }

  protected:
    CSerialPort* _serialPort;                      //�ں�������
    CReadComThread* m_pReadThread;                  //�������߳� 

    LPVOID m_pSender;                               //����ĸ�����ָ��
    int m_CurPortNum;                               //��ǰ�˿ں� 
    FOnReceiveData m_pfnOnReceiveData;              //�����źź���
    FOnComBreak m_pfnOnComBreak;                    //�����¼�������
    ReceiveMode m_RecvMode;                         //����ģʽ

    HANDLE m_hWriteEvent;                           //д�¼�
    OVERLAPPED m_WriteOverlapped;                   //д�ص��ṹ

    bool m_IsOverlapped;                            //�Ƿ��ص��ṹ;
    bool m_bIsOpen;

  private:
    virtual void _OnCommReceive(LPVOID pSender, void* pBuf, DWORD InBufferCount) = 0;
    virtual void _OnCommBreak(LPVOID pSender, DWORD dwMask, COMSTAT stat) = 0;
    
    HANDLE m_hCloseEvent; //E: A event handle to close thread  //Chinese:�����߳��¼�
  };

  //DWORD WINAPI ThreadFunc(LPVOID  lpParam ); //�̵߳��ú��� 
  
  class CReadComThread
  {
  public:
    CReadComThread();
    virtual ~CReadComThread();

    /* 2.����,����,��λ*/
    void Create();                         //�����߳�
    void Terminate();                      //�����߳�
    void Resume();                         //��λ 
    bool IsTerminated() { return this->m_IsTerminated; }

    /*3.�󶨴���,�첽��ȡ*/
    void BandSerialPort(CComPort* pPort);  //�󶨴���
    DWORD ReadInput(void* pBuf, DWORD Count, DWORD dwMilliseconds);//�첽��ȡ����

    friend DWORD WINAPI ThreadFunc(LPVOID  lpParam);
  protected:
    DWORD dwThreadId;//�̺߳�  
    bool IsClose;
    /*4.�����첽��ȡ�¼�,�첽�ж��¼��Լ������¼�*/
    bool SetReadEvent(OVERLAPPED& overlapped);//�����������¼�
    bool HandleReadEvent(OVERLAPPED& overlapped);//������¼�
    bool HandleData(); //�����ȡ����

    bool SetBreakEvent(DWORD& dwMask);//���ô����ж��¼�,ͨ��DWMask�����ĸı䷵�ؼ���״̬
    bool HandleBreakEvent(DWORD dwMask);//�������ж��¼�

                                        /*5.�ֶ�ģʽ,�ź�ģʽ,�ж�ģʽִ���߳�
                                        */
    void ExecuteByAutoSignalRecvMode();
    void ExecuteByAutoBreakRecvMode();
    void ExecuteByManualQueryRecvMode();
    void ExecuteByManualConstRecvMode();
    void Execute(void);                    //�߳�ִ��

  private:
    HANDLE m_hThread;           //�߳̾��
    CComPort* m_pPort;          //��������ָ��

    byte  m_InputBuffer[2048];  //���ջ�����
    byte* m_pBuffer;            //ʵ�ʵ��ڴ� 
    DWORD m_InBufferCount;      //���ո��� 

    OVERLAPPED m_ReadOverlapped;     //��ȡ�ص��ṹ
    OVERLAPPED m_BreakOverlapped;    //�����ж��¼��ṹ

    bool m_IsTerminated;            //�Ƿ�����߳�
  };
}

#endif