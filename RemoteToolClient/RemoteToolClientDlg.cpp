
// RemoteToolClientDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "RemoteToolClient.h"
#include "RemoteToolClientDlg.h"
#include "afxdialogex.h"
#include "FileProcess.h"
#include <Tlhelp32.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


#define HAVE_TIMER
std::vector<CString> split(std::wstring str, std::wstring pattern)
{
	std::wstring::size_type pos;
	std::vector<CString> result;
	size_t size = str.size();

	for (size_t i = 0; i < size; i++)
	{
		pos = str.find(pattern, i);
		if (pos < size)
		{
			CString s;
			s = str.substr(i, pos - i).c_str();
			result.push_back(s);
			i = pos + pattern.size() - 1;
		}
	}
	if (result.back() == "")
		result.erase(result.end() - 1);
	return result;
}

typedef struct tagParamData
{
	HANDLE hMyRead;
	HWND hWnd;
}ParamData, *lpParamData;
// 用于应用程序“关于”菜单项的 CAboutDlg 对话框

class CAboutDlg : public CDialogEx
{
	public:
	CAboutDlg();

	// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_ABOUTBOX };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);    // DDX/DDV 支持

// 实现
	protected:
	DECLARE_MESSAGE_MAP()
};

CAboutDlg::CAboutDlg() : CDialogEx(IDD_ABOUTBOX)
{
}

void CAboutDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
}

BEGIN_MESSAGE_MAP(CAboutDlg, CDialogEx)
END_MESSAGE_MAP()


// CRemoteToolClientDlg 对话框



CRemoteToolClientDlg::CRemoteToolClientDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_REMOTETOOLCLIENT_DIALOG, pParent)
	, m_strIP(_T(""))
	, m_strPort(_T(""))
	, m_client(nullptr)
	, m_threadPool(8)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

// 标准构造函数

CRemoteToolClientDlg::~CRemoteToolClientDlg()
{
}

void CRemoteToolClientDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Text(pDX, IDC_EDIT_IP, m_strIP);
	DDX_Text(pDX, IDC_EDIT_PORT, m_strPort);
}

BEGIN_MESSAGE_MAP(CRemoteToolClientDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON_CONNECT, &CRemoteToolClientDlg::OnBnClickedButtonConnect)
	ON_BN_CLICKED(IDC_BUTTON_DISCONNECT, &CRemoteToolClientDlg::OnBnClickedButtonDisconnect)
	ON_MESSAGE(WM_UPDATE_OUTPUT, &CRemoteToolClientDlg::OnUpdateOutput)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CRemoteToolClientDlg 消息处理程序

BOOL CRemoteToolClientDlg::OnInitDialog()
{
	CDialog::OnInitDialog();

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

	// TODO: 在此添加额外的初始化代码
	TcpClient::GlobalInit();


	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}


void CRemoteToolClientDlg::OnSysCommand(UINT nID, LPARAM lParam)
{
	if ((nID & 0xFFF0) == IDM_ABOUTBOX)
	{
		CAboutDlg dlgAbout;
		dlgAbout.DoModal();
	}
	else
	{
		CDialog::OnSysCommand(nID, lParam);
	}
}

// 如果向对话框添加最小化按钮，则需要下面的代码
//  来绘制该图标。  对于使用文档/视图模型的 MFC 应用程序，
//  这将由框架自动完成。

void CRemoteToolClientDlg::OnPaint()
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
		CDialog::OnPaint();
	}
}

//当用户拖动最小化窗口时系统调用此函数取得光标
//显示。
HCURSOR CRemoteToolClientDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRemoteToolClientDlg::OnBnClickedButtonConnect()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_client == nullptr)
	{
		m_client = new TcpClient(10086, "127.0.0.1");
		m_client->SetReceiveCallback(ReceiveCallback, this);
		TcpClient::MessageData message;
		message.length = 0;
		message.type = TcpClient::MessageType::Nop;
		message.data = nullptr;
		m_client->Send(message);
#ifdef HAVE_TIMER
		SetTimer(1, 5000, NULL);

		SetTimer(2, 10000, NULL);
#endif

		//CreateThread(NULL, 0, ReceiveThreadProc, this, 0, NULL);
	}
}


