#include "json.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "list.h"
#include "tree.h"

typedef struct {
    void * key;
    void * val;
} pair_s;

typedef union {
    bool   boo;
    double num;
    char * str;
    list_s * arr;
    tree_s * obj;
} json_u;

struct json_s {
    json_u data;
    json_e type;
};

static json_s * _jsonMake(json_e type);

static size_t _jsonArrStringifyHandler(void * val, char * buffer, size_t size);
static FILE * _jsonArrDisplayHandler(void * val, FILE * stream);

static size_t _jsonObjStringifyHandler(void * val, char * buffer, size_t size);
static FILE * _jsonObjDisplayHandler(void * val, FILE * stream);
static int _jsonObjReleaseHandler(void * val);
static int _jsonObjCompareHandler(void * valA, void * valB);

/* public */
size_t jsonStringify(json_s * refs, char * buffer, size_t size)
{
    size_t ret = 0;
    char * position = buffer ? buffer : NULL;
    size_t boundary = position ? size : 0;

    switch ( jsonType(refs) )
    {
        case JNul: 
            ret += snprintf(position, boundary, "null");
            break;

        case JBoo: 
            ret += refs->data.boo ? 
                snprintf(position, boundary, "true") : 
                snprintf(position, boundary, "false");
            break;

        case JInt: 
            ret += snprintf(position, boundary, "%ld", (long)refs->data.num);
            break;

        case JFlt: 
            ret += snprintf(position, boundary, "%lf", refs->data.num);
            break;

        case JStr:
            ret += snprintf(position, boundary, "\"%s\"", refs->data.str);
            break;

        case JArr:
            ret += snprintf(position, boundary, "[");
            if (buffer)
            {
                position = buffer + ret;
                boundary = size > ret ? size - ret : 0;
            }
            ret += listStringify(refs->data.arr, position, boundary, ",", _jsonArrStringifyHandler);
            if (buffer)
            {
                position = buffer + ret;
                boundary = size > ret ? size - ret : 0;
            }
            ret += snprintf(position, boundary, "]");
            break;

        case JObj: 
            ret += snprintf(position, boundary, "{");
            if (buffer)
            {
                position = buffer + ret;
                boundary = size > ret ? size - ret : 0;
            }
            ret += treeStringify(refs->data.obj, position, boundary, ",", _jsonObjStringifyHandler);
            if (buffer)
            {
                position = buffer + ret;
                boundary = size > ret ? size - ret : 0;
            }
            ret += snprintf(position, boundary, "}");
            break;

        default:
            break;
    }

    return ret;
}

