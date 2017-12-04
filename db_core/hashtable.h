#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <openssl/sha.h>
#include <stdint.h>
#include <immintrin.h>
#include "hashlist.h"

#define SHA_DIGEST_LENGTH 64

typedef struct hashtable {
    int size;
    list **table;
} hashtable;

int hashindex(hashtable *table, char *key, unsigned char *output)
{
    uint64_t *data64;
    uint64_t total4;
    total4 = 0;
    data64 = (uint64_t *) key;
    total4 = _mm_crc32_u64 (total4, *data64++);
//    printf ("CRC32 result using 64-bit chunks: %08X\n", total4);

 // takes a key, gives you the index for it
   /* size_t len = sizeof(key);
    SHA1(key, len, output);

    int i = 1;
    int arraykey = 1;
    int j = 0;

    while (i < table->size) {
        arraykey *= output[j++];
        i *= 256;
    }
    return arraykey % table->size;*/
    return total4 % table->size;
}

char *hashlookup(hashtable *hashtab, char *key)
{ // find the value for key in hashtab
    unsigned char keyhash[SHA_DIGEST_LENGTH];
    int index;
    index = hashindex(hashtab, key, keyhash);
    list *templist = hashtab->table[index];
    if (empty(templist))
        return "not found";
    else {
        node *tempnode = listkeysearch(templist, key);
        return tempnode->value;
    }
}

void inserthash(hashtable *hashtab, char *key, char *value)
{ // insert key,value pair into hashtab
    unsigned char keyhash[SHA_DIGEST_LENGTH];
    int index = hashindex(hashtab, key, keyhash);
    list *temp = hashtab->table[index];
    listinsert(temp, nodegen(key, value, keyhash));
}

hashtable *hashinit(int size)
{ // allocates hashtable array, returns pointer to array
    list **hasharray;
    hasharray = (list **)malloc(sizeof (list) * size);
    hashtable *hashtab;
    hashtab = (hashtable *)malloc(sizeof (hashtable));
/*
    int i;
    for (i = 0; i < size; i++) {
        hasharray[i] = listinit();
    }
*/
    hashtab->table = hasharray;
    hashtab->size = size;
    return hashtab;
}

void destroyhash(hashtable *oldtable)
{ // destroy!!
    int i;
    for (i = 0; i < oldtable->size; i++) {
        destroylist(oldtable->table[i]);
    }
    free(oldtable);
}

void printhashtab(hashtable *toprint)
{
    int i;
    list *templist;
    printf("keys:\tvalues:\n");
    for (i = 0; i < toprint->size; i++) {
        templist = toprint->table[i];
        if (! (empty(templist)))
            printlist(templist);
    }
}

void hashdel(hashtable *hashtab, char *toremove)
{ // delete the key/value pair from the hashtable
    unsigned char keyhash[SHA_DIGEST_LENGTH];
    int index = hashindex(hashtab, toremove, keyhash);
    list *templist = hashtab->table[index];
    if (! (empty(templist))) {
        node *delnode = listkeysearch(templist, toremove);
        listremove(templist, delnode);
    }
}



void hash(unsigned char *key, unsigned char *output)
{ // for testing
    size_t len = sizeof(key);
    SHA1(key, len, output);
}
