// ASCII only has 1-byte and starts with 0
// Mandarin words have 3-bytes, looks like below
// 1110xxxx 10xxxxxx 10xxxxxx, first byte starts with 1110, and the following byte starts with 10
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#define MaxLine 1024
#define TableSizeA (1<<8)
#define MaxName 1024
#define uchar unsigned char

int TermCnt = 0;    // Calculate how many words we've got

struct hnode{
    char key[MaxName];
    int cnt;
    struct hnode *next;
};

// Hash Function, string polynomial
// "apple" -> 'a' * r^4 + 'p' * r^3 + 'p' * r^2 + 'l' * r^1 + 'e' * r^0
// ((((a*r + p) * r + p) * r + l) * r + e)
unsigned int hash33(unsigned char *key){     
    unsigned int hv;
    uchar *ptr;
    hv = 13;    // this is the "r" of string polynomial, prime numbers is more approriate
    ptr = key;
    while(*ptr){
        hv = (hv<<5) + (hv + *ptr);     //shift left 5 bits and add itself, that means multiply 33
        ptr++;
        //hv = hv * 33 + *ptr;         //This will be slower since computers do shifts alot faster than multiply
    }
    return hv;
}

// Initialze the hash tables
void HashInit(struct hnode **hashTab, unsigned int HSize){
    int i;
    for(i=0; i<HSize; i++){
        hashTab[i] = NULL;
    }
}

// return hnode if key exists in hash table, NULL if not exists in hash table
struct hnode *HashFind(struct hnode *hashTab[], unsigned int HSize, uchar *key){
    unsigned int hv;
    struct hnode *p;
    hv = hash33(key) % HSize;   // Get the hash value
    p = hashTab[hv];            // Get the head of linked list since it might have collisions, we have to traverse the linked list
    while(p != NULL){
        if(strcmp(p->key, key) == 0){
            return p;           // Found key, return the node
        }
        p = p->next;
    }
    return NULL;                // We get here if key doesn't exists in hash table
}

void HashInsert(struct hnode *hashTab[], unsigned int HSize, uchar *key){
    struct hnode *p;
    unsigned int hv;
    p = HashFind(hashTab, HSize, key);  // To see if this key already exists
    if(p){
        //key already exists, add 1 to count and return
        p->cnt++;
        return;
    }

    // Before we insert, we have to first get the hash value
    hv = hash33(key) % HSize;   // Get the hash value
    p = (struct hnode *)malloc(sizeof(struct hnode));
    if(p == NULL){
        // If p is null that means malloc error
        fprintf(stderr, "malloc failed at HashInsert() !\n");
    }
    TermCnt++;
    strcpy(p->key, key);
    p->cnt = 1;
    p->next = hashTab[hv];  // Insert At Front
    hashTab[hv] = p;        // Insert at front

}

// Traverse through hash table and put all words in array
void HTraverse(struct hnode *hashTab[], unsigned int HSize, struct hnode array[]){
    int i, cnt=0;
    struct hnode *p;
    for(i=0; i<HSize; i++){
        p = hashTab[i];     // get the head of linked list, traverse that list
        while(p != NULL){
            array[cnt] = *p;
            cnt++;
            p = p->next;
        }
    }
}

void rmnewline(uchar *line){
    uchar *ptr = line;
    while(*ptr && *ptr != '\n')
        ptr++;
    *ptr = '\0';
}

uchar *utf8getchar(uchar *text, uchar *Ch){
    int len;
    uchar *ptr, *qtr;
    if(*text == '\0')
        return NULL;
    if(*text >= 224){   //  11100000 -> 128+64+32 = 224
        // Enter this block if current pointer is greater than 11100000, that means we have a Mandarin word
        len = 3;
    }
    else if(*text >= 192){  //  11000000 -> 128+64 = 192
        len = 2;
    }
    else if(*text >= 128){
        Ch[0] = '\0';       //  if current pointer starts with 10, that means there are some problems, we just skip it
        return text+1;
    }
    else
        len = 1;

    // Put the words in Ch variable
    ptr = text;
    qtr = Ch;
    while(len){
        *qtr++ = *ptr++;
        len--;
    }
    *qtr = '\0';
    return ptr;
}

// Sort words with counts
int mycompare(const void *p, const void *q){
    struct hnode *x = (struct hnode *)p;
    struct hnode *y = (struct hnode *)q;
    return y->cnt - x->cnt;
}

// If there is a tie, sort with memcmp
int mycomparekey(const void *p, const void *q){
    struct hnode *x = (struct hnode *)p;
    struct hnode *y = (struct hnode *)q;
    if(x->cnt != y->cnt)
        return 0;
    return memcmp(x->key, y->key, 3);
}

int main(int argc, char *argv[]){
    
    uchar line[MaxLine];
    uchar utf8Char[4];
    uchar *ptr, *qtr;
    struct lnode *node, *head, *tmp;
    struct hnode *hashTab[TableSizeA + 13];
    unsigned int HSize = TableSizeA + 13;     //Hash Table Size
    HashInit(hashTab, HSize);

    while(fgets(line, MaxLine, stdin) != NULL){
        rmnewline(line);    // remove newline
        ptr = line;         // start parsing this line
        while(ptr){
            ptr = utf8getchar(ptr, utf8Char);   // this function will return the new position of ptr, and put the Mandarin word in utf8char
            if(strlen(utf8Char) < 3)
                // if length is less than 3, we don't care since we only care about Mandarin words
                continue;
            else if(ptr == NULL)
                break;
            // Insert this word to hash table
            HashInsert(hashTab, HSize, utf8Char);
        }
    }
    struct hnode array[TermCnt];
    // Put all Mandarin words in hash table into array
    HTraverse(hashTab, HSize, array);
    // Sort with occurrence
    qsort(array, TermCnt, sizeof(struct hnode), mycompare);

    // There might be ties, so we need to sort it again with memcmp()
    int num, num1, i;
    num = num1 = i = 0;
    num = array[i].cnt;
    while(1){
        while(num == array[i].cnt){
            i++;
            if(i == TermCnt){
                break;
            }
        }
        qsort(&array[num1], i-num1, sizeof(struct hnode), mycomparekey);
        num1 = i;
        i++;
        num = array[i].cnt;
        if(i >= TermCnt)
            break;
    }
    // Print out the results
    i = 0;
    while(i<TermCnt){
        printf("@count:%d %s\n", array[i].cnt, array[i].key);
        i++;
    }

    return 0;
}
