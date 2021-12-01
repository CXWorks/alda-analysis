//
// Created by cxworks on 2020/6/16.
//

//#ifndef AUTO_ANALYSIS_MSAN_HEADER_H
//#define AUTO_ANALYSIS_MSAN_HEADER_H

#include <unistd.h>
#include <sys/socket.h>
#include <stdarg.h>
#include <sys/types.h>          /* See NOTES */
//typedef char int8;
//typedef short int16;
//typedef int int32;
//typedef long long int64;
#define COMMON_INTERCEPTOR_INITIALIZE_RANGE(ptr, size) \
  COMMON_INTERCEPTOR_WRITE_RANGE(ptr, size);

#define COMMON_INTERCEPTOR_READ_STRING(s, n)                   \
    COMMON_INTERCEPTOR_READ_RANGE((s), strlen(s) + 1 )

#define COMMON_INTERCEPTOR_COPY_STRING(to, from, size) \
    do{                                                \
        COMMON_INTERCEPTOR_COPY_RANGE(to, from, size);            \
        COMMON_INTERCEPTOR_WRITE_RANGE(&(((char*)to)[size]), 1);           \
    }while (false)

#define COMMON_SYSCALL_PRE_READ_RANGE(p, s) COMMON_INTERCEPTOR_READ_RANGE(p, s)

#define COMMON_SYSCALL_POST_WRITE_RANGE(p, s) COMMON_INTERCEPTOR_WRITE_RANGE(p, s)

typedef unsigned long SIZE_T;
struct __sanitizer_iovec {
    void *iov_base;
    unsigned long iov_len;
};

typedef struct msan_valist {
    unsigned int gp;
    unsigned int fp;
    void *overflow;
    void *reg;
} msan_valist;

typedef struct __sanitizer_tm {
    int tm_sec;
    int tm_min;
    int tm_hour;
    int tm_mday;
    int tm_mon;
    int tm_year;
    int tm_wday;
    int tm_yday;
    int tm_isdst;
    long int tm_gmtoff;
    const char *tm_zone;
} __sanitizer_tm;

typedef struct stat stat_t;


typedef struct {
    unsigned long fds_bits[1024 / (8 * sizeof(long))];
} __sanitizer___kernel_fd_set;

typedef struct __sanitizer__obstack_chunk {
    char *limit;
    struct __sanitizer__obstack_chunk *prev;
} __sanitizer__obstack_chunk;

typedef struct __sanitizer_obstack {
    long chunk_size;
    struct __sanitizer__obstack_chunk *chunk;
    char *object_base;
    char *next_free;
    unsigned long long more_fields[7];
} __sanitizer_obstack;

typedef struct __sanitizer_FILE {
    int _flags;
    char *_IO_read_ptr;
    char *_IO_read_end;
    char *_IO_read_base;
    char *_IO_write_base;
    char *_IO_write_ptr;
    char *_IO_write_end;
    char *_IO_buf_base;
    char *_IO_buf_end;
    char *_IO_save_base;
    char *_IO_backup_base;
    char *_IO_save_end;
    void *_markers;
    struct __sanitizer_FILE *_chain;
    int _fileno;
} __sanitizer_FILE;


typedef struct __sanitizer_dirent {
    unsigned long long d_ino;
    unsigned long long d_off;
    unsigned short d_reclen;
    // more fields that we don't care about
} __sanitizer_dirent;

static void handle_mmap(int64 pt, int64 size){
    void* p = (void*)pt;
    if(p!=(void *)-1){
        COMMON_INTERCEPTOR_WRITE_RANGE(p, (size / getpagesize() + size%getpagesize() == 0?0:1) * getpagesize());
    }
}


static void write_iovec(int64 iovecv_l, SIZE_T iovlen, SIZE_T maxlen) {
    void* iovecv = (void*)iovecv_l;
    __sanitizer_iovec *iovec = (__sanitizer_iovec *) iovecv;
    for (SIZE_T i = 0; i < iovlen && maxlen; ++i) {
        SIZE_T sz = min(iovec[i].iov_len, maxlen);
        COMMON_INTERCEPTOR_WRITE_RANGE(iovec[i].iov_base, sz);
        maxlen -= sz;
    }
}

static void handle_recvmsg(int64 msgp, int64 size){
    struct msghdr* msg = (struct msghdr*)msgp;
    COMMON_INTERCEPTOR_WRITE_RANGE(msg, sizeof(struct msghdr));
    if(msg->msg_control)
        COMMON_INTERCEPTOR_WRITE_RANGE(msg->msg_control, msg->msg_controllen);
    if(msg->msg_name)
        COMMON_INTERCEPTOR_WRITE_RANGE(msg->msg_name, msg->msg_namelen);
    write_iovec((int64)msg->msg_iov, msg->msg_iovlen, size);
}

