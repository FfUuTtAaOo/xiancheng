#include "flash.h"
#include <stdio.h>

#define FLASH_USER_START_ADDR ((uint32_t)0x0800C000) // Flash操作起始地址
#define FLASH_USER_END_ADDR ((uint32_t)0x0800FFFF)   // Flash操作结束地址

void FLASH_Inside_Wr(uint32_t addr, uint32_t Pdata)
{
  // 定义局部变量
  uint32_t PageError = 0;
  HAL_StatusTypeDef HAL_Status;
  // 擦除配置信息结构体，包括擦除地址、方式、页数等
  FLASH_EraseInitTypeDef pEraseInit;
  pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES; // 按页擦除
  pEraseInit.PageAddress = addr;                // 擦除地址
  pEraseInit.NbPages = 1;                       // 擦除页数量
  // step1 解锁内部FLASH，允许读写功能
  HAL_FLASH_Unlock();
  // step2 开始擦除addr对应页
  HAL_Status = HAL_FLASHEx_Erase(&pEraseInit, &PageError); // 擦除
  if (HAL_Status != HAL_OK)
    printf("内部FlASH擦除失败!\r\n");
  // step3 写入数据
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr, Pdata);
  // step4 锁定FLASH
  HAL_FLASH_Lock();
}

void FLASH_Inside_Wr_Multi(uint32_t addr, uint32_t* Pdata, uint32_t len)
{
  // 定义局部变量
  uint32_t PageError = 0, i = 0;
  HAL_StatusTypeDef HAL_Status;
  // 擦除配置信息结构体，包括擦除地址、方式、页数等
  FLASH_EraseInitTypeDef pEraseInit;

  // 参数检查
  if (Pdata == NULL || len == 0) {
    printf("parameter error!\r\n");
    return;
  }

  uint32_t totalbytes = len * sizeof(uint32_t);
  uint32_t pageneeded = (totalbytes + 1023) / 1024; //向上取整

  pEraseInit.TypeErase = FLASH_TYPEERASE_PAGES; // 按页擦除
  pEraseInit.PageAddress = addr;                // 擦除地址
  pEraseInit.NbPages = pageneeded;              // 擦除页数量

  HAL_FLASH_Unlock();
  HAL_Status = HAL_FLASHEx_Erase(&pEraseInit, &PageError);
  if (HAL_Status != HAL_OK)
    printf("内部FlASH擦除失败!\r\n");
  
  for (i = 0; i < len; i++) {
    HAL_Status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i * 4, *(Pdata + i));
    if (HAL_Status != HAL_OK) {
      printf("内部FlASH写入失败!\r\n");
      break;
    }
  }

  HAL_FLASH_Lock();
}

uint32_t FLASH_Inside_Rd(uint32_t addr)
{
  // 定义局部变量
  uint32_t RdData = 0;
  // step1 解锁内部FLASH，允许读写功能
  HAL_FLASH_Unlock();
  RdData = *(__IO uint32_t *)addr;
  // step2 锁定FLASH
  HAL_FLASH_Lock();

  return RdData;
}
