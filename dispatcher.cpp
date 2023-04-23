#include "dispatcher.h"

#include <setjmp.h>

#ifdef DEBUG
#include <iostream>
#endif /* DEBUG */

unsigned int
Dispatcher::save(tcb_ptr& thrd)
{
  /*
#ifdef DEBUG
  std::cout << "Dispatcher::save was called, saving thread #" << thrd->id
            << std::endl;
#endif
  */

  int status;

  status = sigsetjmp(thrd->m_env, 1);

  if (status == 0) {
    /*
     * successfully saved the function state
     * (regs and such).
     */
    return 0;
  } else {
    /*
     * status holds the paused thread id + 1.
     */
    return status - 1;
  }
}

void
Dispatcher::resume(tcb_ptr& thrd)
{
  /*
#ifdef DEBUG
  std::cout << "Dispatcher::resume was called, resuming thread #" << thrd->id
            << std::endl;
#endif
  */

  siglongjmp(thrd->m_env, thrd->m_tid + 1);
}
