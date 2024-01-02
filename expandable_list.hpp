#ifndef EXPANDABLE_LIST_H
#define EXPANDABLE_LIST_H

#pragma comment(lib, "user32.lib")
#include "Windows.h"

u_int u_ceil(float x)
{
    u_int iPart = static_cast<u_int>(x);
    return (iPart + 1);
}

template <typename T>
struct expandable_list
{
    // BOOL initialised = FALSE;
    DWORD pageSize;
    SIZE_T currentSize;
    SIZE_T numElements;
    SIZE_T availableElements;
    T *data;

    SIZE_T CalcAvailableElements()
    {
        availableElements = static_cast<SIZE_T>(currentSize / sizeof(T));
        return (availableElements);
    }

    void Init()
    {
        SYSTEM_INFO sysInfo;
        GetSystemInfo(&sysInfo);
        pageSize = sysInfo.dwPageSize;

        data = (T *)VirtualAlloc(NULL, pageSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (data)
        {
            currentSize = pageSize;
            CalcAvailableElements();
            numElements = 0;
        }
        else
        {
            // TODO: error;
        }
    }

    // pages: list is expanded by pageSize*pages bytes:
    void Expand(SIZE_T pages)
    {
        SIZE_T newSize = currentSize + pageSize * pages;
        LPVOID result = VirtualAlloc(NULL, newSize, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
        if (result)
        {
            memcpy(result, data, currentSize);
            this->Free();
            data = (T *)result;
            currentSize = newSize;
            CalcAvailableElements();        
        }
        else
        {
            DWORD err = GetLastError();
            char buff[1024] = {};
            wsprintfA(buff, "%d", err);
            OutputDebugStringA(buff);
        }
    }

    void Add(T element)
    {
        if (numElements > availableElements)
        {
            Expand(1);
        }
        data[numElements] = element;
        numElements++;
    }

    void Free()
    {
        BOOL result = VirtualFree(data, currentSize, MEM_RELEASE);
        if (!result)
        {
            DWORD err = GetLastError();
            char buff[1024] = {};
            wsprintfA(buff, "%d", err);
            OutputDebugStringA(buff);
        }
    }
};

#endif