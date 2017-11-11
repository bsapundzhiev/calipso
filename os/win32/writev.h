#ifdef WIN32

/* same as WSABUF */
struct iovec {
    u_long iov_len;
    char *iov_base;
};

#ifndef inline
#define inline __inline
#endif

static inline int writev(int sock, struct iovec *iov, int nvecs)
{
    DWORD ret;
    if (WSASend(sock, (LPWSABUF)iov, nvecs, &ret, 0, NULL, NULL) == 0) {
        return ret;
    }
    return -1;
}

#endif /* WIN32 */
