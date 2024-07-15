
#include <stdlib.h>
#include "bircd.h"

//monitor the file descriptors that are ready for reading or writing
void	do_select(t_env *e)
{
  e->r = select(e->max + 1, &e->fd_read, &e->fd_write, NULL, NULL); // wait for events on the file descriptors specified
}
// returns the number of file descriptors that are ready for reading or writing