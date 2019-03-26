// ComPort.cpp : 实现文件
//

#include "stdafx.h"
#include "ComPort.h"

namespace DebugLabComm {

  CComPort::CComPort(LPVOID pSender)
    :m_pSender(pSender)
  {
    this->m_pfnOnReceiveData = NULL;
    this->m_pfnOnComBreak = NULL;
    this->_serialPort = NULL;
    this->m_pReadThread = NULL;
    ::ZeroMemory(&this->m_WriteOverlapped, sizeof(this->m_WriteOverlapped));
    this->m_hWriteEvent = NULL;
    this->m_bIsOpen = false;
  }

  CComPort::~CComPort()
  {
    if (this->_serialPort)
    {
      if (this->_serialPort->IsOpen())
      {        
        this->Close();
      }
      delete this->_serialPort;
      this->_serialPort = NULL;
    }
  }

  bool CComPort::IsOpen(void)
  {
    return this->m_bIsOpen;
  }

  bool CComPort::Open(int nPort, ReceiveMode mode, DWORD dwBaud, Parity parity, BYTE DataBits,
    StopBits stopbits, FlowControl fc)
  {
    //1.新建串口
    if (this->_serialPort)
    {
      delete this->_serialPort;
    }
    this->_serialPort = new CSerialPort();
    this->m_bIsOpen = false;

    //2.判断收发模式
    if (mode == ReceiveMode::ManualReceiveByQuery)
    {
      this->m_IsOverlapped = false;
    }
    else
    {
      this->m_IsOverlapped = true;
    }
    this->m_RecvMode = mode;

    //3.转换参数,打开串口
    int index;
    index = parity - CComPort::EvenParity;
    CSerialPort::Parity spParity = (CSerialPort::Parity)(CSerialPort::EvenParity + index);
    index = stopbits - CComPort::OneStopBit;
    CSerialPort::StopBits spStopbits = (CSerialPort::StopBits)(CSerialPort::OneStopBit + index);
    index = fc - CComPort::NoFlowControl;
    CSerialPort::FlowControl spFC = (CSerialPort::FlowControl)(CSerialPort::NoFlowControl + index);

    try
    {
      this->_serialPort->Open(nPort, dwBaud, spParity, DataBits, spStopbits, spFC, m_IsOverlapped);
    }
    catch (CSerialException* pE)
    {
      //AfxMessageBox(pE->GetErrorMessage());
      pE->Delete();
      return false;
    }

    //it is important!!
    COMMTIMEOUTS timeouts;
    this->_serialPort->GetTimeouts(timeouts);
    timeouts.ReadIntervalTimeout = 20;
    //timeouts.WriteTotalTimeoutConstant = 1000;
    this->_serialPort->SetTimeouts(timeouts);
    this->_serialPort->Purge(PURGE_TXABORT | PURGE_RXABORT | PURGE_TXCLEAR | PURGE_RXCLEAR);
    this->_serialPort->Setup(4096, 4096);
    this->m_CurPortNum = nPort;

    //创建关闭事件
    this->m_hCloseEvent = CreateEvent(NULL, true, false, NULL);
    if (this->m_hCloseEvent == NULL)
    {
      return false;
    }

    //4.创建线程类
    this->m_pReadThread = new CReadComThread();
    this->m_pReadThread->BandSerialPort(this);
    this->m_pReadThread->Create();
    this->m_pReadThread->Resume();

    if (this->IsOverlapped())
    {
      this->m_hWriteEvent = ::CreateEvent(NULL, true, false, NULL);
      if (this->m_hCloseEvent == NULL)
        return false;
      this->m_WriteOverlapped.hEvent = this->m_hWriteEvent;
    }
    this->m_bIsOpen = true;
    return true;
  }

  void CComPort::Close()
  {
    //1.串口
    if (this->_serialPort == NULL)
      return;
    if (!this->_serialPort->IsOpen())
      return;

    //2.事件
    ::SetEvent(this->m_hCloseEvent);//通知关闭系统
    Sleep(1000);

    //3.结束读线程
    try
    {
      this->m_pReadThread->Terminate();
      delete this->m_pReadThread;
      this->m_pReadThread = NULL;
    }
    catch (char e[150])
    {
      ::AfxMessageBox(e);
    }

    //4.结束关闭线程
    ::CloseHandle(this->m_hCloseEvent);
    this->_serialPort->Close();

    //5.结束写事件
    if (this->m_hWriteEvent)
    {
      ::CloseHandle(this->m_hWriteEvent);
      this->m_hWriteEvent = NULL;
    }
    //6.释放串口对象
    if (this->_serialPort)
    {
      delete this->_serialPort;
      this->_serialPort = NULL;
    }
  }

