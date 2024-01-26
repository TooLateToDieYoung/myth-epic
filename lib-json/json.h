/**
 * @file json.h
 * @author ZHANG, Zhen-Yu (tolatetodieyoung1204@gmail.com)
 * @brief 
 * @version 1.0
 * @date 2024-01-07
 * 
 * 
 */

#ifndef __MYTH_EPIC_LIB_JSON
#define __MYTH_EPIC_LIB_JSON

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include <stdio.h>
#include <stdbool.h>
#include <stddef.h>

typedef enum { JErr = -1, JNul, JBoo, JInt, JFlt, JStr, JArr, JObj } json_e;

typedef struct json_s json_s;

size_t jsonStringify(json_s * refs, char * buffer, size_t size);

json_s * jsonParseFromFile(char * filename);
json_s * jsonParseByString(char * string, char ** endptr);

FILE * jsonDump(json_s * refs, FILE * stream);

json_e jsonType(json_s * refs);

json_s * jsonMakeNul();
json_s * jsonMakeBoo(bool data);
json_s * jsonMakeInt(long data);
json_s * jsonMakeFlt(double data);
json_s * jsonMakeStr(char * data);
json_s * jsonMakeArr();
json_s * jsonMakeObj();

void jsonFree(void * refs);

json_s * jsonSetBoo(json_s * refs, bool data);
json_s * jsonSetInt(json_s * refs, long data);
json_s * jsonSetFlt(json_s * refs, double data);
json_s * jsonSetStr(json_s * refs, char * data);

bool jsonGetBoo(json_s * refs);
long jsonGetInt(json_s * refs);
double jsonGetFlt(json_s * refs);
char * jsonGetStr(json_s * refs);

json_s * jsonArrInsert(json_s * refs, size_t idx, json_s * val);
json_s * jsonArrAccess(json_s * refs, size_t idx);
json_s * jsonArrRemove(json_s * refs, size_t idx);
json_s * jsonArrChange(json_s * refs, size_t idx, json_s * val);
size_t jsonArrLength(json_s * refs);

json_s * jsonObjInsert(json_s * refs, char * key, json_s * val);
json_s * jsonObjAccess(json_s * refs, char * key);
json_s * jsonObjRemove(json_s * refs, char * key);
size_t jsonObjSize(json_s * refs);

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif /* __MYTH_EPIC_LIB_JSON */
