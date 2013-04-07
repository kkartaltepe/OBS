/********************************************************************************
 Copyright (C) 2012 Hugh Bailey <obs.jim@gmail.com>

 This program is free software; you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation; either version 2 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program; if not, write to the Free Software
 Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307, USA.
********************************************************************************/


#include "GraphicsCaptureHook.h"


HANDLE hSignalRestart=NULL, hSignalEnd=NULL;
HANDLE hSignalReady=NULL, hSignalExit=NULL;

HINSTANCE hinstMain = NULL;
HWND hwndSender = NULL, hwndOBS = NULL;
HANDLE textureMutexes[2] = {NULL, NULL};
bool bStopRequested = false;
bool bCapturing = true;
bool bTargetAcquired = false;

HANDLE hFileMap = NULL;
LPBYTE lpSharedMemory = NULL;
UINT sharedMemoryIDCounter = 0;

HANDLE hInfoFileMap = NULL;
CaptureInfo *infoMem = NULL;


LARGE_INTEGER clockFreq, startTime;
LONGLONG prevElapsedTime;
DWORD startTick;

wstring strKeepAlive;
LONGLONG keepAliveTime = 0;


void WINAPI OSInitializeTimer()
{
    QueryPerformanceFrequency(&clockFreq);
    QueryPerformanceCounter(&startTime);
    startTick = GetTickCount();
    prevElapsedTime = 0;
    keepAliveTime = 0;
}

LONGLONG WINAPI OSGetTimeMicroseconds()
{
    LARGE_INTEGER currentTime;
    QueryPerformanceCounter(&currentTime);
    LONGLONG elapsedTime = currentTime.QuadPart - 
        startTime.QuadPart;

    // Compute the number of millisecond ticks elapsed.
    unsigned long msecTicks = (unsigned long)(1000 * elapsedTime / 
        clockFreq.QuadPart);

    // Check for unexpected leaps in the Win32 performance counter.  
    // (This is caused by unexpected data across the PCI to ISA 
    // bridge, aka south bridge.  See Microsoft KB274323.)
    unsigned long elapsedTicks = GetTickCount() - startTick;
    signed long msecOff = (signed long)(msecTicks - elapsedTicks);
    if (msecOff < -100 || msecOff > 100)
    {
        // Adjust the starting time forwards.
        LONGLONG msecAdjustment = min(msecOff * 
            clockFreq.QuadPart / 1000, elapsedTime - 
            prevElapsedTime);
        startTime.QuadPart += msecAdjustment;
        elapsedTime -= msecAdjustment;
    }

    // Store the current elapsed time for adjustments next time.
    prevElapsedTime = elapsedTime;

    // Convert to microseconds.
    LONGLONG usecTicks = (1000000 * elapsedTime / 
        clockFreq.QuadPart);

    return usecTicks;
}

HANDLE WINAPI OSCreateMutex()
{
    CRITICAL_SECTION *pSection = (CRITICAL_SECTION*)malloc(sizeof(CRITICAL_SECTION));
    InitializeCriticalSection(pSection);

    return (HANDLE)pSection;
}

void   WINAPI OSEnterMutex(HANDLE hMutex)
{
    if(hMutex)
        EnterCriticalSection((CRITICAL_SECTION*)hMutex);
}

BOOL   WINAPI OSTryEnterMutex(HANDLE hMutex)
{
    if(hMutex)
        return TryEnterCriticalSection((CRITICAL_SECTION*)hMutex);
    return FALSE;
}

void   WINAPI OSLeaveMutex(HANDLE hMutex)
{
    if(hMutex)
        LeaveCriticalSection((CRITICAL_SECTION*)hMutex);
}

void   WINAPI OSCloseMutex(HANDLE hMutex)
{
    if(hMutex)
    {
        DeleteCriticalSection((CRITICAL_SECTION*)hMutex);
        free(hMutex);
    }
}


void QuickLog(LPCSTR lpText)
{
    HANDLE hFile = CreateFile(TEXT("d:\\log.txt"), GENERIC_WRITE, 0, NULL, OPEN_ALWAYS, 0, NULL);
    DWORD bla;
    SetFilePointer(hFile, 0, 0, FILE_END);
    WriteFile(hFile, lpText, (DWORD)strlen(lpText), &bla, NULL);
    FlushFileBuffers(hFile);
    CloseHandle(hFile);
}


