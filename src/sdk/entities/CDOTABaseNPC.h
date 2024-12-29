#pragma once
#include "CBaseEntity.h"
#include "CDOTAItem.h"
#include "CDOTAUnitInventory.h"
#include "CDOTAModifierManager.h"
#include "../Enums.h"

class CEconWearable;

class CDOTABaseNPC : public CBaseEntity {
public:
	inline static float(*GetAttackSpeed)(CDOTABaseNPC* npc) = nullptr;

	// Reversed via xref "CDOTA_Hud_HealthMana::Update"
	// in the beginning after the repeated section with xrefs the hero is acquired
	// trace the variable in the call with "max_physical_barrier" to the assignment of a value where an offset to hero is used
	// that offset ultimately leads to such a structure
	struct BarrierData {
		float
			maxPhys,
			maxMagic,
			maxAll,
			phys,  // blocks physical damage
			magic, // blocks magical damage
			all;   // blocks all forms of damage
	};

	GETTER(BarrierData, GetBarriers, 0x17E4);

	// GETTER(CAssetModifierContainer*, GetAssetModifierContainer, 0xa20);

	IGETTER(CDOTAModifierManager, GetModifierManager, 0xc80);

	bool HasModifier(std::string_view name) const {
		return GetModifier(name) != nullptr;
	}

	float GetPhysicalArmorValue() const {
		return GetVFunc(VMI::CDOTABaseNPC::GetPhysicalArmorValue).Call<float>(0ull);
	}

	float GetMagicalArmorValue() const {
		return GetVFunc(VMI::CDOTABaseNPC::GetMagicalArmorValue).Call<float>(0ull, 0ull);
	}

	VGETTER(float, GetIdealSpeed, VMI::CDOTABaseNPC::GetIdealSpeed);
	VGETTER(void, OnWearablesChanged, VMI::CDOTABaseNPC::OnWearablesChanged);

	// Wrapper function combining the following conditions: 
	// Is not dormant
	// Is alive
	// Is not waiting to spawn
	bool IsTargetable() const {
		return !IsDormant() && IsAlive() && !IsWaitingToSpawn();
	}

	// JS func, uses another vtable at offset
	bool IsRoshan() const {
		return MemberInline<VClass>(0xA38)->GetVFunc(VMI::CDOTABaseNPC::IsRoshan).Call<bool>();
	}

	int GetAttackDamageMin() const { return Member<int>(0xc68) + Member<int>(0xc70); }

	float GetAttackRange() const {
		return GetVFunc(VMI::CDOTABaseNPC::GetAttackRange).Call<float>(0, 1);
	};

	GETTER(bool, IsWaitingToSpawn, 0x1510);
	GETTER(bool, IsAncient, 0xab0);
	GETTER(float, GetArmor, 0x1408);
	GETTER(float, GetBaseMagicalResistance, 0x140c);
	GETTER(const char*, GetUnitName, 0xbc8);
	GETTER(DOTAUnitAttackCapability_t, GetAttackCapabilities, 0x1184);
	GETTER(float, GetHealthRegen, 0xac0);
	GETTER(float, GetMana, 0xb00);
	GETTER(float, GetMaxMana, 0xb04);
	GETTER(float, GetManaRegen, 0xb08);
	GETTER(float, GetBaseAttackTime, 0xaf0);
	GETTER(float, GetHullRadius, 0xbb8);
	GETTER(int, GetMoveSpeed, 0xae8);
	GETTER(bool, IsMoving, 0x1528);

	FIELD(CUtlVector<CHandle<CEconWearable>>, Wearables, 0x9d0);
	FIELD(CUtlVector<CHandle<CEconWearable>>, OldWearables, 0x16e0);

	IGETTER(CDOTAUnitInventory, GetInventory, 0xfa0);

	std::span<CHandle<CDOTAItem>, 19> GetItems() const {
		return GetInventory()->GetItems();
	}

	std::span<CHandle<CDOTABaseAbility>, 35> GetAbilities() const {
		auto hAbilities = MemberInline<CHandle<CDOTABaseAbility>>(0xb28);
		return std::span<CHandle<CDOTABaseAbility>, 35>(hAbilities, 35);
	}

	CDOTABaseAbility* GetAbility(int index) const
	{
		if (index < 0 || index >= 35)
			return nullptr;

		return MemberInline<CHandle<CDOTABaseAbility>>(0xb28)[index];
	}

	CDOTABaseAbility* GetAbility(std::string_view name) const
	{
		auto abilities = GetAbilities();
		auto ability = std::find_if(abilities.begin(), abilities.end(),
			[name](auto x) {
				return x.IsValid() && x->GetIdentity()->GetName() && x->GetIdentity()->GetName() == name;
			}
		);
		return ability != abilities.end() ? *ability : nullptr;
	}

	CDOTAModifier* GetModifier(std::string_view name) const {
		auto arr = GetModifierManager()->GetModifierList();
		auto res = std::find_if(arr.begin(), arr.end(),
			[name](auto x) {
				return x->GetName() == name;
			}
		);
		return res != arr.end() ? *res : nullptr;
	}

	CDOTAItem* FindItemBySubstring(const char* str)  const {
		for (const auto& item : GetItems())
			if (
				item.IsValid()
				&& item->GetIdentity()->GetName()
				&& strstr(item->GetIdentity()->GetName(), str)
				)
				return item;

		return nullptr;
	}

	CDOTAItem* FindItem(std::string_view name) const {
		auto items = GetItems();
		auto item = std::find_if(items.begin(), items.end(),
			[name](auto item) {
				return item.IsValid() && item->GetIdentity()->GetName() && item->GetIdentity()->GetName() == name;
			}
		);
		return item != items.end() ? **item : nullptr;
	}

	bool HasState(ModifierState state) const {
		auto unitState = Member<uint64_t>(0x1080);
		return (unitState & (1Ui64 << (int)state));
	}

	// This checks for modifier states under which you cannot give orders to the hero
	bool IsDisabled() const {
		return HasState(MODIFIER_STATE_FEARED)
			|| HasState(MODIFIER_STATE_HEXED)
			|| HasState(MODIFIER_STATE_MUTED)
			|| HasState(MODIFIER_STATE_NIGHTMARED)
			|| HasState(MODIFIER_STATE_OUT_OF_GAME)
			|| HasState(MODIFIER_STATE_SILENCED)
			|| HasState(MODIFIER_STATE_STUNNED)
			|| HasState(MODIFIER_STATE_TAUNTED);
	}

	bool CanUseAbility(CDOTABaseAbility* ability) const {
		for (auto& ab : GetAbilities()) {
			if (ab->Member<float>(0x5b8))
				return false;
		}
		return IsAlive() &&
			ability->GetLevel() > 0
			&& !IsDisabled()
			&& !ability->GetCooldown()
			&& ability->GetManaCost() <= GetMana()
			&& !ability->IsInAbilityPhase();
	}

	Vector GetHealthBarPos() const {
		auto pos = GetPos();
		pos.z += Member<int>(0xaf8);
		return pos;
	};

	const char* GetModifiedAssetString(const char* str) const {
		return MemberInline<VClass>(0xA38)->GetVFunc(67).Call<const char*>(str);
	}

	const char* GetSmallIcon() const {
		return GetModifiedAssetString(GetIdentity()->GetName());
	}
};
