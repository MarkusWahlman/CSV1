#include "g_Offsets.h"

bool g_Offsets::Intialize(Process* proc)
{
	if (flag_has_value(proc->State(), Process::STATE::OffsetsFound))
		return true;

	try
	{
		using namespace signatures;

		EntityList = proc->FindPattern(Process::Module::CLIENTDLL, "BB ? ? ? ? 83 FF 01 0F 8C ? ? ? ? 3B F8", { 1 }, 0);
		LocalPlayer = proc->FindPattern(Process::Module::CLIENTDLL, "8D 34 85 ? ? ? ? 89 15 ? ? ? ? 8B 41 08 8B 48 04 83 F9 FF", { 3 }, 4);
		dwViewMatrix = proc->FindPattern(Process::Module::CLIENTDLL, "0F 10 05 ? ? ? ? 8D 85 ? ? ? ? B9", { 3 }, 176);

		ClientState = proc->FindPattern(Process::Module::ENGINEDLL, "A1 ? ? ? ? 33 D2 6A 00 6A 00 33 C9 89 B0", { 1 }, 0);
		ClientState_ViewAngles = proc->FindPattern(Process::Module::ENGINEDLL, "F3 0F 11 86 ? ? ? ? F3 0F 10 44 24 ? F3 0F 11 86", { 4 }, 0);
		ClientState_MaxPlayer = proc->FindPattern(Process::Module::ENGINEDLL, "A1 ? ? ? ? 8B 80 ? ? ? ? C3 CC CC CC CC 55 8B EC 8A 45 08", { 7 }, 0);
		dwClientState_Map = proc->FindPattern(Process::Module::ENGINEDLL, "05 ? ? ? ? C3 CC CC CC CC CC CC CC A1", { 1 }, 0);

		m_bDormant = proc->FindPattern(Process::Module::CLIENTDLL, "8A 81 ? ? ? ? C3 32 C0", { 2 }, 8);
	}
	catch (std::exception e)
	{
		std::cout << "Error: Process.cpp [bool Process::InitializeOffsets()] " << e.what() << std::endl;
		return false;
	}
	return true;
}