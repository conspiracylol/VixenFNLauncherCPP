#pragma once
#include <iostream>
#include <windows.h>
#include "logging.h"

void PrintErrorMessage(const char* action)
{
    LPSTR messageBuffer = nullptr;
    DWORD errorMessageID = GetLastError();

    FormatMessageA(
        FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        errorMessageID,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPSTR)&messageBuffer,
        0, NULL);

    std::string errorMessage = "(INJECTOR FAILED) Failed to " + std::string(action) + ". Error code: " + std::to_string(errorMessageID);
    LogUnrecoverable(errorMessage);

    if (messageBuffer)
    {
        std::cout << "Error message: " << messageBuffer << std::endl;
        LocalFree(messageBuffer);
    }
}

BOOL InjectDLL(DWORD processId, const char* dllPath)
{
    // Open the target process
    HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, processId);
    if (hProcess == NULL)
    {
        PrintErrorMessage("open the target process");
        return FALSE;
    }

    // Allocate memory for the DLL path in the target process
    LPVOID dllPathAddress = VirtualAllocEx(hProcess, NULL, strlen(dllPath) + 1, MEM_COMMIT, PAGE_READWRITE);
    if (dllPathAddress == NULL)
    {
        PrintErrorMessage("allocate memory in the target process");
        CloseHandle(hProcess);
        return FALSE;
    }

    // Write the DLL path into the allocated memory
    if (!WriteProcessMemory(hProcess, dllPathAddress, dllPath, strlen(dllPath) + 1, NULL))
    {
        PrintErrorMessage("write DLL path into the target process");
        VirtualFreeEx(hProcess, dllPathAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    // Get the address of the LoadLibraryA function in the target process
    HMODULE kernel32Module = GetModuleHandle(L"kernel32.dll");
    if (kernel32Module == NULL)
    {
        PrintErrorMessage("retrieve handle to kernel32.dll");
        VirtualFreeEx(hProcess, dllPathAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    FARPROC loadLibraryAddress = GetProcAddress(kernel32Module, "LoadLibraryA");
    if (loadLibraryAddress == NULL)
    {
        PrintErrorMessage("retrieve address of LoadLibraryA function");
        VirtualFreeEx(hProcess, dllPathAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    // Create a remote thread in the target process to call LoadLibraryA with the DLL path
    HANDLE hThread = CreateRemoteThread(hProcess, NULL, 0, (LPTHREAD_START_ROUTINE)loadLibraryAddress, dllPathAddress, 0, NULL);
    if (hThread == NULL)
    {
        PrintErrorMessage("create remote thread in the target process");
        VirtualFreeEx(hProcess, dllPathAddress, 0, MEM_RELEASE);
        CloseHandle(hProcess);
        return FALSE;
    }

    std::cout << "DLL injected successfully!" << std::endl;

    // Cleanup resources
    WaitForSingleObject(hThread, INFINITE);
    VirtualFreeEx(hProcess, dllPathAddress, 0, MEM_RELEASE);
    CloseHandle(hThread);
    CloseHandle(hProcess);

    return TRUE;
}