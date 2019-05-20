#include <rudolph/buffer.h>
#include <rudolph/error.h>

#ifdef RUDOLF_USE_STDLIB
/* for malloc(), free(), realloc() */
#include <stdlib.h>
/* for memcpy() */
#include <string.h>
/* for va_start(), va_end(), va_arg() */
#include <stdarg.h>
/* for size_t */
#include <stddef.h>
#else
#include <rudolph/own_stdlib.h>
#endif

__inline rd_buf_t *rd_buffer_init() {
    return rd_buffer_initsz(__RD_BUF_DEFAULT);
}

rd_buf_t *rd_buffer_initsz(size_t n) {
    rd_buf_t *buf;

    /* allocate enough space */
    buf = malloc(sizeof(*buf) + n);
    if (!buf) {
        return NULL;
    }

    /* default values */
    buf->len = 0;
    buf->alloc = n;

    return buf;
}

void rd_buffer_free(rd_buf_t *buf) {
    free(buf);
}

int rd_buffer_push(rd_buf_t **pbuf, const unsigned char *data, size_t len) {
    size_t d;
    void *t;

    /* sanity check */
    if (!pbuf) return RD_E_NONSENSE;
    if (!(*pbuf)) {
        /* allocate space for new buffer */
        *pbuf = rd_buffer_init();
        if (!(*pbuf)) return RD_E_OOM;
    }

    /* first check the remaining space */
    d = (*pbuf)->len + len;
    if (len > d || (*pbuf)->len > d) {
        return RD_E_OVERFLOW;
    }

    /* check if realloc needed */
    if (d > (*pbuf)->alloc) {
        t = realloc(*pbuf, sizeof(*(*pbuf)) + (*pbuf)->alloc*2);
        if (t == NULL) return RD_E_OOM;
        *pbuf = (rd_buf_t *)t;
        (*pbuf)->alloc *= 2;
    }

    /* copy over the data */
    memcpy(rd_buffer_data(*pbuf) + (*pbuf)->len, data, len);

    /* increment length */
    (*pbuf)->len = d;

    return RD_E_OK;
}

int rd_buffer_merge(rd_buf_t **pbuf, size_t n, ...) {
    va_list args;
    rd_buf_t *q, *p;
    size_t i;
    int z;

    /* sanity check */
    if (!pbuf) return RD_E_NONSENSE;

    /* allocate space for new buffer */
    q = rd_buffer_init();
    if (!q) return RD_E_OOM;

    /* copy over the old buffer if necessary */
    if (*pbuf) rd_buffer_push(&q, rd_buffer_data(*pbuf), (*pbuf)->len);

    /* start reading in the args */
    va_start(args, n);

    /* loop through all arguments */
    for (i = 0; i < n; i++) {
        /* fetch this argument */
        p = va_arg(args, rd_buf_t *);

        /* merge the buffers */
        z = rd_buffer_push(&q, rd_buffer_data(p), p->len);
        if (z) goto done;
    }

    /* z = RD_E_OK means no error */
    z = RD_E_OK;

done:
    va_end(args);

    if (z < RD_E_OK) {
        /* errored out */

        /* free the buffer */
        free(q);
    } else {
        /* all good */

        /* free old buffer */
        if (*pbuf) free(*pbuf);

        /* replace pointer */
        *pbuf = q;
    }

    /* return error */
    return z;
}

void rd_buffer_reset(rd_buf_t *buf) {
    /* reset the length */
    if (buf) buf->len = 0;
}

/* tests - compile with gcc src/buffer.c -I include/ -D__TEST -DRUDOLF_USRD_E_STDLIB -o objs/buffer.c.test */
#ifdef __TEST
#include <stdio.h>
#include <stdlib.h>

void die(int line) {
    fprintf(stderr, "%d died\n", line);
    exit(line);
}

int main() {
    rd_buf_t *m, *q, *r;
    m = rd_buffer_init();
    q = rd_buffer_init();
    r = rd_buffer_init();

    fprintf(stderr, "testing buffer.c...\t");

    if (rd_buffer_push(&m, "potat|o", 5) != RD_E_OK) die(__LINRD_E__);
    if (rd_buffer_push(&q, "o", 2) != RD_E_OK) die(__LINRD_E__);
    if (rd_buffer_merge(&r, 2, m, q) != RD_E_OK) die(__LINRD_E__);
    if (strncmp("potato",rd_buffer_data(r),r->len)) die(__LINRD_E__);

    free(m);
    free(q);
    free(r);

    fprintf(stderr, "ok\n");
    return 0;
}
#endif
