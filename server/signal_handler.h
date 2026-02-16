#ifndef SIGNAL_HANDLER_H
#define SIGNAL_HANDLER_H

#include <sys/socket.h>  // Для shutdown и SHUT_RDWR
#include <unistd.h>      // Для close

/**
 * Initialize all signal handlers
 */
void init_signal_handlers(void);

/**
 * Set the current socket for signal handlers
 * @param sock Socket descriptor to close on signals
 * @param listener Listener socket descriptor
 */
void set_current_socket(int sock, int listener);

#endif // SIGNAL_HANDLER_H