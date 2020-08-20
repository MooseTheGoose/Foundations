#include "gcproto.hpp"
#include <vector>
#include <string.h>
#include <stdio.h>

using std::vector;

struct gc_ll
{ 
  gc_ll *prev, *next; 
  void *data;
};

struct gc_tree
{
  gc_tree *parent;
  gc_tree *children;
  gc_tree *prev, *next;
  void *data;
};

struct gc_weak_test
{ void *data; };

/*
 *  Records of:
 *
 *  1: # of strong reference children (n)
 *  2-n+1: Children offsets
 */

static gcofs_t agg_table[] = 
{
  /* No child strong references*/
  0,

  /* gc_ll */
  3,
  0, sizeof(void *), 2 * sizeof(void *),

  /* gc_tree */
  5,
  0, sizeof(void *), 2 * sizeof(void *), 3 * sizeof(void *), 4 * sizeof(void *)
};

/*
 *  Similar records as above.
 */
static gcofs_t weak_table[] = 
{
  /* No weak references*/
  0,
  /* gc_weak_test */
  1,
  0
};

#define HEAP_SZ (1 << 20)
static gc_meta *begin;
static align_t test_heap[HEAP_SZ / sizeof(align_t)];

void *gc_create_ref(gclen_t len, gcofs_t atptr, gcofs_t wtptr, int flags)
{
  /* Go through heap and look for space. */
  gclen_t true_len = (len + sizeof(gc_meta) + (sizeof(align_t) - 1)) & 
                     ~(sizeof(align_t) - 1);

  gc_meta *trail = begin;
  char *test = 0;
  gclen_t last_len = 0;

  while(trail)
  {
    last_len = trail->len;
    test = (char *)trail + last_len;

    if(test + true_len < (char *)trail->next)
    { 
      gc_meta *retmeta = (gc_meta *)test;
      retmeta->len = true_len;
      retmeta->prev = trail;
      retmeta->next = trail->next;

      if(trail->next) { trail->next->prev = retmeta; }
      trail->next = retmeta;

      retmeta->atptr = atptr;
      retmeta->wtptr = wtptr;
      retmeta->mark = 0;
      retmeta->rrcnt = 0;
      if(flags & ROOT_FLAG) { retmeta->rrcnt = 1; }
      if(flags & REFARRAY_FLAG) { retmeta->refarray = 0; }
      memset(retmeta + 1, 0, true_len);

      return retmeta + 1;
    }
    else { trail = trail->next; }
  }

  if(!trail && test + true_len < (char *)test_heap + HEAP_SZ)
  {
    if(test) 
    {
      gc_meta *retmeta = (gc_meta *)test;
      retmeta->len = true_len;
      retmeta->next = 0;
      retmeta->prev = (gc_meta *)(test - last_len);
      retmeta->prev->next = retmeta;

      retmeta->atptr = atptr;
      retmeta->wtptr = wtptr;
      retmeta->mark = 0;
      retmeta->collected = 0;
      retmeta->rrcnt = 0;
      if(flags & ROOT_FLAG) { retmeta->rrcnt = 1; }
      if(flags & REFARRAY_FLAG) { retmeta->refarray = 0; }
      memset(retmeta + 1, 0, true_len);

      return retmeta + 1;
    } 
    else
    {
      begin = (gc_meta *)test_heap;
      begin->len = true_len;
      begin->next = 0;
      begin->prev = 0;

      begin->atptr = atptr;
      begin->wtptr = wtptr;
      begin->mark = 0;
      begin->collected = 0;
      begin->rrcnt = 0;
      if(flags & ROOT_FLAG) { begin->rrcnt = 1; }
      if(flags & REFARRAY_FLAG) { begin->refarray = 0; }
      memset(begin + 1, 0, true_len);

      return begin + 1;
    }
  }
 
  return 0; 
}

void gc_destroy_ref(void *alloc)
{
  gc_meta *metadata = ((gc_meta *)alloc) - 1;

  if(metadata->prev) 
  { metadata->prev->next = metadata->next; }
  if(metadata->next)
  { metadata->next->prev = metadata->prev; }

  if(metadata == begin) { begin = metadata->next; }
  metadata->collected = 1;
}

void gc_dec_ref(void *alloc)
{
  ((gc_meta *)alloc)[-1].rrcnt--;
}

void gc_inc_ref(void *alloc)
{
  ((gc_meta *)alloc)[-1].rrcnt++;
}
/* Tracing */
void gc_collect()
{
  printf("Alive:\n");

  for(gc_meta *trail = begin; trail; trail = trail->next)
  { 
    printf("%p: %lld\n", trail, trail->rrcnt);
    if(trail->rrcnt > 0 && !trail->mark)
    {
      trail->mark = 1;
      vector<gc_meta *>rem = vector<gc_meta *>();
      rem.push_back(trail);
      
      while(rem.size())
      {
        gc_meta *current = rem.back();
        void *base = current + 1;
        rem.pop_back();

        if(current->refarray)
        {
          for(int i = 0; 
              i < (current->len - sizeof(gc_meta)) / sizeof(void *); 
              i += sizeof(void *))
          {
            gc_meta *meta = ((gc_meta **)base)[i];

            if(meta && !meta->mark) 
            { 
              rem.push_back(meta); 
              meta->mark = 1; 
            }
          }
        }
        else
        {
          gcofs_t nchildren = agg_table[current->atptr];
          for(gcofs_t i = current->atptr + 1; nchildren--; i++)
          {
            gc_meta *meta = *(gc_meta **)((char *)base + agg_table[i]);

            if(meta-- && !meta->mark)
            {
              meta->mark = 1;
              rem.push_back(meta);
            }
          } 
        }
      }
    } 
  }

  printf("\nUnmarked:\n");

  for(gc_meta *trail = begin; trail; trail = trail->next)
  {
    if(trail->mark) { trail->mark = 0; }
    else { printf("%p\n", trail); gc_destroy_ref(trail + 1); }
  }

  for(gc_meta *trail = begin; trail; trail = trail->next)
  {
    gcofs_t nchildren = weak_table[trail->wtptr];
    for(gcofs_t i = trail->wtptr + 1; nchildren--; i++)
    {
      void **weakref = (void **)((char *)(trail + 1) + weak_table[i]);

      if(*weakref && ((gc_meta *)*weakref)[-1].collected)
      {
         *weakref = 0;
      }
    }
  }

  printf("\n");

}

int main()
{
  gc_ll *my_ref = (gc_ll *)gc_create_ref(sizeof (gc_ll), 1, 0, 0);
  gc_ll *my_ref1 = (gc_ll *)gc_create_ref(sizeof (gc_ll), 1, 0, 0);
  void **my_ref2 = (void **)gc_create_ref(sizeof(void *), 0, 1, ROOT_FLAG);  // Introduce weak reference

  my_ref->next = my_ref1;
  my_ref1->prev = my_ref;
  *my_ref2 = my_ref;


  gc_collect();
  printf("%p\n", *my_ref2);
  
  return 0; 
}