  void CComPort::ReceiveData(void* pBuf, DWORD InBufferCount)
  {
    if (this->m_pfnOnReceiveData) {
      this->m_pfnOnReceiveData(this->m_pSender, pBuf, InBufferCount);
    }
    else
    {
      _OnCommReceive(this->m_pSender, pBuf, InBufferCount);
    }
  }

  void CComPort::SetReceiveFunc(FOnReceiveData pfnOnReceiveData, LPVOID pSender)
  {
    this->m_pfnOnReceiveData = pfnOnReceiveData;
    this->m_pSender = pSender;
  }

  void CComPort::ComBreak(DWORD dwMask)
  {
    COMSTAT stat;
    this->_serialPort->GetStatus(stat);

    if (this->m_pfnOnComBreak)
    {
      this->m_pfnOnComBreak(this->_serialPort, dwMask, stat);
    }
    else
    {
      _OnCommBreak(this->_serialPort, dwMask, stat);
    }
  }

  void CComPort::SetBreakHandleFunc(FOnComBreak pfnOnComBreak)
  {
    this->m_pfnOnComBreak = pfnOnComBreak;
  }



  CComPort::ReceiveMode CComPort::GetReceiveMode()
  {
    return this->m_RecvMode;
  }

  DWORD CComPort::GetInBufferCount()
  {
    if (this->IsOverlapped())
    {
      ::AfxMessageBox("this methord is only used for ManualQueryMode!");
      return 0;
    }
    COMSTAT stat;
    ::ZeroMemory(&stat, sizeof(stat));
    this->_serialPort->GetStatus(stat);
    return stat.cbInQue;
  }


  DWORD CComPort::GetInput(void* pBuf, DWORD Count, DWORD dwMilliseconds)
  {
    //不能在自动模式下getinput
    if (this->GetReceiveMode() == CComPort::AutoReceiveByBreak ||
      this->GetReceiveMode() == CComPort::AutoReceiveBySignal)
    {
      ::AfxMessageBox("Can't use GetInput methord in this mode!");
      return 0;
    }

    if (this->IsOverlapped())
    {
      ASSERT(this->m_pReadThread);
      DWORD dwBytes = this->m_pReadThread->ReadInput(pBuf, Count, dwMilliseconds);
      this->_serialPort->TerminateOutstandingReads();
      return dwBytes;
    }
    else
      return this->_serialPort->Read(pBuf, Count);
  }

  DWORD CComPort::Output(const char* pBuf, DWORD Count)
  {
    DWORD dwWriteBytes = 0;
    if (this->IsOverlapped())//异步模式
    {
      this->_serialPort->Write(pBuf, Count, this->m_WriteOverlapped);
      if (WaitForSingleObject(this->m_WriteOverlapped.hEvent, INFINITE) == WAIT_OBJECT_0)
      {
        this->_serialPort->GetOverlappedResult(this->m_WriteOverlapped, dwWriteBytes, false);
      }
    }
    else
    {
      dwWriteBytes = this->_serialPort->Write(pBuf, Count);
    }
    return dwWriteBytes;
  }

  CSerialPort* CComPort::GetSerialPort()
  {
    ASSERT(_serialPort);
    return _serialPort;
  }
  HANDLE CComPort::GetCloseHandle()
  {
    ASSERT(this->m_hCloseEvent);
    return this->m_hCloseEvent;
  }

  //CReadComThread

  CReadComThread::CReadComThread()
  {
    this->m_hThread = NULL;
    this->m_pPort = NULL;
    this->IsClose = false;
    ::ZeroMemory(&this->m_BreakOverlapped, sizeof(this->m_BreakOverlapped));
    ::ZeroMemory(&this->m_ReadOverlapped, sizeof(this->m_ReadOverlapped));

    memset(this->m_InputBuffer, 0, 2048);
  }

