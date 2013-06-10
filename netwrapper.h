#include <unistd.h>
#include <errno.h>

extern ssize_t readline(int fd, void *vptr, size_t maxlen);
extern ssize_t writen(int fd, const void *vptr, size_t n);
