// RemoteFileDialog.cpp : 实现文件
//

#include "stdafx.h"
#include "RemoteToolServer.h"
#include "RemoteFileDialog.h"
#include "afxdialogex.h"
#include <stack>
void FindAllFile(CString pstr, std::vector<CString>& allFilePath)
{
	CFileFind finder;

	// build a string with wildcards
	CString strWildcard(pstr);
	strWildcard += _T("\\*.*");

	// start working for files
	BOOL bWorking = finder.FindFile(strWildcard);

	while (bWorking)
	{
		bWorking = finder.FindNextFile();

		// skip . and .. files; otherwise, we'd
		// recur infinitely!

		if (finder.IsDots())
			continue;

		// if it's a directory, recursively search it

		if (finder.IsDirectory())
		{
			CString str = finder.GetFilePath();
			FindAllFile(str, allFilePath);
		}
		else
		{
			allFilePath.push_back(finder.GetFilePath());
		}
	}

	finder.Close();
}

bool MyCreateDirectory(CString dirPath)
{
	std::stack<CString> vecDir;
	CString dir = dirPath;
	while (!CreateDirectory(dir, NULL) && GetLastError() == ERROR_PATH_NOT_FOUND)
	{
		vecDir.push(dir);
		dir = dir.Left(dir.ReverseFind('\\'));
	}
	while (!vecDir.empty())
	{
		if (!CreateDirectory(vecDir.top(), NULL))
			return false;
		vecDir.pop();
	}
	return true;
}
int GetIconIndex(CString strFilePath)
{
	//获取文件属性根据路径
	DWORD dwAttribute = GetFileAttributes(strFilePath);

	SHFILEINFO sfi = { 0 };
	SHGetFileInfo(strFilePath,//文件路径
		dwAttribute,//文件属性
		&sfi,//输出参数结构体
		sizeof(SHFILEINFO),//结构体Size
						   //获取文件的图标和属性
		SHGFI_SYSICONINDEX | SHGFI_USEFILEATTRIBUTES | SHGFI_TYPENAME);
	return sfi.iIcon;

}
CString GetFileTypeName(CString strFilePath)
{
	//获取文件属性根据路径
	DWORD dwAttribute = GetFileAttributes(strFilePath);

	SHFILEINFO sfi = { 0 };
	SHGetFileInfo(strFilePath,//文件路径
		dwAttribute,//文件属性
		&sfi,//输出参数结构体
		sizeof(SHFILEINFO),//结构体Size
						   //获取文件的图标和属性
		SHGFI_TYPENAME);
	return sfi.szTypeName;

}

// RemoteFileDialog 对话框
//字符串分割函数
static std::vector<CString> split(std::string str, std::string pattern)
{
	std::string::size_type pos;
	std::vector<CString> result;
	str += pattern;//扩展字符串以方便操作
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

std::vector<CString> split(std::wstring str, std::wstring pattern)
{
	std::wstring::size_type pos;
	std::vector<CString> result;
	str += pattern;//扩展字符串以方便操作
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

IMPLEMENT_DYNAMIC(RemoteFileDialog, CDialogEx)

RemoteFileDialog::RemoteFileDialog(CWnd* pParent /*=NULL*/)
	: CDialogEx(IDD_DIALOG_FILE, pParent)
	, m_strDisplayTypeRemote(_T(""))
	, m_strDirPathRemote(_T(""))
	, m_strDisplayTypeLocal(_T(""))
	, m_strDirPathLocal(_T(""))
{

}

RemoteFileDialog::~RemoteFileDialog()
{
}

void RemoteFileDialog::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);
	DDX_Control(pDX, IDC_COMBO_DISPLAYTYPE, m_comboDisplayTypeRemote);
	DDX_CBString(pDX, IDC_COMBO_DISPLAYTYPE, m_strDisplayTypeRemote);
	DDX_Control(pDX, IDC_EDIT_DIRPATH, m_editDirPathRemote);
	DDX_Text(pDX, IDC_EDIT_DIRPATH, m_strDirPathRemote);
	DDX_Control(pDX, IDC_LIST_FILELIST, m_listFileRemote);
	DDX_Control(pDX, IDC_COMBO_DISPLAYTYPE2, m_comboDisplayTypeLocal);
	DDX_CBString(pDX, IDC_COMBO_DISPLAYTYPE2, m_strDisplayTypeLocal);
	DDX_Control(pDX, IDC_EDIT_DIRPATH2, m_editDirPathLocal);
	DDX_Text(pDX, IDC_EDIT_DIRPATH2, m_strDirPathLocal);
	DDX_Control(pDX, IDC_LIST_FILELIST2, m_listFileLocal);
}


