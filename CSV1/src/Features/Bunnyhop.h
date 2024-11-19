#pragma once

#include "Process/Struct/CSGO.h"

#include <thread>

class KeyInput
{
public:
	KeyInput(char Key);
	~KeyInput();

	void ChangeKey(char Key);
	UINT CurrentKey() { return m_Key; }

	void Send();

private:
	UINT m_Key;
	INPUT m_Input;
};

struct BunnyhopConfig
{
	BunnyhopConfig()
		:
		BunnyhopActive(false),
		MissChance(0),
		StartKey1(0x4E /*N key*/),
		StartKey2(0)
	{}

	bool BunnyhopActive;
	int MissChance;
	UINT StartKey1;
	UINT StartKey2;
};

class Bunnyhop
{
public:
	Bunnyhop(BunnyhopConfig& bunnyhopConfig, CSGO::EntityList& entityList, CSGO::ClientState& clientState);
	~Bunnyhop();

	void HandleThreads();

private:
	void BunnyhopThread();

	BunnyhopConfig& m_BunnyhopConfig;

	CSGO::EntityList& m_EntityList;
	CSGO::ClientState& m_ClientState;

	std::thread* m_currentThread;
};

