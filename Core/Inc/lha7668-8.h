#ifndef __LHA7668_8_H_
#define __LHA7668_8_H_

#include "lha7668.h"
#include "main.h"
#include "spi.h"
#include "rs485.h"

#ifdef __cplusplus
extern "C" {
#endif

#define ADC_SAMPLING_RATE       3

void LHA7668_Platform_Init(void *handle);
void LHA7668_Platform_Set(const void *port, const uint32_t pin);
void LHA7668_Platform_Reset(const void *port, const uint32_t pin);
int32_t LHA7668_Platform_ReadWrite(uint8_t *txdata, uint8_t *rxdata, const uint16_t size);
int32_t int_adcData_3ch(uint32_t *fx, uint32_t *fy, uint32_t *fz);
int32_t int_adcData_2_3ch(uint32_t *tx, uint32_t *ty, uint32_t *tz);
int32_t int_adcData_6ch(uint32_t *raw);
float adcData(void);
void lha7668_init(uint8_t rate);


#ifdef __cplusplus
}
#endif
#endif

/*********************************End of File**********************************/
