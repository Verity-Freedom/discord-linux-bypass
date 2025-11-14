#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <fcntl.h>
#include <pthread.h>
#include "sockmgr.h"

#define STALE_SEC 30

static socket_entry_t *head = NULL;
static pthread_mutex_t list_lock = PTHREAD_MUTEX_INITIALIZER;

static time_t now_time(void) { return time(NULL); }

static void cleanup_entries(void) {
    pthread_mutex_lock(&list_lock);
    socket_entry_t **cur = &head;
    time_t now = now_time();
    while (*cur) {
        socket_entry_t *e = *cur;
        int stale = (now - e->last_seen) > STALE_SEC;
        if (stale) {
            *cur = e->next;
            free(e);
        } else {
            cur = &e->next;
        }
    }
    pthread_mutex_unlock(&list_lock);
}

void sm_add_fd(int fd) {
    cleanup_entries();
    pthread_mutex_lock(&list_lock);
    socket_entry_t *e = head;
    while (e) {
        if (e->fd == fd) {
            pthread_mutex_unlock(&list_lock);
            return;
        }
        e = e->next;
    }
    socket_entry_t *n = malloc(sizeof(socket_entry_t));
    n->fd = fd;
    n->sent = 0;
    n->last_seen = now_time();
    n->next = head;
    head = n;
    pthread_mutex_unlock(&list_lock);
}

static socket_entry_t *find_entry(int fd) {
    pthread_mutex_lock(&list_lock);
    socket_entry_t *e = head;
    while (e) {
        if (e->fd == fd) {
            pthread_mutex_unlock(&list_lock);
            return e;
        }
        e = e->next;
    }
    pthread_mutex_unlock(&list_lock);
    return NULL;
}

int sm_was_sent(int fd, socket_entry_t **sock) {
    *sock = find_entry(fd);
    if (*sock == NULL) return 0;

    if ((*sock)->sent == 0) {
        (*sock)->sent = 1;
        return 0;
    }
    return (*sock)->sent;
}

void sm_reset(int fd) {
    socket_entry_t *e = find_entry(fd);
    if (e) {
        pthread_mutex_lock(&list_lock);
        e->sent = 0;
        e->last_seen = now_time();
        pthread_mutex_unlock(&list_lock);
    }
}

void sm_cleanup_all(void) {
    pthread_mutex_lock(&list_lock);
    socket_entry_t *e = head;
    while (e) {
        socket_entry_t *tmp = e;
        e = e->next;
        free(tmp);
    }
    head = NULL;
    pthread_mutex_unlock(&list_lock);
}
