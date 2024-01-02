#include "expandable_list.hpp"

int main(void)
{
    expandable_list<int> el;
    el.Init();
    el.Add(0xffffffff);
    


    el.Expand(1);    
    
    for (int i = 0; i < el.numElements; ++i) {
        el.data[i] = 1;        
    }
}