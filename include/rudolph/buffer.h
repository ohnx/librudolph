#ifndef __BUFFER_H_INC
#define __BUFFER_H_INC

#define BUF_DEFAULT 256

#define E_BUF_OK        0
#define E_BUF_OVERFLOW  -1
#define E_BUF_OOM       -2
#define E_BUF_NONSENSE  -4

#define buffer_data(x) (unsigned char *)(x+1)

typedef struct _buf_t {
    size_t len;
    size_t alloc;
} buf_t;

/* allocate space for a new buffer */
__inline buf_t *buffer_init();
buf_t *buffer_initsz(size_t size);
/* free the buffer space */
void buffer_free(buf_t *buf);
/* push some data onto the buffer. if invalid buffer specified, a new buffer will be made. */
int buffer_push(buf_t **pbuf, const unsigned char *data, size_t len);
/* merge N buffers together into the 1st buffer */
int buffer_merge(buf_t **pbuf, size_t n, ...);
/* reset the length of the buffer */
void buffer_reset(buf_t *buf);

#endif /* __BUFFER_H_INC */