BEGIN_MESSAGE_MAP(RemoteFileDialog, CDialogEx)
	ON_CBN_SELCHANGE(IDC_COMBO_DISPLAYTYPE, &RemoteFileDialog::OnSelchangeRemoteComboDisplaytype)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILELIST, &RemoteFileDialog::OnDblclkRemoteListFilelist)
	ON_CBN_SELCHANGE(IDC_COMBO_DISPLAYTYPE2, &RemoteFileDialog::OnSelchangeLocalComboDisplaytype)
	ON_NOTIFY(NM_DBLCLK, IDC_LIST_FILELIST2, &RemoteFileDialog::OnDblclkLocalListFilelist)
	ON_BN_CLICKED(IDC_BUTTON_DOWNLOAD, &RemoteFileDialog::OnBnClickedButtonDownload)
	ON_BN_CLICKED(IDC_BUTTON_UPLOAD, &RemoteFileDialog::OnBnClickedButtonUpload)
END_MESSAGE_MAP()


// RemoteFileDialog 消息处理程序


void RemoteFileDialog::OnSelchangeRemoteComboDisplaytype()
{
	SHFILEINFO sfi;
	HIMAGELIST hIconLst;

	switch (m_comboDisplayTypeRemote.GetCurSel())
	{
	case 0:
		hIconLst = (HIMAGELIST)::SHGetFileInfo(
			_T("C:\\"), //系统盘符
			FILE_ATTRIBUTE_NORMAL,
			&sfi,
			sizeof(SHFILEINFO),
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON);//获取小图标与图标索引
		m_pImageList = CImageList::FromHandle(hIconLst);
		m_listFileRemote.ModifyStyle(LVS_REPORT | LVS_ICON, LVS_SMALLICON, TRUE);
		m_listFileRemote.SetImageList(m_pImageList, LVSIL_SMALL);
		//FileFindInit();
		break;
	case 1:
		hIconLst = (HIMAGELIST)::SHGetFileInfo(
			_T("C:\\"), //系统盘符
			FILE_ATTRIBUTE_NORMAL,
			&sfi,
			sizeof(SHFILEINFO),
			SHGFI_SYSICONINDEX | SHGFI_ICON);//获取小图标与图标索引
		m_pImageList = CImageList::FromHandle(hIconLst);
		m_listFileRemote.ModifyStyle(LVS_REPORT | LVS_SMALLICON, LVS_ICON, TRUE);
		m_listFileRemote.SetImageList(m_pImageList, LVSIL_NORMAL);
		//FileFindInit();
		break;
	case 2:
		hIconLst = (HIMAGELIST)::SHGetFileInfo(
			_T("C:\\"), //系统盘符
			FILE_ATTRIBUTE_NORMAL,
			&sfi,
			sizeof(SHFILEINFO),
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON);//获取小图标与图标索引
		m_pImageList = CImageList::FromHandle(hIconLst);
		m_listFileRemote.ModifyStyle(LVS_ICON | LVS_SMALLICON, LVS_REPORT, TRUE);
		m_listFileRemote.SetImageList(m_pImageList, LVSIL_SMALL);
		m_listFileRemote.SetColumnWidth(0, 300);
		//FileFindInit();
		break;
	}
}


void RemoteFileDialog::OnDblclkRemoteListFilelist(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	if (pNMItemActivate->iItem >= 0)
	{
		if (m_vecFilenameRemote[pNMItemActivate->iItem] == _T(".."))
			m_vecPathRemote.pop_back();
		else
			m_vecPathRemote.push_back(m_vecFilenameRemote[pNMItemActivate->iItem]);
		// TODO 发送 m_vecPath 请求数据
		CString str;
		for (size_t i = 0; i < m_vecPathRemote.size(); i++)
		{
			str += m_vecPathRemote[i] + '?';
		}
		m_socketClient->Send(Socket::MessageType::FileQueryRequset, str.GetBuffer());
	}
	*pResult = 0;
}

