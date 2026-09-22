#ifndef __CRC_H__
#define __CRC_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/** Modbus CRC-16 (polynomial 0xA001), used by RS485 */
uint16_t crc16_modbus(const uint8_t *buf, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __CRC_H__ */
