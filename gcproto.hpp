#ifndef GCPROTO_HPP
#define GCPROTO_HPP

#include <stdint.h>

typedef int64_t gcrcnt_t;
typedef uint64_t gclen_t;
typedef uint64_t align_t;
typedef uint64_t gcofs_t;

/* Prototype garbage collector. Tracing. */
/* Uses aggregate table to keep track of children offsets. */
/* Allocation similar to free lists. */

struct gc_meta
{
  gcrcnt_t rrcnt : 60;  /* Roots Reference count */
  gcrcnt_t mark : 1;    /* Mark-sweep as back-up */
  gcrcnt_t refarray : 1; /* Is an array of references. */
  gcofs_t  atptr;        /* Aggregate table pointer */ 
  gclen_t len;           /* Length of metadata */
  gc_meta *prev;         /* Metadata make doubly-linked list */
  gc_meta *next;
};

#define ROOT_FLAG 1
#define REFARRAY_FLAG 2
void *gc_create_ref(gclen_t len, gcofs_t atptr, int flags);
void gc_dec_ref(void *alloc);
void gc_inc_ref(void *alloc);
void gc_collect();

#endif
