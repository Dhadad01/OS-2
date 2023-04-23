#ifndef __UTILS_H_INCLUDED
#define __UTILS_H_INCLUDED

int initialize_signal_masks(void);
int disable_signals(void);
int enable_signals(void);

namespace utils {
	void cleanup(int exitcode);
};

/*
#define CRITICAL_SECTION(code) {          \
	int status;                       \
	status = disable_signals();       \
	if (status != 0) { exit(1); }     \
	code;                             \
	status = enable_signals();        \
	if (status != 0) { exit(1); }     \
}
*/

#define CRITICAL_SECTION(code) code

#ifdef __x86_64__
/* code for 64 bit Intel arch */
typedef unsigned long address_t;

#define JB_SP 6
#define JB_PC 7

#else  /* __x86_32__ */

/* code for 32 bit Intel arch */
typedef unsigned int address_t;

#define JB_SP 4
#define JB_PC 5

#endif /* __x86_64__ */

address_t translate_address(address_t addr);

#endif /* __UTILS_H_INCLUDED */