UINT InitializeSharedMemoryCPUCapture(UINT textureSize, DWORD *totalSize, MemoryCopyData **copyData, LPBYTE *textureBuffers)
{
    UINT alignedHeaderSize = (sizeof(MemoryCopyData)+15) & 0xFFFFFFF0;
    UINT alignedTexureSize = (textureSize+15) & 0xFFFFFFF0;

    *totalSize = alignedHeaderSize + alignedTexureSize*2;

    wstringstream strName;
    strName << TEXTURE_MEMORY << ++sharedMemoryIDCounter;
    hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, *totalSize, strName.str().c_str());
    if(!hFileMap)
        return 0;

    lpSharedMemory = (LPBYTE)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, *totalSize);
    if(!lpSharedMemory)
    {
        CloseHandle(hFileMap);
        hFileMap = NULL;
        return 0;
    }

    *copyData = (MemoryCopyData*)lpSharedMemory;
    (*copyData)->texture1Offset = alignedHeaderSize;
    (*copyData)->texture2Offset = alignedHeaderSize+alignedTexureSize;
    (*copyData)->frameTime = 0;

    textureBuffers[0] = lpSharedMemory+alignedHeaderSize;
    textureBuffers[1] = lpSharedMemory+alignedHeaderSize+alignedTexureSize;

    return sharedMemoryIDCounter;
}

UINT InitializeSharedMemoryGPUCapture(SharedTexData **texData)
{
    int totalSize = sizeof(SharedTexData);

    wstringstream strName;
    strName << TEXTURE_MEMORY << ++sharedMemoryIDCounter;
    hFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, totalSize, strName.str().c_str());
    if(!hFileMap)
        return 0;

    lpSharedMemory = (LPBYTE)MapViewOfFile(hFileMap, FILE_MAP_ALL_ACCESS, 0, 0, totalSize);
    if(!lpSharedMemory)
    {
        CloseHandle(hFileMap);
        hFileMap = NULL;
        return 0;
    }

    *texData = (SharedTexData*)lpSharedMemory;
    (*texData)->frameTime = 0;

    return sharedMemoryIDCounter;
}

void DestroySharedMemory()
{
    if(lpSharedMemory && hFileMap)
    {
        UnmapViewOfFile(lpSharedMemory);
        CloseHandle(hFileMap);

        hFileMap = NULL;
        lpSharedMemory = NULL;
    }
}


bool bD3D9Hooked = false;
bool bDXGIHooked = false;
bool bGLHooked = false;
bool bDirectDrawHooked = false;

inline bool AttemptToHookSomething()
{
    bool bFoundSomethingToHook = false;
    if(!bD3D9Hooked && InitD3D9Capture())
    {
        logOutput << "D3D9 Present" << endl;
        bFoundSomethingToHook = true;
        bD3D9Hooked = true;
    }
    if(!bDXGIHooked && InitDXGICapture())
    {
        logOutput << "DXGI Present" << endl;
        bFoundSomethingToHook = true;
        bDXGIHooked = true;
    }
    if(!bGLHooked && InitGLCapture())
    {
        logOutput << "GL Present" << endl;
        bFoundSomethingToHook = true;
        bGLHooked = true;
    }
    /*
    if(!bDirectDrawHooked && InitDDrawCapture())
    {
        OutputDebugString(TEXT("DirectDraw Present\r\n"));
        bFoundSomethingToHook = true;
        bDirectDrawfHooked = true;
    }*/

    return bFoundSomethingToHook;
}

fstream logOutput;

#define SENDER_WINDOWCLASS TEXT("OBSGraphicsCaptureSender")

