#ifndef __SENSOR_TYPES_H__
#define __SENSOR_TYPES_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ================================================================
 *  Global shared data types — used by all protocol modules
 *  Instances are defined in main.c
 * ================================================================ */

/** 6-axis force/torque sensor readings */
typedef struct {
    float    force[6];       /* index 0-5: final output (format-dependent)       */
    float    force_raw[6];   /* raw mV from ADC (6 channels, single-shot)       */
    float    force_mv[6];    /* 20-pt filtered mV (before matrix multiply)      */
    float    force_zero[6];  /* zero offsets in force-space (from 20-pt avg)    */
} sensor_data_t;

/** System runtime state (mapped to Modbus registers + RS485 queries) */
typedef struct {
    uint8_t  send_mode;      /* 0 = stop, 1 = continuous, 2 = single  */
    uint8_t  data_format;    /* 0 = mV,  1 = kg,      2 = N           */
    uint8_t  freq_mode;      /* 0 = 500Hz, 1 = 1000Hz                 */
    uint8_t  debug_mode;     /* 0 = normal, 1 = debug (RS485 cmd 0x35) */
    uint8_t  zero_calib_busy;/* 1 = collecting 20 force samples for zero */
    uint8_t  zero_calib_count;
    uint8_t  adc_ready;      /* set by ADC ISR, cleared after reading    */
    uint8_t  _pad[2];
    uint16_t status_flags;   /* BIT0 = auto-start, BIT1 = active       */
    uint16_t comm_error_cnt; /* RS485 communication error counter      */
    uint32_t frame_seq;      /* continuous-send frame sequence number  */
} system_state_t;

/** Persistent configuration (stored in Flash, mirrored in RAM) */
typedef struct {
    char     sn[16];         /* serial number, null-terminated          */
    uint32_t fw_version;     /* BCD, e.g. 0x00010701 = v1.7.1               */
    uint16_t reserved;
} config_t;

/** 6×6 decoupling matrix */
typedef struct {
    float m[6][6];
} decouple_matrix_t;

/** Overload thresholds and range limits (Modbus R/W, not in Flash) */
typedef struct {
    float overload[6];       /* overload threshold per axis             */
    float range_min[6];      /* measuring range minimum per axis       */
    float range_max[6];      /* measuring range maximum per axis       */
} threshold_data_t;

/* ---- Global extern declarations (instances in main.c) ---- */
extern volatile sensor_data_t   g_sensor;
extern volatile system_state_t  g_sys;
extern config_t                 g_config;
extern decouple_matrix_t        g_matrix;
extern threshold_data_t         g_threshold;

#ifdef __cplusplus
}
#endif

#endif /* __SENSOR_TYPES_H__ */
