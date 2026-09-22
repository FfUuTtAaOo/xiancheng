#include "at24c04.h"
#include "i2c.h"
#include <stdio.h>

/**
  * @brief  根据要访问的EEPROM内部地址，计算带P0位的7位I2C器件地址
  * @param  memAddr: EEPROM内部地址 (0-511)
  * @param  isWrite: 操作类型，1为写，0为读
  * @retval 完整的8位地址（含R/W位）
  */
uint16_t at24c04_getdevaddr(uint16_t memAddr, uint8_t isWrite) {
  uint16_t baseAddr = AT24C04_I2C_ADDR_WRITE;
  uint16_t pageBit = ((memAddr >> 7) & 0x01) << 1;
  return baseAddr | pageBit | (isWrite ? 0x00 : 0x01);
}

/**
  * @brief  向AT24C04写入数据
  * @param  memAddr: 起始地址 (0-511)
  * @param  pData: 待写入数据指针
  * @param  size: 数据长度（字节）
  * @retval HAL_StatusTypeDef HAL状态
  */
HAL_StatusTypeDef at24c04_write(uint16_t memAddr, uint8_t* pData, uint16_t size)
{
  HAL_StatusTypeDef status;
  uint16_t bytesWritten = 0;
  uint16_t toWrite;

  // 1. 循环写入，直到所有数据写完
  while(bytesWritten < size) {
    // 计算当前地址所在的页内剩余空间
    uint16_t pageRemaining = AT24C04_PAGE_SIZE - (memAddr % AT24C04_PAGE_SIZE);
    toWrite = (size - bytesWritten) < pageRemaining ? (size - bytesWritten) : pageRemaining;
    
    // 2. 获取当前地址对应的I2C器件地址
    uint16_t devAddr = at24c04_getdevaddr(memAddr, 1); // 写操作
    
    // 3. 使用HAL库的存储器写函数
    status = HAL_I2C_Mem_Write(&hi2c2,            // I2C2句柄
                                devAddr,           // 器件地址
                                memAddr & 0xFF,    // 发送的字节地址（低8位）
                                I2C_MEMADD_SIZE_8BIT, // 地址长度8位
                                pData + bytesWritten, // 数据指针偏移
                                toWrite,           // 本次写入长度
                                100);              // 超时时间(ms)
    if(status != HAL_OK) {
      return status;
    }
    
    // 4. 更新计数器和地址
    bytesWritten += toWrite;
    memAddr += toWrite;
    
    // 5. 等待EEPROM内部写周期完成（必需）
    HAL_Delay(5); // AT24C04典型写周期为5ms
  }
  return HAL_OK;
}

/**
  * @brief  从AT24C04读取数据
  * @param  memAddr: 起始地址 (0-511)
  * @param  pData: 数据存储缓冲区指针
  * @param  size: 数据长度（字节）
  * @retval HAL_StatusTypeDef HAL状态
  */
HAL_StatusTypeDef at24c04_read(uint16_t memAddr, uint8_t* pData, uint16_t size)
{
  // 1. 获取当前地址对应的I2C器件地址（读操作）
  uint16_t devAddr = at24c04_getdevaddr(memAddr, 0); // 读操作

  // 2. 使用HAL库的存储器读函数
  //    该函数会发送器件地址+内存地址，然后启动读取
  return HAL_I2C_Mem_Read(&hi2c2,          // I2C2句柄
                          devAddr,         // 器件地址
                          memAddr & 0xFF,  // 发送的字节地址（低8位）
                          I2C_MEMADD_SIZE_8BIT, // 地址长度8位
                          pData,           // 存储缓冲区
                          size,            // 读取长度
                          100);            // 超时时间(ms)
}
