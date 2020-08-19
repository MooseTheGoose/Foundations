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

/*
 *  Records of:
 *
 *  1: # of children (n)
 *  2-n+1: Children offsets
 */

static gcofs_t agg_table[] = 
{
  /* No child references*/
  0,

  /* gc_ll */
  3,
  0, 8, 16,

  /* gc_tree */
  5,
  0, 8, 16, 24, 32
};

#define HEAP_SZ (1 << 20)
static gc_meta *begin;
static align_t test_heap[HEAP_SZ / sizeof(align_t)];

void *gc_create_ref(gclen_t len, gcofs_t atptr, int flags)
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
      retmeta->mark = 0;
      retmeta->sweeped = 0;
      if(flags & ROOT_FLAG) 
      { retmeta->rrcnt = 1; retmeta->orcnt = 0; }
      else
      { retmeta->rrcnt = 0; retmeta->orcnt = 1; }
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
      retmeta->mark = 0;
      retmeta->sweeped = 0;
      if(flags & ROOT_FLAG) 
      { retmeta->rrcnt = 1; retmeta->orcnt = 0; }
      else
      { retmeta->rrcnt = 0; retmeta->orcnt = 1; }
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
      begin->mark = 0;
      begin->sweeped = 0;
      if(flags & ROOT_FLAG) 
      { begin->rrcnt = 1; begin->orcnt = 0; }
      else
      { begin->rrcnt = 0; begin->orcnt = 1; }
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

  if(metadata->sweeped) { return; }
  metadata->sweeped = 1;

  if(metadata->prev) 
  { metadata->prev->next = metadata->next; }
  if(metadata->next)
  { metadata->next->prev = metadata->prev; }

  if(metadata == begin) { begin = metadata->next; }

  if(metadata->refarray) 
  {
    for(int i = 0; 
        i < (metadata->len - sizeof(gc_meta)) / sizeof(void *); 
        i += sizeof(void *))
      {
        void *ref = ((void **)alloc)[i];

        if(ref) 
        { gc_dec_ref(ref, 0); }
      }
  }
  else
  {
    gcofs_t nchildren = agg_table[metadata->atptr];
    for(gcofs_t i = metadata->atptr + 1; nchildren--; i++)
    {
      void *ref = *(void **)((char *)alloc + agg_table[i]);

      if(ref)
      { gc_dec_ref(ref, 0); }
    } 
  }
}

void gc_dec_ref(void *alloc, int root_deref)
{
  gc_meta *metadata = ((gc_meta *)alloc) - 1;
  
  if(root_deref) { metadata->rrcnt--; }  
  else { metadata->orcnt--; }

  if(metadata->rrcnt <= 0 && metadata->orcnt <= 0)
  {
    gc_destroy_ref(alloc);
  }
}

void gc_inc_ref(void *alloc, int root_ref)
{
  if(root_ref) { ((gc_meta *)alloc)[-1].rrcnt++; }
  else         { ((gc_meta *)alloc)[-1].orcnt++; }
}

void gc_collect()
{
  printf("Alive:\n");

  /* Use tracing to handle cyclic references */
  for(gc_meta *trail = begin; trail; trail = trail->next)
  { 
    printf("%p: %lld %lld\n", trail, trail->rrcnt, trail->orcnt);
    if(trail->rrcnt > 0 && !trail->mark)
    {
      trail->mark = 1;
      vector<gc_meta *>rem = vector<gc_meta *>();
      rem.push_back(trail);
      
      while(rem.size())
      {
        gc_meta *current = rem.back();
        rem.pop_back();

        gcofs_t nchildren = agg_table[current->atptr];
        for(gcofs_t i = current->atptr + 1; nchildren--; i++)
        {
          gc_meta *meta = *(gc_meta **)((char *)current + 
                                        sizeof(gc_meta) + 
                                        agg_table[i]);
          if(meta)
          {
            meta--;
            rem.push_back(meta);
            meta->mark = 1;
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
  printf("\n");

}

int main()
{
  gc_ll *my_ref = (gc_ll *)gc_create_ref(sizeof (gc_ll), 1, ROOT_FLAG);
  gc_ll *my_ref1 = (gc_ll *)gc_create_ref(sizeof (gc_ll), 1, 0);

  for(gc_meta *trail = begin; trail; trail = trail->next)
  { printf("%p ", trail); } printf("\n");

  my_ref->next = my_ref1;
  my_ref1->prev = my_ref;
  gc_inc_ref(my_ref, 0);
  gc_dec_ref(my_ref, ROOT_FLAG);

  printf("%p %p\n", (gc_meta *)my_ref - 1, (gc_meta *)my_ref1 - 1);

  gc_collect();

  for(gc_meta *trail = begin; trail; trail = trail->next)
  { printf("%p\n", trail); }
  
  return 0; 
}
