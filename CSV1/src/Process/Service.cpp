#include "Service.h"

#include <iostream>


namespace Service
{
	HANDLE WINAPI ServiceRunProgram(LPCSTR lpFilename, LPCSTR lpArguments, LPCSTR lpDir, LPPROCESS_INFORMATION ProcessInformation, BOOL Inherit, HANDLE hParent)
	{
		HANDLE processToken = NULL, userToken = NULL;
		LPVOID pEnvironment = NULL;
		STARTUPINFOEXA  si = { 0 };
		SIZE_T cbAttributeListSize = 0;
		PPROC_THREAD_ATTRIBUTE_LIST pAttributeList = NULL;
		BOOL Status = TRUE;
		ZeroMemory(&si, sizeof(si));
		si.StartupInfo.cb = sizeof(STARTUPINFOEXA);

		if (!ProcessInformation)
		{
			Status = false;
			goto EXIT;
		}

		if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ALL_ACCESS, &processToken))
		{
			Status = false;
			goto EXIT;
		}
		if (!DuplicateTokenEx(processToken, TOKEN_ALL_ACCESS, NULL, SecurityImpersonation, TokenPrimary, &userToken) ||
			!CreateEnvironmentBlock(&pEnvironment, userToken, TRUE))
		{
			Status = false;
			goto EXIT;
		}
		InitializeProcThreadAttributeList(NULL, 1, 0, &cbAttributeListSize);
		pAttributeList = reinterpret_cast<PPROC_THREAD_ATTRIBUTE_LIST>(HeapAlloc(GetProcessHeap(), 0, cbAttributeListSize));
		if (!pAttributeList)
		{
			Status = false;
			goto EXIT;
		}
		if (!InitializeProcThreadAttributeList(pAttributeList, 1, 0, &cbAttributeListSize))
		{
			Status = false;
			goto EXIT;
		}
		if (!UpdateProcThreadAttribute(pAttributeList, 0, PROC_THREAD_ATTRIBUTE_PARENT_PROCESS, &hParent, sizeof(HANDLE), NULL, NULL))
		{
			Status = false;
			goto EXIT;
		}
		si.lpAttributeList = pAttributeList;
		if (!CreateProcessAsUserA(userToken, lpFilename, const_cast<LPSTR>(lpArguments), NULL, NULL, TRUE, CREATE_UNICODE_ENVIRONMENT | CREATE_NEW_CONSOLE | EXTENDED_STARTUPINFO_PRESENT, pEnvironment, lpDir, reinterpret_cast<LPSTARTUPINFOA>(&si), ProcessInformation))
		{
			std::cout << "Failed at CreateProcessAsuserA " << GetLastError() << " <- Last Error\n";
			Status = false;
			goto EXIT;
		}
	EXIT:
		if (pEnvironment)
			DestroyEnvironmentBlock(pEnvironment);
		CloseHandle(userToken);
		if (ProcessInformation->hThread)
			CloseHandle(ProcessInformation->hThread);
		if (processToken)
			CloseHandle(processToken);
		if (pAttributeList)
		{
			DeleteProcThreadAttributeList(pAttributeList);
			HeapFree(GetProcessHeap(), 0, pAttributeList);
		}
		return Status ? ProcessInformation->hProcess : 0;
	}
}

