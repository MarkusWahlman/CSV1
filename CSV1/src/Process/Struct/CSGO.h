#pragma once
#include "Windows.h"
#include "../Process.h"
#include "../g_Offsets.h"

#include "glm/glm.hpp"
#include "glm/gtc/type_ptr.hpp"

#include <array>

#define CSGO_MAXBONES 128   /*128 Maxium bones, but most are useless for us*/

namespace CSGO
{
	enum class ObjectSizes
	{
		ENTITY = 0x10,
	};

	enum class BoneFlags 
	{
		PELVIS =		1 << 0, 
		/*Empty*/
		/*Empty*/
		HIP	=			1 << 3,
		LOWER_SPINE =	1 << 4,
		MIDDLE_SPINE =	1 << 5,
		UPPER_SPINE =	1 << 6,
		NECK =			1 << 7,
		HEAD =			1 << 8,
		SIZE = 9,
	};

	enum class EntityFlags
	{
		FL_ONGROUND = (1 << 0), 	// At rest / on the ground
		FL_DUCKING = (1 << 1),		// Player flag -- Player is fully crouched
		FL_WATERJUMP = (1 << 2),	// player jumping out of water
		FL_ONTRAIN = (1 << 3),		// Player is _controlling_ a train, so movement commands should be ignored on client during prediction.
		FL_INRAIN = (1 << 4),		// Indicates the entity is standing in rain
		FL_FROZEN = (1 << 5),		// Player is frozen for 3rd person camera
		FL_ATCONTROLS = (1 << 6),	// Player can't move, but keeps key inputs for controlling another entity
		FL_CLIENT = (1 << 7),		// Is a player
		FL_FAKECLIENT = (1 << 8),	// Fake client, simulated server side; don't send network messages to them
		FL_INWATER = (1 << 10),		// In water
	};

	enum class WeaponID
	{
		weapon_none,
		weapon_deagle,
		weapon_elite,
		weapon_fiveseven,
		weapon_glock,
		weapon_p228,
		weapon_usp,
		weapon_ak47,
		weapon_aug,
		weapon_awp,
		weapon_famas,
		weapon_g3sg1,
		weapon_galil,
		weapon_galilar,
		weapon_m249,
		weapon_m3,
		weapon_m4a1,
		weapon_mac10,
		weapon_mp5navy,
		weapon_p90,
		weapon_scout,
		weapon_sg550,
		weapon_sg552,
		weapon_tmp,
		weapon_ump45,
		weapon_xm1014,
		weapon_bizon,
		weapon_mag7,
		weapon_negev,
		weapon_sawedoff,
		weapon_tec9,
		weapon_taser,
		weapon_hkp2000,
		weapon_mp7,
		weapon_mp9,
		weapon_nova,
		weapon_p250,
		weapon_scar17,
		weapon_scar20,
		weapon_sg556,
		weapon_ssg08,
		weapon_knifegg,
		weapon_knife,
		weapon_flashbang,
		weapon_hegrenade,
		weapon_smokegrenade,
		weapon_molotov,
		weapon_decoy,
		weapon_incgrenade,
		weapon_c4
	};

	class Entity
	{
	public:
		virtual void Update();

	protected:
		Entity(uintptr_t entityPtrAddress, Process& proc)
			: m_EntityPtrAddress(entityPtrAddress),
			  m_Process(proc)
		{
			Update();
		}

		const glm::vec3& GetOrigin() const { return m_Origin; }
		uintptr_t GetEntityAddress() const { return m_EntityAddress; }
		void ChangeEntityPtrAddress(uintptr_t address) { m_EntityPtrAddress = address; }

		uintptr_t m_EntityAddress;
		uintptr_t m_EntityPtrAddress;
		Process& m_Process;

		glm::vec3 m_Origin;
		
	};

	class Player : public Entity
	{
	public:
		Player(uintptr_t entityPtrAddress, Process& proc) 
			: Entity(entityPtrAddress, proc),
			m_BoneMatrices({}),
			m_BonePositions({}),
			m_Health(0),
			m_Team(0),
			m_fFlags(0),
			m_IsDormant(false),
			m_IsDefusing(false),
			m_ViewAnglesY(0.0f)
		{}

		virtual void Update() override;

		using Entity::GetOrigin;
		using Entity::GetEntityAddress;

		const std::array<glm::mat3x4, CSGO_MAXBONES>& GetBoneMatricies() const { return m_BoneMatrices; }
		const std::array<glm::vec3, CSGO_MAXBONES>& GetBonePositions() const { return m_BonePositions; }
		float GetViewAnglesY() const { return m_ViewAnglesY; }