static void read_iovec(int64 iovecv_l, SIZE_T iovlen, SIZE_T maxlen) {
    void* iovecv = (void*)iovecv_l;
    __sanitizer_iovec *iovec = (__sanitizer_iovec *) iovecv;
    COMMON_INTERCEPTOR_READ_RANGE(iovec, sizeof(*iovec) * iovlen);
    for (SIZE_T i = 0; i < iovlen && maxlen; ++i) {
        SIZE_T sz = min(iovec[i].iov_len, maxlen);
        COMMON_INTERCEPTOR_READ_RANGE(iovec[i].iov_base, sz);
        maxlen -= sz;
    }
}

static void initialize_obstack(address obstack) {
    COMMON_INTERCEPTOR_INITIALIZE_RANGE((void*)obstack, sizeof(__sanitizer_obstack));
    if (((__sanitizer_obstack *) obstack)->chunk)
        COMMON_INTERCEPTOR_INITIALIZE_RANGE(((__sanitizer_obstack *) obstack)->chunk,
                                            sizeof(__sanitizer__obstack_chunk));
}

static void unpoison_file(int64 fpv_l) {
    void * fpv = (void*)fpv_l;
    __sanitizer_FILE *fp = (__sanitizer_FILE *) fpv;
    COMMON_INTERCEPTOR_INITIALIZE_RANGE(fp, sizeof(*fp));
    if (fp->_IO_read_base && fp->_IO_read_base < fp->_IO_read_end)
        COMMON_INTERCEPTOR_INITIALIZE_RANGE(fp->_IO_read_base,
                                            fp->_IO_read_end - fp->_IO_read_base);
}

static void on_ob_newchunk(int64 obstack_l) {
    void* obstack = (void*)obstack_l;
    if (((__sanitizer_obstack *) obstack)->chunk) {

        COMMON_INTERCEPTOR_INITIALIZE_RANGE(
                ((__sanitizer_obstack *) obstack)->chunk,
                ((__sanitizer_obstack *) obstack)->next_free - (char *) ((__sanitizer_obstack *) obstack)->chunk);
    }
}

static void handle_sockopt(ull v1, ull v2){
    void * p = (void *)v1;
    socklen_t * s = (socklen_t*)v2;
    if(p && *s > 0){
        COMMON_INTERCEPTOR_WRITE_RANGE(p, *s);
    }
}

static ull on_binary_op(uint op, ull v1, ull v2, ull t1, ull t2) {
    switch (op) {
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
            return t1;
        case 23:
            return t2 == 0 ? (t1 << v2) : t2;
        case 24:
        case 25:
            return t2 == 0 ? (t1 >> v2) : t2;
        case 26:
        case 27:
            return (t1 & t2) | ((v1 & t2) | (t1 & v2));
        default:
            return t1 | t2;
    }
}

static void on_read_dir(int64 res_l) {
    void * res = (void*) res_l;
    if (res) COMMON_INTERCEPTOR_WRITE_RANGE(res, ((__sanitizer_dirent *) res)->d_reclen);
}

static void handle_accept(int64 res, int64 t_l) {
    void* t = (void*)t_l;
    socklen_t* size = (socklen_t*)res;
    COMMON_INTERCEPTOR_WRITE_RANGE(t, *size);
}

static void on_time(unsigned long long res, int64 t_l) {
    void* t = (void*)t_l;
    if (t && res != (unsigned long) -1) {
        COMMON_INTERCEPTOR_WRITE_RANGE(t, sizeof(unsigned long));
    }
}

inline static ull min(uint a, ull b) {
    return a < b ? a : b;
}

inline static int ptr_dis(int64 a, int64 b) {
    return (char *) a - (char *) b;
}


inline static void handle_va_start(int64 p_l, int64 tls_l, int64 va_num) {
    void* p = (void *)p_l;
    void* tls = (void *)tls_l;
    COMMON_INTERCEPTOR_WRITE_RANGE(p, sizeof(msan_valist));
    msan_valist *mva = (msan_valist *) p;
    int regs = (48 - mva->gp) / 8;
    int real_in_reg = regs > va_num ? va_num : regs;
    memcpy(&addr2Label.get((char *)mva->reg + mva->gp), tls, real_in_reg * 8);
    if(va_num - real_in_reg > 0)
        memcpy(&addr2Label.get(mva->overflow), (void *) ((ull) tls + real_in_reg * 8), (va_num - real_in_reg) * 8);
}

//#endif //AUTO_ANALYSIS_MSAN_HEADER_H
