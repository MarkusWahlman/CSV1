#include "Process.h"
#include <stdexcept>
#include <memory>

#include "Service.h"
#include "g_Offsets.h"

#include <winternl.h>

#include <iostream>
#include <vector>
#include <fstream>

#include "ntdllUserDef.h"

//#define PROCESS_DEBUG_MODE

#ifdef PROCESS_DEBUG_MODE
#define PRINT_DEBUG_INFO(x) std::cout << x << '\n'
#else 
#define PRINT_DEBUG_INFO(x)
#endif

Process::Process(const std::wstring ProcessName, const std::wstring SecondProcessName, ACCESS_MASK NecessaryAccess, bool AbsoluteAccess)
	: 
	m_hFIRST(INVALID_HANDLE_VALUE),
	m_hSECOND(INVALID_HANDLE_VALUE),
	m_FIRSTPIDS({}),
	m_SECONDPID(0),
	m_STATE(Process::STATE(0)),
	m_MODULES(new MODULEENTRY32[Module::LAST]),
	m_FIRSTPROCNAME(ProcessName),
	m_SECONDPROCNAME(SecondProcessName),

	m_NECESSARYACCESS(NecessaryAccess),
	m_ABSOLUTEACCESS(AbsoluteAccess)
{
	DebugPrivileges(true);

	Setup();
}

Process::~Process()
{
	delete(m_MODULES);
	if (flag_has_value(m_STATE, STATE::HandleFound))
	{
		if(m_hFIRST != INVALID_HANDLE_VALUE)
			CloseHandle(m_hFIRST);
		CloseHandle(m_hSECOND);
	}
		
}

bool Process::Setup()
{
	IsChild();

	if (Find())
	{
		if (FindHandles())
		{
			if (FindProcessModules())
			{
				if (g_Offsets::Intialize(this))
				{
					m_STATE |= STATE::OffsetsFound;

					CloseHandle(m_hFIRST); /*Not needed anymore*/
					DebugPrivileges(false);//
					m_hFIRST = INVALID_HANDLE_VALUE;
				}
			}
		}
	}
	return false;
}

bool Process::Find()
{
	if (flag_has_value(m_STATE, STATE::ProcessFound))
		return true;

	FindProcessIDS();
	if (m_FIRSTPIDS.empty() || m_SECONDPID == 0)
	{
		return false;
	}
	m_STATE |= STATE::ProcessFound;
	return true;
}

void Process::FindProcessIDS()
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap == INVALID_HANDLE_VALUE)
		return;

	PROCESSENTRY32 procEntry{};
	procEntry.dwSize = sizeof(procEntry);

	if(Process32First(hSnap, &procEntry))
	{
		do
		{
			if (!_wcsicmp(procEntry.szExeFile, m_FIRSTPROCNAME.c_str())) {
				m_FIRSTPIDS.push_back(procEntry.th32ProcessID);
			}
			if (!_wcsicmp(procEntry.szExeFile, m_SECONDPROCNAME.c_str())) {
				m_SECONDPID = procEntry.th32ProcessID;
			}
		} while (Process32Next(hSnap, &procEntry));
		CloseHandle(hSnap);
	}
}

