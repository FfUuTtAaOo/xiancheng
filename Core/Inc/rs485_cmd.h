#ifndef __RS485_CMD_H__
#define __RS485_CMD_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ================================================================
 *  RS485 Command Set  (§13 in 设计方案)
 *  All commands use frame format: AA 55 [CMD] [PAYLOAD...] 0D 0A
 *  ================================================================ */

/** @name Command codes */
/*@{*/
#define RS485_CMD_STOP          0x01  /* Stop continuous data output         */
#define RS485_CMD_START         0x02  /* Start continuous 500Hz output       */
#define RS485_CMD_SINGLE        0x03  /* Single-shot data request            */
#define RS485_CMD_SET_BAUD      0x04  /* Set baud rate: AA 55 04 [code] 0D 0A*/
#define RS485_CMD_QUERY_SN      0x05  /* Query serial number                 */
#define RS485_CMD_QUERY_VER     0x06  /* Query firmware version              */
#define RS485_CMD_QUERY_STATUS  0x07  /* Query system status                 */
#define RS485_CMD_SET_FREQ      0x08  /* Set frequency: 0=500Hz, 1=1000Hz    */
#define RS485_CMD_ZERO          0x30  /* Zero calibration (set offsets)      */
#define RS485_CMD_UNZERO        0x36  /* Cancel zero (clear offsets)         */
#define RS485_CMD_QUERY_MATRIX  0x37  /* Query the 6×6 matrix (144 bytes)    */
#define RS485_CMD_HEX_SEND      0x31
#define RS485_CMD_ASC_SEND      0x32
#define RS485_CMD_SET_FORMAT_KG 0x33  /* Set data format: 0=mV,1=kg,2=N      */
#define RS485_CMD_SET_FORMAT    0x34  /* Set data format: 0=mV,1=kg,2=N      */
#define RS485_CMD_SET_FORMAT_N  0x35  /* Set data format: 0=mV,1=kg,2=N      */

#define ETHERCAT_DISABLED       0x40
#define ETHERCAT_ENABLED        0x41
#define RS485_CMD_SET_IP        0x0B  /* Set IP address   (4 bytes)          */
#define RS485_CMD_SET_MAC       0x0C  /* Set MAC address  (6 bytes)          */
#define RS485_CMD_SET_SUBNET    0x0D  /* Set subnet mask  (4 bytes)          */
#define RS485_CMD_SET_GW        0x0E  /* Set gateway      (4 bytes)          */
#define RS485_CMD_QUERY_NET     0x0F  /* Query network config                */
#define RS485_CMD_SET_MATRIX    0x20  /* Upload decoupling matrix (144 bytes) */
#define RS485_CMD_DEBUG_MODE    0x35  /* Toggle debug mode                   */

/** @name Baud-rate codes — payload byte of RS485_CMD_SET_BAUD (AA 55 04 [code] 0D 0A) */
/*@{*/
#define RS485_BAUD_460800       0x01  /* 460800 bps                          */
#define RS485_BAUD_256000       0x02  /* 256000 bps                          */
#define RS485_BAUD_115200       0x03  /* 115200 bps                          */
#define RS485_BAUD_19200        0x04  /* 19200  bps                          */
#define RS485_BAUD_9600         0x05  /* 9600   bps                          */
/*@}*/

/** @name Continuous data frame CMD byte */
#define RS485_CMD_CONT_DATA     0x02  /* Continuous/single data frame        */
/*@}*/

/** @name Response flag (OR-ed with command code in response frame) */
#define RS485_RESP_FLAG         0x80  /* AA 55 [CMD|0x80] [DATA] 0D 0A       */

/**
 * Send one data frame carrying the current 6-axis values.
 *
 * debug_mode == 0:  AA 55 [cmd] [6 × float32 LE] 0D 0A
 * debug_mode == 1:  [6 × float32 LE] 00 00 80 7F          (cmd unused)
 * any other value:  silent — no frame is transmitted at all
 *
 * @param cmd  command byte placed after the AA 55 header.
 *             Pass RS485_CMD_START  (0x02) for continuous-stream frames,
 *             or   RS485_CMD_SINGLE (0x03) for a single-shot reply.
 */
void rs485_send_data_frame(uint8_t cmd);

/**
 * Dispatch an incoming RS485 command.
 * Called by the rs485_process() frame callback.
 *
 * @param cmd      command byte (0x01..0x35, or 0x00 for data frame)
 * @param payload  payload bytes (after CMD, before 0D 0A)
 * @param len      payload length
 */
void rs485_cmd_dispatch(uint8_t cmd, const uint8_t *payload, uint16_t len);

#ifdef __cplusplus
}
#endif

#endif /* __RS485_CMD_H__ */