		int GetHealth() const { return m_Health; }
		int GetTeam() const { return m_Team; }
		int GetFFlags() const { return m_fFlags; }

		bool IsDormant() const { return m_IsDormant; }
		bool IsDefusing() const { return m_IsDefusing; }

	protected:
		std::array<glm::mat3x4, CSGO_MAXBONES> m_BoneMatrices;
		std::array<glm::vec3, CSGO_MAXBONES> m_BonePositions;
		int m_Health;
		int m_Team;
		int m_fFlags;
		bool m_IsDormant;
		bool m_IsDefusing;
		float m_ViewAnglesY;
	};
	
	class Weapon : public Entity
	{
	public:
		Weapon(uintptr_t entityPtrAddress, Process& proc)
			:
			Entity(entityPtrAddress, proc),
			m_WeaponZoom(1.0f),
			m_WeaponID(0)
		{}
		using Entity::GetOrigin;
		using Entity::ChangeEntityPtrAddress;

		void Update() override;

		const float GetWeaponZoom() { return m_WeaponZoom; }
		const int GetWeaponID() { return m_WeaponID; }

	private:
		float m_WeaponZoom;
		int m_WeaponID;
	};

	class LocalPlayer : public Player
	{
	public:	//LocalPlayer is like any other player another player, you can get anything you need from ClientState
		LocalPlayer(Process& proc)
			: 
			Player(g_Offsets::signatures::LocalPlayer, proc),
			m_WeaponHandle(m_Process.ReadMem<int>(uintptr_t(m_EntityAddress + g_Offsets::netvars::m_hActiveWeapon))),
			m_WeaponEntityIndex(m_WeaponHandle & 0xFFF),
			m_ActiveWeapon(g_Offsets::signatures::EntityList + (m_WeaponEntityIndex * (int)ObjectSizes::ENTITY - (int)ObjectSizes::ENTITY), proc),
			m_FOV(90)
		{}
		
		using Entity::GetOrigin;
		using Entity::GetEntityAddress;

		const Weapon& GetActiveWeapon() const { return m_ActiveWeapon; }
		int GetFOV() const { return m_FOV; }

		void Update() override;

	private:

		uintptr_t m_WeaponHandle;
		int m_WeaponEntityIndex;
		Weapon m_ActiveWeapon;

		int m_FOV;
	};

	class ClientState
	{
	public:
		ClientState(Process& proc) :
			m_Process(proc), 
			m_ClientStateAddr(proc.ReadMem<uintptr_t>(g_Offsets::signatures::ClientState)),

			m_WindowX(1920),
			m_WindowY(1080),

			m_IsMouseLocked(false),
			m_ServerMaxPlayers(0),
			m_ViewAngles({}),
			m_ViewMatrix({})
		{}

		void Update();	
		bool GetIsMouseLocked() { return m_IsMouseLocked; }
		int GetServerMaxPlayers() { return m_ServerMaxPlayers; };
		const glm::vec3& GetViewAngles() { return m_ViewAngles; }
		const glm::mat4& GetViewMatrix() { return m_ViewMatrix; }
		const std::string& GetCurrentMapName() { return m_CurrentMapName; }

		int GetWindowX() { return m_WindowX; }
		int GetWindowY() { return m_WindowY; }

	private:
		uintptr_t m_ClientStateAddr;
		Process& m_Process;

		int m_WindowX;
		int m_WindowY;

		bool m_IsMouseLocked;
		int m_ServerMaxPlayers;
		glm::vec3 m_ViewAngles;
		glm::mat4 m_ViewMatrix;
		std::string m_CurrentMapName;
	};

	class EntityList
	{
	public:
		EntityList(Process& proc, ClientState& clientState) :
			m_Process(proc), m_ClientState(clientState), m_LocalPlayer(proc) {}

		void Update();

		const std::vector<Player>& GetPlayers() { return m_Players; }
		const std::vector<Player*>& GetValidPlayers() { return m_ValidPlayers; }
		const LocalPlayer& GetLocalPlayer() { return m_LocalPlayer; }

	private:
		std::vector<Player> m_Players;
		std::vector<Player*> m_ValidPlayers;
		LocalPlayer m_LocalPlayer;

		Process& m_Process;
		ClientState& m_ClientState;
	};

	glm::vec3 WorldToScreen(const glm::vec3& world, const glm::mat4& ViewMatrix, const int& WindowX, const int& WindowY);
	glm::vec3 BoneMatrixToWorld(const glm::mat3x4& boneMatrix);
}