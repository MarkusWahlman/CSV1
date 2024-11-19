#pragma once

#include <Userenv.h>
#include <winternl.h>

#include "Process.h"
#include "ntdllUserDef.h"

/*	Service.h and Service.cpp have been modified, 
	but they are from https://github.com/Schnocker/HLeaker/	*/

namespace Service
{
	HANDLE WINAPI ServiceRunProgram(LPCSTR lpFilename, LPCSTR lpArguments, LPCSTR lpDir, LPPROCESS_INFORMATION ProcessInformation, BOOL Inherit, HANDLE hParent);
}