#ifndef __RINGBUF_H__
#define __RINGBUF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ================================================================
 *  Lightweight lock-free byte ring buffer
 *  ================================================================ */
typedef struct {
    volatile uint8_t *buf;
    volatile uint16_t size;
    volatile uint16_t head;   /* write position   */
    volatile uint16_t tail;   /* read position    */
} ringbuf_t;

void     ringbuf_init  (ringbuf_t *rb, volatile uint8_t *buf, uint16_t size);
void     ringbuf_reset (ringbuf_t *rb);

uint16_t ringbuf_avail (const ringbuf_t *rb);          /* bytes readable    */
uint16_t ringbuf_free  (const ringbuf_t *rb);          /* bytes writable    */

void     ringbuf_put   (ringbuf_t *rb, uint8_t byte);
uint8_t  ringbuf_get   (ringbuf_t *rb);
uint8_t  ringbuf_peek  (const ringbuf_t *rb, uint16_t ofs);

void     ringbuf_write (ringbuf_t *rb, const uint8_t *src, uint16_t len);
uint16_t ringbuf_read  (ringbuf_t *rb,       uint8_t *dst, uint16_t len);
void     ringbuf_skip  (ringbuf_t *rb, uint16_t len);  /* discard from head */

#ifdef __cplusplus
}
#endif

#endif /* __RINGBUF_H__ */
