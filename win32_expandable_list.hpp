#ifndef EXPANDABLE_LIST_HPP
#define EXPANDABLE_LIST_HPP

#pragma comment(lib, "user32.lib")
#pragma warning(push, 0)
#pragma warning(disable : 4365)
#pragma warning(disable : 4668)
#include <Windows.h>
#pragma warning(pop)

template <typename T>
struct win32_expandable_list
{
    // BOOL initialised = FALSE;
private:
    DWORD pageSize;
    SIZE_T currentSize;
    BOOL isInitialised = FALSE;

public:
    SIZE_T numElements;
    SIZE_T size;
    SIZE_T availableElements;
    T *data;

    SIZE_T CalcAvailableElements()
    {
        availableElements = static_cast<SIZE_T>(currentSize / sizeof(T));
        return (availableElements);
    }

    // T Get(SIZE_T index)
    // {
    //     T result = NULL;
    //     if (index < numElements) {
    //         result = data[index];
    //     }
    //     return(result);
    // }

    // TODO: perhaps create an alloc function which is called in both init and expand
    // todo allow for intial reserve size to be specified
    void Init()
    {
        if (!isInitialised)
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
                size = 0;
                isInitialised = TRUE;
            }
            else
            {
                DWORD err = GetLastError();
                char buff[1024] = {};
                wsprintfA(buff, "WIN32 ERROR: %d", err);
                OutputDebugStringA(buff);
            }
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
            wsprintfA(buff, "WIN32 ERROR: %d", err);
            OutputDebugStringA(buff);
        }
    }

    void Add(T element)
    {
        if (!isInitialised)
            Init();
        if (sizeof(T) > currentSize)
        {
            Expand(sizeof(T) / pageSize);
        }
        if (numElements >= availableElements)
        {
            Expand(1);
        }
        data[numElements] = element;
        numElements++;
        size += sizeof(T);
    }

    void Free()
    {
        BOOL result = VirtualFree(data, currentSize, MEM_RELEASE);
        if (!result)
        {
            DWORD err = GetLastError();
            char buff[1024] = {};
            wsprintfA(buff, "WIN32 ERROR: %d", err);
            OutputDebugStringA(buff);
        }
    }    
};

#endif