void CRemoteToolClientDlg::OnBnClickedButtonDisconnect()
{
	// TODO: 在此添加控件通知处理程序代码
	if (m_client != nullptr)
	{
		delete m_client;
		m_client = nullptr;
	}
}


BOOL CRemoteToolClientDlg::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类
	if (m_client != nullptr)
	{
		delete m_client;
		m_client = nullptr;
	}
	TcpClient::GlobalCleanUp();
	return CDialog::DestroyWindow();
}

int CRemoteToolClientDlg::ReceiveCallback(TcpClient::MessageData* message, void* uesrParam)
{
	CRemoteToolClientDlg* thisDlg = (CRemoteToolClientDlg*)uesrParam;
	TcpClient::MessageData *newMessage = new TcpClient::MessageData;
	newMessage->length = message->length;
	newMessage->data = new char[newMessage->length];
	memcpy_s(newMessage->data, newMessage->length, message->data, message->length);
	newMessage->type = message->type;
	switch (message->type)
	{
	case Socket::MessageType::CmdOpen:
	{
		ITask* task = new TaskCmdOpen(thisDlg, newMessage);
		thisDlg->m_threadPool.AddWork(task);
	}
	break;
	case Socket::MessageType::CmdCommandRequset:
	{
		ITask* task = new TaskCmdCommandRequset(thisDlg, newMessage);
		thisDlg->m_threadPool.AddWork(task);
	}
	break;
	case Socket::MessageType::ScreenOpen:
	case Socket::MessageType::ScreenCommandRequset:
	{
		ITask* task = new TaskScreenCommandRequset(thisDlg, newMessage);
		thisDlg->m_threadPool.AddWork(task);
	}
	break;
	case Socket::MessageType::FileQueryRequset:
	{
		ITask* task = new TaskFileQueryRequset(thisDlg, newMessage);
		thisDlg->m_threadPool.AddWork(task);
	}
	break;
	case Socket::MessageType::FileDownloadRequset:
	{
		ITask* task = new TaskFileDownloadRequset(thisDlg, newMessage);
		thisDlg->m_threadPool.AddWork(task);
	}
	break;
	case Socket::MessageType::FileUploadReply:
	{
		ITask* task = new TaskFileUploadReply(thisDlg, newMessage);
		thisDlg->m_threadPool.AddWork(task);
	}
	break;
	case Socket::MessageType::FileUploadReplyIsDir:
	{
		ITask* task = new TaskFileUploadReplyIsDir(thisDlg, newMessage);
		thisDlg->m_threadPool.AddWork(task);
	}
	break;
	case Socket::MessageType::ProcessOpenRequest:
	{
		ITask* task = new TaskProcessOpenRequest(thisDlg, newMessage);
		thisDlg->m_threadPool.AddWork(task);
	}
	break;
	case Socket::MessageType::ProcessTerminateRequest:
	{
		ITask* task = new TaskProcessTerminateRequest(thisDlg, newMessage);
		thisDlg->m_threadPool.AddWork(task);
	}
	break;
	case Socket::MessageType::HeartBeatReply:
	{
		ITask* task = new TaskHeartBeatReply(thisDlg, newMessage);
		thisDlg->m_threadPool.AddWork(task);
	}
	break;
	}

	return 0;
}

