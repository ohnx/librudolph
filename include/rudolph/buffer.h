#ifndef __BUFFER_H_INC
#define __BUFFER_H_INC
#ifdef RUDOLF_USE_STDLIB
/* for size_t */
#include <stddef.h>
#else
#include <rudolph/own_stdlib.h>
#endif

#define __RD_BUF_DEFAULT 256

#define RD_E_BUF_OK         0
#define RD_E_BUF_OVERFLOW   -1
#define RD_E_BUF_OOM        -2
#define RD_E_BUF_NONSENSE   -4

#define rd_buffer_data(x) (unsigned char *)(x+1)

typedef struct _rd_buf_t {
    size_t len;
    size_t alloc;
} rd_buf_t;

/* allocate space for a new buffer */
__inline rd_buf_t *rd_buffer_init();
rd_buf_t *rd_buffer_initsz(size_t size);
/* free the buffer space */
void rd_buffer_free(rd_buf_t *buf);
/* push some data onto the buffer. if invalid buffer specified, a new buffer will be made. */
int rd_buffer_push(rd_buf_t **pbuf, const unsigned char *data, size_t len);
/* merge N buffers together into the 1st buffer */
int rd_buffer_merge(rd_buf_t **pbuf, size_t n, ...);
/* reset the length of the buffer */
void rd_buffer_reset(rd_buf_t *buf);

#endif /* __BUFFER_H_INC */