void RemoteFileDialog::OnSelchangeLocalComboDisplaytype()
{
	SHFILEINFO sfi;
	HIMAGELIST hIconLst;

	switch (m_comboDisplayTypeLocal.GetCurSel())
	{
	case 0:
		hIconLst = (HIMAGELIST)::SHGetFileInfo(
			_T("C:\\"), //系统盘符
			FILE_ATTRIBUTE_NORMAL,
			&sfi,
			sizeof(SHFILEINFO),
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON);//获取小图标与图标索引
		m_pImageList = CImageList::FromHandle(hIconLst);
		m_listFileLocal.ModifyStyle(LVS_REPORT | LVS_ICON, LVS_SMALLICON, TRUE);
		m_listFileLocal.SetImageList(m_pImageList, LVSIL_SMALL);
		//FileFindInit();
		break;
	case 1:
		hIconLst = (HIMAGELIST)::SHGetFileInfo(
			_T("C:\\"), //系统盘符
			FILE_ATTRIBUTE_NORMAL,
			&sfi,
			sizeof(SHFILEINFO),
			SHGFI_SYSICONINDEX | SHGFI_ICON);//获取小图标与图标索引
		m_pImageList = CImageList::FromHandle(hIconLst);
		m_listFileLocal.ModifyStyle(LVS_REPORT | LVS_SMALLICON, LVS_ICON, TRUE);
		m_listFileLocal.SetImageList(m_pImageList, LVSIL_NORMAL);
		//FileFindInit();
		break;
	case 2:
		hIconLst = (HIMAGELIST)::SHGetFileInfo(
			_T("C:\\"), //系统盘符
			FILE_ATTRIBUTE_NORMAL,
			&sfi,
			sizeof(SHFILEINFO),
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON);//获取小图标与图标索引
		m_pImageList = CImageList::FromHandle(hIconLst);
		m_listFileLocal.ModifyStyle(LVS_ICON | LVS_SMALLICON, LVS_REPORT, TRUE);
		m_listFileLocal.SetImageList(m_pImageList, LVSIL_SMALL);
		m_listFileLocal.SetColumnWidth(0, 300);
		//FileFindInit();
		break;
	}
}

void RemoteFileDialog::OnDblclkLocalListFilelist(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMITEMACTIVATE pNMItemActivate = reinterpret_cast<LPNMITEMACTIVATE>(pNMHDR);
	// TODO: 在此添加控件通知处理程序代码
	if (pNMItemActivate->iItem >= 0)
	{
		if (m_vecFilenameLocal[pNMItemActivate->iItem] == _T(".."))
			m_vecPathLocal.pop_back();
		else
			m_vecPathLocal.push_back(m_vecFilenameLocal[pNMItemActivate->iItem]);
		FindLocalFile();
		FillLocalFileList();


	}
	*pResult = 0;
}


