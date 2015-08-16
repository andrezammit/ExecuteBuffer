// ExecuteBuffer.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include "ExecuteBuffer.h"

int main()
{
	CExecuteBuffer executeBuffer;
	executeBuffer.Run();

    return 0;
}

CExecuteBuffer::CExecuteBuffer()
{
}

CString CExecuteBuffer::GetProcessPath()
{
	CString rProcessPath;

	GetModuleFileName(NULL, rProcessPath.GetBuffer(MAX_PATH), MAX_PATH);
	rProcessPath.ReleaseBuffer();

	return rProcessPath;
}

BOOL CExecuteBuffer::CreateBaseProcess(PROCESS_INFORMATION& processInfo)
{
	STARTUPINFO startupInfo;
	ZeroMemory(&startupInfo, sizeof(startupInfo));

	CString rBaseExePath = GetProcessPath();

	BOOL bRet = CreateProcess(NULL, rBaseExePath.GetBuffer(), NULL, NULL, FALSE, CREATE_SUSPENDED, NULL, NULL, &startupInfo, &processInfo);
	rBaseExePath.ReleaseBuffer();
	
	return bRet;
}

BOOL CExecuteBuffer::GetBaseProcessContext(PROCESS_INFORMATION& processInfo, CONTEXT& threadContext)
{
	threadContext.ContextFlags = CONTEXT_FULL;
	
	if (!GetThreadContext(processInfo.hThread, &threadContext))
		return FALSE;

	return TRUE;
}

BOOL CExecuteBuffer::GetImageSize(PROCESS_INFORMATION& processInfo, CONTEXT& threadContext, DWORD& dwImageSize)
{
	DWORD* pPEBInfo = (DWORD*) threadContext.Ebx;

	DWORD dwBytesRead = 0;
	DWORD* pBaseAddr = NULL;

	if (!ReadProcessMemory(processInfo.hProcess, &pPEBInfo[2], (void*) &pBaseAddr, sizeof(DWORD), &dwBytesRead))
		return FALSE;

	DWORD* pCurrentAddr = pBaseAddr;

	MEMORY_BASIC_INFORMATION memInfo;
	ZeroMemory(&memInfo, sizeof(memInfo));

	while (VirtualQueryEx(processInfo.hProcess, (void*) pCurrentAddr, &memInfo, sizeof(memInfo)))
	{
		if (memInfo.State == MEM_FREE)
			break;

		pCurrentAddr += memInfo.RegionSize;
	}

	dwImageSize = pCurrentAddr - pBaseAddr;
	return TRUE;
}

BOOL CExecuteBuffer::ReadTargetExeData(const CString& rTargetExePath, vector<BYTE>& exeBuffer)
{
	CFile targetExe;

	if (!targetExe.Open(rTargetExePath, CFile::modeRead))
		return FALSE;

	ULONGLONG lExeSize = targetExe.GetLength();
	exeBuffer.resize(lExeSize);

	ULONGLONG lBytesRead = targetExe.Read(&exeBuffer[0], lExeSize);

	if (lBytesRead != lExeSize)
		return FALSE;

	return TRUE;
}

void CExecuteBuffer::Run()
{
	PROCESS_INFORMATION processInfo;
	ZeroMemory(&processInfo, sizeof(processInfo));

	if (!CreateBaseProcess(processInfo))
		return;

	CONTEXT threadContext;
	ZeroMemory(&threadContext, sizeof(threadContext));

	if (!GetBaseProcessContext(processInfo, threadContext))
		return;

	DWORD dwImageSize = 0;
	
	if (!GetImageSize(processInfo, threadContext, dwImageSize))
		return;

	TerminateProcess(processInfo.hProcess, 0);
}
