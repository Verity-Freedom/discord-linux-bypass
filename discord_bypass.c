#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>

#define DISCORD_PACKET_SIZE 74
#define VESKTOP_PACKET_SIZE 100

// used in Vesktop
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen) {
    static ssize_t (*real_sendto)(int, const void*, size_t, int, const struct sockaddr*, socklen_t) = NULL;
    if (!real_sendto) {
        real_sendto = dlsym(RTLD_NEXT, "sendto");
        if (!real_sendto) {
            fprintf(stderr, "dlsym error: %s\n", dlerror());
            return -1;
        }
    }

    if (len == VESKTOP_PACKET_SIZE) {
        char payload = 0;
        real_sendto(sockfd, &payload, 1, 0, dest_addr, addrlen);
        char payload2 = 1;
        real_sendto(sockfd, &payload2, 1, 0, dest_addr, addrlen);
    }

    return real_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
}

// used in official Discord app
ssize_t sendmsg(int sockfd, const struct msghdr *msg, int flags) {
    static ssize_t (*real_sendmsg)(int, const struct msghdr *, int) = NULL;
    if (!real_sendmsg) {
        real_sendmsg = dlsym(RTLD_NEXT, "sendmsg");
        if (!real_sendmsg) {
            fprintf(stderr, "dlsym error: %s\n", dlerror());
            return -1;
        }
    }

    size_t total_len = 0;
    for (size_t i = 0; i < msg->msg_iovlen; i++) {
        total_len += msg->msg_iov[i].iov_len;
    }

    if (total_len == DISCORD_PACKET_SIZE) {
        char payload1 = 0;
        char payload2 = 1;

        struct iovec iov1 = { .iov_base = &payload1, .iov_len = 1 };
        struct iovec iov2 = { .iov_base = &payload2, .iov_len = 1 };

        struct msghdr new_msg;
        memset(&new_msg, 0, sizeof(new_msg));

        new_msg.msg_name = msg->msg_name;
        new_msg.msg_namelen = msg->msg_namelen;
        new_msg.msg_control = NULL;
        new_msg.msg_controllen = 0;
        new_msg.msg_flags = 0;

        new_msg.msg_iov = &iov1;
        new_msg.msg_iovlen = 1;
        real_sendmsg(sockfd, &new_msg, flags);

        new_msg.msg_iov = &iov2;
        new_msg.msg_iovlen = 1;
        real_sendmsg(sockfd, &new_msg, flags);
    }

    return real_sendmsg(sockfd, msg, flags);
}