DWORD CRemoteToolClientDlg::PrintScreen(LPVOID lpParameter)
{
	CRemoteToolClientDlg* thisDlg = (CRemoteToolClientDlg*)lpParameter;
	//while (true)
	{
		HWND hWnd = ::GetDesktopWindow();//获得屏幕的HWND.  
		HDC hScreenDC = ::GetDC(hWnd);   //获得屏幕的HDC.  
		HDC MemDC = ::CreateCompatibleDC(hScreenDC);
		RECT rect;
		::GetWindowRect(hWnd, &rect);
		SIZE screensize;
		screensize.cx = rect.right - rect.left;
		screensize.cy = rect.bottom - rect.top;
		HBITMAP hBitmap = ::CreateCompatibleBitmap(hScreenDC, screensize.cx, screensize.cy);
		HGDIOBJ hOldBMP = ::SelectObject(MemDC, hBitmap);
		::BitBlt(MemDC, 0, 0, screensize.cx, screensize.cy, hScreenDC, rect.left, rect.top, SRCCOPY);
		::SelectObject(MemDC, hOldBMP);
		::DeleteObject(MemDC);
		::ReleaseDC(hWnd, hScreenDC);

		LONG size = screensize.cx * screensize.cy * 4;
		TcpClient::MessageData* newMessage = new TcpClient::MessageData;
		newMessage->length = size + sizeof(LONG) * 2;
		newMessage->data = new char[size + sizeof(LONG) * 2];
		newMessage->type = Socket::MessageType::ScreenCommandReply;
		LONG ret = GetBitmapBits(hBitmap, size, newMessage->data + sizeof(LONG) * 2);
		memcpy_s(newMessage->data, sizeof(LONG), &screensize.cx, sizeof(LONG));
		memcpy_s(newMessage->data + sizeof(LONG), sizeof(LONG), &screensize.cy, sizeof(LONG));
		thisDlg->m_client->Send(*newMessage);
		DeleteObject(hOldBMP);
		DeleteObject(hBitmap);
		delete newMessage;
		//Sleep(100);
	}
	return 0;
}

DWORD CRemoteToolClientDlg::CmdThreadProc(LPVOID lpParameter)
{
	Sleep(100);
	CRemoteToolClientDlg* thisDlg = (CRemoteToolClientDlg*)lpParameter;
	HANDLE hMyRead = INVALID_HANDLE_VALUE;                      // read handle
	thisDlg->m_hCmdWritePipe = INVALID_HANDLE_VALUE;                      // write handle
	HANDLE hCmdRead = INVALID_HANDLE_VALUE;                      // read handle
	HANDLE hCmdWrite = INVALID_HANDLE_VALUE;                      // write handle


	SECURITY_ATTRIBUTES sa = { 0 };  // security attributes
	sa.nLength = sizeof(SECURITY_ATTRIBUTES);
	sa.bInheritHandle = TRUE;
	DWORD nSize = 0;
	BOOL bRet = CreatePipe(&hCmdRead,
		&thisDlg->m_hCmdWritePipe,
		&sa,
		nSize);

	if (!bRet)
	{
		return -1;
	}

	bRet = CreatePipe(&hMyRead,
		&hCmdWrite,
		&sa,
		nSize);

	if (!bRet)
	{
		return -1;
	}


	//1. 创建真正的cmd.exe
	STARTUPINFO si = { 0 };
	si.cb = sizeof(STARTUPINFO);

	si.hStdInput = hCmdRead;
	si.hStdOutput = hCmdWrite;
	si.hStdError = hCmdWrite;

	si.dwFlags = STARTF_USESTDHANDLES;

	thisDlg->m_pi = { 0 };
	bRet = CreateProcess(_T("C:\\windows\\system32\\cmd.exe"),
		NULL,
		NULL,
		NULL,
		TRUE,
		CREATE_NO_WINDOW,
		NULL,
		NULL,
		&si,
		&thisDlg->m_pi);

	DWORD dwBytesRead = 0;            // number of bytes read
	DWORD dwTotalBytesAvail = 0;     // number of bytes available

	lpParamData param = new ParamData;
	param->hMyRead = hMyRead;
	param->hWnd = thisDlg->GetSafeHwnd();
	HANDLE hThread = CreateThread(NULL,
		0,
		CMDThreadProc,
		param,
		0,
		NULL);

	if (hThread == NULL)
	{
		return -1;
	}

	return 0;
}

