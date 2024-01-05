#ifndef WIN32_HASH_TABLE_HPP
#define WIN32_HASH_TABLE_HPP

#include "win32_expandable_list.hpp"

struct key_value_pair
{
    unsigned int value;
    char key[256] = {};
};

key_value_pair* GetValueFromKeyLinear(win32_expandable_list<key_value_pair> hashTable, char* key)
{    
    for (int i = 0; i < hashTable.numElements; ++i) {
        key_value_pair kvp = hashTable.data[i];
        if (strcmp(key, kvp.key) == 0) {
            return &(hashTable.data[i]);
        }
    }
    return(NULL);
}

#endif