/*************************************************************************************************
*
*        模块名称:多线程串口通讯类(MultiThread Com class)
*        编制人(author): liu_sir
*        创建日期(date): 2003.4.20 -4.30
*        修改日前      : 2004.6.01 -6.14
*        当前版本(Version) :v1.2
*        主要类说明(Main class):
*             CComPort 串口类  内聚合一个CSerialPort类用于串口的操作
*                      接收模式说明(Receive Mode):
*   	                 1.ManualReceiveByQuery,  //手动查询接收
*		                   2.ManualReceiveByConst,  //定数接收(按照一定数量接收)
*		                   3.AutoReceiveBySignal,   //信号自动接收
*                      4.AutoReceiveByBreak,	   //自动中断接收
*                     推荐使用:1或3
*		     CReadComThread 串口读线程类  和CComPort关联起来进行操作
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
      ManualReceiveByQuery,  //手动查询接收
      ManualReceiveByConst,  //定数接收
      AutoReceiveBySignal,   //信号自动接收
      AutoReceiveByBreak,	   //自动中断接收
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

    //1.打开,关闭串口函数
    bool Open(int nPort, ReceiveMode mode = AutoReceiveBySignal, DWORD dwBaud = 9600, Parity parity = NoParity, BYTE DataBits = 8,
      StopBits stopbits = OneStopBit, FlowControl fc = NoFlowControl);
    bool IsOpen(void);
    void Close();

    //2.设置接收函数,中断处理函数 
    void SetReceiveFunc(FOnReceiveData pfnOnReceiveData, LPVOID pSender);
    void SetBreakHandleFunc(FOnComBreak pfnOnComBreak);
    //3.获取自身参数
    int GetCurPortNum() { return this->m_CurPortNum; }
    CSerialPort* GetSerialPort();
    HANDLE GetCloseHandle();
    ReceiveMode GetReceiveMode();

    //4.(线程类)通知接收处理函数     
    void ReceiveData(void* pBuf, DWORD InBufferCount); //线程调用的接收函数
    void ComBreak(DWORD dwMask);

    //6.输入,输出函数--返回实际个数
    DWORD GetInBufferCount();
    DWORD GetInput(void* pBuf, DWORD Count, DWORD dwMilliseconds = 1000);
    DWORD Output(const char* pBuf, DWORD Count);
    bool IsOverlapped() { return m_IsOverlapped; }

  protected:
    CSerialPort* _serialPort;                      //内含串口类
    CReadComThread* m_pReadThread;                  //读串口线程 

    LPVOID m_pSender;                               //保存的父窗体指针
    int m_CurPortNum;                               //当前端口号 
    FOnReceiveData m_pfnOnReceiveData;              //接收信号函数
    FOnComBreak m_pfnOnComBreak;                    //串口事件处理函数
    ReceiveMode m_RecvMode;                         //接收模式

    HANDLE m_hWriteEvent;                           //写事件
    OVERLAPPED m_WriteOverlapped;                   //写重叠结构

    bool m_IsOverlapped;                            //是否重叠结构;
    bool m_bIsOpen;

  private:
    virtual void _OnCommReceive(LPVOID pSender, void* pBuf, DWORD InBufferCount) = 0;
    virtual void _OnCommBreak(LPVOID pSender, DWORD dwMask, COMSTAT stat) = 0;
    
    HANDLE m_hCloseEvent; //E: A event handle to close thread  //Chinese:结束线程事件
  };

  //DWORD WINAPI ThreadFunc(LPVOID  lpParam ); //线程调用函数 
  
  class CReadComThread
  {
  public:
    CReadComThread();
    virtual ~CReadComThread();

    /* 2.创建,结束,复位*/
    void Create();                         //创建线程
    void Terminate();                      //结束线程
    void Resume();                         //复位 
    bool IsTerminated() { return this->m_IsTerminated; }

    /*3.绑定串口,异步读取*/
    void BandSerialPort(CComPort* pPort);  //绑定串口
    DWORD ReadInput(void* pBuf, DWORD Count, DWORD dwMilliseconds);//异步读取输入

    friend DWORD WINAPI ThreadFunc(LPVOID  lpParam);
  protected:
    DWORD dwThreadId;//线程号  
    bool IsClose;
    /*4.设置异步读取事件,异步中断事件以及处理事件*/
    bool SetReadEvent(OVERLAPPED& overlapped);//设置启动读事件
    bool HandleReadEvent(OVERLAPPED& overlapped);//处理读事件
    bool HandleData(); //处理读取数据

    bool SetBreakEvent(DWORD& dwMask);//设置串口中断事件,通过DWMask参数的改变返回监听状态
    bool HandleBreakEvent(DWORD dwMask);//处理串口中断事件

                                        /*5.手动模式,信号模式,中断模式执行线程
                                        */
    void ExecuteByAutoSignalRecvMode();
    void ExecuteByAutoBreakRecvMode();
    void ExecuteByManualQueryRecvMode();
    void ExecuteByManualConstRecvMode();
    void Execute(void);                    //线程执行

  private:
    HANDLE m_hThread;           //线程句柄
    CComPort* m_pPort;          //关联串口指针

    byte  m_InputBuffer[2048];  //接收缓冲区
    byte* m_pBuffer;            //实际的内存 
    DWORD m_InBufferCount;      //接收个数 

    OVERLAPPED m_ReadOverlapped;     //读取重叠结构
    OVERLAPPED m_BreakOverlapped;    //串口中断事件结构

    bool m_IsTerminated;            //是否结束线程
  };
}

#endif