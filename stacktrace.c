#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>

#if defined(HAVE_LIBBACKTRACE)
#include <backtrace.h>

void crash_handler(int signum) {
#ifdef WIN32
    fprintf(stderr, "Received signal %d\n", signum);
#else
    fprintf(stderr, "Received signal %d (%s)\n", signum, strsignal(signum));
#endif

    struct backtrace_state *s = backtrace_create_state(NULL, 0, NULL, NULL);

    backtrace_print(s, 1, stderr);
    _exit(signum);
}
#endif


void install_crash_handler() {
#if defined(HAVE_LIBBACKTRACE)

#ifdef SIGSEGV
    signal(SIGSEGV, crash_handler);
#endif
#ifdef SIGABRT
    signal(SIGABRT, crash_handler);
#endif
#ifdef SIGFPE
    signal(SIGFPE, crash_handler);
#endif
#ifdef SIGILL
    signal(SIGILL, crash_handler);
#endif
#ifdef SIGTRAP
    signal(SIGTRAP, crash_handler);
#endif

#endif
}

void crash() {
    int *ptr = NULL;
    *ptr = 42; // Dereference a NULL pointer to cause a crash
}
