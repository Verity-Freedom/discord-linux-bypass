#include <time.h>

typedef struct socket_entry {
    int fd;
    int sent;
    time_t last_seen;
    struct socket_entry *next;
} socket_entry_t;

void sm_add_fd(int fd);
int sm_was_sent(int fd, socket_entry_t **sock);
void sm_reset(int fd);
void sm_cleanup_all(void);