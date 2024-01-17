#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>

#include "list.h"

typedef struct node_s node_s;
struct node_s
{
    void *val;
    node_s *l;
    node_s *r;
};

struct list_s
{
    node_s *head;
    node_s *tail;
    node_s *curr;
    size_t record;
    size_t length;
    size_t (*stringify)(void *, char *, size_t);
    FILE *(*display)(void *, FILE *);
    int (*release)(void *);
};

static bool _listTryAccess(list_s *refs, size_t idx);
static void _listQuickSort(node_s *head, node_s *tail, int (*compare)(void *, void *));

/* public */
list_s *
listMake(
    size_t (*stringify)(void *, char *, size_t),
    FILE *(*display)(void *, FILE *),
    int (*release)(void *)
) {
assert(stringify);
assert(display);
assert(release);

    list_s *const refs = (list_s *)calloc(1, sizeof(list_s));
    if (refs)
    {
        refs->stringify = stringify;
        refs->display = display;
        refs->release = release;
    }

    return refs;
}

void listFree(
    list_s *refs
) {
    if (refs)
    {
        while (listLength(refs))
        {
            listRemove(refs, 0);
        }

        free(refs);
    }
}

size_t
listStringify(
    list_s * refs,
    char * buffer,
    size_t size,
    char *sign
) {
    size_t ret = 0;
    node_s *temp = NULL;
    char * position = buffer ? buffer : NULL;
    size_t boundary = position ? size : 0;

    if (listLength(refs))
    {
        for (
            temp = refs->head; 
            temp != refs->tail; 
            temp = temp->r
        ) {
            ret += refs->stringify(temp->val, position, boundary);
            if (buffer)
            {
                position = buffer + ret;
                boundary = size > ret ? size - ret : 0;
            }
            ret += snprintf(position, boundary, "%s", sign);
            if (buffer)
            {
                position = buffer + ret;
                boundary = size > ret ? size - ret : 0;
            }
        }
        ret += refs->stringify(refs->tail->val, position, boundary);
    }

    return ret;
}

FILE *
listDisplay(
    list_s *refs,
    FILE *stream,
    char *sign
) {
    node_s *temp = NULL;

    if (listLength(refs))
    {
        if (stream)
        {
            for (
                temp = refs->head; 
                temp != refs->tail; 
                temp = temp->r
            ) {
                refs->display(temp->val, stream);
                fprintf(stream, "%s", sign);
            }
            refs->display(refs->tail->val, stream);
        }
    }

    return stream;
}

list_s *
listInsert(
    list_s *refs,
    size_t idx,
    void *val
) {
    node_s *target = NULL;

    if (refs)
    {
        target = (node_s *)calloc(1, sizeof(node_s));
        if (target)
        {
            target->val = val;

            if (0 == refs->length)
            {
                refs->curr = refs->head = refs->tail = target;
                refs->record = 0;
            }
            else if (idx == 0) // ? in front of the head
            {
                target->r = refs->head;
                refs->head->l = target;
                refs->curr = refs->head = target;
                refs->record = 0;
            }
            else if (idx >= refs->length) // ? append to the tail
            {
                target->l = refs->tail;
                refs->tail->r = target;
                refs->curr = refs->tail = target;
                refs->record = refs->length;
            }
            else if (_listTryAccess(refs, idx))
            {
                target->l = refs->curr->l;
                target->r = refs->curr;

                target->l->r = target;
                target->r->l = target;

                refs->curr = target;
                refs->record = idx;
            }
            else // ! Error: cannot find correct position
            {
                free(target);
                return NULL;
            }

            refs->length++;
        }
        else  // ! Error: calloc failed
        {
            return NULL;
        }
    }

    return refs;
}

void *
listAccess(
    list_s *refs,
    size_t idx
) {
    return _listTryAccess(refs, idx) ? refs->curr->val : NULL;
}

int 
listRemove(
    list_s *refs,
    size_t idx
) {
    int ret = -1;
    node_s *target = NULL;

    if (_listTryAccess(refs, idx))
    {
        target = refs->curr;

        if (1 == refs->length)
        {
            refs->curr = refs->head = refs->tail = NULL;
            refs->record = 0;
        }
        else if (idx == 0)
        {
            refs->head = refs->head->r;
            refs->head->l = NULL;
            refs->curr = refs->head;
            refs->record = 0;
        }
        else if (idx == refs->length - 1)
        {
            refs->tail = refs->tail->l;
            refs->tail->r = NULL;
            refs->curr = refs->tail;
            refs->record = idx - 1;
        }
        else
        {
            target->l->r = target->r;
            target->r->l = target->l;
            refs->curr = target->r;
            refs->record = idx;
        }

        ret = refs->release(target->val);

        free(target);

        refs->length--;
    }

    return ret;
}

list_s *
listChange(
    list_s *refs,
    size_t idx,
    void *val
) {
    list_s *ret = refs;

    if (_listTryAccess(refs, idx))
    {
        (void)refs->release(refs->curr->val);
        refs->curr->val = val;
    }
    else
    {
        ret = listInsert(refs, idx, val);
    }

    return ret;
}

size_t
listLength(
    list_s *refs
) {
    return refs ? refs->length : 0;
}

list_s *
listQuickSort(
    list_s *refs,
    int (*compare)(void *, void *))
{
assert(compare);

    if (listLength(refs) > 1)
    {
        _listQuickSort(refs->head, refs->tail, compare);
    }

    return refs;
}

/* private */
static 
bool 
_listTryAccess(
    list_s *refs, 
    size_t idx
) {
    if (!refs)
    {
        return false;
    }

    if (idx >= refs->length)
    {
        return false;
    }

    if (idx == 0)
    {
        refs->curr = refs->head;
        refs->record = idx;
    }
    else if (idx == refs->length - 1)
    {
        refs->curr = refs->tail;
        refs->record = idx;
    }
    else
    {
        while (refs->record < idx)
        {
            refs->curr = refs->curr->r;
            refs->record++;
        }
        while (refs->record > idx)
        {
            refs->curr = refs->curr->l;
            refs->record--;
        }
    }

    return true;
}

// TODO: be better
static 
void 
_listQuickSort(
    node_s *head, 
    node_s *tail, 
    int (*compare)(void *, void *)
) {
    int chk = 0;
    void *const pick = head->val;
    void *temp = NULL;
    node_s *tipH = head;
    node_s *tipT = tail;
    node_s *curr = tipH->r;

    if (head == tail)
    {
        return;
    }

    while (curr != tipT->r)
    {
        chk = compare(pick, curr->val);

        if (chk > 0)
        {
            temp = curr->val;
            curr->val = tipH->val;
            tipH->val = temp;
            tipH = tipH->r;
        }

        if (chk < 0)
        {
            temp = curr->val;
            curr->val = tipT->val;
            tipT->val = temp;
            tipT = tipT->l;
        }

        curr = (chk >= 0) ? (curr->r) : (curr);
    }

    if (head != tipT) 
    {
        _listQuickSort(head, tipT, compare);
    }

    if (NULL != curr) 
    {
        _listQuickSort(curr, tail, compare);
    }
}
