#pragma once

#include "Process/Struct/CSGO.h"

#include <glm/common.hpp>

#include <thread>

struct AimbotConfig
{
	AimbotConfig()
		:
		StartKey1(0),
		StartKey2(0),
		AimbotActive(false),
		StopWhenShooting(true),
		Speed(10),
		StopAimingDistance(21),
		ChangeDirectionIf(1),
		ActivateDistance(105),
		AimTeammates(false),
		BoneFlags(CSGO::BoneFlags::HEAD)
	{}

	UINT StartKey1;
	UINT StartKey2;
	bool AimbotActive;
	bool StopWhenShooting;
	int Speed;
	int StopAimingDistance;
	int ChangeDirectionIf;
	float ActivateDistance;
	bool AimTeammates;
	CSGO::BoneFlags BoneFlags;
};

class HumanizedInput
{
public:
	HumanizedInput(int WindowX, int WindowY)
		: 
		m_MouseInput({}),
		m_WindowX(WindowX),
		m_WindowY(WindowY)
	{
		m_MouseInput.type = INPUT_MOUSE;
		m_MouseInput.mi.dwFlags = MOUSEEVENTF_MOVE;
	}


	void MoveToward(glm::vec3 point, AimbotConfig& aimConfig);

private:
	INPUT m_MouseInput;

	int m_WindowX;
	int m_WindowY;
};

class Aimbot
{
public:
	Aimbot(AimbotConfig& aimbotConfig, CSGO::EntityList& entityList, CSGO::ClientState& clientState);
	~Aimbot();

	void HandleThreads();

private:
	AimbotConfig& m_AimbotConfig;
	
	int m_WindowX;
	int m_WindowY;

	glm::mat4 m_DefaultProj;
	glm::mat4 m_Proj;
	glm::mat4 m_View;

	CSGO::EntityList& m_EntityList;
	CSGO::ClientState& m_ClientState;

	std::thread* m_currentThread;
	void AimbotThread();
};

