#include <assert.h>
#include <stdlib.h>

#include "tree.h"

typedef struct node_s node_s;
struct node_s
{
    void *val;
    node_s *p;
    node_s *l;
    node_s *r;
    size_t height;
};

struct tree_s
{
    node_s *root;
    size_t size;
    size_t (*stringify)(void *, char *, size_t);
    FILE *(*display)(void *, FILE *);
    int (*compare)(void *, void *);
    int (*release)(void *);
};

static node_s *_treeTidyUp(node_s *refs);
static node_s *_treeFillUp(node_s *refs);
static node_s *_treeSearch(node_s *refs, void *val, int (*compare)(void *, void *), node_s **last);
static size_t _treeHeight(node_s *refs);
static size_t _treeMaxHeight(node_s *refsL, node_s *refsR);
static size_t _treeStringifyRecursion(node_s *refs, char *buffer, size_t size, char *sign, size_t (*stringify)(void *, char *, size_t));
static FILE *_treeDumpRecursion(node_s *refs, FILE *stream, char *sign, FILE *(*display)(void *, FILE *));

/* public */
tree_s *
treeMake(
    size_t (*stringify)(void *, char *, size_t),
    FILE *(*display)(void *, FILE *),
    int (*compare)(void *, void *),
    int (*release)(void *)
) {
assert(stringify);
assert(display);
assert(compare);
assert(release);

    tree_s *const refs = (tree_s *)calloc(1, sizeof(tree_s));
    if (refs)
    {
        refs->stringify = stringify;
        refs->display = display;
        refs->compare = compare;
        refs->release = release;
    }

    return refs;
}

void 
treeFree(
    tree_s *refs
) {
    if (refs)
    {
        while (treeSize(refs))
        {
            treeRemove(refs, refs->root->val);
        }

        free(refs);
    }
}

size_t
treeStringify(
    tree_s * refs,
    char * buffer,
    size_t size,
    char * sign
) {
    size_t ret = 0;

    if (treeSize(refs))
    {
        ret = _treeStringifyRecursion(refs->root, buffer, size, sign, refs->stringify);
    }

    return ret;
}

FILE *
treeDisplay(
    tree_s *refs,
    FILE *stream,
    char *sign
) {
    if (treeSize(refs))
    {
        if (stream)
        {
            _treeDumpRecursion(refs->root, stream, sign, refs->display);
        }
    }

    return stream;
}

tree_s *
treeInsert(
    tree_s *refs,
    void *val
) {
    node_s *target = NULL;
    node_s *last = NULL;
    node_s *curr = NULL;
    int chk = 0;

    if (refs)
    {
        curr = _treeSearch(refs->root, val, refs->compare, &last);
        if (curr) /* already exist */
        {
            (void)refs->release(curr->val);
            curr->val = val;
        }
        else
        {
            target = (node_s *)calloc(1, sizeof(node_s));
            if (target)
            {
                target->val = val;
                target->height = 1;
                target->p = last;

                if (!last) /* refs->size == 0 */
                {
                    refs->root = target;
                }
                else
                {
                    chk = refs->compare(val, last->val);
                    if (chk < 0)
                    {
                        last->l = target;
                    }
                    if (chk > 0)
                    {
                        last->r = target;
                    }

                    refs->root = _treeTidyUp(last);
                }

                refs->size++;
            }
            else  // ! Error: calloc failed
            {
                return NULL;
            }
        }
        
    }

    return refs;
}

void *
treeAccess(
    tree_s *refs,
    void *val
) {
    node_s *target = NULL;

    if (refs)
    {
        target = _treeSearch(refs->root, val, refs->compare, NULL);
    }

    return target ? target->val : NULL; 
}

int treeRemove(
    tree_s *refs,
    void *val)
{
    int ret = -1;
    node_s *target = NULL;
    node_s *endptr = NULL;

    if (treeSize(refs))
    {
        target = _treeSearch(refs->root, val, refs->compare, NULL);
        if (target)
        {
            ret = refs->release(target->val);

            // ? fill up & remove the lastest one from this topology
            endptr = _treeFillUp(target);

            // ? tidy up this tree and reset the root
            refs->root = (1 == refs->size) ? NULL : _treeTidyUp(endptr->p);
            refs->size--;

            // ? release useless node
            free(endptr);
        }
    }

    return ret;
}

