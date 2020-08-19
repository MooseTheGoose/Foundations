#include "gcproto.hpp"

static gc_meta *begin;
static gc_meta *recent;

void gc_dec_ref(void *alloc)
{
  gc_meta *metadata = ((gc_meta *)alloc) - 1;
  metadata->rcnt--;
  if(metadata->rcnt <= 0)
  {
    /* Destruction here... */
  }
}

void gc_inc_ref(void *alloc)
{
  ((gc_meta *)alloc)[-1].rcnt++;
}

int main()
{ return 0; }

