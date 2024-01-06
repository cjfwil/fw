#ifndef FILE_NAVIGATION_H
#define FILE_NAVIGATION_H

#include "windows.h"
#include "win32_expandable_list.hpp"

char* get_extension(char* path)
{
    int index = 0;
    for (int i = 0; path[i] != '.'; ++i) {
        index = i;
    }
    return(&path[index+1]);
}

void get_parent_folder(char* output, const char* path)
{
    int size = 0;
    for (int i = 0; path[i] != '\0'; ++i) {
        size = i;
    }
    strcpy(output, path);
    for (int i = size-1; (path[i] != '\\') == (path[i] != '/'); --i) {
        output[i] = '\0';
    }
}

struct path_string {
    char path[MAX_PATH];
};

void build_path_list(const char *path, const char* fileExtension, win32_expandable_list<path_string>* pathList)
{    
    char searchPath[MAX_PATH];
    WIN32_FIND_DATA FindFileData;
    HANDLE hFind;

    wsprintfA(searchPath, "%s\\*", path);

    hFind = FindFirstFile(searchPath, &FindFileData);
    if (hFind != INVALID_HANDLE_VALUE)
    {
        do
        {
            if (strcmp(FindFileData.cFileName, ".") != 0 && strcmp(FindFileData.cFileName, "..") != 0)
            {                
                char fullPath[MAX_PATH];
                wsprintfA(fullPath, "%s\\%s", path, FindFileData.cFileName);
             
                if (FindFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
                {
                    build_path_list(fullPath, fileExtension, pathList);
                }
                else
                {
                    char* ext = get_extension(fullPath);
                    if (strcmp(ext, fileExtension) == 0)
                    {                                          
                        path_string p = {};
                        strcpy(p.path, fullPath);
                        pathList->Add(p);
                    }
                }
            }
        } while (FindNextFile(hFind, &FindFileData));

        FindClose(hFind);
    }    
}

#endif