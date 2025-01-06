#pragma once
enum DOTA_GC_TEAM : int {
	DOTA_GC_TEAM_GOOD_GUYS = 0,
	DOTA_GC_TEAM_BAD_GUYS = 1,
	DOTA_GC_TEAM_BROADCASTER = 2,
	DOTA_GC_TEAM_SPECTATOR = 3,
	DOTA_GC_TEAM_PLAYER_POOL = 4,
	DOTA_GC_TEAM_NOTEAM = 5,
	DOTA_GC_TEAM_CUSTOM_1 = 6,
	DOTA_GC_TEAM_CUSTOM_2 = 7,
	DOTA_GC_TEAM_CUSTOM_3 = 8,
	DOTA_GC_TEAM_CUSTOM_4 = 9,
	DOTA_GC_TEAM_CUSTOM_5 = 10,
	DOTA_GC_TEAM_CUSTOM_6 = 11,
	DOTA_GC_TEAM_CUSTOM_7 = 12,
	DOTA_GC_TEAM_CUSTOM_8 = 13,
	DOTA_GC_TEAM_NEUTRALS = 14
};

#include "../base/VClass.h"
#include "../base/NormalClass.h"
#include "../base/Definitions.h"
#include "../base/Vector.h"
#include "../base/Color.h"


#include "CEntityIdentity.h"
#include "CHandle.h"
#include "../VMI.h"

class CDOTAPlayerController;

struct CEntSchemaClassBinding {
	const char
		* binaryName, // ex: C_DOTA_Unit_Hero_Nevermore
		*GetCEntityClassNetworkInstance,
		* serverName; // ex: CDOTA_Unit_Hero_Nevermore

	void* parent;
	const char* fullName; // ex: client.dll!C_DOTA_Unit_Hero_Nevermore
	void* idk;

	int listIndex;
	PAD(4 + 8 * 2);
};

class CBaseEntity : public VClass {
public:
	struct CModelState : public NormalClass {
		FIELD(uint64_t, MeshGroupMask, 0x198);
		GETTER(const char*, GetModelName, 0xa8);
		NormalClass* GetModel() const {
			return *Member<NormalClass**>(0xa0);
		};
	};

	struct CSkeletonInstance : public VClass {
		// reversed from xref: "CBaseModelEntity::SetBodygroup(%d,%d) failed: CBaseModelEntity has no model!\n"
		// last two subs are get and set
		GETTER(CBaseEntity*, GetOwner, 0x30);
		IGETTER(CModelState, GetModelState, 0x170)
	};

	CEntSchemaClassBinding* SchemaBinding() const {
		return GetVFunc(VMI::CBaseEntity::GetSchemaBinding).Call<CEntSchemaClassBinding*>();
	};

	inline static void(__fastcall* OnColorChanged)(void*) = {};

	GETTER(CEntityIdentity*, GetIdentity, 0x10);
	GETTER(int, GetHealth, 0x34c);
	GETTER(int, GetMaxHealth, 0x348);
	GETTER(DOTA_GC_TEAM, GetTeam, 0x3eb);
	GETTER(int8_t, GetLifeState, 0x350);
	GETTER(int8_t, GetUnitType, 0xa94);
	GETTER(CHandle<CDOTAPlayerController>, GetOwnerEntityHandle, 0x43c);
	GETTER(CSkeletonInstance*, GetGameSceneNode, 0x330);

	bool IsAlive() const {
		return GetLifeState() == 0;
	}

	bool IsDormant() const {
		return GetIdentity() && GetIdentity()->IsDormant();
	}

	const char* GetModelName() const {
		// og's explanation:
		// CModelState has 3 CStrongHandle pointers at 0xA0 and below
		// These strong handles have a model pointer and its name
		return GetGameSceneNode()->GetModelState()->GetModelName();
	}

	bool IsSameTeam(CBaseEntity* other) const {
		return GetTeam() == other->GetTeam();
	}

	uint32_t GetHandle()  const {
		auto id = GetIdentity();
		if (!IsValidReadPtr(id))
			return INVALID_HANDLE;
		return id->entHandle;
	}

	// Returns the index of this entity in the entity system
	uint32_t GetIndex()  const {
		return H2IDX(GetHandle());
	}

	void SetColor(Color clr)
	{
		Field<Color>(0x5f0) = clr;
		OnColorChanged(this);
	}

	float& ModelScale() const {
		return Member<VClass*>(0x330)
			->Field<float>(0xcc);
	}

	Vector GetPos() const {
		return Member<VClass*>(0x330)
			->Member<Vector>(0xd0);
	}

	// In degrees from 180 to -180(on 0 it looks right)
	float GetRotation() const {
		return Member<VClass*>(0x330)
			->Member<Vector>(0xc0).y;
	}

	// Gets the point in front of the entity at the specified distance
	Vector GetForwardVector(float dist) const {
		auto pos = GetPos();
		float rotation = GetRotation() * M_PI / 180;

		float sine = sinf(rotation), cosine = cosf(rotation);
		auto forwardVec = Vector(cosine * dist, sine * dist, 0);
		return pos + forwardVec;
	}
};
