#include <Windows.h>
#include <Psapi.h>  // Äëÿ GetModuleInformation
#include <iostream>
#include <sstream>
#include <vector>
#include "./src/sdk/entities/CGameEntitySystem.h"
#include "./src/sdk/entities/CBaseEntity.h"
#include "./src/sdk/entities/Chandle.h"
#include "./src/sdk/entities/CDOTABaseAbility.h"
#include "./src/sdk/entities/CDOTABaseNPC.h"
#include "./src/sdk/base/Vector.h"

#pragma comment(lib, "Psapi.lib")  // Linking with Psapi.lib

// Define the function type PrepareUnitOrders
typedef void(__fastcall* _PrepareUnitOrders)(CBaseEntity* player, int orderType, int entHandle, Vector* movePosition,
    int abilityIndex, int orderIssuer, CBaseEntity* entity,
    bool queue, int queueBehavior);
_PrepareUnitOrders PrepareUnitOrders = nullptr;

// Function for searching for a signature in memory
uintptr_t FindSignature(HMODULE module, const std::vector<BYTE>& pattern) {
    MODULEINFO modinfo = { 0 };
    if (!GetModuleInformation(GetCurrentProcess(), module, &modinfo, sizeof(MODULEINFO))) {
        return 0;
    }

    BYTE* data = reinterpret_cast<BYTE*>(modinfo.lpBaseOfDll);
    const size_t size = modinfo.SizeOfImage;

    for (size_t i = 0; i < size - pattern.size() + 1; i++) {
        bool found = true;
        for (size_t j = 0; j < pattern.size(); j++) {
            if (data[i + j] != pattern[j]) {
                found = false;
                break;
            }
        }
        if (found) {
            return reinterpret_cast<uintptr_t>(&data[i]);
        }
    }
    return 0;
}

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
}

void HackThread(HMODULE instance) {
    AllocConsole();
    FILE* file;
    freopen_s(&file, "CONOUT$", "w", stdout);

    // signature from x64dbg
    std::vector<BYTE> signature = {
        0x4C, 0x89, 0x4C, 0x24, 0x20, 0x44, 0x89, 0x44, 0x24, 0x18,
        0x89, 0x54, 0x24, 0x10, 0x48, 0x89, 0x4C, 0x24, 0x08, 0x55,
        0x53, 0x56, 0x57, 0x41, 0x54, 0x48, 0x8D, 0x6C, 0x24, 0xF0
    };

    // get adress client.dll
    HMODULE clientDll = GetModuleHandleA("client.dll");
    if (!clientDll) {
        std::cout << "client.dll not found" << std::endl;
        return;
    }

    // searching signature
    uintptr_t prepareUnitOrdersAddress = FindSignature(clientDll, signature);
    if (prepareUnitOrdersAddress == 0) {
        std::cout << "Sign not found" << std::endl;
        return;
    }

    std::cout << "adress PrepareUnitOrders: 0x" << std::hex << prepareUnitOrdersAddress << std::endl;
    PrepareUnitOrders = reinterpret_cast<_PrepareUnitOrders>(prepareUnitOrdersAddress);

    while (!GetAsyncKeyState(VK_END)) {
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            auto GameResourceServiceClient =
                ((uintptr_t(*)(const char*, void*))GetProcAddress(GetModuleHandleA("engine2.dll"),
                    "CreateInterface"))("GameResourceServiceClientV001", nullptr);
            CGameEntitySystem* gameEntitySystem = *(CGameEntitySystem**)((uintptr_t)GameResourceServiceClient + 0x58);
            std::cout << "GetHighestEntityIndex: " << gameEntitySystem->GetHighestEntityIndex() << std::endl;
            CBaseEntity* localPlayer = gameEntitySystem->GetEntity(1);
            if (localPlayer && PrepareUnitOrders) {
                Vector movePosition(100.0f, 100.0f, 100.0f);  
                PrepareUnitOrders(localPlayer,
                    1,
                    0,
                    &movePosition,
                    0,
                    0,
                    localPlayer,
                    false,
                    0);
                std::cout << "Hero going." << std::endl;
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