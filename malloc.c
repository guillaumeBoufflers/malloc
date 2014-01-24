#include		<pthread.h>
#include		<unistd.h>
#include		<stdlib.h>
#include		<stdio.h>
#include		<string.h>
#include		"malloc.h"

static void		*doRealloc(void *ptr, t_malloc *node, size_t size)
{
  t_malloc		*next;
  void			*s;

  s = ptr;
  ptr = malloc(size);
  memcpy(ptr, s, node->size);
  free(s);
  pthread_mutex_unlock(&lock[1]);
  return (ptr);
}

static void		*fitAllocatedSpace(size_t size)
{
  t_malloc		*tmp;
  t_malloc		*bestNode;

  tmp = MALLOC_LIST;
  bestNode = NULL;
  while (tmp)
    {
      if (tmp->isFree == 1 && tmp->size > size &&
	  (!bestNode || bestNode->size < tmp->size))
	bestNode = tmp;
      tmp = tmp->next;
    }
  if (bestNode)
    {
      bestNode->size = size;
      bestNode->isFree = 0;
      pthread_mutex_unlock(&lock[0]);
      return ((void*)bestNode + MALLOC_BLOCK_SIZE);
    }
  pthread_mutex_unlock(&lock[0]);
  return (NULL);
}

void			*malloc(size_t size)
{
  t_malloc		*new;
  void			*p;

  pthread_mutex_lock(&lock[0]);
  size = fit32Up(size);
  if ((p = fitAllocatedSpace(size)) != NULL)
    return (p);
  new = sbrk(0);
  if (sbrk(MALLOC_BLOCK_SIZE + size) == (void*) -1)
    {
      pthread_mutex_unlock(&lock[0]);
      return (NULL);
    }
  new->size = size;
  new->isFree = 0;
  new->next = NULL;
  new->prev = NULL;
  if (MALLOC_PTR_END == NULL)
    MALLOC_LIST = new;
  else
    {
      MALLOC_PTR_END->next = new;
      new->prev = MALLOC_PTR_END;
    }
  MALLOC_PTR_END = new;
  pthread_mutex_unlock(&lock[0]);
  return ((void*)new + MALLOC_BLOCK_SIZE);
}

void			*calloc(size_t nmemb, size_t size)
{
  return (malloc(nmemb * size));
}

static void		deleteNode(t_malloc  *ptr)
{
  if (ptr == MALLOC_PTR_END)
    {
      MALLOC_PTR_END = ptr->prev;
      ptr->prev->next = NULL;
    }
  else if (ptr->prev)
    {
      ptr->next->prev = ptr->prev;
      ptr->prev->next = ptr->next;
    }
}

void			handleLeaks(t_malloc *tmp)
{

  if (tmp && tmp->prev &&
      ((void*)tmp - ((void*)tmp->prev + MALLOC_BLOCK_SIZE + tmp->prev->size) == 0))
      tmp = memcpy((void*)tmp->prev + MALLOC_BLOCK_SIZE + tmp->prev->size, tmp,  MALLOC_BLOCK_SIZE);
  if (tmp && tmp->next && tmp->isFree == 1)
    tmp->size = fit32Down((void*)tmp->next - ((void*)tmp + MALLOC_BLOCK_SIZE));
}

static void		freeMemory(t_malloc *tmp)
{
  if (tmp == MALLOC_LIST && !MALLOC_PTR_END)
    {
      brk(MALLOC_LIST);
      MALLOC_LIST = NULL;
      MALLOC_PTR_END = NULL;
    }
  else if (tmp == MALLOC_PTR_END && tmp->prev)
    {
      MALLOC_PTR_END = tmp->prev;
      MALLOC_PTR_END->next = NULL;
      brk((void*)tmp->prev + MALLOC_BLOCK_SIZE + tmp->prev->size);
    }
}

void			free(void *ptr)
{
  t_malloc		*tmp;

  tmp = MALLOC_LIST;

  pthread_mutex_lock(&lock[0]);
  while (tmp)
    {
      if ((void*)tmp + MALLOC_BLOCK_SIZE == ptr)
	{
	  tmp->isFree = 1;
	  if (tmp->prev && tmp->prev->isFree == 1)
	    {
	      tmp->prev->size += tmp->size + MALLOC_BLOCK_SIZE;
	      deleteNode(tmp);
	    }
	  if (tmp->next && tmp->next->isFree == 1)
	    {
	      tmp->size += fit32Down(tmp->next->size + MALLOC_BLOCK_SIZE);
	      deleteNode(tmp->next);
	    }
	  handleLeaks(tmp);
	  freeMemory(tmp);
	  pthread_mutex_unlock(&lock[0]);
	  return;
	}
      tmp = tmp->next;
    }
  pthread_mutex_unlock(&lock[0]);
}

void			*realloc(void *ptr, size_t size)
{
  t_malloc		*node;

  size = fit32Up(size);
  if (!ptr)
    return(malloc(size));
  else if (size == 0)
    {
      free(ptr);
      return (NULL);
    }
  else
    {
      pthread_mutex_lock(&lock[1]);
      node = ptr - MALLOC_BLOCK_SIZE;
      if (node->size < size)
	return(doRealloc(ptr, node, size));
      else
	node->size = size;
      pthread_mutex_unlock(&lock[1]);
      return (ptr);
    }
}

void			show_alloc_mem()
{
  t_malloc		*tmp;

  tmp = MALLOC_LIST;
  printf("break: %p\n", sbrk(0));
  while (tmp)
    {
      if (tmp->isFree == 0)
	printf("%p - %p : %ld octets\n",
	       (void*)tmp + MALLOC_BLOCK_SIZE,
	       (void*)tmp + MALLOC_BLOCK_SIZE + tmp->size,
	       tmp->size);
      tmp = tmp->next;
    }
}
