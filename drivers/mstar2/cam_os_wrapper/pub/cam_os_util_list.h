#ifndef __CAM_OS_UTIL_LIST_H__
#define __CAM_OS_UTIL_LIST_H__

#ifdef __cplusplus
extern "C" {
#endif /* __cplusplus */

#include "cam_os_wrapper.h"

// List API
struct CamOsListHead_t
{
    struct CamOsListHead_t *pNext, *pPrev;
};

#ifndef offsetof
#ifdef __compiler_offsetof
#define offsetof(TYPE,MEMBER) __compiler_offsetof(TYPE,MEMBER)
#else
#define offsetof(TYPE, MEMBER) ((size_t) &((TYPE *)0)->MEMBER)
#endif
#endif


#define CAM_OS_POISON_POINTER_DELTA 0
#define CAM_OS_LIST_POISON1  ((void *) 0x00100100 + CAM_OS_POISON_POINTER_DELTA)
#define CAM_OS_LIST_POISON2  ((void *) 0x00200200 + CAM_OS_POISON_POINTER_DELTA)

#define CAM_OS_LIST_HEAD_INIT(name) { &(name), &(name) }

#define CAM_OS_LIST_HEAD(name) \
    struct CamOsListHead_t name = CAM_OS_LIST_HEAD_INIT(name)

#define CAM_OS_LIST_ENTRY(ptr, type, member) \
    CAM_OS_CONTAINER_OF(ptr, type, member)

#define CAM_OS_LIST_FOR_EACH(pos, head) \
    for (pos = (head)->pNext; pos != (head); pos = pos->pNext)

#define CAM_OS_LIST_FOR_EACH_SAFE(pos, n, head) \
    for (pos = (head)->pNext, n = pos->pNext; pos != (head); \
            pos = n, n = pos->pNext)

#define CAM_OS_LIST_FIRST_ENTRY(ptr, type, member) \
    CAM_OS_LIST_ENTRY((ptr)->pNext, type, member)

#define CAM_OS_LIST_NEXT_ENTRY(pos, member) \
    CAM_OS_LIST_ENTRY((pos)->member.pNext, typeof(*(pos)), member)

#define CAM_OS_LIST_FOR_EACH_ENTRY_SAFE(pos, n, head, member)                  \
    for (pos = CAM_OS_LIST_FIRST_ENTRY(head, typeof(*pos), member),        \
            n = CAM_OS_LIST_NEXT_ENTRY(pos, member);                       \
            &pos->member != (head);                                    \
            pos = n, n = CAM_OS_LIST_NEXT_ENTRY(n, member))

#define CAM_OS_LIST_FOR_EACH_ENTRY(pos, head, member)                          \
    for (pos = CAM_OS_LIST_FIRST_ENTRY(head, typeof(*pos), member);        \
            &pos->member != (head);                                    \
            pos = CAM_OS_LIST_NEXT_ENTRY(pos, member))


    static inline void CAM_OS_INIT_LIST_HEAD(struct CamOsListHead_t *pList)
    {
        pList->pNext = pList;
        pList->pPrev = pList;
    }

    static inline void _CAM_OS_LIST_ADD(struct CamOsListHead_t *new,
                                struct CamOsListHead_t *pPrev,
                                struct CamOsListHead_t *pNext)
    {
        pNext->pPrev = new;
        new->pNext = pNext;
        new->pPrev = pPrev;
        pPrev->pNext = new;
    }

    static inline void CAM_OS_LIST_ADD(struct CamOsListHead_t *new, struct CamOsListHead_t *head)
    {
        _CAM_OS_LIST_ADD(new, head, head->pNext);
    }


    static inline void CAM_OS_LIST_ADD_TAIL(struct CamOsListHead_t *new, struct CamOsListHead_t *head)
    {
        _CAM_OS_LIST_ADD(new, head->pPrev, head);
    }

    static inline void _CAM_OS_LIST_DEL(struct CamOsListHead_t * pPrev, struct CamOsListHead_t * pNext)
    {
        pNext->pPrev = pPrev;
        pPrev->pNext = pNext;
    }

    static inline void _CAM_OS_LIST_DEL_ENTRY(struct CamOsListHead_t *entry)
    {
        _CAM_OS_LIST_DEL(entry->pPrev, entry->pNext);
    }


    static inline void CAM_OS_LIST_DEL(struct CamOsListHead_t *pEntry)
    {
        _CAM_OS_LIST_DEL(pEntry->pPrev, pEntry->pNext);
        pEntry->pNext = CAM_OS_LIST_POISON1;
        pEntry->pPrev = CAM_OS_LIST_POISON2;
    }

    static inline void CAM_OS_LIST_DEL_INIT(struct CamOsListHead_t *entry)
    {
        _CAM_OS_LIST_DEL_ENTRY(entry);
        CAM_OS_INIT_LIST_HEAD(entry);
    }

    static inline int CAM_OS_LIST_IS_LAST(const struct CamOsListHead_t *list,
                const struct CamOsListHead_t *head)
    {
        return list->pNext == head;
    }

    static inline int CAM_OS_LIST_EMPTY(const struct CamOsListHead_t *head)
    {
        return head->pNext == head;
    }

    static inline int CAM_OS_LIST_EMPTY_CAREFUL(const struct CamOsListHead_t *head)
    {
        struct CamOsListHead_t *pNext = head->pNext;
        return (pNext == head) && (pNext == head->pPrev);
    }


void CamOsListSort(void *priv, struct CamOsListHead_t *head,
           int (*cmp)(void *priv, struct CamOsListHead_t *a,
              struct CamOsListHead_t *b));

// HList API
static __always_inline
void _CAM_OS_READ_ONCE_SIZE(const volatile void *p, void *res, int size)
{
    switch (size) {
    case 1: *(uint8_t *)res = *(volatile uint8_t *)p; break;
    case 2: *(uint16_t *)res = *(volatile uint16_t *)p; break;
    case 4: *(uint32_t *)res = *(volatile uint32_t *)p; break;
    case 8: *(uint64_t *)res = *(volatile uint64_t *)p; break;
    default:
        asm volatile("": : :"memory"); // barrier()
        __builtin_memcpy((void *)res, (const void *)p, size);
        asm volatile("": : :"memory"); // barrier()
    }
}

#define CAM_OS_READ_ONCE(x)                     \
({                                  \
    union { typeof(x) __val; char __c[1]; } __u;            \
    _CAM_OS_READ_ONCE_SIZE(&(x), __u.__c, sizeof(x));       \
    __u.__val;                          \
})

static __always_inline void _CAM_OS_WRITE_ONCE_SIZE(volatile void *p, void *res, int size)
{
    switch (size) {
    case 1: *(volatile uint8_t *)p = *(uint8_t *)res; break;
    case 2: *(volatile uint16_t *)p = *(uint16_t *)res; break;
    case 4: *(volatile uint32_t *)p = *(uint32_t *)res; break;
    case 8: *(volatile uint64_t *)p = *(uint64_t *)res; break;
    default:
        asm volatile("": : :"memory"); // barrier()
        __builtin_memcpy((void *)p, (const void *)res, size);
        asm volatile("": : :"memory"); // barrier()
    }
}

#define CAM_OS_WRITE_ONCE(x, val) \
({                          \
    union { typeof(x) __val; char __c[1]; } __u =   \
        { .__val = (typeof(x)) (val) }; \
    _CAM_OS_WRITE_ONCE_SIZE(&(x), __u.__c, sizeof(x));  \
    __u.__val;                  \
})

/* 2^31 + 2^29 - 2^25 + 2^22 - 2^19 - 2^16 + 1 */
#define CAM_OS_GOLDEN_RATIO_PRIME_32 0x9e370001UL
/*  2^63 + 2^61 - 2^57 + 2^54 - 2^51 - 2^18 + 1 */
#define CAM_OS_GOLDEN_RATIO_PRIME_64 0x9e37fffffffc0001UL

#if CAM_OS_BITS_PER_LONG == 32
static inline uint32_t CAM_OS_HASH_32(uint32_t val, uint32_t bits)
{
    /* On some cpus multiply is faster, on others gcc will do shifts */
    uint32_t hash = val * CAM_OS_GOLDEN_RATIO_PRIME_32;

    /* High bits are more random, so use them. */
    return hash >> (32 - bits);
}

#define CAM_OS_GOLDEN_RATIO_PRIME CAM_OS_GOLDEN_RATIO_PRIME_32
#define CAM_OS_HASH_LONG(val, bits) CAM_OS_HASH_32(val, bits)
#elif CAM_OS_BITS_PER_LONG == 64
static __always_inline uint64_t CAM_OS_HASH_64(uint64_t val, u32 bits)
{
    u64 hash = val;

    hash = hash * CAM_OS_GOLDEN_RATIO_PRIME_64;

    /* High bits are more random, so use them. */
    return hash >> (64 - bits);
}

#define CAM_OS_HASH_LONG(val, bits) CAM_OS_HASH_64(val, bits)
#define CAM_OS_GOLDEN_RATIO_PRIME CAM_OS_GOLDEN_RATIO_PRIME_64
#else
#error CAM_OS_BITS_PER_LONG not 32 or 64
#endif

struct CamOsHListHead_t {
    struct CamOsHListNode_t *pFirst;
};

struct CamOsHListNode_t {
    struct CamOsHListNode_t *pNext, **ppPrev;
};

#define CAM_OS_HLIST_HEAD_INIT { .pFirst = NULL }
#define CAM_OS_HLIST_HEAD(name) struct CamOsHListHead_t name = {  .pFirst = NULL }
#define CAM_OS_INIT_HLIST_HEAD(ptr) ((ptr)->pFirst = NULL)

#define CAM_OS_HASH_MIN(val, bits)                          \
    (sizeof(val) <= 4 ? CAM_OS_HASH_32(val, bits) : CAM_OS_HASH_LONG(val, bits))

static inline void CAM_OS_INIT_HLIST_NODE(struct CamOsHListNode_t *h)
{
    h->pNext = NULL;
    h->ppPrev = NULL;
}

static inline int CAM_OS_HLIST_UNHASHED(const struct CamOsHListNode_t *h)
{
    return !h->ppPrev;
}

static inline int CAM_OS_HLIST_EMPTY(const struct CamOsHListHead_t *h)
{
    return !CAM_OS_READ_ONCE(h->pFirst);
}

static inline void _CAM_OS_HLIST_DEL(struct CamOsHListNode_t *n)
{
    struct CamOsHListNode_t *pNext = n->pNext;
    struct CamOsHListNode_t **ppPrev = n->ppPrev;

    CAM_OS_WRITE_ONCE(*ppPrev, pNext);
    if (pNext)
        pNext->ppPrev = ppPrev;
}

static inline void CAM_OS_HLIST_DEL_INIT(struct CamOsHListNode_t *n)
{
    if (!CAM_OS_HLIST_UNHASHED(n)) {
        _CAM_OS_HLIST_DEL(n);
        CAM_OS_INIT_HLIST_NODE(n);
    }
}

static inline void CAM_OS_HLIST_ADD_HEAD(struct CamOsHListNode_t *n, struct CamOsHListHead_t *h)
{
    struct CamOsHListNode_t *pFirst = h->pFirst;
    n->pNext = pFirst;
    if (pFirst)
        pFirst->ppPrev = &n->pNext;
    h->pFirst = n;
    n->ppPrev = &h->pFirst;
}

#define CAM_OS_HLIST_ENTRY(ptr, type, member) CAM_OS_CONTAINER_OF(ptr,type,member)

#define CAM_OS_HLIST_ENTRY_SAFE(ptr, type, member) \
    ({ typeof(ptr) ____ptr = (ptr); \
       ____ptr ? CAM_OS_HLIST_ENTRY(____ptr, type, member) : NULL; \
    })

#define CAM_OS_HLIST_FOR_EACH_ENTRY(pos, head, member)              \
    for (pos = CAM_OS_HLIST_ENTRY_SAFE((head)->pFirst, typeof(*(pos)), member);\
         pos;                           \
         pos = CAM_OS_HLIST_ENTRY_SAFE((pos)->member.pNext, typeof(*(pos)), member))

#ifdef __cplusplus
}
#endif /* __cplusplus */

#endif //__CAM_OS_UTIL_LIST_H__