json_s * jsonParseFromFile(char * filename)
{
    json_s * ret = NULL;
    FILE * fd = NULL;
    size_t len = 0;
    char * content = NULL;

    if ( NULL != filename )
    {
        fd = fopen(filename, "r");
        if ( NULL != fd )
        {
            fseek(fd, 0, SEEK_END);
            len = ftell(fd);
            fseek(fd, 0, SEEK_SET);
            content = (char *)calloc(len + 1, sizeof(char));
            if ( NULL != content )
            {
                if ( len != fread(content, sizeof(char), len, fd) )
                {
                    // ! catch error
                }
                ret = jsonParseByString(content, NULL);
            }
            fclose(fd);
        }
    }

    return ret;
}
json_s * jsonParseByString(char * string, char ** endptr)
{
    json_s * ret = NULL;
    char * head = string;
    char * tail = head;
    char temp = '\0';
    double num = 0.0f;
    char * key = NULL;
    json_s * val = NULL;
    pair_s * pair = NULL;

    if ( NULL == string ) { return NULL; }

    for ( ; '\0' != *head && !isgraph(*head); head++ ) { }

    switch ( *head )
    {
        case '"': {
            head++;
            tail = strchr(head, '"');
            if ( NULL == tail ) { goto __error; }
            
            temp = *tail;
            *tail = '\0';
            ret = jsonMakeStr(head);
            *tail = temp;
            tail++;
            break;
        }
        
        case 'n': {
            if ( 0 != strncmp(head, "null", 4) ) { goto __error; }
            tail = head + 4;
            if ( isgraph(*tail) && !strchr(",]}", *tail) ) { goto __error; }
            ret = jsonMakeNul();
            break;
        }
        
        case 't': {
            if ( 0 != strncmp(head, "true", 4) ) { goto __error; }
            tail = head + 4;
            if ( isgraph(*tail) && !strchr(",]}", *tail) ) { goto __error; }
            ret = jsonMakeBoo(true);
            break;
        }

        case 'f': {
            if ( 0 != strncmp(head, "false", 5) ) { goto __error; }
            tail = head + 5;
            if ( isgraph(*tail) && !strchr(",]}", *tail) ) { goto __error; }
            ret = jsonMakeBoo(false);
            break;
        }

        case '-': {
            tail = head + 1;
            if ( !isdigit(*tail) ) { goto __error; }
//          break;
        }
        case '0':
        case '1':
        case '2':
        case '3':
        case '4':
        case '5':
        case '6':
        case '7':
        case '8':
        case '9': {
            num = strtod(head, &tail);
            if ( NULL == tail ) { goto __error; }
            if ( isgraph(*tail) && !strchr(",]}", *tail) ) { goto __error; }
            temp = *tail;
            *tail = '\0';
            ret = ( 
                NULL != strchr(head, '.') ||
                NULL != strchr(head, 'e') || 
                NULL != strchr(head, 'E') 
            ) ? jsonMakeFlt(num) : jsonMakeInt((long)num) ;
            *tail = temp;
            break;
        }

        case '[': {
            ret = jsonMakeArr();
            tail = head;
            for ( head = tail + 1; '\0' != *head && !isgraph(*head); head++ ) { }
            if ( ']' == *head ) { tail = head + 1; break; }

            do {
                /* val */
                head = tail + 1;
                val = jsonParseByString(head, &tail);
                if ( NULL == val ) { goto __error; }
                if ( ret->data.arr != listInsert(ret->data.arr, ~0, val) ) { goto __error; }
            } while ( ',' == *tail );
            if ( ']' != *tail ) { goto __error; }
            else { tail++; }
            break;
        }

        case '{': {
            ret = jsonMakeObj();
            tail = head;
            for ( head = tail + 1; '\0' != *head && !isgraph(*head); head++ ) { }
            if ( '}' == *head ) { tail = head + 1; break; }

            do {
                /* key */
                for ( head = tail + 1; '\0' != *head && !isgraph(*head); head++ ) { }
                if ( '"' != *head ) { goto __error; }

                head++;
                tail = strchr(head, '"'); // TODO: maybe better?
                if ( NULL == tail ) { goto __error; }

                key = (char *)calloc(tail - head + 1, sizeof(char));
                if ( NULL == key ) { goto __error; }

                temp = *tail;
                *tail = '\0';
                strcpy(key, head);
                *tail = temp;

                /* val */
                for ( head = tail + 1; '\0' != *head && !isgraph(*head); head++ ) { }
                if ( ':' != *head ) { free(key); goto __error; }

                head++; 
                val = jsonParseByString(head, &tail); 
                if ( NULL == val ) { free(key); goto __error; }

                /* make a pair */
                pair = (pair_s *)calloc(1, sizeof(pair_s));
                if ( NULL == pair ) { free(key); goto __error; }
                pair->key = key;
                pair->val = val;

                if ( ret->data.obj != treeInsert(ret->data.obj, pair) ) { goto __error; }
            } while ( ',' == *tail );
            if ( '}' != *tail ) { goto __error; }
            else { tail++; }
            break;
        }

        default: { goto __error; }
    }

    if ( NULL != endptr ) 
    { 
        if ( NULL != tail )
        {
            for ( ; '\0' != *tail && !isgraph(*tail); tail++ ) { }
        }
        *endptr = tail; 
    }

    return ret;

__error:
    if ( NULL != endptr ) { *endptr = tail; }

    jsonFree(ret);

    return NULL;
}

