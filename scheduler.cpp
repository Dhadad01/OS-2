#include "scheduler.h"

#ifdef DEBUG
#include <iostream>
#endif /* DEBUG */

#include <iostream>

#include "dispatcher.h"
#include "utils.h"

Scheduler::Scheduler() {}

int
Scheduler::set_quantum_usecs(int quantum_usecs)
{
  m_quantum_usecs = quantum_usecs;
  return 0;
}

tcb_ptr
Scheduler::operator[](int tid)
{
  if (tid < 0 or static_cast<size_t>(tid) > m_threads.size()) {
    /* out of bounds - no such thread. */
    return nullptr;
  }

  return m_threads[tid];
}

int
Scheduler::add_thread(tcb_ptr new_thread)
{
  int tid = 0;

  for (tid = 0; static_cast<size_t>(tid) < m_threads.size(); tid++) {
    if (m_threads[tid] == nullptr) {
      m_threads[tid] = new_thread;
      goto set_state;
    }
  }

  m_threads.push_back(new_thread);
  tid = get_last_tid();

set_state:
  m_threads[tid]->set_state(READY);
  m_threads[tid]->m_tid = tid;
  m_ready.push_back(tid);
  m_number_of_threads++;
  return tid;
}

int
Scheduler::get_last_tid(void) const
{
  return m_threads.size() - 1;
}

int
Scheduler::get_running_tid(void) const
{
  return m_running_tid;
}

void
Scheduler::block(int tid)
{
  switch (m_threads[tid]->get_state()) {
    case BLOCKED:
      /*
       * already blocked, there is nothing to do.
       */
      break;
    case RUNNING:
      m_threads[tid]->set_state(BLOCKED);
      this->schedule();
      break;
    case SLEEPING:
      m_threads[tid]->set_state(BLOCKED);
      break;
    case READY:
      m_threads[tid]->set_state(BLOCKED);
      m_ready.remove(tid);
      break;
    default:
      break;
  }
}

void
Scheduler::sleep(int tid, int num_quantums)
{
  m_threads[tid]->sleep(num_quantums);
  m_threads[tid]->set_state(SLEEPING);
  m_sleeping.push_back(tid);
  m_threads[tid]->m_is_sleeping = true;

  /*
   * this is the running thread - shouldn't be in the ready queue.
   */
  m_ready.remove(tid);

  set_alarm();
  m_quantums_passed += 1;
}

void
Scheduler::set_passed_quantums(int initial)
{
  m_quantums_passed = initial;
}
int
Scheduler::get_passed_quantums(void) const
{
  return m_quantums_passed;
}

void
Scheduler::switch_running_thread(void)
{
  m_running_tid = m_ready.front();
  m_ready.pop_front();
  get_running_thread()->set_state(RUNNING);
  get_running_thread()->reset_quantums_in_running();
  get_running_thread()->reset_usec_in_running();

  enable_signals();
  Dispatcher::resume(get_running_thread());
}

void
Scheduler::schedule(void)
{
  unsigned int tid;

  if (m_threads[m_running_tid] == nullptr) {
    /* current thread have terminated */
    switch_running_thread();
  } else if ((m_threads[m_running_tid]->get_state() == BLOCKED) or
             (m_threads[m_running_tid]->get_state() == SLEEPING)) {
    tid = Dispatcher::save(m_threads[m_running_tid]);
    if (tid == 0) {
      switch_running_thread();
    }
    /*
     } else if (get_running_thread()->get_quantums_in_running() >=
               m_quantum_usecs) {
    */
  } else if (get_running_thread()->get_usec_in_running() >= m_quantum_usecs) {
    tid = Dispatcher::save(get_running_thread());
    if (tid == 0) {
      m_ready.push_back(m_running_tid);
      get_running_thread()->set_state(READY);
      switch_running_thread();
    }
  }
}

void
Scheduler::periodic_schedule(void)
{
  std::list<int> wakeup;

  m_quantums_passed += 1;

  // std::cout << "Scheduler::periodic_schedule: total quantums passed is = "
  //	  << m_quantums_passed << std::endl;

  /*
#ifdef DEBUG
  std::cout << "Scheduler::periodic_schedule was called for the  "
            << m_quantums_passed << " time." << std::endl;
#endif
  */

  m_threads[m_running_tid]->update_quantums(1);

  for (auto& sleeping : m_sleeping) {
    if (m_threads[sleeping]->get_state() == SLEEPING) {
      /*
       * don't increament sleeping & blocked threads amount of passed
       * quantums.
       */
      m_threads[sleeping]->update_sleeping(1);
      if (m_threads[sleeping]->get_remaining_sleep() == 0) {
        wakeup.push_back(sleeping);
      }
    }
  }

  for (auto& wake : wakeup) {
    m_sleeping.remove(wake);
    m_ready.push_back(wake);
    m_threads[wake]->set_state(READY);
    m_threads[wake]->m_is_sleeping = false;
  }

  this->schedule();
}

struct sigaction*
Scheduler::get_sigaction(void)
{
  return &m_sa;
}

struct itimerval*
Scheduler::get_timer(void)
{
  return &m_timer;
}

int
Scheduler::get_nunmber_of_threads(void) const
{
  return m_number_of_threads;
}

void
Scheduler::run(void)
{
  m_threads[0]->set_state(RUNNING);
  m_running_tid = 0;
  m_ready.pop_front();

  // /*
  //  * jump to the control thread.
  //  */
  // get_running_thread()->reset_quantums_in_running();
  // get_running_thread()->reset_usec_in_running();
  // Dispatcher::resume(get_running_thread());
}

void
Scheduler::remove(int tid)
{
  bool remove_running = false;

  switch (m_threads[tid]->get_state()) {
    case READY:
      m_ready.remove(tid);
      break;
    case BLOCKED:
      break;
    case SLEEPING:
      m_sleeping.remove(tid);
      break;
    case RUNNING:
      remove_running = true;
      break;
    default:
      break;
  }

  /* delete thread #tid control block */
  m_threads[tid] = nullptr;

  m_number_of_threads--;

  if (remove_running) {
    /*
     * removing the running thread should trigger a scheduling decision.
     */
    this->schedule();
  }
}

void
Scheduler::resume(int tid)
{
  switch (m_threads[tid]->get_state()) {
    case SLEEPING:
      /*
       * according the forum a sleeping & unblocked thread won't
       * change it's state when he's the subject of a uthread_resume
       * call.
       */
      break;
    case BLOCKED:
      if (m_threads[tid]->m_is_sleeping) {
        m_threads[tid]->set_state(SLEEPING);
      } else {
        m_threads[tid]->set_state(READY);
        m_ready.push_back(tid);
      }
      break;
    case RUNNING:
      /*
       * fallthrough
       */
    case READY:
      /*
       * for a running/ready thread resume won't do anything.
       */
      break;
    default:
      break;
  }
}

void
Scheduler::cleanup(void)
{
  for (size_t i = 0; i < m_threads.size(); i++) {
    /* delete thread #i */
    m_threads[i] = nullptr;
  }

  m_ready.clear();
  m_sleeping.clear();
  m_threads.clear();
}

int
Scheduler::set_alarm(void)
{
  struct itimerval* timer = this->get_timer();

  if (setitimer(ITIMER_VIRTUAL, timer, NULL) == -1) {
    /* failed to set alarm/timer */
    fprintf(stderr, "system error: %s\n", "setitimer failed.");
    utils::cleanup(1);
    return -1;
  }

  disable_signals();

  return 0;
}
