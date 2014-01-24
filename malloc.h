#ifndef			__MALLOC_H__
#define			__MALLOC_H__

typedef	struct			s_malloc
{
  struct	s_malloc	*next;
  struct	s_malloc	*prev;
  size_t			size;
  char				isFree;
}				t_malloc;

static t_malloc		*MALLOC_LIST = NULL;
static t_malloc		*MALLOC_PTR_END = NULL;
static int		MALLOC_BLOCK_SIZE = sizeof(t_malloc);

#define fit32Up(x)	(((((x) -1)>>2)<<2)+4)
#define fit32Down(x)	(((((x))>>2)<<2))

pthread_mutex_t lock[2] = {PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER};

#endif
