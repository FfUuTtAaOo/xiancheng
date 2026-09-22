#include "flash_storage.h"
#include "sensor_types.h"
#include "crc.h"
#include "at24c04.h"
#include "stm32f1xx_hal.h"
#include <string.h>

/**
 * Erase the storage page.  Must be called with flash unlocked.
 */
void flash_erase_storage(void)
{
    // FLASH_EraseInitTypeDef erase = {0};
    // uint32_t page_error = 0;

    // erase.TypeErase   = FLASH_TYPEERASE_PAGES;
    // erase.PageAddress = FLASH_STORAGE_ADDR;
    // erase.NbPages     = 1;

    // HAL_FLASHEx_Erase(&erase, &page_error);
}

/**
 * Write a buffer of uint8_t to flash.
 * Handles unaligned start: pads odd-byte start with 0xFF in the same
 * half-word, and handles an odd final byte.
 */
static int flash_write_buf(uint32_t addr, const uint8_t *data, uint16_t len)
{
    /* If addr is odd, read-modify-write the first halfword */
    // if (addr & 1) {
    //     uint16_t hw = 0xFF00 | data[0];
    //     if (!flash_write_halfword(addr & ~1U, hw)) return 0;
    //     addr += 1;
    //     data += 1;
    //     len  -= 1;
    // }

    // while (len >= 2) {
    //     uint16_t hw = ((uint16_t)data[0])
    //                 | ((uint16_t)data[1] << 8);
    //     if (!flash_write_halfword(addr, hw)) return 0;
    //     addr += 2;
    //     data += 2;
    //     len  -= 2;
    // }

    // /* Last odd byte */
    // if (len == 1) {
    //     uint16_t hw = (uint16_t)data[0] | 0xFF00U;
    //     if (!flash_write_halfword(addr, hw)) return 0;
    // }

    return 1;
}

/* ================================================================
 *  CRC-16 for stored config integrity check
 * ================================================================ */

static uint16_t calc_crc(const uint8_t *data, uint16_t len)
{
    // return crc16_modbus(data, len);
    return 0;
}

/* ================================================================
 *  Load from flash
 * ================================================================ */

int flash_load_all(void)
{
    int i = 0, j = 0, offset = 0, tmp = 0;
    uint8_t at24c04_read_buffer[32] = { 0 };
    floatuint_t matrix_data_tmp[6] = { 0 };
    for (i = 0; i < MATRIX_ROWS; i++) {
        if (i != 5) {
            at24c04_read(FLASH_STORAGE_ADDR + i * 0x18, at24c04_read_buffer, MATRIX_SIZE);
        } else {
            at24c04_read(0x80, at24c04_read_buffer, MATRIX_SIZE);
        }

        for (j = 0; j < MATRIX_SIZE; j++) {
            offset = j / 4;
            tmp = j % 4;
            matrix_data_tmp[offset].u32 = U8_TO_U32_SAFE(at24c04_read_buffer[j], matrix_data_tmp[offset].u32, 3 - tmp);
        }
        for (j = 0; j < 6; j++) {
            g_matrix.m[i][j] = matrix_data_tmp[j].f32;
        }
    }

    return 1;
}

/* ================================================================
 *  Save to flash  (erase-then-write-all pattern)
 * ================================================================ */

/* Temporary buffer for assembling a full page image */

/* ---- Individual save functions (convenience wrappers) ---- */

int flash_save_config(void)
{
    HAL_FLASH_Unlock();
    flash_erase_storage();

    uint8_t buf[256];
    memset(buf, 0xFF, sizeof(buf));

    /* Read back existing matrix and zero from flash before erasing...
       Actually we need to preserve them.  Since erase destroys everything,
       the simplest approach is to rebuild the full page from RAM.
       All data lives in RAM anyway. */
    memcpy(buf + FLASH_OFF_ZERO,
           (const void *)g_sensor.force_zero, 24);
    memcpy(buf + FLASH_OFF_FW_VERSION, &g_config.fw_version, 2);

    uint32_t magic = FLASH_MAGIC;
    memcpy(buf + FLASH_OFF_MAGIC, &magic, 4);
    uint16_t crc = calc_crc(buf, FLASH_OFF_CRC);
    memcpy(buf + FLASH_OFF_CRC, &crc, 2);

    int ok = flash_write_buf(FLASH_STORAGE_ADDR, buf, FLASH_OFF_CRC + 2);
    HAL_FLASH_Lock();
    return ok;
}

int flash_save_matrix(uint8_t index)
{
    if (index != 5) {
        at24c04_write(FLASH_STORAGE_ADDR + index * 0x18, (uint8_t *)&g_matrix.m[index], MATRIX_SIZE);
    } else if (index == 5) {
        at24c04_write(0x80, (uint8_t *)&g_matrix.m[index], MATRIX_SIZE);
    } else {
        return 0;
    }
    return 1;
}

int flash_save_zero(void)
{
    at24c04_write(FLASH_STORAGE_ADDR + FLASH_OFF_ZERO, (uint8_t *)g_sensor.force_zero, 24);
    return 1;
}