FILE * jsonDump(json_s * refs, FILE * stream)
{
    if ( NULL == refs ) { return stream; }
    if ( NULL == stream ) { return NULL; }

    switch ( jsonType(refs) )
    {
        case JNul: 
            fprintf(stream, "null");
            break;

        case JBoo:
            ( true == refs->data.boo ) ? fprintf(stream, "true") : fprintf(stream, "false") ;
            break;

        case JInt:
            fprintf(stream, "%ld", (long)refs->data.num);
            break;

        case JFlt:
            fprintf(stream, "%lf", refs->data.num);
            break;

        case JStr:
            fprintf(stream, "\"");
            fprintf(stream, "%s", refs->data.str);
            fprintf(stream, "\"");
            break;

        case JArr:
            fprintf(stream, "[");
            listDisplay(refs->data.arr, stream, ",", _jsonArrDisplayHandler);
            fprintf(stream, "]");
            break;

        case JObj:
            fprintf(stream, "{");
            treeDisplay(refs->data.obj, stream, ",", _jsonObjDisplayHandler);
            fprintf(stream, "}");
            break;

        default:
            break;
    }

    return stream;
}

json_e jsonType(json_s * refs)
{
    if ( NULL != refs )
    {
        switch ( refs->type )
        {
            case JNul:
            case JBoo:
            case JInt:
            case JFlt:
            case JStr:
            case JArr:
            case JObj:
                return refs->type;
            
            default:
                return JErr;
        }
    }

    return JErr;
}

json_s * jsonMakeNul()
{
    return _jsonMake(JNul);
}
json_s * jsonMakeBoo(bool data)
{
    json_s * const refs = _jsonMake(JBoo);
    if ( NULL != refs )
    {
        refs->data.boo = data;
    }

    return refs;
}
json_s * jsonMakeInt(long data)
{
    json_s * const refs = _jsonMake(JInt);
    if ( NULL != refs )
    {
        refs->data.num = (double)data;
    }

    return refs;
}
json_s * jsonMakeFlt(double data)
{
    json_s * const refs = _jsonMake(JFlt);
    if ( NULL != refs )
    {
        refs->data.num = data;
    }

    return refs;
}
json_s * jsonMakeStr(char * data)
{
    json_s * refs = _jsonMake(JStr);
    if ( NULL != refs )
    {
        refs->data.str = (char *)calloc(strlen(data) + 1, sizeof(char));
        if ( NULL != refs->data.str )
        {
            strcpy(refs->data.str, data);
        }
        else // ! Error
        {
            jsonFree(refs);
            refs = NULL;
        }
    }

    return refs;
}
json_s * jsonMakeArr()
{
    json_s * refs = _jsonMake(JArr);
    if ( NULL != refs )
    {
        refs->data.arr = listMake(jsonFree);
        if ( NULL == refs->data.arr )
        {
            jsonFree(refs);
            refs = NULL;
        }
    }

    return refs;
}
json_s * jsonMakeObj()
{
    json_s * refs = _jsonMake(JObj);
    if ( NULL != refs )
    {
        refs->data.obj = treeMake(_jsonObjCompareHandler, _jsonObjReleaseHandler);
        if ( NULL == refs->data.obj )
        {
            jsonFree(refs);
            refs = NULL;
        }
    }

    return refs;
}

void jsonFree(void * refs)
{
    json_s * const jptr = (json_s *)refs;
    switch ( jsonType(refs) )
    {
        case JStr:
            if ( NULL != jptr->data.str ) { free(jptr->data.str); }
            break;

        case JArr:
            listFree(jptr->data.arr);
            break;
        
        case JObj:
            treeFree(jptr->data.obj);
            break;

        default:
            break;
    }

    if ( NULL != refs ) { free(refs); }
}

json_s * jsonSetBoo(json_s * refs, bool data)
{
    if ( JBoo == jsonType(refs) )
    {
        refs->data.boo = data;
    }

    return refs;
}
json_s * jsonSetInt(json_s * refs, long data)
{
    if ( JInt == jsonType(refs) )
    {
        refs->data.num = (double)data;
    }

    return refs;
}
json_s * jsonSetFlt(json_s * refs, double data)
{
    if ( JFlt == jsonType(refs) )
    {
        refs->data.num = data;
    }

    return refs;
}
json_s * jsonSetStr(json_s * refs, char * data)
{
    char * const temp = refs->data.str;
    if ( JStr == jsonType(refs) )
    {
        refs->data.str = (char *)calloc(strlen(data) + 1, sizeof(char));
        if ( NULL != refs->data.str )
        {
            strcpy(refs->data.str, data);
            free(temp);
        }
        else
        {
            refs->data.str = temp;
        }
    }

    return refs;
}

