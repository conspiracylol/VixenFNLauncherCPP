#pragma once
#include <Windows.h>
#include <iostream>
#include <tlhelp32.h>
#ifndef SuspendProcess
void SuspendProcess(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, processId);
    if (hProcess != NULL) {
        SuspendThread(hProcess);
        CloseHandle(hProcess);
    }
}
#endif
#ifndef ResumeProcess
void ResumeProcess(DWORD processId) {
    HANDLE hProcess = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, processId);
    if (hProcess != NULL) {
        ResumeThread(hProcess);
        CloseHandle(hProcess);
    }
}
#endif


DWORD GetProcessIdByName(const char* processName)
{
    DWORD processId = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

    if (snapshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 processEntry;
        processEntry.dwSize = sizeof(PROCESSENTRY32);

        if (Process32First(snapshot, &processEntry))
        {
            do
            {
                char szExeFile[MAX_PATH];
                size_t convertedChars = 0;
                wcstombs_s(&convertedChars, szExeFile, MAX_PATH, processEntry.szExeFile, MAX_PATH);

                if (strcmp(szExeFile, processName) == 0)
                {
                    processId = processEntry.th32ProcessID;
                    break;
                }
            } while (Process32Next(snapshot, &processEntry));
        }

        CloseHandle(snapshot);
    }

    return processId;
}