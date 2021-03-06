
// RemoteToolServerDlg.cpp: 实现文件
//

#include "stdafx.h"
#include "RemoteToolServer.h"
#include "RemoteToolServerDlg.h"
#include "afxdialogex.h"
#include "TcpSocketManager.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif
#define HAVE_TIMER

static CRemoteToolServerDlg* g_this;
static std::vector<CString> split(std::wstring str, std::wstring pattern)
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


// CRemoteToolServerDlg 对话框



CRemoteToolServerDlg::CRemoteToolServerDlg(CWnd* pParent /*=NULL*/)
	: CDialog(IDD_REMOTETOOLSERVER_DIALOG, pParent),
	m_server(nullptr)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CRemoteToolServerDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialog::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_LIST_CLIENT, m_clientList);
}

BEGIN_MESSAGE_MAP(CRemoteToolServerDlg, CDialog)
	ON_WM_SYSCOMMAND()
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_COMMAND(ID_MENU_CMD, &CRemoteToolServerDlg::OnMenuCmd)
	ON_COMMAND(ID_MENU_PROCESS, &CRemoteToolServerDlg::OnMenuProcess)
	ON_COMMAND(ID_MENU_DESKTOP, &CRemoteToolServerDlg::OnMenuDesktop)
	ON_NOTIFY(NM_RCLICK, IDC_LIST_CLIENT, &CRemoteToolServerDlg::OnRclickListClient)
	ON_COMMAND(ID_MENU_FILE, &CRemoteToolServerDlg::OnMenuFile)
	ON_WM_TIMER()
END_MESSAGE_MAP()


// CRemoteToolServerDlg 消息处理程序

BOOL CRemoteToolServerDlg::OnInitDialog()
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
	g_this = this;
	Socket::GlobalInit();
	m_clientList.ModifyStyle(LVS_ICON, LVS_REPORT | LVS_SINGLESEL);
	m_clientList.SetExtendedStyle(m_clientList.GetExtendedStyle() | LVS_EX_DOUBLEBUFFER | LVS_EX_GRIDLINES | LVS_EX_FULLROWSELECT);
	m_clientList.InsertColumn(0, _T("clientName"), LVCFMT_LEFT, 100);
	m_clientList.InsertColumn(1, _T("ip"), LVCFMT_LEFT, 100);
	m_clientList.InsertColumn(2, _T("port"), LVCFMT_LEFT, 50);

	tcpSocketManager.StartServer(10086);
	m_server = tcpSocketManager.geTcpServer();
	m_server->SetAcceptCallback(AcceptCallback, this);
	//CreateThread(NULL, 0, AcceptThreadProc, this, 0, NULL);
	return TRUE;  // 除非将焦点设置到控件，否则返回 TRUE
}

void CRemoteToolServerDlg::OnSysCommand(UINT nID, LPARAM lParam)
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

void CRemoteToolServerDlg::OnPaint()
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
HCURSOR CRemoteToolServerDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CRemoteToolServerDlg::OnMenuCmd()
{
	// TODO: 在此添加命令处理程序代码
	Socket::MessageData message;
	message.data = nullptr;
	message.length = 0;
	message.type = Socket::MessageType::CmdOpen;
	m_selectClient->remoteCmdDialog = new RemoteCmdDialog;
	m_selectClient->remoteCmdDialog->m_socketClient = m_selectClient->tcpClient;
	m_selectClient->remoteCmdDialog->Create(IDD_DIALOG_CMD);
	m_selectClient->remoteCmdDialog->ShowWindow(SW_SHOWNORMAL);
	m_selectClient->tcpClient->Send(message);
}


void CRemoteToolServerDlg::OnMenuProcess()
{
	// TODO: 在此添加命令处理程序代码
	Socket::MessageData message;
	message.data = nullptr;
	message.length = 0;
	message.type = Socket::MessageType::ProcessOpenRequest;
	m_selectClient->remoteProcessDialog = new RemoteProcess;
	m_selectClient->remoteProcessDialog->m_socketClient = m_selectClient->tcpClient;
	m_selectClient->remoteProcessDialog->Create(IDD_DIALOG_PROCESS);
	m_selectClient->remoteProcessDialog->ShowWindow(SW_SHOWNORMAL);
	m_selectClient->tcpClient->Send(message);
}