DWORD WINAPI CaptureThread(HANDLE hDllMainThread)
{
    bool bSuccess = false;

    //wait for dll initialization to finish before executing any initialization code
    if(hDllMainThread)
    {
        WaitForSingleObject(hDllMainThread, INFINITE);
        CloseHandle(hDllMainThread);
    }

    TCHAR lpLogPath[MAX_PATH];
    SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, lpLogPath);
    wcscat_s(lpLogPath, MAX_PATH, TEXT("\\OBS\\pluginData\\captureHookLog.txt"));

    if(!logOutput.is_open())
        logOutput.open(lpLogPath, ios_base::in | ios_base::out | ios_base::trunc);

    wstringstream str;
    str << OBS_KEEPALIVE_EVENT << int(GetCurrentProcessId());
    strKeepAlive = str.str();

    WNDCLASS wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.hInstance = hinstMain;
    wc.lpszClassName = SENDER_WINDOWCLASS;
    wc.lpfnWndProc = (WNDPROC)DefWindowProc;

    DWORD procID = GetCurrentProcessId();

    wstringstream strRestartEvent, strEndEvent, strReadyEvent, strExitEvent, strInfoMemory;
    strRestartEvent << RESTART_CAPTURE_EVENT << procID;
    strEndEvent     << END_CAPTURE_EVENT     << procID;
    strReadyEvent   << CAPTURE_READY_EVENT   << procID;
    strExitEvent    << APP_EXIT_EVENT        << procID;
    strInfoMemory   << INFO_MEMORY           << procID;

    hSignalRestart  = GetEvent(strRestartEvent.str().c_str());
    hSignalEnd      = GetEvent(strEndEvent.str().c_str());
    hSignalReady    = GetEvent(strReadyEvent.str().c_str());
    hSignalExit     = GetEvent(strExitEvent.str().c_str());

    hInfoFileMap = CreateFileMapping(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(CaptureInfo), strInfoMemory.str().c_str());
    if(!hInfoFileMap)
    {
        logOutput << "CaptureThread: could not info file mapping" << endl;
        return 0;
    }

    infoMem = (CaptureInfo*)MapViewOfFile(hInfoFileMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(CaptureInfo));
    if(!infoMem)
    {
        logOutput << "CaptureThread: could not map view of info shared memory" << endl;
        CloseHandle(hInfoFileMap);
        hInfoFileMap = NULL;
        return 0;
    }

    hwndOBS = FindWindow(OBS_WINDOW_CLASS, NULL);
    if(!hwndOBS)
    {
        logOutput << "CaptureThread: could not find main application window?  wtf?  seriously?" << endl;
        return 0;
    }

    if(RegisterClass(&wc))
    {
        hwndSender = CreateWindow(SENDER_WINDOWCLASS, NULL, 0, 0, 0, 0, 0, NULL, 0, hinstMain, 0);
        if(hwndSender)
        {
            textureMutexes[0] = OpenMutex(MUTEX_ALL_ACCESS, FALSE, TEXTURE_MUTEX1);
            if(textureMutexes[0])
            {
                textureMutexes[1] = OpenMutex(MUTEX_ALL_ACCESS, FALSE, TEXTURE_MUTEX2);
                if(textureMutexes[1])
                {
                    while(!AttemptToHookSomething())
                        Sleep(50);

                    MSG msg;
                    while(GetMessage(&msg, NULL, 0, 0))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);

                        AttemptToHookSomething();
                    }

                    CloseHandle(textureMutexes[1]);
                    textureMutexes[1] = NULL;
                }

                CloseHandle(textureMutexes[0]);
                textureMutexes[0] = NULL;
            }

            DestroyWindow(hwndSender);
        }
    }

    return 0;
}


BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD dwReason, LPVOID lpBlah)
{
    if(dwReason == DLL_PROCESS_ATTACH)
    {
        HANDLE hThread = NULL;
        hinstMain = hinstDLL;

        HANDLE hDllMainThread = OpenThread(THREAD_ALL_ACCESS, NULL, GetCurrentThreadId());

        if(!(hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)CaptureThread, (LPVOID)hDllMainThread, 0, 0)))
        {
            CloseHandle(hDllMainThread);
            return FALSE;
        }

        CloseHandle(hThread);
    }
    else if(dwReason == DLL_PROCESS_DETACH)
    {
        /*FreeGLCapture();
        FreeD3D9Capture();
        FreeD3D10Capture();
        FreeD3D101Capture();
        FreeD3D11Capture();*/

        if(hSignalRestart)
            CloseHandle(hSignalRestart);
        if(hSignalEnd)
            CloseHandle(hSignalEnd);
        if(hSignalReady)
            CloseHandle(hSignalReady);
        if(hSignalExit)
        {
            SetEvent(hSignalExit);
            CloseHandle(hSignalExit);
        }

        if(infoMem)
        {
            UnmapViewOfFile(lpSharedMemory);
            CloseHandle(hInfoFileMap);
        }

        hFileMap = NULL;
        lpSharedMemory = NULL;

        if(hwndSender)
            DestroyWindow(hwndSender);

        for(UINT i=0; i<2; i++)
        {
            if(textureMutexes[i])
                CloseHandle(textureMutexes[i]);
        }

        if(logOutput.is_open())
            logOutput.close();
    }

    return TRUE;
}
