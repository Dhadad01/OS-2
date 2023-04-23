#include "utils.h"
#include "globals.h"

#include <cstdlib>  /* for exit */
#include <errno.h>  /* for errno */
#include <signal.h> /* for sigset_t, sigemptyset, sigprocmask */
#include <stdio.h>  /* for fprintf, stderr */
#include <string.h> /* for strerror */

static sigset_t s_ignore_all_mask;
static sigset_t s_enable_all_mask;

int
initialize_signal_masks(void)
{
  int status;

  status = sigfillset(&s_ignore_all_mask);
  if (status != 0) {
    fprintf(stderr,
            "system error: sigfillset failed with error: %s\n",
            strerror(errno));
    return -1;
  }

  status = sigemptyset(&s_enable_all_mask);
  if (status != 0) {
    fprintf(stderr,
            "system error: sigemptyset failed with error: %s\n",
            strerror(errno));
    return -1;
  }

  return 0;
}

int
disable_signals(void)
{
  int status;

  status = sigprocmask(SIG_BLOCK, &s_ignore_all_mask, NULL);
  if (status != 0) {
    fprintf(stderr,
            "system error: sigprocmask failed with error: %s\n",
            strerror(errno));
    return -1;
  }

  return 0;
}

int
enable_signals(void)
{
  int status;

  status = sigprocmask(SIG_UNBLOCK, &s_enable_all_mask, NULL);
  if (status != 0) {
    fprintf(stderr,
            "system error: sigprocmask failed with error: %s\n",
            strerror(errno));
    return -1;
  }

  return 0;
}

namespace utils {
void
cleanup(int exitcode)
{
  g_scheduler.cleanup();
  exit(exitcode);
}

}

#ifdef __x86_64__
/* code for 64 bit Intel arch */

#define JB_SP 6
#define JB_PC 7

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t
translate_address(address_t addr)
{
  address_t ret;
  asm volatile("xor    %%fs:0x30,%0\n"
               "rol    $0x11,%0\n"
               : "=g"(ret)
               : "0"(addr));
  return ret;
}

#else
/* code for 32 bit Intel arch */

#define JB_SP 4
#define JB_PC 5

/* A translation is required when using an address of a variable.
   Use this as a black box in your code. */
address_t
translate_address(address_t addr)
{
  address_t ret;
  asm volatile("xor    %%gs:0x18,%0\n"
               "rol    $0x9,%0\n"
               : "=g"(ret)
               : "0"(addr));
  return ret;
}

#endif
