#ifndef _PARALLEL_H_
#define _PARALLEL_H_

#ifndef NOPAR
#define parallel cilk_spawn
#define syncpoint cilk_sync
#elif
#define parallel
#define syncpoint
#endif

#endif
