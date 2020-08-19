#ifndef GCPROTO_HPP
#define GCPROTO_HPP

#include <stdint.h>

typedef uint64_t gcrcnt_t;
typedef uint64_t gclen_t;

/* Prototype garbage collector. Reference counting. */
/* Uses reference table to keep track of struct offsets. */
/* Allocation similar to free lists. */

struct gc_meta
{
  gcrcnt_t rcnt : 62; /* Reference count */
  gcrcnt_t mark : 1;  /* Mark-sweep as back-up */
  gcrcnt_t root : 1; 
  void (*finalizer)(void *obj); /* Finalizer function */
  gclen_t len;                  /* Length of metadata */
  gc_meta *prev;                /* Metadata make doubly-linked list */
  gc_meta *next;
};

void *gc_create_ref(gclen_t len, void (*finalizer)(void *), int root);
void gc_dec_ref(void *alloc);
void gc_inc_ref(void *alloc);
void gc_collect();

#endif