void CRemoteToolServerDlg::OnMenuDesktop()
{
	// TODO: 在此添加命令处理程序代码
	Socket::MessageData message;
	message.data = nullptr;
	message.length = 0;
	message.type = Socket::MessageType::ScreenOpen;
	m_selectClient->remoteDialog = new RemoteDialog;
	m_selectClient->remoteDialog->Create(IDD_DIALOG_DESKTOP);
	m_selectClient->remoteDialog->ShowWindow(SW_SHOWNORMAL);
	m_selectClient->tcpClient->Send(message);
}

void CRemoteToolServerDlg::OnMenuFile()
{
	// TODO: 在此添加命令处理程序代码
	m_selectClient->remoteFileDialog = new RemoteFileDialog;
	m_selectClient->remoteFileDialog->Create(IDD_DIALOG_FILE);
	m_selectClient->remoteFileDialog->ShowWindow(SW_SHOWNORMAL);
	m_selectClient->remoteFileDialog->m_socketClient = m_selectClient->tcpClient;
	m_selectClient->tcpClient->Send(Socket::MessageType::FileQueryRequset, L"");
}

void CRemoteToolServerDlg::OnRclickListClient(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	POINT pt;
	GetCursorPos(&pt);

	if (pNMItemActivate->iItem >= 0 && pNMItemActivate->iItem <= m_clientList.GetItemCount())
	{
		m_selectClient = (TcpClientData*)m_clientList.GetItemData(pNMItemActivate->iItem);
		CMenu menu;
		menu.LoadMenu(IDR_MENU1);
		CMenu *pSubMenu = NULL;
		pSubMenu = menu.GetSubMenu(0);
		pSubMenu->TrackPopupMenu(TPM_LEFTALIGN | TPM_RIGHTBUTTON,
			pt.x, pt.y, this);
	}
	*pResult = 0;
}

int CRemoteToolServerDlg::AcceptCallback(TcpClient* client, void* uesrParam)
{
	CRemoteToolServerDlg* thisDlg = (CRemoteToolServerDlg*)uesrParam;
	TcpClientData* clientData = new TcpClientData;
	clientData->tcpClient = client;
	client->SetReceiveCallback(ReceiveCallback, clientData);
	int itemCount = thisDlg->m_clientList.GetItemCount();
	thisDlg->m_clientList.InsertItem(itemCount, L"");

	CString strIP;
	strIP = client->GetIP().c_str();
	thisDlg->m_clientList.SetItemText(itemCount, 1, strIP);
	CString strPort;
	strPort.Format(L"%d", client->GetPort());
	thisDlg->m_clientList.SetItemText(itemCount, 2, strPort);
	thisDlg->m_clientList.SetItemData(itemCount, (DWORD_PTR)clientData);
#ifdef HAVE_TIMER
	thisDlg->SetTimer(client->m_socket, 20000, NULL);
#endif

	return 0;
}


BOOL CRemoteToolServerDlg::DestroyWindow()
{
	// TODO: 在此添加专用代码和/或调用基类

	Socket::GlobalCleanUp();
	return CDialog::DestroyWindow();
}