DWORD WINAPI CRemoteToolClientDlg::CMDThreadProc(LPVOID lpParameter)   // thread data);
{
	HANDLE hMyRead = ((lpParamData)lpParameter)->hMyRead;
	while (true)
	{
		DWORD dwBytesRead = 0;            // number of bytes read
		DWORD dwTotalBytesAvail = 0;     // number of bytes available
		BOOL bRet = PeekNamedPipe(hMyRead,
			NULL,
			0,
			&dwBytesRead,
			&dwTotalBytesAvail,
			NULL);

		if (bRet && dwTotalBytesAvail <= 0)
		{
			Sleep(100);
			continue;
		}
		else if (!bRet)
		{
			break;
		}

		char* pszReadBuf = new char[dwTotalBytesAvail + 1];
		memset(pszReadBuf, 0, dwTotalBytesAvail + 1);
		DWORD dwReadeddBytes = 0;
		bRet = ReadFile(hMyRead, pszReadBuf, dwTotalBytesAvail, &dwReadeddBytes, NULL);
		::PostMessage(((lpParamData)lpParameter)->hWnd, WM_UPDATE_OUTPUT, NULL, (LPARAM)pszReadBuf);
	}
	return 0;
}

afx_msg LRESULT CRemoteToolClientDlg::OnUpdateOutput(WPARAM wParam, LPARAM lParam)
{
	m_client->Send(Socket::MessageType::CmdCommandReply, (char*)lParam);
	delete (char*)lParam;
	return 0;
}


BOOL CRemoteToolClientDlg::OnProcessOpenRequset()
{
	HANDLE hProcessSnap;
	HANDLE hProcess;
	PROCESSENTRY32 pe32;
	CString csProcessInfo;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (hProcessSnap == INVALID_HANDLE_VALUE)
	{
		TRACE(TEXT("CreateToolhelp32Snapshot (of processes)"));
		return FALSE;
	}

	// Set the size of the structure before using it.
	pe32.dwSize = sizeof(PROCESSENTRY32);

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		TRACE(TEXT("Process32First")); // show cause of failure
		CloseHandle(hProcessSnap);          // clean the snapshot object
		return FALSE;
	}
	DWORD dwRow = 0;
	// Now walk the snapshot of processes, and
	// display information about each process in turn
	do
	{
		int nPos = 0;
		//TRACE(TEXT("\n\n====================================================="));
		//TRACE(TEXT("\nPROCESS NAME:  %s"), pe32.szExeFile);
		//TRACE(TEXT("\n-------------------------------------------------------"));
		csProcessInfo.Append(pe32.szExeFile);
		csProcessInfo.Append(L"\n");
		CString strPID;
		strPID.Format(_T("%u"), pe32.th32ProcessID);
		csProcessInfo.Append(strPID);
		csProcessInfo.Append(L"\n");

		CString strParentPID;
		strParentPID.Format(_T("%u"), pe32.th32ParentProcessID);
		csProcessInfo.Append(strPID);
		csProcessInfo.Append(L"\n");
		hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, pe32.th32ProcessID);
		if (hProcess == NULL)
		{
			TRACE(TEXT("OpenProcess"));
			csProcessInfo.Append(L" \n");
		}
		else
		{


			DWORD dwMaxSize = MAX_PATH;
			TCHAR szPath[MAX_PATH] = { 0 };
			QueryFullProcessImageName(hProcess, 0, szPath, &dwMaxSize);
			csProcessInfo.Append(szPath);
			csProcessInfo.Append(L"\n");
			CloseHandle(hProcess);
		}
		dwRow++;
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	m_client->Send(Socket::MessageType::ProcessOpenReply, csProcessInfo.GetBuffer());
	return TRUE;
}

void CRemoteToolClientDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值
	if (nIDEvent == 1)
	{
		m_client->Send(Socket::MessageType::HeartBeatRequset, "");
	}
	else if (nIDEvent == 2)
	{
		DestroyWindow();
	}
	CDialog::OnTimer(nIDEvent);
}