bool jsonGetBoo(json_s * refs)
{
    switch ( jsonType(refs) )
    {
        case JBoo:
            return refs->data.boo == true;

        case JInt:
        case JFlt:
            return refs->data.num != 0;

        case JStr:
            return refs->data.str != NULL;

        case JArr:
            return jsonArrLength(refs) != 0;
        
        case JObj:
            return jsonObjSize(refs) != 0;

        default:
            break;
    }

    return false;
}
long jsonGetInt(json_s * refs)
{
    switch ( jsonType(refs))
    {
        case JBoo:
            return refs->data.boo ? 1 : 0 ;

        case JInt:
        case JFlt:
            return (long)refs->data.num;

        case JStr:
            char * chk = NULL;
            double res = strtod(refs->data.str, &chk);
            return ( *chk == '\0' ) ? (long)res : 0 ;

        case JArr:
            return jsonArrLength(refs);
        
        case JObj:
            return jsonObjSize(refs);

        default:
            break;
    }

    return 0L;
}
double jsonGetFlt(json_s * refs)
{
    switch ( jsonType(refs) )
    {
        case JBoo:
            return refs->data.boo ? 1.0F : 0.0F ;

        case JInt:
        case JFlt:
            return (double)refs->data.num;

        case JStr:
            char * chk = NULL;
            double res = strtod(refs->data.str, &chk);
            return ( *chk == '\0' ) ? res : 0.0F ;

        default:
            break;
    }

    return 0.0F;
}
char * jsonGetStr(json_s * refs)
{
    if ( JStr != jsonType(refs) ) { return NULL; }
    return refs->data.str;
}

json_s * jsonArrInsert(json_s * refs, size_t idx, json_s * val)
{
    if ( JArr != jsonType(refs) ) { return 0; }
    if ( NULL == val ) { return NULL; }
    return refs->data.arr == listInsert(refs->data.arr, idx, val) ? refs : NULL ;
}
json_s * jsonArrAccess(json_s * refs, size_t idx)
{
    if ( JArr != jsonType(refs) ) { return 0; }
    return (json_s *)listAccess(refs->data.arr, idx);
}
json_s * jsonArrRemove(json_s * refs, size_t idx)
{
    if ( JArr != jsonType(refs) ) { return 0; }
    return 0 == listRemove(refs->data.arr, idx) ? refs : NULL ;
}
json_s * jsonArrChange(json_s * refs, size_t idx, json_s * val)
{
    if ( JArr != jsonType(refs) ) { return 0; }

    return refs->data.arr == listChange(refs->data.arr, idx, val) ? refs : NULL ;
}
size_t jsonArrLength(json_s * refs)
{
    if ( JArr != jsonType(refs) ) { return 0; }
    return listLength(refs->data.arr);
}

json_s * jsonObjInsert(json_s * refs, char * key, json_s * val)
{
    json_s * ret = NULL;

    if ( JObj != jsonType(refs) ) { return NULL; }
    if ( NULL == val ) { return NULL; }
    if ( NULL == key ) { return NULL; }

    pair_s * const pair = (pair_s *)calloc(1, sizeof(pair_s));
    if ( NULL != pair )
    {
        pair->key = (char *)calloc(strlen(key) + 1, sizeof(char));
        pair->val = val;
        if ( NULL != pair->key )
        {
            strcpy(pair->key, key);
            if ( refs->data.obj == treeInsert(refs->data.obj, pair) )
            {
                ret = refs; // ? successed
            }
#if 0 // ! failed: auto free in treeInsert()
            else { _jsonObjReleaseHandler(pair); }
#endif
        }
    }

    return ret;
}
json_s * jsonObjAccess(json_s * refs, char * key)
{
    if ( JObj != jsonType(refs) ) { return NULL; }

    pair_s * const pair = (pair_s *)treeAccess(
        refs->data.obj, 
        &(pair_s){ .key = key, .val = NULL }
    );

    return ( NULL != pair ) ? (json_s *)(pair->val) : NULL ;
}
json_s * jsonObjRemove(json_s * refs, char * key)
{
    if ( JObj != jsonType(refs) ) { return NULL; }

    return 0 == treeRemove(
        refs->data.obj, 
        &(pair_s){ .key = key, .val = NULL }
    ) ? refs : NULL ;
}
size_t jsonObjSize(json_s * refs)
{
    if ( JObj != jsonType(refs) ) { return 0; }
    return treeSize(refs->data.obj);
}