int CRemoteToolServerDlg::ReceiveCallback(TcpClient::MessageData* message, void* uesrParam)
{
	
	TcpClientData* clientData = (TcpClientData*)uesrParam;
	if (message == nullptr)
	{
		int itemCount = g_this->m_clientList.GetItemCount();
		for(int i = 0; i < itemCount; i++)
		{
			if(clientData == (TcpClientData*)g_this->m_clientList.GetItemData(i))
			{
				g_this->m_clientList.DeleteItem(i);
				return 0;
			}
		}
	}
	switch (message->type)
	{
	case Socket::MessageType::CmdCommandReply:
	{
		char* szbuf = new char[message->length + 1];
		memcpy_s(szbuf, message->length, message->data, message->length);
		szbuf[message->length] = '\0';
		::PostMessage(clientData->remoteCmdDialog->GetSafeHwnd(), WM_UPDATE_OUTPUT, NULL, (LPARAM)szbuf);
	}
	break;
	case Socket::MessageType::ScreenCommandReply:
	{
		SIZE screensize;
		memcpy_s(&screensize.cx, sizeof(LONG), message->data, sizeof(LONG));
		memcpy_s(&screensize.cy, sizeof(LONG), message->data + sizeof(LONG), sizeof(LONG));
		CDC memDC;
		CDC* pDlgDC = clientData->remoteDialog->GetDC();
		memDC.CreateCompatibleDC(pDlgDC);

		DWORD dwWidth = screensize.cx;
		DWORD dwHeight = screensize.cy;

		CBitmap bitmap;
		bitmap.CreateCompatibleBitmap(pDlgDC, dwWidth, dwHeight);
		LONG size = screensize.cx * screensize.cy * 4;
		bitmap.SetBitmapBits(size, message->data + sizeof(LONG) * 2);
		memDC.SelectObject(bitmap);

		pDlgDC->BitBlt(0, 0, dwWidth, dwHeight, &memDC, 0, 0, SRCCOPY);
		EnterCriticalSection(&clientData->remoteDialog->m_cs);
		if (clientData->remoteDialog->m_isSaveBMP)
		{
			clientData->remoteDialog->m_isSaveBMP = false;
			SaveBitmap(L"1.bmp", bitmap);
		}
		LeaveCriticalSection(&clientData->remoteDialog->m_cs);
		clientData->remoteDialog->ReleaseDC(pDlgDC);
		clientData->tcpClient->Send(Socket::MessageType::ScreenCommandRequset, "");
	}
	break;
	case Socket::MessageType::FileQueryReply:
	{
		wchar_t* szbuf = new wchar_t[message->length / 2 + 1];
		memcpy_s(szbuf, message->length, message->data, message->length);
		szbuf[message->length / 2] = L'\0';
		clientData->remoteFileDialog->FillRemoteFileList(szbuf);
	}
	break;
	case Socket::MessageType::FileDownloadReply:
	{
		clientData->remoteFileDialog->SaveRemoteFile(message->data, message->length);
	}
	break;
	case Socket::MessageType::FileDownloadReplyIsDir:
	{
		wchar_t* szbuf = new wchar_t[message->length / 2 + 1];
		memcpy_s(szbuf, message->length, message->data, message->length);
		szbuf[message->length / 2] = L'\0';
		std::vector<CString> vecPath = split(szbuf, L"?");
		assert(vecPath.size() % 2 == 0);
		for (size_t i = 0; i < vecPath.size(); i += 2)
		{
			clientData->remoteFileDialog->SendDownloadRequset(vecPath[i], vecPath[i + 1]);
		}
	}
	break;
	case Socket::MessageType::FileUploadRequset:
	{
		size_t offset = *(size_t*)message->data;
		std::wstring str((wchar_t*)(message->data + sizeof(size_t)), (message->length - sizeof(size_t)) / 2);
		clientData->remoteFileDialog->OnDownloadQuere(str, offset);
	}
	break;
	case Socket::MessageType::ProcessOpenReply:
	{
		std::wstring str((wchar_t*)message->data, message->length / 2);
		clientData->remoteProcessDialog->UpdateListCtrl(str);
	}
	break;
	case Socket::MessageType::HeartBeatRequset:
	{
		clientData->tcpClient->Send(Socket::MessageType::HeartBeatReply, "");
		g_this->KillTimer(clientData->tcpClient->m_socket);
		g_this->SetTimer(clientData->tcpClient->m_socket, 20000, NULL);
	}
	break;
	}

	return 0;
}

void CRemoteToolServerDlg::SaveBitmap(CString strFilePath, CBitmap &Bitmap)
{
	if (Bitmap.m_hObject)
	{
		CImage imgTemp;      // CImage是MFC中的类。
		imgTemp.Attach(Bitmap.operator HBITMAP());
		imgTemp.Save(strFilePath);
	}
}




void CRemoteToolServerDlg::OnTimer(UINT_PTR nIDEvent)
{
	// TODO: 在此添加消息处理程序代码和/或调用默认值

	KillTimer(nIDEvent);
	CDialog::OnTimer(nIDEvent);
}