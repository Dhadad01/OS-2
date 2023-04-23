#ifndef __DISPATCHER_H_INCLUDED
#define __DISPATCHER_H_INCLUDED

#include "thread_control_block.h"

class Dispatcher
{
public:
  /*
   * save the running thread state, i.e. registers & stuff.
   *
   * @param thrd: the thread to save it's state.
   * @return: 0 - state was saved successfully.
   *          otherwise - tid of the stopped thread.
   */
  static unsigned int save(tcb_ptr& thrd);

  /*
   *
   */
  static void resume(tcb_ptr&);
};

#endif /* __DISPATCHER_H_INCLUDED */
