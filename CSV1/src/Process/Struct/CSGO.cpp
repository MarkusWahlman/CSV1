#include "CSGO.h"
#include "glm/glm.hpp"

#include "GLFW/glfw3.h"

#include <array>

namespace CSGO
{
    void Entity::Update() 
    { 
        m_EntityAddress = m_Process.ReadMem<uintptr_t>(m_EntityPtrAddress); 

        m_Origin = m_Process.ReadMem<glm::vec3>(uintptr_t(m_EntityAddress + g_Offsets::netvars::m_vecOrigin));
    }

    void Player::Update()
    {
        Entity::Update();

        m_ViewAnglesY = m_Process.ReadMem<float>(uintptr_t(m_EntityAddress + g_Offsets::netvars::m_angEyeAnglesY));

        m_IsDefusing = m_Process.ReadMem<bool>(uintptr_t(m_EntityAddress + g_Offsets::netvars::m_bIsDefusing));
       
        m_Health = m_Process.ReadMem<int>(uintptr_t(m_EntityAddress + g_Offsets::netvars::m_iHealth));
        m_Team = m_Process.ReadMem<int>(uintptr_t(m_EntityAddress + g_Offsets::netvars::m_iTeamNum));
        m_fFlags = m_Process.ReadMem<int>(uintptr_t(m_EntityAddress + g_Offsets::netvars::m_fFlags));
        m_IsDormant = m_Process.ReadMem<bool>(uintptr_t(m_EntityAddress + g_Offsets::signatures::m_bDormant));

        uintptr_t BoneMatriciesPtr = m_Process.ReadMem<uintptr_t>(uintptr_t(m_EntityAddress + g_Offsets::netvars::m_dwBoneMatrix));
        m_BoneMatrices = m_Process.ReadMem<std::array<glm::mat3x4, CSGO_MAXBONES>>(BoneMatriciesPtr);

        for (int i = 0; i < CSGO_MAXBONES; ++i)
        {
            m_BonePositions[i] = CSGO::BoneMatrixToWorld(m_BoneMatrices[i]);
        }
    }

    void Weapon::Update()
    {
        Entity::Update();
       
        m_WeaponZoom = m_Process.ReadMem<float>(uintptr_t(m_EntityAddress + g_Offsets::netvars::m_zoomLevel));
        m_WeaponID = m_Process.ReadMem<int>(uintptr_t(m_EntityAddress + g_Offsets::netvars::m_iItemDefinitionIndex));
    }

    void LocalPlayer::Update()
    {
        Player::Update();

        m_FOV = m_Process.ReadMem<int>(uintptr_t(m_EntityAddress + g_Offsets::netvars::m_iFOV));

        /*Update the weapon*/
        m_WeaponHandle = m_Process.ReadMem<int>(uintptr_t(m_EntityAddress + g_Offsets::netvars::m_hActiveWeapon));
        m_WeaponEntityIndex = m_WeaponHandle & 0xFFF;
        m_ActiveWeapon.ChangeEntityPtrAddress(g_Offsets::signatures::EntityList + (m_WeaponEntityIndex * (int)ObjectSizes::ENTITY - (int)ObjectSizes::ENTITY));
        m_ActiveWeapon.Update();
    }

    void ClientState::Update()
    {
        m_ServerMaxPlayers = m_Process.ReadMem<int>(m_ClientStateAddr + g_Offsets::signatures::ClientState_MaxPlayer);
        m_ViewAngles = m_Process.ReadMem<glm::vec3>(m_ClientStateAddr + g_Offsets::signatures::ClientState_ViewAngles);
        m_CurrentMapName = std::string(m_Process.ReadMem<std::array<char, 0x80>>(m_ClientStateAddr + g_Offsets::signatures::dwClientState_Map).data());

        m_ViewMatrix = m_Process.ReadMem<glm::mat4>(g_Offsets::signatures::dwViewMatrix);

        const GLFWvidmode* mode = glfwGetVideoMode(glfwGetPrimaryMonitor());
        m_WindowX = mode->width;
        m_WindowY = mode->height;

        POINT cursorPos{};
        GetCursorPos(&cursorPos);
        if ((cursorPos.x < m_WindowX / 2 - 10 || cursorPos.x > m_WindowX / 2 + 10)
            ||
            (cursorPos.y < m_WindowY / 2 - 10 || cursorPos.y > m_WindowY / 2 + 10)
            )
                m_IsMouseLocked = false;
        else 
            m_IsMouseLocked = true;
    }

    void EntityList::Update()
    {
        m_ValidPlayers.clear();
        m_LocalPlayer.Update();

        /*Deal with updating players*/
        for (unsigned int i = 0; i < (unsigned int)m_ClientState.GetServerMaxPlayers(); ++i)
        {
            if(i >= m_Players.size())
                m_Players.push_back(Player(g_Offsets::signatures::EntityList + i * (int)ObjectSizes::ENTITY, m_Process));
            m_Players[i].Update();

            if (m_Players[i].GetHealth() < 1 || m_Players[i].GetHealth() > 100 || m_Players[i].IsDormant())
                continue;

            if (i >= m_ValidPlayers.size()) 
                m_ValidPlayers.push_back(&m_Players[i]);
        }
    }
 
    glm::vec3 WorldToScreen(const glm::vec3& world, const glm::mat4& ViewMatrix, const int& WindowX, const int& WindowY)
    {
        glm::vec3 returnVec{};

        returnVec.x = ViewMatrix[0][0] * world.x + ViewMatrix[0][1] * world.y + ViewMatrix[0][2] * world.z + ViewMatrix[0][3];
        returnVec.y = ViewMatrix[1][0] * world.x + ViewMatrix[1][1] * world.y + ViewMatrix[1][2] * world.z + ViewMatrix[1][3];
        returnVec.z = ViewMatrix[2][0] * world.x + ViewMatrix[2][1] * world.y + ViewMatrix[2][2] * world.z + ViewMatrix[2][3];
        float flTemp = ViewMatrix[3][0] * world.x + ViewMatrix[3][1] * world.y + ViewMatrix[3][2] * world.z + ViewMatrix[3][3];

        if (flTemp < 0.01f)
            return glm::vec3(-1.0f, -1.0f, -1.0f);

        returnVec.x /= flTemp;
        returnVec.y /= flTemp;
        returnVec.z /= flTemp;

        float x = (float)WindowX / 2.f;
        float y = (float)WindowY / 2.f;
        float z = (float)WindowY / 2.f;

        x += 0.5f * returnVec.x * (float)WindowX + 0.5f;
        y -= 0.5f * returnVec.y * (float)WindowY + 0.5f;

        returnVec.x = x;
        returnVec.y = WindowY - y;  //As we're drawing from the bottom left

        return returnVec;
    }

    glm::vec3 BoneMatrixToWorld(const glm::mat3x4& boneMatrix)
    {
        return glm::vec3( boneMatrix[0][3],
                          boneMatrix[1][3],
                          boneMatrix[2][3] );
    }
}