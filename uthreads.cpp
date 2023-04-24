#include "uthreads.h"

#include <signal.h>   /* for struct sigaction, sigaction */
#include <sys/time.h> /* for struct itimerval */

#include <cstddef> /* for size_t */
#include <cstdint> /* for uint8_t */
#include <cstdio>  /* for fprintf, stderr */
#include <cstdlib> /* for exit */
#include <cstring> /* for memset */
#include <list>    /* for std::list */
#include <memory>  /* for std::shared_ptr, std::make_shared */
#include <queue>   /* for std::queue */
#include <vector>  /* for std::vector */

#include "dispatcher.h"
#include "globals.h"
#include "scheduler.h"
#include "thread_control_block.h"
#include "utils.h"

static int
_uthread_spawn_unsafe(thread_entry_point entry_point, bool is_main = false);

/* Globlas */
// Scheduler g_scheduler;

/* Library Implementation. */

static void
schedule(void)
{
  g_scheduler.schedule();
}

static void
periodic_schedule(int signum)
{
  UNUSED(signum);

  disable_signals();
  g_scheduler.set_alarm();

  CRITICAL_SECTION(g_scheduler.periodic_schedule(););
}

/*
void
uthread_control_thread(void)
{
  printf("uthread_control_thread: Look I'm a real running thread!\n");

  while (g_scheduler.get_nunmber_of_threads() > 1) {
  }

  utils::cleanup(EXIT_SUCCESS);
}
*/

int
uthread_init(int quantum_usecs)
{
  struct sigaction* sa = g_scheduler.get_sigaction();
  struct itimerval* timer = g_scheduler.get_timer();

  if (quantum_usecs <= 0) {
    /* invalid input */
    fprintf(stderr, "thread library error: %s\n", "quantum_usecs <= 0");
    return -1;
  }

  if (initialize_signal_masks() != 0) {
    utils::cleanup(1);
  }

  (void)memset(sa, 0, sizeof(struct sigaction));
  sa->sa_handler = &periodic_schedule;

  // TODO: check return code.
  // sigfillset(&(sa->sa_mask));

  // (void)memset(&g_sa, 0, sizeof(struct sigaction));
  // g_sa.sa_handler = &periodic_schedule;
  if (sigaction(SIGVTALRM, sa, NULL) < 0) {
    // if (sigaction(SIGVTALRM, &g_sa, NULL) < 0) {
    /* sigaction failed, error is stored in errno */
    fprintf(stderr, "system error: %s\n", "sigaction failed.");
    utils::cleanup(1);
    return -1;
  }

  /* configure the timer to expire every quantum_usecs micro-seconds. */
  // it_value can't be all 0's since it causes the it_interval to never
  // expire...
  timer->it_value.tv_sec = 0;
  // timer->it_value.tv_usec = 1;
  timer->it_value.tv_usec = quantum_usecs;
  timer->it_interval.tv_sec = 0;
  // timer->it_interval.tv_usec = quantum_usecs;
  timer->it_interval.tv_usec = 0;
  // timer->it_interval.tv_usec = 1;

  if (setitimer(ITIMER_VIRTUAL, timer, NULL) == -1) {
    /* failed to set alarm/timer */
    fprintf(stderr, "system error: %s\n", "setitimer failed.");
    utils::cleanup(1);
    return -1;
  }

  CRITICAL_SECTION(
    g_scheduler.set_passed_quantums(1);
    if (g_scheduler.set_quantum_usecs(quantum_usecs) != 0) {
      fprintf(stderr, "thread library error: %s\n", "quantum_usecs <= 0");
      return -1;
    }

    /*
    if (uthread_spawn(uthread_control_thread) == -1) {
      fprintf(stderr, "thread library error: %s\n", "quantum_usecs <= 0");
      return -1;
    }
    */
    if (_uthread_spawn_unsafe(NULL, true) == -1) {
      fprintf(stderr, "thread library error: %s\n", "quantum_usecs <= 0");
      return -1;
    }

    g_scheduler.run();

  );

  // g_scheduler.run();

  return 0;
}

