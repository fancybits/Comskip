#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>

#if defined(__APPLE__) && defined(__MACH__)
#include <execinfo.h>

#define MAX_STACK_FRAMES 64

void fallback_handler() {
    void *stack_frames[MAX_STACK_FRAMES];
    int num_frames = backtrace(stack_frames, MAX_STACK_FRAMES);
    char **symbols = backtrace_symbols(stack_frames, num_frames);

    fprintf(stderr, "Stack trace (most recent call first):\n");
    for (int i = 0; i < num_frames; ++i) {
        fprintf(stderr, "%s\n", symbols[i]);
    }

    free(symbols);
}
#else
void fallback_handler() {
}
#endif

#if defined(HAVE_LIBBACKTRACE)
#include <backtrace.h>

int empty_backtrace_full_callback(void *data, uintptr_t pc,
					const char *filename, int lineno,
					const char *function) {
    return 0;
}
void empty_backtrace_error_callback(void *data, const char *msg,
					  int errnum) {
    bool *error = (bool *)data;

    if (errnum == -1) {
        *error = true;
    }
}

bool backtrace_supported() {
    struct backtrace_state *s = backtrace_create_state(NULL, 0, NULL, NULL);
    if (s == NULL) {
        return false;
    }

    bool error;

    int res = backtrace_full(s, 0, empty_backtrace_full_callback,
                   empty_backtrace_error_callback, &error);


    return !error;
}

void crash_handler(int signum) {
#ifdef WIN32
    fprintf(stderr, "Received signal %d\n", signum);
#else
    fprintf(stderr, "Received signal %d (%s)\n", signum, strsignal(signum));
#endif

    if (backtrace_supported()) {
        struct backtrace_state *s = backtrace_create_state(NULL, 0, NULL, NULL);
        backtrace_print(s, 1, stderr);
    } else {
        fallback_handler();
    }

    _exit(signum);
}

#else

void crash_handler(int signum) {
    #ifdef WIN32
    fprintf(stderr, "Received signal %d\n", signum);
#else
    fprintf(stderr, "Received signal %d (%s)\n", signum, strsignal(signum));
#endif

    fallback_handler();

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