BOOL RemoteFileDialog::OnInitDialog()
{
	CDialogEx::OnInitDialog();
	//remote数据
	{
		m_comboDisplayTypeRemote.AddString(_T("小图标"));
		m_comboDisplayTypeRemote.AddString(_T("大图标"));
		m_comboDisplayTypeRemote.AddString(_T("详细信息"));
		m_comboDisplayTypeRemote.SetCurSel(0);

		//获取当前List的风格状态
		DWORD dwExtStyle = m_listFileRemote.GetExtendedStyle();
		//设置选中时为整行选中
		dwExtStyle |= LVS_EX_FULLROWSELECT;
		m_listFileRemote.SetExtendedStyle(dwExtStyle);


		int nCol = 0;
		m_listFileRemote.InsertColumn(nCol++, _T("名称"), LVCFMT_LEFT, 300);
		m_listFileRemote.InsertColumn(nCol++, _T("修改日期"), LVCFMT_LEFT, 100);
		m_listFileRemote.InsertColumn(nCol++, _T("类型"), LVCFMT_LEFT, 100);
		m_listFileRemote.InsertColumn(nCol++, _T("大小"), LVCFMT_LEFT, 100);
		SHFILEINFO sfi;
		HIMAGELIST hIconLst = (HIMAGELIST)::SHGetFileInfo(
			_T("C:\\"), //系统盘符
			FILE_ATTRIBUTE_NORMAL,
			&sfi,
			sizeof(SHFILEINFO),
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON);//获取小图标与图标索引
		m_pImageList = CImageList::FromHandle(hIconLst);
		m_listFileRemote.ModifyStyle(0, LVS_SMALLICON);
		m_listFileRemote.SetImageList(m_pImageList, LVSIL_SMALL);
	}


	//本地数据
	{
		m_comboDisplayTypeLocal.AddString(_T("小图标"));
		m_comboDisplayTypeLocal.AddString(_T("大图标"));
		m_comboDisplayTypeLocal.AddString(_T("详细信息"));
		m_comboDisplayTypeLocal.SetCurSel(0);

		//获取当前List的风格状态
		DWORD dwExtStyle = m_listFileLocal.GetExtendedStyle();
		//设置选中时为整行选中
		dwExtStyle |= LVS_EX_FULLROWSELECT;
		m_listFileLocal.SetExtendedStyle(dwExtStyle);


		int nCol = 0;
		m_listFileLocal.InsertColumn(nCol++, _T("名称"), LVCFMT_LEFT, 300);
		m_listFileLocal.InsertColumn(nCol++, _T("修改日期"), LVCFMT_LEFT, 100);
		m_listFileLocal.InsertColumn(nCol++, _T("类型"), LVCFMT_LEFT, 100);
		m_listFileLocal.InsertColumn(nCol++, _T("大小"), LVCFMT_LEFT, 100);
		SHFILEINFO sfi;
		HIMAGELIST hIconLst = (HIMAGELIST)::SHGetFileInfo(
			_T("C:\\"), //系统盘符
			FILE_ATTRIBUTE_NORMAL,
			&sfi,
			sizeof(SHFILEINFO),
			SHGFI_SYSICONINDEX | SHGFI_SMALLICON);//获取小图标与图标索引
		m_pImageList = CImageList::FromHandle(hIconLst);
		m_listFileLocal.ModifyStyle(0, LVS_SMALLICON);
		m_listFileLocal.SetImageList(m_pImageList, LVSIL_SMALL);

		FindLocalFile();
		FillLocalFileList();
	}
	//TODO 发消息请求数据
	return TRUE;  // return TRUE unless you set the focus to a control
				  // 异常: OCX 属性页应返回 FALSE
}

void RemoteFileDialog::FillRemoteFileList(wchar_t* data)
{
	//struct MessageInfo
	//{
	//	CString filename;
	//	CString iconIndex;
	//	CString fileTypeName;
	//};
	auto vecData = split(data, L"?");
	m_vecFilenameRemote.resize(vecData.size() / 3);
	for (size_t i = 0; i < vecData.size() - 1; i += 3)
	{
		m_vecFilenameRemote[i / 3] = vecData[i];
	}
	m_listFileRemote.DeleteAllItems();
	int nCount = m_vecFilenameRemote.size();
	for (size_t i = 0; i < vecData.size() - 1; i += 3)
	{
		m_listFileRemote.InsertItem(i / 3, m_vecFilenameRemote[i / 3], _wtoi(vecData[i + 1]));
		m_listFileRemote.SetItemText(i / 3, 2, vecData[i + 2]);
	}
	m_editDirPathRemote.SetWindowText(vecData.back());
	m_strDirPathRemote = vecData.back();
}


void RemoteFileDialog::OnBnClickedButtonDownload()
{
	// TODO: 在此添加控件通知处理程序代码
	UINT uSelectedCount = m_listFileRemote.GetSelectedCount();
	if (uSelectedCount == 0)
	{
		return;
	}
	int  nItem = -1;
	nItem = m_listFileRemote.GetNextItem(nItem, LVNI_SELECTED);


	CString remoteFileName = m_strDirPathRemote + m_vecFilenameRemote[nItem];
	CString LocalPathFileName = m_strDirPathLocal + m_vecFilenameRemote[nItem];
	SendDownloadRequset(remoteFileName, LocalPathFileName);
}


void RemoteFileDialog::OnBnClickedButtonUpload()
{
	// TODO: 在此添加控件通知处理程序代码
	UINT uSelectedCount = m_listFileLocal.GetSelectedCount();
	if (uSelectedCount == 0)
	{
		return;
	}
	int  nItem = -1;
	nItem = m_listFileLocal.GetNextItem(nItem, LVNI_SELECTED);
	CString remoteFileName = m_strDirPathRemote + m_vecFilenameLocal[nItem];
	CString LocalPathFileName = m_strDirPathLocal + m_vecFilenameLocal[nItem];
	OnDownloadQuere((LocalPathFileName + "?" + remoteFileName).GetBuffer(), 0);
}