bool Process::FindHandles()
{
	if (flag_has_value(m_STATE, STATE::HandleFound))
		return true;

	if (!flag_has_value(m_STATE, STATE::ProcessFound))
		if(!Find())
			return false;

	ULONG systemInformationLength = sizeof(SYSTEM_HANDLE_INFORMATION);
	ULONG returnLength = 0;
	PVOID systemInformationPtr = malloc(systemInformationLength);

	while (NtQuerySystemInformation(SYSTEM_INFORMATION_CLASS(SYSTEM_INFORMATION_CLASS_EX::SystemHandleInformation), systemInformationPtr, systemInformationLength, &returnLength)
		== NtStatus::InfoLengthMismatch)
	{
		// get the return length
		systemInformationLength = returnLength;

		// free the previously allocated memory
		free(systemInformationPtr);

		// allocate a new memory region
		systemInformationPtr = malloc(systemInformationLength);;
	}

	if (systemInformationPtr == nullptr)
	{
		PRINT_DEBUG_INFO("xsystemInformationPtr == nullptr");
		return false;
	}

	/*At the top there should be the total number of Handles*/
	int numberOfHandles = *(int*)systemInformationPtr;
	std::vector <SYSTEM_HANDLE_TABLE_ENTRY_INFO> usableHandlesInfo;

	uintptr_t handleEntryPtr = (uintptr_t)systemInformationPtr + sizeof(long);

	for (int i = 0; i < numberOfHandles; ++i)
	{
		SYSTEM_HANDLE_TABLE_ENTRY_INFO handleTableEntry = *(SYSTEM_HANDLE_TABLE_ENTRY_INFO*)handleEntryPtr;
		handleEntryPtr += sizeof(SYSTEM_HANDLE_TABLE_ENTRY_INFO);

		if (std::count(m_FIRSTPIDS.begin(), m_FIRSTPIDS.end(), handleTableEntry.ProcessId))
		{
			if (m_ABSOLUTEACCESS)
			{
				if (handleTableEntry.GrantedAccess == m_NECESSARYACCESS)
				{
					usableHandlesInfo.push_back(handleTableEntry);
				}
			}
			else
			{
				if (handleTableEntry.GrantedAccess & m_NECESSARYACCESS)
					usableHandlesInfo.push_back(handleTableEntry);
			}
		}
	}
	free(systemInformationPtr);

	DWORD curPID = 0;
	HANDLE firstProcHandle = INVALID_HANDLE_VALUE;

	for (const SYSTEM_HANDLE_TABLE_ENTRY_INFO& curInfo : usableHandlesInfo)
	{
		if (curInfo.ProcessId != curPID)
		{
			curPID = curInfo.ProcessId;
			if (firstProcHandle != INVALID_HANDLE_VALUE)
				CloseHandle(firstProcHandle);
			firstProcHandle = OpenProcess(PROCESS_ALL_ACCESS, FALSE, curPID);
		}

		if (firstProcHandle == INVALID_HANDLE_VALUE)
			continue;	/*Couldn't open handle*/

		if (firstProcHandle == 0)
		{
			PRINT_DEBUG_INFO("Can't open handle to first process, no access");
			continue;
		}

		HANDLE hDuplicate;
		if (!DuplicateHandle(firstProcHandle, (HANDLE)curInfo.Handle, GetCurrentProcess(), &hDuplicate, NULL, FALSE, DUPLICATE_SAME_ACCESS))
		{
			PRINT_DEBUG_INFO("Couldn't duplicate HANDLE");
			continue;
		}

		if (hDuplicate == INVALID_HANDLE_VALUE || hDuplicate == 0)
		{
			PRINT_DEBUG_INFO("hDuplicate is 0 or invalid");
			continue;
		}

		ULONG objectInfoLength = sizeof(OBJECT_TYPE_INFORMATION);
		PVOID objectInfoPtr = malloc(objectInfoLength);
		ULONG returnLength = 0;

		while (NtQueryObject(hDuplicate, OBJECT_INFORMATION_CLASS::ObjectTypeInformation, objectInfoPtr, objectInfoLength, &returnLength) == NtStatus::InfoLengthMismatch)
		{
			objectInfoLength = returnLength;
			free(objectInfoPtr);
			objectInfoPtr = malloc(objectInfoLength);;
		}

		OBJECT_TYPE_INFORMATION objectInfo = *(OBJECT_TYPE_INFORMATION*)objectInfoPtr;
		free(objectInfoPtr);
		if (objectInfo.TypeName.Length == 0)
			continue;

		//@todo check that the objectInfo typeName is "Process"

		WCHAR bufferSecond[MAX_PATH];
		DWORD bufSizeSecond = MAX_PATH;
		if (QueryFullProcessImageName(hDuplicate, 0, bufferSecond, &bufSizeSecond))
		{
			std::filesystem::path secondProcExePath(bufferSecond);
			if (secondProcExePath.wstring().find(m_SECONDPROCNAME) != std::wstring::npos)
			{
				g_SECONDPROCPATH = secondProcExePath.parent_path().string();
				CloseHandle(hDuplicate);

				m_hFIRST = firstProcHandle;
				m_hSECOND = (HANDLE)curInfo.Handle;
				m_STATE |= STATE::HandleFound;

				return true;
			}
		}
		else
		{
			PRINT_DEBUG_INFO("firstProcessHandle doesn't have QueryFullProcessImageName access");
		}
	}

	if (firstProcHandle != INVALID_HANDLE_VALUE)
		CloseHandle(firstProcHandle);

	return false;
}

