/**
 * Signal handling module for graceful shutdown and child process management
 */

#include "signal_handler.h"
#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>
#include <sys/socket.h>  // Добавлено для shutdown и SHUT_RDWR
#include <syslog.h>

static int current_socket = -1;
static int current_listener = -1;

/**
 * Signal handler for SIGINT - graceful shutdown
 * Note: Only async-signal-safe functions are used here
 */
static void sigint_handler(int signo) {
    (void)signo;  // Suppress unused parameter warning
    
    if (current_socket >= 0) {
        shutdown(current_socket, SHUT_RDWR);
        close(current_socket);
    }
    
    if (current_listener >= 0) {
        close(current_listener);
    }
    
    _exit(0);
}

/**
 * Signal handler for SIGCHLD - reap zombie processes
 */
static void sigchild_handler(int signo) {
    (void)signo;
    pid_t pid;
    int status;
    
    // Reap all terminated child processes without blocking
    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        // Child process terminated, just log it
        openlog("file_server", LOG_PID | LOG_CONS, LOG_DAEMON);
        syslog(LOG_INFO, "Child process %d terminated", pid);
        closelog();
    }
}

/**
 * Signal handler for SIGALRM - connection timeout
 */
static void sigalrm_handler(int signo) {
    (void)signo;
    
    openlog("file_server", LOG_PID | LOG_CONS, LOG_DAEMON);
    syslog(LOG_WARNING, "Connection timeout, closing socket");
    closelog();
    
    if (current_socket >= 0) {
        shutdown(current_socket, SHUT_RDWR);
        close(current_socket);
    }
    
    _exit(1);
}

void init_signal_handlers(void) {
    struct sigaction sa_int, sa_chld, sa_alrm;
    
    // Setup SIGINT handler
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);
    
    // Setup SIGCHLD handler
    sa_chld.sa_handler = sigchild_handler;
    sigemptyset(&sa_chld.sa_mask);
    sa_chld.sa_flags = SA_RESTART;  // Restart interrupted system calls
    sigaction(SIGCHLD, &sa_chld, NULL);
    
    // Setup SIGALRM handler
    sa_alrm.sa_handler = sigalrm_handler;
    sigemptyset(&sa_alrm.sa_mask);
    sa_alrm.sa_flags = 0;
    sigaction(SIGALRM, &sa_alrm, NULL);
}

void set_current_socket(int sock, int listener) {
    current_socket = sock;
    current_listener = listener;
}