size_t
treeSize(
    tree_s *refs
) {
    return refs ? refs->size : 0;
}

size_t
treeHeight(
    tree_s *refs
) {
    return refs ? _treeHeight(refs->root) : 0;
}

/* private */
static 
node_s *
_treeTidyUp(
    node_s *refs
) {
    node_s *parent = refs->p;
    node_s *center = refs;
    const size_t hL = _treeHeight(refs->l);
    const size_t hR = _treeHeight(refs->r);

    if (hL > hR + 1)
    {
        center = refs->l;

        refs->l = center->r;
        if (refs->l)
        {
            refs->l->p = refs;
        }

        refs->p = center;
        center->r = refs;

        center->p = parent;
        if (parent)
        {
            if (refs == parent->l)
            {
                parent->l = center;
            }
            if (refs == parent->r)
            {
                parent->r = center;
            }
        }

        refs->height = 1 + _treeMaxHeight(refs->l, refs->r);
    }

    if (hL + 1 < hR)
    {
        center = refs->r;

        refs->r = center->l;
        if (refs->r)
        {
            refs->r->p = refs;
        }

        refs->p = center;
        center->l = refs;

        center->p = parent;
        if (parent)
        {
            if (refs == parent->l)
            {
                parent->l = center;
            }
            if (refs == parent->r)
            {
                parent->r = center;
            }
        }

        refs->height = 1 + _treeMaxHeight(refs->l, refs->r);
    }

    center->height = 1 + _treeMaxHeight(center->l, center->r);

    return parent ? _treeTidyUp(parent) : center;
}

static 
node_s *
_treeFillUp(
    node_s *refs
) {
    node_s *parent = refs->p;
    node_s *winner = refs->l ? refs->l : refs->r;

    if (!winner) /* reach to the tail */
    {
        if (parent) /* remove from the tree */
        {
            if (refs == parent->l)
            {
                parent->l = NULL;
            }
            if (refs == parent->r)
            {
                parent->r = NULL;
            }
        }

        return refs; /* release by caller */
    }

    refs->val = winner->val;

    return _treeFillUp(winner);
}

static 
node_s *
_treeSearch(
    node_s *refs,
    void *val,
    int (*compare)(void *, void *),
    node_s **last
) {
    int chk = 0;
    node_s *temp = NULL;
    node_s *curr = refs;

    while (curr)
    {
        temp = curr;
        chk = compare(val, curr->val);
        if (chk < 0)
        {
            curr = curr->l;
            continue;
        }
        if (chk > 0)
        {
            curr = curr->r;
            continue;
        }
        break;
    }

    if (last)
    {
        *last = temp;
    }
    return curr;
}

static
size_t
_treeHeight(
    node_s *refs
) {
    return refs ? refs->height : 0;
}

static 
size_t
_treeMaxHeight(
    node_s *refsL,
    node_s *refsR
) {
    const size_t hL = _treeHeight(refsL);
    const size_t hR = _treeHeight(refsR);

    return hL > hR ? hL : hR;
}

static 
size_t 
_treeStringifyRecursion(
    node_s *refs, 
    char *buffer, 
    size_t size, 
    char *sign, 
    size_t (*stringify)(void *, char *, size_t)
) {
    size_t ret = 0;
    char * position = buffer ? buffer : NULL;
    size_t boundary = position ? size : 0;

    if (refs->l)
    {
        ret += _treeStringifyRecursion(refs->l, position, boundary, sign, stringify);
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

    if (refs->r)
    {
        ret += _treeStringifyRecursion(refs->r, position, boundary, sign, stringify);
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

    return ret + stringify(refs->val, position, boundary);
}

static 
FILE *
_treeDumpRecursion(
    node_s *refs,
    FILE *stream,
    char *sign,
    FILE *(*display)(void *, FILE *)
) {
    if (refs->l)
    {
        _treeDumpRecursion(refs->l, stream, sign, display);
        fprintf(stream, "%s", sign);
    }

    if (refs->r)
    {
        _treeDumpRecursion(refs->r, stream, sign, display);
        fprintf(stream, "%s", sign);
    }

    return display(refs->val, stream);
}
