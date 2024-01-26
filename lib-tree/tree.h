/**
 * @file tree.h
 * @author ZHANG, Zhen-Yu (tolatetodieyoung1204@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2024-01-05
 * 
 * 
 */

#ifndef __MYTH_EPIC_LIB_TREE
#define __MYTH_EPIC_LIB_TREE

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stddef.h>

typedef struct tree_s tree_s;

/**
 * @brief 
 * 
 * @param compare 
 * @param release 
 * @return tree_s* 
 */
tree_s * 
treeMake(
    int (*compare)(void *, void *),
    int (*release)(void *)
);

/**
 * @brief 
 * 
 * @param refs 
 */
void 
treeFree(
    void * refs
);

/**
 * @brief 
 * 
 * @param refs 
 * @param buffer 
 * @param size 
 * @param sign 
 * @param stringify 
 * @return size_t 
 */
size_t
treeStringify(
    tree_s * refs,
    char * buffer,
    size_t size,
    char * sign,
    size_t (*stringify)(void *, char *, size_t)
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
treeDisplay(
    tree_s * refs, 
    FILE * stream,
    char * sign,
    FILE * (*display)(void *, FILE *)
);

/**
 * @brief 
 * 
 * @param refs 
 * @param val 
 * @return tree_s* 
 */
tree_s * 
treeInsert(
    tree_s * refs, 
    void * val
);

/**
 * @brief 
 * 
 * @param refs 
 * @param val 
 * @return void* 
 */
void * 
treeAccess(
    tree_s * refs, 
    void * val
);

/**
 * @brief 
 * 
 * @param refs 
 * @param val 
 * @return int 
 */
int 
treeRemove(
    tree_s * refs, 
    void * val
);

/**
 * @brief 
 * 
 * @param refs 
 * @return size_t 
 */
size_t 
treeSize(
    tree_s * refs
);

/**
 * @brief 
 * 
 * @param refs 
 * @return size_t 
 */
size_t 
treeHeight(
    tree_s * refs
);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MYTH_EPIC_LIB_TREE */
