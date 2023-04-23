#ifndef __SCHEDULER_H_INCLUDED
#define __SCHEDULER_H_INCLUDED

#include <signal.h>   /* for struct sigaction, sigaction */
#include <sys/time.h> /* for struct itimerval */
#include "thread_control_block.h"

class Scheduler
{
public:
  Scheduler();

  int set_quantum_usecs(int quantum_usecs);

  tcb_ptr operator[](int tid);

  int add_thread(tcb_ptr new_thread);

  int get_last_tid(void) const;

  int get_running_tid(void) const;

  void block(int tid);

  void sleep(int tid, int num_quantums);

  void set_passed_quantums(int initial);
  int get_passed_quantums(void) const;

  void switch_running_thread(void);

  void schedule(void);

  void periodic_schedule(void);

  struct sigaction * get_sigaction(void);

  struct itimerval * get_timer(void);

  int get_nunmber_of_threads(void) const;

  void run(void);

  void remove(int tid);

  void resume(int tid);

  void cleanup(void);

  int set_alarm(void);

private:
  struct sigaction m_sa;
  struct itimerval m_timer;
  int m_quantum_usecs;
  int m_quantums_passed;
  int m_running_tid;
  int m_number_of_threads;
  std::vector<tcb_ptr> m_threads;
  std::list<int> m_sleeping;
  std::list<int> m_ready;

  tcb_ptr& get_running_thread(void) { return m_threads[m_running_tid]; }
};

#endif /* __SCHEDULER_H_INCLUDED */
