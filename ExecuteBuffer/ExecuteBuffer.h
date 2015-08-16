#pragma once

class CExecuteBuffer
{
public:
	CExecuteBuffer();

	void Run();

private:
	BOOL CreateBaseProcess(PROCESS_INFORMATION& processInfo);

	BOOL ReadTargetExeData(const CString& rTargetExePath, vector<BYTE>& exeBuffer);
	BOOL GetBaseProcessContext(PROCESS_INFORMATION& processInfo, CONTEXT& threadContext);
	BOOL GetImageSize(PROCESS_INFORMATION& processInfo, CONTEXT& threadContext, DWORD& dwImageSize);

	CString GetProcessPath();
};