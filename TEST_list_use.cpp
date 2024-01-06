#pragma warning(push, 0)
#include "win32_expandable_list.hpp"
#include "win32_hash_table.hpp"

#include "file_navigation.h"

int main(void)
{
    const char *path = "models";

    hash_table ht;
    
    win32_expandable_list<path_string> pathList;
    build_path_list(path, ".obj", &pathList); 
    pathList.Add({"models/Sponza-master/sponza.obj"});

    for (int i = 0; i < pathList.numElements; ++i) {
        // OutputDebugStringA(pathList.data[i].path);
        // OutputDebugStringA("\n");
        char buf[MAX_PATH];
        get_parent_folder(buf, pathList.data[i].path);
        OutputDebugStringA(buf);
        OutputDebugStringA("\n");
    }    
}
#pragma warning(pop)