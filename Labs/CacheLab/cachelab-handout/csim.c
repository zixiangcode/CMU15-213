/*
 *   Author:zixiang
 *   Email :zixiangcode@gmail.com
 */

#include <getopt.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "cachelab.h"
typedef struct cache_line {
    int is_valid;   // 是否有效
    int tag;        // 标记位
    int time_tamp;  // 时间戳
} Cache_line;

typedef struct cache_ {
    int S;
    int E;
    int B;
    Cache_line **line;  // 一个 Cache 包含 S 个组，每个组包含多个 Cache_line
} Cache;

/*
    定义一些全局变量
*/
Cache *cache = NULL;
int hits = 0, misses = 0, evictions = 0;
char filename[FILENAME_MAX];
int t = 1;

void InitCache(int s, int E, int b) {
    int S = 1 << s;  // S = 2^s
    int B = 1 << b;  // B = 2^b

    cache = (Cache *)malloc(sizeof(Cache));
    cache->S = S;
    cache->E = E;
    cache->B = B;
    cache->line = (Cache_line **)malloc(sizeof(Cache_line *) * S);  // 有 S 个组

    for (int i = 0; i < S; i++) {
        cache->line[i] = (Cache_line *)malloc(sizeof(Cache_line) *
                                              E);  // 每个组有 E 个 Cache_line
        for (int j = 0; j < E; j++) {
            cache->line[i][j].is_valid = 0;   // 初始化缓存全为空
            cache->line[i][j].tag = -1;       // tag 可能为 0
            cache->line[i][j].time_tamp = 0;  // 初始时间戳全为 0
        }
    }
}

/*
    LRU 逻辑：时间戳为 0 代表未使用，非 0 值越大，越是最近使用的
*/
void UpdateTimeTamp(int i, int s, int tag) {
    cache->line[s][i].is_valid = 1;
    cache->line[s][i].tag = tag;

    // cache->line[s][i].time_tamp += 1;
    // cache->line[s][i].time_tamp += (++t); //
    // ！！！无论怎么相加，总会出现相等的情况
    cache->line[s][i].time_tamp = (++t);  // 自增值可以保证唯一性！！！
}

/*
    查询组 s 中的每一行 Cache_line，找到时间戳非 0 且最小的替换即可
*/
int FindLRU(int s) {
    int min_time_tamp = INT_MAX;
    int index = -1;

    for (int i = 0; i < cache->E; i++) {
        if (cache->line[s][i].time_tamp == 0) {
            return i;
        } else {
            if (cache->line[s][i].time_tamp < min_time_tamp) {
                min_time_tamp = cache->line[s][i].time_tamp;
                index = i;
            }
        }
    }

    return index;
}

/*
    匹配组 s 中的 tag，判断是否命中以及是否需要替换
*/
int Find(int s, int tag) {
    for (int i = 0; i < cache->E; i++) {
        if (cache->line[s][i].is_valid == 1 &&
            cache->line[s][i].tag == tag) {  // 有效且匹配成功
            return i;
        }
    }
    return -1;
}

/*
    判断组 s 是否已经塞满
*/
int IsFull(int s) {
    for (int i = 0; i < cache->E; i++) {
        if (cache->line[s][i].is_valid == 0) {
            return i;
        }
    }
    return -1;
}

void Update(int tag, int s) {
    int index = Find(s, tag);
    if (index != -1) {  // 命中
        hits += 1;
        UpdateTimeTamp(index, s, tag);
    } else {  // 未命中
        misses += 1;
        int i = IsFull(s);
        if (i == -1) {  // 未找到空位，需要替换
            evictions += 1;
            i = FindLRU(s);
        }
        UpdateTimeTamp(i, s, tag);
    }
}

/*
    获取测试的 trace 文件
*/
void GetTrace(int s, int E, int b) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        exit(EXIT_FAILURE);
    }

    // 指令的三个信息
    char operation;
    unsigned int address;
    int size;

    // 读指令 "I"指令没有实际作用，可以忽略
    while (fscanf(file, " %s %x,%d", &operation, &address, &size) > 0) {
        int tag = address >> (s + b);                   // 标记位是高位
        int s_index = (address >> b) & ((1 << s) - 1);  // 组序号是中间的位

        switch (operation) {
            case 'M':  // 修改，也就是取一次，存一次
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

/*
    释放资源
*/
void Free() {
    for (int i = 0; i < cache->S; i++) {
        free(cache->line[i]);
    }
    free(cache->line);
    free(cache);
}

int main(int argc, char *argv[]) {
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