/* private */
static json_s * _jsonMake(json_e type)
{
    json_s * refs = (json_s *)calloc(1, sizeof(json_s));
    if ( NULL != refs )
    {
        refs->type = type;
    }

    return refs;
}

static size_t _jsonArrStringifyHandler(void * val, char * buffer, size_t size)
{
    return jsonStringify((json_s *)val, buffer, size);
}
static FILE * _jsonArrDisplayHandler(void * val, FILE * stream)
{
    return jsonDump((json_s *)val, stream);
}
/*
static int _jsonArrCompareHandler(void * valA, void * valB)
{
    json_s * const jValA = (json_s *)valA;
    json_s * const jValB = (json_s *)valB;
    const json_e typeA = jsonType(jValA);
    const json_e typeB = jsonType(jValB);

    if ( typeA != typeB ) { return (int)( typeA - typeB ); }

    switch ( typeA )
    {
        case JBoo:
            if ( true == jValA->data.boo && true != jValB->data.boo ) { return +1; }
            if ( true != jValA->data.boo && true == jValB->data.boo ) { return -1; }
            return 0;

        case JInt:
        case JFlt:
            if ( jValA->data.num > jValB->data.num ) { return +1; }
            if ( jValA->data.num < jValB->data.num ) { return -1; }
            return 0;

        case JStr:
            return strcmp(jValA->data.str, jValA->data.str);

        case JArr:
            if ( jsonArrLength(jValA) > jsonArrLength(jValB) ) { return +1; }
            if ( jsonArrLength(jValA) < jsonArrLength(jValB) ) { return -1; }
            return 0;
        
        case JObj:
            if ( jsonObjSize(jValA) > jsonObjSize(jValB) ) { return +1; }
            if ( jsonObjSize(jValA) < jsonObjSize(jValB) ) { return -1; }
            return 0;
        
        default:
            break;
    }

    return 0;
}
*/

static size_t _jsonObjStringifyHandler(void * val, char * buffer, size_t size)
{
    size_t ret = 0;
    char * position = buffer ? buffer : NULL;
    size_t boundary = position ? size : 0;
    pair_s * const pair = (pair_s *)val;
    if (pair)
    {
        ret += snprintf(position, boundary, "\"%s\":", (char *)pair->key);
        if (buffer)
        {
            boundary = size > ret ? size - ret : 0;
            if (!boundary) { goto __exit; }
            else { position = buffer + ret; }
        }
        ret += jsonStringify((json_s *)pair->val, position, boundary);
    }

__exit:
    if (buffer)
    {
        ret = size > ret ? ret : 0;
    }

    return ret;
}
static FILE * _jsonObjDisplayHandler(void * val, FILE * stream)
{
    pair_s * const pair = (pair_s *)val;

    if ( NULL == pair ) { return NULL; }

    fprintf(stream, "\"%s\":", (char *)(pair->key));
    return jsonDump((json_s *)(pair->val), stream);
}
static int _jsonObjReleaseHandler(void * val)
{
    pair_s * const pair = (pair_s *)val;
    if ( NULL != pair ) 
    {
        free(pair->key);
        jsonFree((json_s *)(pair->val));
        free(pair);
    }

    return 0;
}
static int _jsonObjCompareHandler(void * valA, void * valB)
{
    const pair_s * const pValA = (pair_s *)valA;
    const pair_s * const pValB = (pair_s *)valB;

    if ( NULL == pValA ) { return 0; }
    if ( NULL == pValB ) { return 0; }

    return strcmp(
        (const char *)(pValA->key), 
        (const char *)(pValB->key)
    );
}
