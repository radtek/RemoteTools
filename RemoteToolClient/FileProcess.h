#pragma once

// FileProcess 命令目标

class FileProcess : public CObject
{
public:
	FileProcess();
	virtual ~FileProcess();

	void FindFile();
	CEdit m_editDirPath;
	CString m_strDirPath;
	std::vector<CString> m_vecPath;
	std::vector<CString> m_vecFilename;
	CString m_strDisplayType;
	CImageList* m_pImageList;
	void FillFileList();
	void OnUploadQuere(std::wstring path);
	TcpClient* m_socketClient;
	void OnDownloadQuere(std::wstring path, size_t offsetRemote);
	void SaveRemoteFile(char* data, size_t length);
	void SendDownloadRequset(CString remoteFileName, CString LocalPathFileName);
};


