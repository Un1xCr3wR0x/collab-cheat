#include <Windows.h>
#include <iostream>
#include <sstream>
#include "./src/sdk/entities/CGameEntitySystem.h"
#include "./src/sdk/entities/CBaseEntity.h"
#include "./src/sdk/entities/Chandle.h"
#include "./src/sdk/entities/CDOTABaseAbility.h"
#include "./src/sdk/entities/CDOTABaseNPC.h"

template <typename T>
T* GetInterface(const char* name, const char* library) {
    const auto handle = GetModuleHandle(library);

    if (!handle) {
        return nullptr;
    }

    const auto functionAddress = GetProcAddress(handle, "CreateInterface");

    if (!functionAddress)
        return nullptr;

    using Fn = T * (*)(const char*, int*);

    const auto CreateInterface = reinterpret_cast<Fn>(functionAddress);

    if (!CreateInterface) {
        return nullptr;
    }

    return CreateInterface(name, nullptr);
};

void HackThread(HMODULE instance) {
    AllocConsole();
    FILE* file;
    freopen_s(&file, "CONOUT$", "w", stdout);

    while (!GetAsyncKeyState(VK_END)) {
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            auto GameResourceServiceClient =
                ((uintptr_t(*)(const char*, void*))GetProcAddress(GetModuleHandleA("engine2.dll"),
                    "CreateInterface"))("GameResourceServiceClientV001", nullptr);
            CGameEntitySystem* gameEntitySystem = *(CGameEntitySystem**)((uintptr_t)GameResourceServiceClient + 0x58);
            std::cout << "GetHighestEntityIndex: " << gameEntitySystem->GetHighestEntityIndex() << std::endl;
            
            for (std::size_t index = 0; index <= 64; ++index)
            {
                CBaseEntity* Entity = gameEntitySystem->GetEntity(index);
                if (Entity && IsValidReadPtr(Entity))
                {
                    constexpr auto offset_m_iszPlayerName = 0x650;
                    constexpr auto offset_m_hAssignedHero = 0x80c;
                    constexpr int offset_m_iTaggedAsVisibleByTeam = 0xc7c;
                    constexpr auto offset_m_hInventory = 0xf30;
                    constexpr auto offset_m_hAbilities = 0xae0;
                    constexpr auto offset_m_fCooldown = 0x580;

                    // Using CHandle from Chandle.h
                    CHandle<CBaseEntity>* heroHandle = (CHandle<CBaseEntity>*)((uintptr_t)Entity + offset_m_hAssignedHero);
                    CDOTABaseNPC* EntityHero = static_cast<CDOTABaseNPC*>(gameEntitySystem->GetEntity(heroHandle->Index()));

                    if (EntityHero && IsValidReadPtr(EntityHero)) {
                        // Get abilities using the GetAbilities method
                        std::cout << "ABILITIES!!!!!!!!!!!" << std::endl;
                        auto abilities = EntityHero->GetAbilities();
                        for (const auto& abilityHandle : abilities) {
                            if (abilityHandle.IsValid()) {
                                auto ability = gameEntitySystem->GetEntity<CDOTABaseAbility>(abilityHandle.Index());
                                if (ability && IsValidReadPtr(ability)) {
                                    std::cout << "Ability Name: " << ability->GetIdentity()->GetName() << std::endl;
                                    std::cout << "Ability Cooldown: " << ability->GetCooldown() << std::endl;
                                    std::cout << "Ability Mana: " << ability->GetLevelSpecialValueFor<int>("AbilityManaCost") << std::endl;
                                }
                            }
                        }
						std::cout << "ITEEEEEEEEEEEEEEEEM!!!!!!!!!!!" << std::endl;
						auto items = EntityHero->GetItems();
                        for (const auto& itemHandle : items) {
                            if (itemHandle.IsValid()) {
                                auto item = gameEntitySystem->GetEntity<CDOTAItem>(itemHandle.Index());
                                if (item && IsValidReadPtr(item)) {
                                    std::cout << "Item Name: " << item->GetIdentity()->GetName() << std::endl;
                                    std::cout << "Item Cooldown: " << item->GetCooldown() << std::endl;
                                    std::cout << "Item Mana: " << item->GetLevelSpecialValueFor<int>("AbilityManaCost") << std::endl;
                                }
                            }
                        }
                    }


                    if (EntityHero) {
                        int EntityHeroTaggedAsVisibleByTeam = (int)((uintptr_t)EntityHero + offset_m_iTaggedAsVisibleByTeam);
                        std::cout << "Entity: " << Entity << std::endl;
                        std::cout << "Entity Health : " << EntityHero->GetHealth() << std::endl;
                        std::cout << "Entity Max Health : " << EntityHero->GetMaxHealth() << std::endl;
                        std::cout << "Entity Name : " << (char*)((uintptr_t)Entity + offset_m_iszPlayerName) << std::endl;
                        std::cout << "Entity Assigned Hero : " << heroHandle->Index() << std::endl;
                        std::cout << "EntityHero: " << EntityHero << std::endl;
                        std::cout << "is visible: " << EntityHeroTaggedAsVisibleByTeam << std::endl;

                        std::stringstream ss;
                        ss << "player " << (char*)((uintptr_t)Entity + offset_m_iszPlayerName) 
                           << "(" << Entity << ") controls hero indexed " 
                           << heroHandle->Index() << "\n";
                        OutputDebugStringA(ss.str().c_str());
                    }
                }
            }
        }
        Sleep(200);
    }
    if (file)
        fclose(file);

    FreeConsole();
    FreeLibraryAndExitThread(instance, 0);
}

BOOL WINAPI DllMain(HMODULE instance, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        DisableThreadLibraryCalls(instance);

        const auto thread = CreateThread(nullptr, 0, reinterpret_cast<LPTHREAD_START_ROUTINE>(HackThread), instance, 0, nullptr);

        if (thread) {
            CloseHandle(thread);
        }
    }
    return TRUE;
}