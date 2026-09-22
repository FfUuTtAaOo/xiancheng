#include "ringbuf.h"

/* ---- Internal helpers ---- */
static inline uint16_t rb_mask(const ringbuf_t *rb) { return rb->size - 1U; }

/* ================================================================
 *  Public API
 * ================================================================ */

void ringbuf_init(ringbuf_t *rb, volatile uint8_t *buf, uint16_t size)
{
    rb->buf  = buf;
    rb->size = size;
    rb->head = 0;
    rb->tail = 0;
}

void ringbuf_reset(ringbuf_t *rb)
{
    rb->head = 0;
    rb->tail = 0;
}

uint16_t ringbuf_avail(const ringbuf_t *rb)
{
    return (uint16_t)(rb->head - rb->tail);   /* wraps naturally via u16 */
}

uint16_t ringbuf_free(const ringbuf_t *rb)
{
    return (uint16_t)(rb->size - ringbuf_avail(rb) - 1U);
}

void ringbuf_put(ringbuf_t *rb, uint8_t byte)
{
    uint16_t next = (uint16_t)(rb->head + 1U);
    if (next == rb->tail) return;            /* full — drop              */
    ((uint8_t *)rb->buf)[rb->head & rb_mask(rb)] = byte;
    rb->head = next;
}

uint8_t ringbuf_get(ringbuf_t *rb)
{
    uint8_t byte = rb->buf[rb->tail & rb_mask(rb)];
    rb->tail = (uint16_t)(rb->tail + 1U);
    return byte;
}

uint8_t ringbuf_peek(const ringbuf_t *rb, uint16_t ofs)
{
    return rb->buf[(uint16_t)(rb->tail + ofs) & rb_mask(rb)];
}

void ringbuf_write(ringbuf_t *rb, const uint8_t *src, uint16_t len)
{
    uint16_t mask = rb_mask(rb);
    while (len--) {
        uint16_t next = (uint16_t)(rb->head + 1U);
        if (next == rb->tail) return;                    /* full          */
        ((uint8_t *)rb->buf)[rb->head & mask] = *src++;
        rb->head = next;
    }
}

uint16_t ringbuf_read(ringbuf_t *rb, uint8_t *dst, uint16_t len)
{
    uint16_t mask = rb_mask(rb);
    uint16_t n = ringbuf_avail(rb);
    if (len > n) len = n;
    for (uint16_t i = 0; i < len; i++) {
        dst[i] = rb->buf[rb->tail & mask];
        rb->tail = (uint16_t)(rb->tail + 1U);
    }
    return len;
}

void ringbuf_skip(ringbuf_t *rb, uint16_t len)
{
    uint16_t n = ringbuf_avail(rb);
    if (len > n) len = n;
    rb->tail = (uint16_t)(rb->tail + len);
}