void
uthread_run(void)
{
  g_scheduler.run();
}

int
_uthread_spawn_unsafe(thread_entry_point entry_point, bool is_main)
{
  int tid;

  if (g_scheduler.get_nunmber_of_threads() >= MAX_THREAD_NUM) {
    fprintf(
      stderr, "thread library error: %s\n", "reached max number of threads");
    return -1;
  }

  CRITICAL_SECTION(
    tid = g_scheduler.add_thread(std::make_shared<TCB>(nullptr, is_main));
    g_scheduler[tid]->set_entry_point(entry_point);

    address_t sp =
      (address_t)g_scheduler[tid]->get_stack() + STACK_SIZE - sizeof(address_t);
    address_t pc = (address_t)entry_point;

    sigsetjmp(g_scheduler[tid]->m_env, 1);
    (g_scheduler[tid]->m_env->__jmpbuf)[JB_SP] = translate_address(sp);
    (g_scheduler[tid]->m_env->__jmpbuf)[JB_PC] = translate_address(pc);
    sigemptyset(&(g_scheduler[tid]->m_env->__saved_mask)););

  return tid;
}

int
uthread_spawn(thread_entry_point entry_point)
{
  if (!entry_point) {
    fprintf(stderr,
            "thread library error: %s\n",
            "entry_point must be a valid pointer");
    return -1;
  }

  return _uthread_spawn_unsafe(entry_point);
}

int
uthread_terminate(int tid)
{
  if (g_scheduler[tid] == nullptr) {
    fprintf(stderr, "thread library error: %s\n", "quantum_usecs <= 0");
    return -1;
  }

  if (tid == 0) {
    utils::cleanup(0);
  }

  CRITICAL_SECTION(g_scheduler.remove(tid););

  return 0;
}

int
uthread_block(int tid)
{
  if (g_scheduler[tid] == nullptr) {
    fprintf(stderr, "thread library error: %s\n", "quantum_usecs <= 0");
    return -1;
  }

  if (0 == tid) {
    /* can't block the control thread. */
    fprintf(
      stderr, "thread library error: %s\n", "there ain't rest for the wicked");
    return -1;
  }

  CRITICAL_SECTION(g_scheduler.block(tid););

  return 0;
}

int
uthread_resume(int tid)
{
  if (g_scheduler[tid] == nullptr) {
    fprintf(stderr, "thread library error: %s\n", "quantum_usecs <= 0");
    return -1;
  }

  if (tid == 0) {
    fprintf(stderr, "thread library error: %s\n", "can't resume tid 0");
    return -1;
  }

  CRITICAL_SECTION(g_scheduler.resume(tid););

  return 0;
}

int
uthread_get_tid()
{
  return g_scheduler.get_running_tid();
}

int
uthread_sleep(int num_quantums)
{
  int tid = g_scheduler.get_running_tid();

  if (num_quantums < 0) {
    fprintf(stderr, "thread library error: %s\n", "num_quantums < 0");
    return -1;
  }

  if (0 == tid) {
    /* the control thread can't sleep. */
    fprintf(
      stderr, "thread library error: %s\n", "there ain't rest for the wicked");
    return -1;
  }

  CRITICAL_SECTION(g_scheduler.sleep(tid, num_quantums);

                   /*
                    * call the scheduler.
                    * resume when the sleep time is over.
                    */
                   schedule(););

  return 0;
}

int
uthread_get_total_quantums()
{
  return g_scheduler.get_passed_quantums();
}

int
uthread_get_quantums(int tid)
{
  if (g_scheduler[tid] == nullptr) {
    fprintf(stderr, "thread library error: %s\n", "quantum_usecs <= 0");
    return -1;
  }

  return g_scheduler[tid]->get_quantums();
}
