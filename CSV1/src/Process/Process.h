#pragma once

#include <Windows.h>
#include <TlHelp32.h>
#include <string>
#include <vector>

#include <filesystem>

/*IMPORTANT: This program must be a child of FirstProcess in order to gain access to the HANDLE*/

inline std::string g_SECONDPROCPATH{"C:\\Program Files (x86)\\Steam\\steamapps\\common\\Counter-Strike Global Offensive"};

class Process
{
//
//Modules and offset patterns depend on the process (Currently setup for CSGO)
//
public:
	enum class STATE
	{
		IsChildProcess	= 1 << 0,
		ProcessFound	= 1 << 1,
		HandleFound		= 1 << 2,
		ModulesFound	= 1 << 3,
		OffsetsFound	= 1 << 4
	};

	enum class Module
	/*Process.cpp [static std::wstring ModuleName(Process::Module)] deals with module names*/
	{
		NONE,
		CLIENTDLL,
		ENGINEDLL,
		LAST
	};

	Process(const std::wstring ProcessName, const std::wstring SecondProcessName, ACCESS_MASK NecessaryAccess, bool absolute = false);
	~Process();

	bool Setup();		/*Setup Process class for use, sets m_STATE and m_STATUS*/
	bool Find();		/*Only setups process m_STATUS not whole state*/
	bool FindHandles();	/*Only works if process has been found*/
	bool SecondProcStillRunning();/*Determines if SECONDPROC is running*/

	bool ChangeFirstProcHandleInherit(BOOL bInherit);
	bool StartProcessAsFirstProcChild();

	//HANDLE GetFirstProcHandle() const { return m_hFIRST; }	/*Will be automatically closed when needed*/
	HANDLE GetSecondProcHandle() const { return m_hSECOND; }
	std::vector<DWORD> GetPIDS() const { return m_FIRSTPIDS; }
	STATE State() const { return m_STATE; }

	uintptr_t GetModuleAddr(Module mod) const { return (uintptr_t)m_MODULES[(int)mod].modBaseAddr; }
	uintptr_t FindPattern(Module mod, const char* patternCombo, std::vector<int> offsets, int extra);

	template <class T>
	T ReadMem(uintptr_t addr)
	{
		T read;
		ReadProcessMemory(m_hSECOND, (LPCVOID)addr, &read, sizeof(read), NULL);
		return read;
	}

protected:
	
	HANDLE m_hFIRST;
	HANDLE m_hSECOND;

private:
	bool IsChild();
	bool FindProcessModules();
	void FindProcessIDS();

	bool DebugPrivileges(bool needDebug);

	std::wstring m_FIRSTPROCNAME;
	std::wstring m_SECONDPROCNAME;
	
	std::vector<DWORD> m_FIRSTPIDS;
	DWORD m_SECONDPID;
	ACCESS_MASK m_NECESSARYACCESS;
	bool m_ABSOLUTEACCESS;

	STATE m_STATE;

	MODULEENTRY32* m_MODULES; /*Array of module addresses*/
};

DEFINE_ENUM_FLAG_OPERATORS(Process::STATE)
template <class T>
constexpr bool flag_has_value(T flags, T value)
{
	return (std::underlying_type_t<T>)flags & (std::underlying_type_t<T>)value;
}