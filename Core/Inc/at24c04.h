#ifndef _AT24C04_H_
#define _AT24C04_H_

#include <main.h>

#ifdef __cplusplus
extern "C" {
#endif

/* 7-bit device base address (write form 8-bit = 0xA0) */
#define AT24C04_I2C_ADDR_WRITE   0xA0u
#define AT24C04_I2C_ADDR_READ    (AT24C04_I2C_ADDR_WRITE | 0x01u)

#define AT24C04_TOTAL_SIZE       512u   /* 4Kbit = 512 bytes */
#define AT24C04_PAGE_SIZE        16u    /* 常见页大小（字节） */

/**
  * @brief  根据要访问的EEPROM内部地址，计算带P0位的7位I2C器件地址（含R/W位的8位形式）
  * @param  memAddr: EEPROM内部地址 (0-511)
  * @param  isWrite: 操作类型，1为写，0为读
  * @retval 完整的8位地址（含R/W位）
  */
uint16_t at24c04_getdevaddr(uint16_t memAddr, uint8_t isWrite);
HAL_StatusTypeDef at24c04_write(uint16_t memAddr, uint8_t* pData, uint16_t size);
HAL_StatusTypeDef at24c04_read(uint16_t memAddr, uint8_t* pData, uint16_t size);

#ifdef __cplusplus
}
#endif

#endif /* AT24C04_H */
