#include "Bunnyhop.h"

#include <random>

KeyInput::KeyInput(char Key)
	:
	m_Input(),
	m_Key(Key)
{
	SHORT key = VkKeyScan(m_Key);
	UINT mappedKey = MapVirtualKey(LOBYTE(key), 0);

	m_Input.type = INPUT_KEYBOARD;
	m_Input.ki.dwFlags = KEYEVENTF_SCANCODE;
	m_Input.ki.wScan = mappedKey;
}

KeyInput::~KeyInput()
{}

void KeyInput::Send()
{
	m_Input.ki.dwFlags = KEYEVENTF_SCANCODE;
	SendInput(1, &m_Input, sizeof(INPUT));

	Sleep(rand() % (10 - 5 + 1) + 5);	/*Wait randomly 5-10 ms*/

	m_Input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
	SendInput(1, &m_Input, sizeof(INPUT));
}

void KeyInput::ChangeKey(char Key)
{
	m_Key = Key;
	SHORT key = VkKeyScan(m_Key);
	UINT mappedKey = MapVirtualKey(LOBYTE(key), 0);

	m_Input.ki.wScan = mappedKey;
}

Bunnyhop::Bunnyhop(BunnyhopConfig& bunnyhopConfig, CSGO::EntityList& entityList, CSGO::ClientState& clientState)
	: 
	m_BunnyhopConfig(bunnyhopConfig),
	m_currentThread(nullptr),
	m_EntityList(entityList),
	m_ClientState(clientState)
{}

Bunnyhop::~Bunnyhop()
{
	m_BunnyhopConfig.BunnyhopActive = false; /*Running threads will stop*/
}

void Bunnyhop::BunnyhopThread()
{
	const std::vector<CSGO::Player*>& validPlayers = m_EntityList.GetValidPlayers();
	const CSGO::LocalPlayer& localPlayer = m_EntityList.GetLocalPlayer();
	
	KeyInput keyInput(' ');

	while (m_BunnyhopConfig.BunnyhopActive)
	{
		Sleep(m_BunnyhopConfig.MissChance);

		if (!m_ClientState.GetIsMouseLocked())
			continue;

		if (m_BunnyhopConfig.StartKey1 == VK_SPACE)
		{
			if(keyInput.CurrentKey() != 'p')
				keyInput.ChangeKey('p');
		}
		else if (keyInput.CurrentKey() != ' ')
		{
			keyInput.ChangeKey(' ');
		}

		if (localPlayer.GetFFlags() & (int)CSGO::EntityFlags::FL_ONGROUND &&
			(GetAsyncKeyState(m_BunnyhopConfig.StartKey1) & 0x8000 || GetAsyncKeyState(m_BunnyhopConfig.StartKey2) & 0x8000))	
		{
			keyInput.Send();
		}
	}
}

void Bunnyhop::HandleThreads()
{
	if (m_BunnyhopConfig.BunnyhopActive)
	{
		if (m_currentThread == nullptr)
		{
			m_currentThread = new std::thread(&Bunnyhop::BunnyhopThread, this);
		}
	}
	else /*Bunnyhop not active*/
	{
		if (m_currentThread != nullptr)
		{
			m_currentThread->join();	/*Thread should end itself, wait for it.*/
			delete(m_currentThread);
			m_currentThread = nullptr;
		}
	}
}
