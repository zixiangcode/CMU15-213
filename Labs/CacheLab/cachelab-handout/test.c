/*
 *   Author:zixiang
 *   Email :zixiangcode@gmail.com
 */

#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>


#include "cachelab.h"

typedef struct Cache_line {
    int is_valid;  // 有效位
    int tag;       // 标记位
    struct Cache_line* prev;
    struct Cache_line* next;
} Cache_line;

typedef struct Hash {
    Cache_line* value;  // 数据的未使用时长
    struct Hash* next;  // 用拉链法解决哈希冲突
} Hash;

typedef struct {
    int size;          // 当前缓存大小
    int capacity;      // 缓存容量
    Hash* table;       // 本组对应的哈希表
    Cache_line* head;  // 指向最近使用的数据的前一个
    Cache_line* tail;  // 指向最久未使用的数据的后一个
} LRUCache;

typedef struct {
    int S;
    int E;
    int B;
    LRUCache* Set;
} Cache;

char filename[FILENAME_MAX];
Cache* cache = NULL;
int hits = 0, misses = 0, evictions = 0;

// 哈希函数，键值映射到哈希索引
Hash* HashMap(Hash* table, int key, int capacity) {
    return &table[key % capacity];
}

// 头插法插入到双向链表中
void HeadInsert(Cache_line* head, Cache_line* cur) {
    if (cur->prev == NULL && cur->next == NULL) {  // cur是一个新节点不在链表中
        cur->prev = head;
        cur->next = head->next;
        head->next->prev = cur;
        head->next = cur;
    } else {  // 如果cur本身就在链表中
        Cache_line* first = head->next;
        if (cur != first) {  // 如果不是链表的头结点
            // 改变前后结点的指向，让它们互连
            cur->prev->next = cur->next;
            cur->next->prev = cur->prev;
            cur->next = first;  // 插入到头结点
            cur->prev = head;
            head->next = cur;
            first->prev = cur;
        }
    }
}

// 写操作
void LRUCachePut(LRUCache* set, int tag) {
    misses += 1;
    // 得到这个组对应的哈希表中 tag 对应的 Cache_line 的地址
    Hash* addr = HashMap(set->table, tag, set->capacity);
    if (set->size >= set->capacity) {  // 本组容量已满，链表向后扩容一个
        // 替换最后一个结点，也就是最久未使用的结点，不过并不是真的替换
        Cache_line* last = set->tail->prev;
        // 最后的结点在哈希表的映射
        Hash* remove = HashMap(set->table, last->tag, set->capacity);
        Hash* ptr = remove;
        remove = remove->next;  // 跳过哈希表中的头结点
        // 因为是拉链法解决冲突，所以确保哈希表删除的确实是最后的结点
        while (remove->value->tag != last->tag) {
            ptr = remove;
            remove = remove->next;
        }
        // 在哈希表中删除这个点，此时 ptr 是 remove 的前一个结点
        ptr->next = remove->next;
        // 解除这个点对 Cache_line 的映射
        remove->next = NULL;
        remove->value = NULL;
        free(remove);
        Hash* new_node = (Hash*)malloc(sizeof(Hash));
        new_node->next = addr->next;
        addr->next = new_node;
        new_node->value = last;
        last->tag = tag;
        HeadInsert(set->head, last);
        evictions += 1;
    } else {  // 本组容量未满，仍可插入
        // 创建新结点，并建立哈希映射
        Hash* new_node = (Hash*)malloc(sizeof(Hash));
        new_node->value = (Cache_line*)malloc(sizeof(Cache_line));
        new_node->next = addr->next;  // 映射到哈希表中
        addr->next = new_node;
        new_node->value->prev = NULL;  // 该结点是新建的，还不在链表中
        new_node->value->next = NULL;
        new_node->value->tag = tag;
        HeadInsert(set->head, new_node->value);
        set->size += 1;  // 更新本组容量大小
    }
}

// 更新操作，与读操作写成一个函数了
void Update(int tag, int s_idx) {
    // 根据组编号找到对应的组
    LRUCache* set = &cache->Set[s_idx];
    // 得到这个组对应的哈希表中 tag 对应的 Cache_line 的地址
    Hash* addr = HashMap(set->table, tag, set->capacity);
    addr = addr->next;  // 跳过头结点
    if (addr == NULL) {
        LRUCachePut(set, tag);  // 缓存中没有，就写入缓存
        return;
    }
    // 匹配 tag
    while (addr->next != NULL && addr->value->tag != tag) {
        addr = addr->next;
    }
    if (addr->value->tag == tag) {
        hits += 1;
        HeadInsert(set->head, addr->value);
        return;
    }
    LRUCachePut(set, tag);
}

// 初始化每个组
LRUCache* InitCacheSet(int capacity) {
    LRUCache* obj = (LRUCache*)malloc(sizeof(LRUCache));
    // 初始化这个组的缓存大小和总容量
    obj->capacity = capacity;
    obj->size = 0;
    // malloc空间
    obj->table = (Hash*)malloc(capacity * sizeof(Hash));
    memset(obj->table, 0, capacity * sizeof(Hash));  // 清空哈希表
    obj->head = (Cache_line*)malloc(sizeof(Cache_line));
    obj->tail = (Cache_line*)malloc(sizeof(Cache_line));
    // 初始化头尾结点
    obj->head->prev = NULL;
    obj->head->next = obj->tail;
    obj->tail->prev = obj->head;
    obj->tail->next = NULL;

    return obj;
}

// 初始化整个缓存
void InitCache(int s, int E, int b) {
    int S = 1 << s;
    int B = 1 << b;

    cache = (Cache*)malloc(sizeof(Cache));
    cache->S = S;
    cache->E = E;
    cache->B = B;
    cache->Set = (LRUCache*)malloc(S * sizeof(LRUCache));  // 有 S 个组

    for (int i = 0; i < S; i++) {
        cache->Set[i] = *InitCacheSet(E);  // 每组的缓存大小为 E
    }
}

void GetTrace(int s, int E, int b) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        exit(EXIT_FAILURE);
    }
    char operation;
    unsigned int address;
    int size;

    while (fscanf(file, " %s %x,%d", &operation, &address, &size) > 0) {
        // tag 就是我们的 s_index 对应组的哈希表中的 key
        unsigned int tag = address >> (s + b);
        // unsigned int s_index = (address >> b) & ((1 << s) - 1);
        unsigned s_index = (address >> b) & ((unsigned)(-1) >> (8 * sizeof(unsigned) - s));

        switch (operation) {
            case 'M':
                Update(tag, s_index);
                Update(tag, s_index);
                break;
            case 'L':  // 取操作
                Update(tag, s_index);
                break;
            case 'S':  // 存操作
                Update(tag, s_index);
                break;
        }
    }
    fclose(file);
}

// 释放内存
void Free() {
    for (int i = 0; i < cache->S; i++) {
        free(cache->Set[i].table);
        free(cache->Set[i].head);
        free(cache->Set[i].tail);
    }
    free(cache);
}

int main(int argc, char* argv[]) {
    char opt;
    int s, E, b;
    while (-1 != (opt = getopt(argc, argv, "hvs:E:b:t:"))) {
        switch (opt) {
            case 's':
                s = atoi(optarg);
                break;
            case 'E':
                E = atoi(optarg);
                break;
            case 'b':
                b = atoi(optarg);
                break;
            case 't':
                strcpy(filename, optarg);
                break;
        }
    }
    InitCache(s, E, b);
    GetTrace(s, E, b);
    Free();
    printSummary(hits, misses, evictions);
    return 0;
}