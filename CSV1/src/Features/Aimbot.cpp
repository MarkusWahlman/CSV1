#include "Aimbot.h"
#include "GLFW/glfw3.h"

#include <iostream>
#include <valarray>
#include <numeric>
#include <stdlib.h>
#include <algorithm>

#include <thread>
#include <bitset>

Aimbot::Aimbot(AimbotConfig& aimbotConfig, CSGO::EntityList& entityList, CSGO::ClientState& clientState)
	: 
	m_AimbotConfig(aimbotConfig),
	m_currentThread(nullptr),
	m_EntityList(entityList),
	m_ClientState(clientState),
	m_Proj(1.0f),
	m_View(1.0f)
{
	const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
	m_WindowX = mode->width;
	m_WindowY = mode->height;

	m_DefaultProj = glm::perspective(glm::radians(90.0f), (float)m_WindowX / (float)m_WindowY, 0.1f, 100.0f);
}

Aimbot::~Aimbot()
{
	m_AimbotConfig.AimbotActive = false; /*Running threads will stop*/
}

void HumanizedInput::MoveToward(glm::vec3 point, AimbotConfig& aimConfig)
{
	static bool useOldLine{ false };

	static int dx{};
	static int dy{};
	static int steps{};
	static float Xinc{};
	static float Yinc{};
	static float X{};
	static float Y{};
	
	static int lastX{};
	static int lastY{};
	
	static glm::vec3 currentTarget{};

	dx = (int)point.x - m_WindowX/2;
	dy = (int)point.y - m_WindowY/2;

	if (abs(point.x - currentTarget.x + Xinc) > aimConfig.ChangeDirectionIf && abs(point.y - currentTarget.y + Yinc) > aimConfig.ChangeDirectionIf)
		useOldLine = false;		/*If the target has moved more than 16px (Not accounting the movement the aimbot has made) use a new line*/

	if (!useOldLine)
	{	/*Essentially "Creates a new line"*/
		currentTarget = point;
		// calculate steps required for generating pixels
		steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);

		// calculate increment in x & y for each steps
		Xinc = dx / (float)steps;
		Yinc = dy / (float)steps;

		// Put pixel for each step
		X = (float)m_WindowX / 2;
		Y = (float)m_WindowY / 2;

		lastX = (int)X;
		lastY = (int)Y;
	}

	useOldLine = true;
	
	X += Xinc;           // increment in x at each step
	Y += Yinc;           // increment in y at each step

	int xRounded = (int)round(X);
	int yRounded = (int)round(Y);

	int xInputValue = xRounded - lastX;
	int yInputValue = -(yRounded - lastY); /*Invert the Y as we're drawing from bottom left*/

	m_MouseInput.mi.dx = (xInputValue <= 1 && xInputValue >= -1) ? xInputValue : 0;
	m_MouseInput.mi.dy = (yInputValue <= 1 && yInputValue >= -1) ? yInputValue : 0;
	SendInput(1, &m_MouseInput, sizeof(INPUT));

	lastX = xRounded;
	lastY = yRounded;

	--steps;
	
	if (steps < 0)
		useOldLine = false;
}

void Aimbot::AimbotThread()
{
	static HumanizedInput input(m_WindowX, m_WindowY);

	const std::vector<CSGO::Player*>& validPlayers = m_EntityList.GetValidPlayers();
	const CSGO::LocalPlayer& localPlayer = m_EntityList.GetLocalPlayer();

	

	while (m_AimbotConfig.AimbotActive)
	{
		std::this_thread::sleep_for(std::chrono::nanoseconds(m_AimbotConfig.Speed * 100));

		if((m_AimbotConfig.StartKey1 != 0 || m_AimbotConfig.StartKey2 != 0)
			&&
			(!(GetAsyncKeyState(m_AimbotConfig.StartKey1) & 0x8000) && !(GetAsyncKeyState(m_AimbotConfig.StartKey2) & 0x8000)))
				continue;

		if (m_AimbotConfig.StopWhenShooting)
			if (GetAsyncKeyState(VK_LBUTTON) < 0)
				continue;

		if(!m_ClientState.GetIsMouseLocked())
			continue;

		if (localPlayer.GetFOV() == 0 || localPlayer.GetFOV() == 90)
		{
			m_Proj = glm::perspective(glm::radians(90.0f), (float)m_WindowX / (float)m_WindowY, 0.1f, 100.0f);
		}
		else
			m_Proj = glm::perspective(glm::radians(float(localPlayer.GetFOV())), (float)m_WindowX / (float)m_WindowY, 0.1f, 100.0f);

		glm::vec3 closestSpotPos{ -1, -1, -1 };
		float closestDistance = FLT_MAX;
		const glm::mat4& viewMatrix = m_ClientState.GetViewMatrix();

		for (CSGO::Player* player : validPlayers)
		{
			if (player->GetEntityAddress() == localPlayer.GetEntityAddress())
				continue;
			
			if (player->GetTeam() == localPlayer.GetTeam() && !m_AimbotConfig.AimTeammates)
				continue;

			const std::array<glm::vec3, CSGO_MAXBONES>& bonePositions = player->GetBonePositions();

			std::bitset<(int)CSGO::BoneFlags::SIZE> bitSet((unsigned int)m_AimbotConfig.BoneFlags);
			for (int i = 0; i < (int)CSGO::BoneFlags::SIZE; ++i)
			{
				if (bitSet[i])
				{
					glm::vec3 bonePos = bonePositions[i];

					glm::vec3 currentSpotPos = CSGO::WorldToScreen(bonePos, viewMatrix, m_WindowX, m_WindowY);

					glm::vec3 unProjectedScreenPos = glm::unProject(currentSpotPos, m_View, m_Proj, glm::vec4(0.0f, 0.0f, m_WindowX, m_WindowY));

					glm::vec3 reProject = glm::project(unProjectedScreenPos, m_View, m_DefaultProj, glm::vec4(0.0f, 0.0f, m_WindowX, m_WindowY));

					float distance = abs(unProjectedScreenPos.z * float(sqrtf(powf(reProject.x - (float)m_WindowX / 2.0f, 2) + powf(reProject.y - (float)m_WindowY / 2.0f, 2) * 1.0f)));

					if (roundf(distance*10000) == 551684) /*Player is on the opposite side*/
						continue;

					if (distance < m_AimbotConfig.ActivateDistance && distance < closestDistance && distance > m_AimbotConfig.StopAimingDistance)
					{
						closestSpotPos = currentSpotPos;
						closestDistance = distance;
					}
				}
			}
		}

		if (closestSpotPos.x < 0 || closestSpotPos.y < 0)
			continue;
		input.MoveToward(closestSpotPos, m_AimbotConfig);

	}
}

void Aimbot::HandleThreads()
{
	m_WindowX = m_ClientState.GetWindowX();
	m_WindowY = m_ClientState.GetWindowY();

	if (m_AimbotConfig.AimbotActive)
	{
		if(m_currentThread == nullptr)
		{
			m_currentThread = new std::thread(&Aimbot::AimbotThread, this);
		}
	}
	else /*Aimbot not active*/
	{
		if (m_currentThread != nullptr)
		{
			m_currentThread->join();	/*Thread should end itself, wait for it.*/
			delete(m_currentThread);
			m_currentThread = nullptr;
		}
	}
}