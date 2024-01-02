#pragma warning(push, 0)
#include "win32_expandable_list.hpp"

int main(void)
{
    win32_expandable_list<int> el;
    el.Init();
    el.Add(0xffffffff);
    


    el.Expand(1);    
    
    for (int i = 0; i < el.numElements; ++i) {
        el.data[i] = 1;        
    }
}
#pragma warning(pop)