void RemoteFileDialog::FindLocalFile()
{
	if (m_vecPathLocal.size() > 0)
	{
		CFileFind finder;
		m_vecFilenameLocal.clear();
		m_strDirPathLocal = _T("");
		// build a string with wildcards
		for (size_t i = 0; i < m_vecPathLocal.size(); i++)
			m_strDirPathLocal += m_vecPathLocal[i] + _T("\\");
		m_strDirPathLocal += _T("*.*");
		m_vecFilenameLocal.push_back(_T(".."));
		// start working for files
		BOOL bWorking = finder.FindFile(m_strDirPathLocal);

		while (bWorking)
		{
			bWorking = finder.FindNextFile();

			// skip . and .. files; otherwise, we'd
			// recur infinitely!

			if (finder.IsDots())
				continue;

			// if it's a directory, recursively search it

			else if (finder.IsDirectory())
			{
				CString str = finder.GetFileName();
				m_vecFilenameLocal.push_back(str);
			}
			else
			{
				CString str = finder.GetFileName();
				m_vecFilenameLocal.push_back(str);
			}
		}
		m_strDirPathLocal = finder.GetRoot();
		finder.Close();
		//m_strDirPath.Delete(m_strDirPath.GetLength() - 4, 4);
	}
	else
	{
		DWORD dwBuffer = 512;
		TCHAR szBuffer[512];
		memset(szBuffer, 0, dwBuffer);
		int dSize = GetLogicalDriveStrings(dwBuffer, szBuffer); //print szBuffer for detail
		if (dSize == 0)
		{
			//no drive found
			return;
		}
		m_vecFilenameLocal.clear();
		TCHAR szTmp[4];
		memset(szTmp, 0, 4);

		for (int i = 0; i < dSize; i += 4)
		{
			if (szBuffer[i] != '\0')
			{
				wmemcpy(szTmp, &szBuffer[i], 4);

				CString strTmp(szTmp, 2);
				m_vecFilenameLocal.push_back(strTmp);
			}
		}
		m_strDirPathLocal = _T("此电脑");
	}
	UpdateData(FALSE);
}

void RemoteFileDialog::FillLocalFileList()
{
	m_listFileLocal.DeleteAllItems();
	CString PathName = _T("");
	for (size_t i = 0; i < m_vecPathLocal.size(); i++)
		PathName += m_vecPathLocal[i] + _T("\\");
	int nCount = m_vecFilenameLocal.size();
	for (int i = 0; i < nCount; i++)
	{
		m_listFileLocal.InsertItem(i, m_vecFilenameLocal[i], GetIconIndex(PathName + m_vecFilenameLocal[i]));
		m_listFileLocal.SetItemText(i, 2, GetFileTypeName(PathName + m_vecFilenameLocal[i]));
	}
}