bool Process::SecondProcStillRunning()
{
	DWORD ExitCode;
	if (GetExitCodeProcess(m_hSECOND, &ExitCode))
	{
		if(ExitCode == STILL_ACTIVE)
			return true;
	}
	return false;
}

bool Process::ChangeFirstProcHandleInherit(BOOL bInherit)
{
	HANDLE hDuplicate;
	if (!DuplicateHandle(m_hFIRST, m_hSECOND, GetCurrentProcess(), &hDuplicate, NULL, bInherit, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
	{
		PRINT_DEBUG_INFO("Couldn't duplicate HANDLE after finding right HANDLE\n");
		return false;
	}

	HANDLE impHandle; /*Won't be needed, and closed by DuplicateHandle.*/
	if (!DuplicateHandle(GetCurrentProcess(), hDuplicate, m_hFIRST, &impHandle, NULL, bInherit, DUPLICATE_SAME_ACCESS | DUPLICATE_CLOSE_SOURCE))
	{
		PRINT_DEBUG_INFO("Couldn't replace HANDLE \n");
		return false;
	}
	return true;
}

bool Process::StartProcessAsFirstProcChild()
{
	char CLine[1024], ModuleName[MAX_PATH];
	ZeroMemory(CLine, _countof(CLine));
	ZeroMemory(ModuleName, MAX_PATH);

	char exeName[MAX_PATH];
	GetModuleFileNameA(nullptr, exeName, MAX_PATH);

	if (!GetFullPathNameA(exeName, MAX_PATH, ModuleName, 0))
	{
		std::cout << "Couldn't GetFullPathNameA of " << exeName << '\n';
		return false;
	}
	sprintf_s(CLine, "%s %d", ModuleName, (int)m_hFIRST);

	PROCESS_INFORMATION pi = { 0,0,0,0 };
	if (!Service::ServiceRunProgram(0, CLine, 0, &pi, TRUE, m_hFIRST))
	{
		std::cout << "Couldn't start new program. CLINE: " << CLine << '\n';
		return false;
	}
	
	return true;
}

static std::wstring ModuleName(Process::Module mod)
{
	switch (mod)
	{
	case Process::Module::NONE:
		return L"NONE";
	case Process::Module::CLIENTDLL:
		return L"client.dll";
	case Process::Module::ENGINEDLL:
		return L"engine.dll";
	case Process::Module::LAST:
		return L"LAST";
	default:
		throw std::runtime_error("Undeclared module given to moduleName in Process.cpp");
	}
}

bool Process::FindProcessModules()
{
	if (flag_has_value(m_STATE, STATE::ModulesFound))
		return true;

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, m_SECONDPID);
	if (hSnap == INVALID_HANDLE_VALUE)
		return false;

	MODULEENTRY32 modEntry{};
	modEntry.dwSize = sizeof(modEntry);

	int totalFound = 0;
	Module32First(hSnap, &modEntry);
	do
	{
		for (int i = (int)Module::NONE + 1; i < (int)Module::LAST; ++i) /*Ignores NONE and LAST*/
		{
			if (!_wcsicmp(modEntry.szModule, ModuleName((Module)i).c_str()))
			{
				m_MODULES[i] = modEntry;
				++totalFound;
			}
		}
	} while (Module32Next(hSnap, &modEntry));

	CloseHandle(hSnap);
	if (totalFound == (int)Module::LAST - 1)
	{
		m_STATE |= STATE::ModulesFound;
		return true;
	}

	return false;
}

//parser made by https://github.com/GH-Rake
static void parsePattern(const char* combo, char* pattern, char* mask)
{
	char lastChar = ' ';
	unsigned int j = 0;

	for (unsigned int i = 0; i < strlen(combo); i++)
	{
		if ((combo[i] == '?' || combo[i] == '*') && (lastChar != '?' && lastChar != '*'))
		{
			pattern[j] = mask[j] = '?';
			j++;
		}

		else if (isspace(lastChar))
		{
			pattern[j] = lastChar = (char)strtol(&combo[i], 0, 16);
			mask[j] = 'x';
			j++;
		}
		lastChar = combo[i];
	}
	pattern[j] = mask[j] = '\0';
}

//
//Pattern scanning
//
static bool scanRegionForPattern(const char* pattern, const char* mask, const std::unique_ptr<char[]>& buffer, intptr_t readSize, uintptr_t* offsetInRegion)
{
	for (int i = 0; i < readSize; ++i) {
		bool found = true;
		for (int j = 0; j < (int)strlen(pattern); ++j)
			if (pattern[j] != buffer[i + j] && mask[j] != '?') {
				found = false;
				break;
			}
		if (found) {
			*offsetInRegion = i;
			return true;
		}
	}
	return false;
}

//
//Returns location of a pattern in a module
//
uintptr_t Process::FindPattern(Module mod, const char* patternCombo, std::vector<int> offsets, int extra)
{
	//Parse input
	char pattern[100];
	char mask[100];
	parsePattern(patternCombo, pattern, mask);

	MODULEENTRY32 modEntry = m_MODULES[(int)mod];
	uintptr_t modSize = modEntry.modBaseSize;
	uintptr_t modBaseAddr = (intptr_t)modEntry.modBaseAddr;

	MEMORY_BASIC_INFORMATION mbi;
	mbi.RegionSize = 0x1000;	//Scan one region at a time

	for (uintptr_t current = modBaseAddr; current < modBaseAddr + modSize; current += mbi.RegionSize)
		/*Loop through every region and scan valid memory for the pattern*/
	{
		VirtualQueryEx(m_hSECOND, (LPCVOID)current, &mbi, sizeof(mbi));
		if (mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS) continue;

		auto buffer = std::unique_ptr<char[]>(new char[mbi.RegionSize]);
		SIZE_T readSize;
		ReadProcessMemory(m_hSECOND, mbi.BaseAddress, buffer.get(), mbi.RegionSize, &readSize);

		uintptr_t offsetInRegion;
		if (scanRegionForPattern(pattern, mask, buffer, readSize, &offsetInRegion)) {
			uintptr_t result = current + offsetInRegion;
			if (!offsets.empty())
				for (int offset : offsets)
					result = ReadMem<int>(result + offset);

			return result + extra;
		}
	}

	throw std::runtime_error("couldn't find pattern " + *pattern);
}


bool Process::DebugPrivileges(bool needDebug)
{
	bool success = true;

	/*Enable windows SeDebugPrivileges*/
	HANDLE Token;
	if (OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES, &Token) == FALSE)
	{
		success = false;
		return false;
	}

	LUID Luid;
	if (LookupPrivilegeValueW(NULL, SE_DEBUG_NAME, &Luid) == FALSE)
	{
		success = false;
		std::cout << "Error LookupPrivilegeValueW when changing DebugPrivileges to " << needDebug << '\n';
	}

	TOKEN_PRIVILEGES NewState;
	NewState.PrivilegeCount = 1;
	NewState.Privileges[0].Luid = Luid;
	NewState.Privileges[0].Attributes = needDebug ? SE_PRIVILEGE_ENABLED : SE_PRIVILEGE_REMOVED;

	if (AdjustTokenPrivileges(Token, FALSE, &NewState, sizeof(NewState), NULL, NULL) == 0
		|| GetLastError() == ERROR_NOT_ALL_ASSIGNED)
	{
		success = false;
		std::cout << "Error AdjustPrivilegeValueW when changing DebugPrivileges to " << needDebug << '\n';
	}
	CloseHandle(Token);

	return success;
}

bool Process::IsChild()
{
	if (flag_has_value(m_STATE, STATE::IsChildProcess))
		return true;

	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (hSnap == INVALID_HANDLE_VALUE)
		return false;

	PROCESSENTRY32 procEntry{};
	procEntry.dwSize = sizeof(procEntry);

	Process32First(hSnap, &procEntry);
	do
	{
		if (procEntry.th32ProcessID == GetCurrentProcessId())
		{
			HANDLE hParent = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, procEntry.th32ParentProcessID);
			if (hParent == INVALID_HANDLE_VALUE)
				return false;

			WCHAR buffer[MAX_PATH];
			DWORD bufferSize = MAX_PATH;
			if (QueryFullProcessImageName(hParent, 0, buffer, &bufferSize))
			{
				std::wstring parentProcPath(buffer);
				if (parentProcPath.find(m_FIRSTPROCNAME) != std::wstring::npos)
				{
					CloseHandle(hParent);
					m_STATE |= STATE::IsChildProcess;
					return true;
				}
			}
			CloseHandle(hParent);
			return false;
		}
	} while (Process32Next(hSnap, &procEntry));

	return false;
	CloseHandle(hSnap);

	/*Second loop check if currentProcess is a child of t*/

	return false;
}