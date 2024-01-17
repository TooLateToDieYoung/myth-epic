/**
 * @file list.h
 * @author ZHANG, Zhen-Yu (tolatetodieyoung1204@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2024-01-05
 * 
 * 
 */

#ifndef __MYTH_EPIC_LIB_LIST
#define __MYTH_EPIC_LIB_LIST

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stddef.h>

typedef struct list_s list_s;

/**
 * @brief 
 * 
 * @param display 
 * @param release 
 * @return list_s* 
 */
list_s * 
listMake(
    size_t (*stringify)(void *, char *, size_t),
    FILE * (*display)(void *, FILE *),
    int (*release)(void *)
);

/**
 * @brief 
 * 
 * @param refs 
 */
void 
listFree(
    list_s * refs
);

/**
 * @brief 
 * 
 * @param refs 
 * @param buffer 
 * @param size 
 * @param sign 
 * @return size_t 
 */
size_t
listStringify(
    list_s * refs,
    char * buffer,
    size_t size,
    char * sign
);

/**
 * @brief 
 * 
 * @param refs 
 * @param stream 
 * @param sign 
 * @return FILE* 
 */
FILE * 
listDisplay(
    list_s * refs, 
    FILE * stream,
    char * sign
);

/**
 * @brief 
 * 
 * @param refs 
 * @param idx 
 * @param val 
 * @return list_s* 
 */
list_s * 
listInsert(
    list_s * refs, 
    size_t idx, 
    void * val
);

/**
 * @brief 
 * 
 * @param refs 
 * @param idx 
 * @return void* 
 */
void * 
listAccess(
    list_s * refs, 
    size_t idx
);

/**
 * @brief 
 * 
 * @param refs 
 * @param idx 
 * @return int 
 */
int 
listRemove(
    list_s * refs, 
    size_t idx
);

/**
 * @brief 
 * 
 * @param refs 
 * @param idx 
 * @param val 
 * @return list_s* 
 */
list_s * 
listChange(
    list_s * refs, 
    size_t idx, 
    void * val
);

/**
 * @brief 
 * 
 * @param refs 
 * @return size_t 
 */
size_t 
listLength(
    list_s * refs
);

/**
 * @brief 
 * 
 * @param refs 
 * @param compare 
 * @return list_s* 
 */
list_s * 
listQuickSort(
    list_s * refs,
    int (*compare)(void *, void *)
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MYTH_EPIC_LIB_LIST */
