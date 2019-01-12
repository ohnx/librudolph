#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <rudolph/buffer.h>

__inline buf_t *buffer_init() {
    return buffer_initsz(BUF_DEFAULT);
}

buf_t *buffer_initsz(size_t n) {
    buf_t *buf;

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

void buffer_free(buf_t *buf) {
    free(buf);
}

int buffer_push(buf_t **pbuf, const unsigned char *data, size_t len) {
    size_t d;
    void *t;

    /* sanity check */
    if (!pbuf) return E_BUF_NONSENSE;
    if (!(*pbuf)) {
        /* allocate space for new buffer */
        *pbuf = buffer_init();
        if (!(*pbuf)) return E_BUF_OOM;
    }

    /* first check the remaining space */
    d = (*pbuf)->len + len;
    if (len > d || (*pbuf)->len > d) {
        return E_BUF_OVERFLOW;
    }

    /* check if realloc needed */
    if (d > (*pbuf)->alloc) {
        t = realloc(*pbuf, sizeof(*(*pbuf)) + (*pbuf)->alloc*2);
        if (t == NULL) return E_BUF_OOM;
        *pbuf = (buf_t *)t;
    }

    /* copy over the data */
    memcpy(buffer_data(*pbuf)+(*pbuf)->len, data, len);

    /* increment length */
    (*pbuf)->len = d;

    return E_BUF_OK;
}

int buffer_merge(buf_t **pbuf, size_t n, ...) {
    va_list args;
    buf_t *q, *p;
    size_t i;
    int z;

    /* sanity check */
    if (!pbuf) return E_BUF_NONSENSE;

    /* allocate space for new buffer */
    q = buffer_init();
    if (!q) return E_BUF_OOM;

    /* copy over the old buffer if necessary */
    if (*pbuf) buffer_push(&q, buffer_data(*pbuf), (*pbuf)->len);

    /* start reading in the args */
    va_start(args, n);

    /* loop through all arguments */
    for (i = 0; i < n; i++) {
        /* fetch this argument */
        p = va_arg(args, buf_t *);

        /* merge the buffers */
        z = buffer_push(&q, buffer_data(p), p->len);
        if (z) goto done;
    }

    /* z = E_BUF_OK means no error */
    z = E_BUF_OK;

done:
    va_end(args);

    if (z < E_BUF_OK) {
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

void buffer_reset(buf_t *buf) {
    /* reset the length */
    if (buf) buf->len = 0;
}

/* tests - compile with gcc src/buffer.c -I include/ -D__TEST -o objs/buffer.c.test */
#ifdef __TEST
#include <stdio.h>
#include <stdlib.h>

void die(int line) {
    fprintf(stderr, "%d died\n", line);
    exit(line);
}

int main() {
    buf_t *m, *q, *r;
    m = buffer_init();
    q = buffer_init();
    r = buffer_init();

    fprintf(stderr, "testing buffer.c...\t");

    if (buffer_push(&m, "potat|o", 5) != E_BUF_OK) die(__LINE__);
    if (buffer_push(&q, "o", 2) != E_BUF_OK) die(__LINE__);
    if (buffer_merge(&r, 2, m, q) != E_BUF_OK) die(__LINE__);
    if (strncmp("potato",buffer_data(r),r->len)) die(__LINE__);

    free(m);
    free(q);
    free(r);

    fprintf(stderr, "ok\n");
    return 0;
}
#endif
