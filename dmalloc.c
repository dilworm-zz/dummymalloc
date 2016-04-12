#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <assert.h>

void* malloc(size_t size)
{
  void* p;
  p = sbrk(0);
  if (sbrk(size) == (void*)-1)
  {
    return NULL;
  }

  return p;
}


typedef struct s_block * t_block;
struct s_block {
  size_t size;
  t_block next;
  int free;
  char data[1];
};

#define align4(x) (((((x)-1)>>2)<<2)+4)
//#define BLOCK_SIZE (sizeof(struct s_block))
#define BLOCK_SIZE 12

void* base = NULL;

t_block find_block(size_t size, t_block* last)
{
  t_block p = base;
  while (p && p->free > 0 && p->size < size)
  { 
    *last = p;
    p = p->next;
  }
  
  return p;
}

t_block extend_heap(t_block last, size_t s)
{
  t_block b;
  b = sbrk(0);
  if (sbrk(BLOCK_SIZE + s) == (void*) -1) return NULL;

  b->size = s;
  b->next = NULL;

  if (last)
    last->next = b;

  b->free = 0;

  return b;
}

void split_block(t_block b, size_t s)
{
  assert(b->size >= BLOCK_SIZE + 4);
  t_block new_block = (t_block)(b->data + s);

  new_block->size = b->size - s - BLOCK_SIZE;
  new_block->next = b->next;
  new_block->free = 1;

  b->size = s;
  b->next = new_block;
}


void* mymalloc(size_t size)
{
  t_block b, last;
  size = align4(size);

  if (base) {
    b = find_block(size, &last);
    if (b) {
      if (b->size - size >=  BLOCK_SIZE + 4) {
        split_block(b, size);
      }
      b->free = 0;
    } else {
      b = extend_heap(last, size);
    }
  } else {
    b = extend_heap(NULL, size);
    if (!b) base = b;
  }

  return b ? b->data : NULL;
}


void main()
{
  char* p = (char*)mymalloc(18);
  if (p)
  {
    printf("allocate memory success, addr = %p", p);
  }
  else
  {
    printf("allocat failed");
  }
}


