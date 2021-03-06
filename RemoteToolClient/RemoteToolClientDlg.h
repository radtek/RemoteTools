
// RemoteToolClientDlg.h: 头文件
//

#pragma once
#include "ThreadPool.h"


// CRemoteToolClientDlg 对话框
class CRemoteToolClientDlg : public CDialog
{
// 构造
public:
	CRemoteToolClientDlg(CWnd* pParent = NULL);	// 标准构造函数
	~CRemoteToolClientDlg();
// 对话框数据
#ifdef AFX_DESIGN_TIME
	enum { IDD = IDD_REMOTETOOLCLIENT_DIALOG };
#endif

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV 支持


// 实现
protected:
	HICON m_hIcon;

	// 生成的消息映射函数
	virtual BOOL OnInitDialog();
	afx_msg void OnSysCommand(UINT nID, LPARAM lParam);
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
	public:
	CString m_strIP;
	CString m_strPort;
	afx_msg void OnBnClickedButtonConnect();
	afx_msg void OnBnClickedButtonDisconnect();

	TcpClient* m_client;
	virtual BOOL DestroyWindow();
	static int ReceiveCallback(TcpClient::MessageData* message, void* uesrParam);

	static DWORD __stdcall PrintScreen(LPVOID lpParameter);
	static DWORD __stdcall CmdThreadProc(LPVOID lpParameter);
	HANDLE m_CmdThreadHandle = INVALID_HANDLE_VALUE;
	HANDLE m_PrintScreenThreadHandle = INVALID_HANDLE_VALUE; 
	static DWORD WINAPI CMDThreadProc(LPVOID lpParameter);
	HANDLE m_hCmdWritePipe;
	PROCESS_INFORMATION m_pi;

	afx_msg LRESULT OnUpdateOutput(WPARAM wParam, LPARAM lParam);
	BOOL OnProcessOpenRequset();

	public:
	afx_msg void OnTimer(UINT_PTR nIDEvent);
	ThreadPool m_threadPool;
};