void RemoteFileDialog::SaveRemoteFile(char* data, size_t length)
{
	std::wstring str = (wchar_t*)data;
	auto vecData = split(str, L"?");
	CString filePathName = vecData[1];
	CString remotePathName = vecData[0];
	DWORD fileLength = *(int*)(data + (str.length() + 1) * sizeof TCHAR);
	DWORD offset = *(int*)(data + sizeof(DWORD) + (str.length() + 1) * sizeof TCHAR);
	DWORD bufferSize = length - (2* sizeof(DWORD) + (str.length() + 1) * sizeof TCHAR);
	char* buffer = data + (2 * sizeof(DWORD) + (str.length() + 1) * sizeof TCHAR);

	CString dir = filePathName.Left(filePathName.ReverseFind('\\'));

	MyCreateDirectory(dir);
	HANDLE hFile = CreateFile(filePathName, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
	
	if (hFile == INVALID_HANDLE_VALUE)
	{
		MessageBox(L"文件无法访问");
		return;
	}
	DWORD hasWrite;
	long hignOffset = 0;
	DWORD dwPtr = SetFilePointer(hFile,
		offset,
		&hignOffset,
		FILE_BEGIN);

	WriteFile(hFile, buffer, bufferSize, &hasWrite, NULL);
	DWORD endOffset = offset + hasWrite;
	if(endOffset == fileLength)
	{
		DeleteFile(filePathName + ".infoxxx");
		CloseHandle(hFile);
	}
	else
	{
		HANDLE hFileInfo = CreateFile(filePathName + ".infoxxx", GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ | FILE_SHARE_DELETE, NULL, OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
		WriteFile(hFileInfo, &endOffset, sizeof(DWORD), &hasWrite, NULL);
		CloseHandle(hFileInfo);
		CloseHandle(hFile);
		m_socketClient->Send(Socket::MessageType::FileDownloadRequset, endOffset, (remotePathName + "?" + filePathName).GetBuffer());
	}
	
	
	//MessageBox(L"文件传输完毕", 0, MB_OK);
}

void RemoteFileDialog::SendDownloadRequset(CString remoteFileName, CString LocalPathFileName)
{
	HANDLE hFileInfo = CreateFile(LocalPathFileName + ".infoxxx", GENERIC_READ, 0, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);

	if(hFileInfo == INVALID_HANDLE_VALUE && PathFileExists(LocalPathFileName))
	{

		m_socketClient->Send(Socket::MessageType::FileDownloadRequset, 0,(remoteFileName + "?" + LocalPathFileName).GetBuffer());
	}
	else
	{
		DWORD offset = 0;
		DWORD readNums = 0;
		ReadFile(hFileInfo, &offset, sizeof(DWORD), &readNums, NULL);
		m_socketClient->Send(Socket::MessageType::FileDownloadRequset, offset, (remoteFileName + "?" + LocalPathFileName).GetBuffer());
	}
	CloseHandle(hFileInfo);
}


void RemoteFileDialog::OnDownloadQuere(std::wstring path, size_t offsetRemote)
{
	auto vecPath = split(path, L"?");
	CString LocalFilePathName = vecPath[0];
	CString RemoteFilePathName = vecPath[1];
	if (PathIsDirectory(LocalFilePathName))
	{
		std::vector<CString> allLocalPathName;
		std::vector<CString> allRemotePathName;
		FindAllFile(LocalFilePathName, allLocalPathName);
		allRemotePathName.resize(allLocalPathName.size());
		for (size_t i = 0; i < allLocalPathName.size(); i++)
		{
			allRemotePathName[i] = RemoteFilePathName;
			allRemotePathName[i].Append(allLocalPathName[i].Right(allLocalPathName[i].GetLength() - LocalFilePathName.GetLength()));
		}
		std::wstring param;
		for (size_t i = 0; i < allLocalPathName.size(); i++)
		{
			param += allLocalPathName[i] + '?' + allRemotePathName[i] + '?';
		}
		m_socketClient->Send(Socket::MessageType::FileUploadReplyIsDir, param);
	}
	else
	{
		HANDLE hFile = CreateFile(LocalFilePathName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
		DWORD fileSize = GetFileSize(hFile, NULL);
		OVERLAPPED ol = { 0 };
		size_t blockSize = 1024 * 1024;
		size_t offset = offsetRemote;
		if (offset < fileSize)
		{
			DWORD readSize = (blockSize < fileSize - offset) ? blockSize : (fileSize - offset);
			Socket::MessageData message;
			message.type = Socket::MessageType::FileUploadReply;
			message.length = (path.length() + 1) * sizeof(wchar_t) + readSize + 2 * sizeof(DWORD);
			message.data = new char[message.length];
			//远程文件名
			memcpy_s(message.data,
				(path.length() + 1) * sizeof(wchar_t),
				path.data(),
				(path.length() + 1) * sizeof(wchar_t));
			//文件大小
			memcpy_s(message.data + (path.length() + 1) * sizeof(wchar_t),
				sizeof(DWORD),
				&fileSize,
				sizeof(DWORD));
			//文件偏移
			memcpy_s(message.data + (path.length() + 1) * sizeof(wchar_t) + sizeof(DWORD),
				sizeof(DWORD),
				&offset,
				sizeof(DWORD));

			char* fileBuffer = message.data + (path.length() + 1) * sizeof(wchar_t) + 2 * sizeof(DWORD);
			long hignOffset = 0;
			DWORD dwPtr = SetFilePointer(hFile,
				offset,
				&hignOffset,
				FILE_BEGIN);
			DWORD szRead = 0;
			ReadFile(hFile, fileBuffer, readSize, &szRead, NULL);

			m_socketClient->Send(message);
		}

		CloseHandle(hFile);
	}

}