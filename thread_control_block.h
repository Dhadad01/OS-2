#ifndef __THREAD_CONTROL_BLOCK_H_INCLUDED
#define __THREAD_CONTROL_BLOCK_H_INCLUDED

#include <signal.h>   /* for struct sigaction, sigaction */
#include <sys/time.h> /* for struct itimerval */

#include <cstddef> /* for size_t */
#include <cstdint> /* for uint8_t */
#include <list>    /* for std::list */
#include <memory>  /* for std::shared_ptr, std::make_shared */
#include <vector>  /* for std::vector */

#include <setjmp.h>

#include "uthreads.h"

#ifndef UNUSED
#define UNUSED(a) ((void)a)
#endif /* UNUSED */

enum state
{
  READY,
  RUNNING,
  BLOCKED,
  SLEEPING,
  TERMINATED,
};

typedef class ThreadControlBlock
{
public:
  ThreadControlBlock(uint8_t* stck, bool is_main);

  ~ThreadControlBlock();

  int get_quantums(void);

  const state& get_state(void);
  void set_state(const state& new_state);

  void sleep(int quantums);

  void update_quantums(int quantums);

  void update_sleeping(int quantums);
  int get_remaining_sleep(void);

  int get_quantums_in_running(void) const;
  void reset_quantums_in_running(void);

  int get_usec_in_running(void) const;
  void reset_usec_in_running(void);

  uint8_t *get_stack(void);

  void set_entry_point(thread_entry_point entry);
  thread_entry_point get_entry_point(void) const;

  sigjmp_buf m_env;
  int m_tid;
  bool m_is_sleeping;

private:
  state m_state;
  uint8_t* m_stack;
  bool m_stack_alloc;

  /*
   * the total quantums of all the quantums in the running state combined
   */
  int m_quantums;
  int m_sleep_quantums;
  /*
   * the amount of quantums since the thread entered the RUNNING state
   */
  int m_quantums_in_running;

  /*
   * the amount of micro-seconds since the thread entered the RUNNING state
   */
  int m_usec_in_running;

  thread_entry_point m_entry_point;
} TCB;

typedef std::shared_ptr<TCB> tcb_ptr;

#endif /* __THREAD_CONTROL_BLOCK_H_INCLUDED */