  CReadComThread::~CReadComThread()
  {
    this->m_hThread = NULL;
  }


  // CReadComThread 成员函数
  bool CReadComThread::SetReadEvent(OVERLAPPED& overlapped)
  {
  BeginSet:
    memset(this->m_InputBuffer, 0, 2048);
    if (this->m_pPort->GetSerialPort()->Read(this->m_InputBuffer, 2048, overlapped, &this->m_InBufferCount))
    {
      if (!this->HandleData())
        return  false;
      ::ResetEvent(this->m_ReadOverlapped.hEvent);
      goto BeginSet;
    }
    DWORD error = ::GetLastError();
    if (error == ERROR_IO_PENDING)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  bool CReadComThread::HandleData() //处理读取数据
  {
    if (this->m_InBufferCount>0)
    {
      this->m_pBuffer = new byte[this->m_InBufferCount];
      for (int i = 0; i<(int)this->m_InBufferCount; i++)
      {
        this->m_pBuffer[i] = this->m_InputBuffer[i];
      }
      //this->m_pSerialPort->  
      this->m_pPort->ReceiveData(this->m_pBuffer, this->m_InBufferCount);
      delete[] this->m_pBuffer;
    }
    return true;
  }

  bool CReadComThread::HandleReadEvent(OVERLAPPED& overlapped)
  {
    if (this->m_pPort->GetSerialPort()->GetOverlappedResult(overlapped, this->m_InBufferCount, false))
    {
      return this->HandleData();
    }

    DWORD dwError = ::GetLastError();
    if (dwError == ERROR_INVALID_HANDLE)
      return false;
    else
      return true;
  }

  bool CReadComThread::SetBreakEvent(DWORD& dwMask)
  {
  SetBegin:
    if (this->m_pPort->GetSerialPort()->WaitEvent(dwMask, this->m_BreakOverlapped))
    {
      if (!this->HandleBreakEvent(dwMask))
        return false;
      goto SetBegin;
    }

    DWORD error = ::GetLastError();
    if (error == ERROR_IO_PENDING)
    {
      return true;
    }
    else
    {
      return false;
    }
  }

  bool CReadComThread::HandleBreakEvent(DWORD dwMask)
  {
    DWORD dwReadBytes;
    bool successed =
      this->m_pPort->GetSerialPort()->GetOverlappedResult(this->m_BreakOverlapped, dwReadBytes, false);
    if (successed)
    {
      this->m_pPort->ComBreak(dwMask); //调用处理过程
      return true;
    }
    return false;
  }

  void CReadComThread::Execute()
  {

    if (this->m_pPort->GetReceiveMode() == CComPort::ManualReceiveByQuery)
    {
      this->ExecuteByManualQueryRecvMode();
    }
    else if (this->m_pPort->GetReceiveMode() == CComPort::ManualReceiveByConst)
    {
      this->ExecuteByManualConstRecvMode();
    }
    else if (this->m_pPort->GetReceiveMode() == CComPort::AutoReceiveBySignal)
    {
      this->ExecuteByAutoSignalRecvMode();
    }
    else//中断模式
    {
      this->ExecuteByAutoBreakRecvMode();
    }
  }

  void CReadComThread::ExecuteByAutoSignalRecvMode()
  {
    DWORD dwMask = 0;
    HANDLE WaitHandles[3]; //监听事件数组
    DWORD dwSignaledHandle;
    //DWORD dwStoredFlags = EV_ERR | EV_RLSD | EV_RING;
    DWORD dwStoredFlags = EV_BREAK | EV_CTS | EV_DSR | EV_ERR | EV_RING | \
      EV_RLSD | EV_RXCHAR | EV_RXFLAG | EV_TXEMPTY;

    WaitHandles[0] = this->m_pPort->GetCloseHandle();
    WaitHandles[1] = this->m_ReadOverlapped.hEvent;
    WaitHandles[2] = this->m_BreakOverlapped.hEvent;

    this->m_pPort->GetSerialPort()->SetMask(dwStoredFlags);
    if (!SetBreakEvent(dwMask))
      goto EndThread;
    //设置读事件
    if (!SetReadEvent(this->m_ReadOverlapped))
      goto EndThread;

    //设置comEvent
    for (;;)
    {
      dwSignaledHandle = ::WaitForMultipleObjects(3, WaitHandles, false, INFINITE);
      switch (dwSignaledHandle)
      {
      case WAIT_OBJECT_0:
        goto EndThread;
        break;

      case WAIT_OBJECT_0 + 1:
        if (!this->HandleReadEvent(this->m_ReadOverlapped))
          goto EndThread;
        if (!this->SetReadEvent(this->m_ReadOverlapped))
          goto EndThread;
        break;

      case WAIT_OBJECT_0 + 2:
        if (!this->HandleBreakEvent(dwMask))
          goto EndThread;

        if (!this->SetBreakEvent(dwMask))
          goto EndThread;
        break;

      default:
        //goto EndThread;
        break;
      }


    }

  EndThread:
    this->m_pPort->GetSerialPort()->Purge(PURGE_RXABORT | PURGE_RXCLEAR);
    ::CloseHandle(this->m_ReadOverlapped.hEvent);
    ::CloseHandle(this->m_BreakOverlapped.hEvent);
    return;

  }

  void CReadComThread::ExecuteByManualQueryRecvMode()
  {
    DWORD dwMask = 0;
    HANDLE WaitHandles[2]; //监听事件数组
    DWORD dwSignaledHandle;

    WaitHandles[0] = this->m_pPort->GetCloseHandle();

    /*this->m_pPort->GetSerialPort()->SetMask(dwStoredFlags);
    this->m_pPort->GetSerialPort()->SetBreak(); */
    for (;;)
    {
      dwSignaledHandle = ::WaitForMultipleObjects(1, WaitHandles, false, INFINITE);
      switch (dwSignaledHandle)
      {
      case WAIT_OBJECT_0:
        goto EndThread;
        break;

      default:
        //goto EndThread;
        break;
      }
      this->m_pPort->GetSerialPort()->GetMask(dwMask);
      if (dwMask>0)
      {
        this->m_pPort->ComBreak(dwMask);
      }
    }

  EndThread:
    this->m_pPort->GetSerialPort()->Purge(PURGE_RXABORT | PURGE_RXCLEAR);
    return;

  }

  void CReadComThread::ExecuteByManualConstRecvMode()
  {
    DWORD dwMask = 0;
    HANDLE WaitHandles[2]; //监听事件数组
    DWORD dwSignaledHandle;
    DWORD dwStoredFlags = EV_BREAK | EV_CTS | EV_DSR | EV_ERR | EV_RING | \
      EV_RLSD | EV_RXCHAR | EV_RXFLAG | EV_TXEMPTY;

    WaitHandles[0] = this->m_pPort->GetCloseHandle();
    WaitHandles[1] = this->m_BreakOverlapped.hEvent;

    this->m_pPort->GetSerialPort()->SetMask(dwStoredFlags);

    if (!SetBreakEvent(dwMask))
      goto EndThread;

    //设置comEvent
    for (;;)
    {
      dwSignaledHandle = ::WaitForMultipleObjects(2, WaitHandles, false, INFINITE);
      switch (dwSignaledHandle)
      {
      case WAIT_OBJECT_0:
        goto EndThread;
        break;

      case WAIT_OBJECT_0 + 1:
        if (!this->HandleBreakEvent(dwMask))
          goto EndThread;
        if (!this->SetBreakEvent(dwMask))
          goto EndThread;
        break;

      default:
        //goto EndThread;
        break;
      }


    }

  EndThread:
    this->m_pPort->GetSerialPort()->Purge(PURGE_RXABORT | PURGE_RXCLEAR);
    ::CloseHandle(this->m_ReadOverlapped.hEvent);
    ::CloseHandle(this->m_BreakOverlapped.hEvent);
    return;

  }
  void CReadComThread::ExecuteByAutoBreakRecvMode()
  {
    DWORD dwMask = 0;
    HANDLE WaitHandles[2]; //监听事件数组
    DWORD dwSignaledHandle;
    DWORD dwStoredFlags = EV_BREAK | EV_CTS | EV_DSR | EV_ERR | EV_RING | \
      EV_RLSD | EV_RXCHAR | EV_RXFLAG;//??| EV_TXEMPTY 添加后在首次执行时总是接收不到全部数据

    WaitHandles[0] = this->m_pPort->GetCloseHandle();
    WaitHandles[1] = this->m_BreakOverlapped.hEvent;


    this->m_pPort->GetSerialPort()->SetMask(dwStoredFlags);

    //this->m_BreakOverlapped??两个事件同时给一个重叠结果的话??
    if (!SetBreakEvent(dwMask))
      goto EndThread;
    //设置读事件
    if (!SetReadEvent(this->m_BreakOverlapped))
      goto EndThread;

    //设置comEvent
    for (;;)
    {
      dwSignaledHandle = ::WaitForMultipleObjects(2, WaitHandles, false, INFINITE);
      switch (dwSignaledHandle)
      {
      case WAIT_OBJECT_0:
        goto EndThread;
        break;

      case WAIT_OBJECT_0 + 1:
        if ((dwMask&EV_RXCHAR) == EV_RXCHAR)
        {
          if (!this->HandleReadEvent(this->m_BreakOverlapped))
            goto EndThread;
          if (!SetReadEvent(this->m_BreakOverlapped))
            goto EndThread;
        }
        else
        {
          if (!this->HandleBreakEvent(dwMask))
            goto EndThread;
          if (!this->SetBreakEvent(dwMask))
            goto EndThread;
        }
        break;

      default:
        //goto EndThread;
        break;
      }


    }

  EndThread:
    this->m_pPort->GetSerialPort()->Purge(PURGE_RXABORT | PURGE_RXCLEAR);
    ::CloseHandle(this->m_ReadOverlapped.hEvent);
    ::CloseHandle(this->m_BreakOverlapped.hEvent);
    return;

  }

  DWORD WINAPI ThreadFunc(LPVOID  lpParam)
  {
    CReadComThread* pThread = (CReadComThread*)lpParam;
    ASSERT(pThread);
    pThread->m_IsTerminated = false;
    pThread->Execute();
    pThread->m_IsTerminated = true;
    return 0;
  }

  void CReadComThread::Create()
  {
    m_hThread = CreateThread(
      NULL,                        // no security attributes 
      0,                           // use default stack size  
      ThreadFunc,                  // thread function 
      this,                // argument to thread function 
      CREATE_SUSPENDED,           // use default creation flags 
      &dwThreadId);                // returns the thread identifier 
    ::SetThreadPriority(m_hThread, THREAD_PRIORITY_HIGHEST);

  }

  void CReadComThread::Terminate()
  {
    char szMsg[80];
    if (m_hThread == NULL)
    {
      wsprintf(szMsg, "CreateThread failed.");
      ::MessageBox(NULL, szMsg, "ok", 0);
    }
    else
    {
      if (!this->IsTerminated())
      {
        Sleep(1000);
      }
      if (!this->IsTerminated())
      {
        Sleep(1000);
        //::TerminateThread(m_hThread,0);
      }
      CloseHandle(m_hThread);
    }
  }

  void CReadComThread::Resume()
  {
    ResumeThread(this->m_hThread);
  }

  void CReadComThread::BandSerialPort(CComPort* pPort)
  {
    ASSERT(pPort);
    this->m_pPort = pPort;
    //创建异步读取事件

    if (this->m_pPort->IsOverlapped())
    {
      this->m_ReadOverlapped.hEvent = ::CreateEvent(NULL, true, false, NULL);
      ASSERT(this->m_ReadOverlapped.hEvent);
      this->m_BreakOverlapped.hEvent = ::CreateEvent(NULL, true, false, NULL);
      ASSERT(this->m_BreakOverlapped.hEvent);
    }

  }

  DWORD CReadComThread::ReadInput(void* pBuf, DWORD Count, DWORD dwMilliseconds)
  {
    DWORD dwRead = 0;
    if (!this->m_pPort->GetSerialPort()->Read(pBuf, Count, this->m_ReadOverlapped, &dwRead))
    {
      if (WaitForSingleObject(this->m_ReadOverlapped.hEvent, dwMilliseconds) == WAIT_OBJECT_0)
      {
        this->m_pPort->GetSerialPort()->GetOverlappedResult(this->m_ReadOverlapped, dwRead, false);
      }
    }

    return dwRead;

  }
}
