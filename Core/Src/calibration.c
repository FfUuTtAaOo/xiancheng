#include "calibration.h"
#include "sensor_types.h"
#include <string.h>

/* ================================================================
 *  20-point moving-average filter  (per-channel ring buffer)
 * ================================================================ */

#define SMOOTH_WINDOW  20

static struct {
    float buf[6][SMOOTH_WINDOW];
    int   idx;          /* next write position (0..WINDOW-1) */
    int   count;        /* samples collected so far (capped at WINDOW) */
    float sum[6];       /* running sum for O(1) average */
} smooth;

/* ---- Zero calibration collection state ---- */

#define ZERO_SAMPLES    20

static struct {
    float buf[6][ZERO_SAMPLES];   /* force samples during collection */
    int   idx;
    int   count;
    float sum[6];
    uint8_t active;               /* 1 = collecting */
    uint8_t done;                 /* 1 = result computed, ready */
} zcalib;

/* ================================================================
 *  Initialisation
 * ================================================================ */

void calib_init(void)
{
    memset(&smooth, 0, sizeof(smooth));
    memset(&zcalib, 0, sizeof(zcalib));
}

/* ================================================================
 *  20-point smoothing filter
 * ================================================================ */

void calib_push_raw(const float raw_mv[6])
{
    /* ---- Update smoothing filter ---- */
    for (int ch = 0; ch < 6; ch++) {
        if (smooth.count < SMOOTH_WINDOW) {
            /* Still filling — just append */
            smooth.buf[ch][smooth.count] = raw_mv[ch];
            smooth.sum[ch] += raw_mv[ch];
        } else {
            /* Replace oldest entry with new */
            float old = smooth.buf[ch][smooth.idx];
            smooth.buf[ch][smooth.idx] = raw_mv[ch];
            smooth.sum[ch] = smooth.sum[ch] - old + raw_mv[ch];
        }
    }

    /* Advance ring pointer */
    smooth.idx = (smooth.idx + 1) % SMOOTH_WINDOW;
    if (smooth.count < SMOOTH_WINDOW) smooth.count++;

    /* If smoothing has not accumulated WINDOW samples yet, skip
       decoupled-force collection to avoid computing with partial data */
    if (smooth.count < SMOOTH_WINDOW) return;

    /* ---- Zero calibration: collect decoupled force samples ---- */
    if (!zcalib.active) return;

    /* U = filtered mV */
    float u[6];
    calib_get_filtered_all(u);

    /* F = K × U */
    float f[6];
    calib_matrix_multiply(u, f);

    /* Collect F into zero calibration buffer */
    for (int ch = 0; ch < 6; ch++) {
        zcalib.buf[ch][zcalib.count] = f[ch];
        zcalib.sum[ch] += f[ch];
    }

    zcalib.count++;
    if (zcalib.count >= ZERO_SAMPLES) {
        /* Compute average F → F_zero */
        for (int ch = 0; ch < 6; ch++) {
            g_sensor.force_zero[ch] = zcalib.sum[ch] / (float)ZERO_SAMPLES;
        }
        zcalib.active = 0;
        zcalib.done   = 1;
    }
}

float calib_get_filtered(int channel)
{
    if (channel < 0 || channel >= 6 || smooth.count == 0) return 0.0f;
    return smooth.sum[channel] / (float)smooth.count;
}

void calib_get_filtered_all(float u[6])
{
    for (int i = 0; i < 6; i++) {
        u[i] = calib_get_filtered(i);
    }
}

/* ================================================================
 *  Float NaN check (bit-level, no libc dependency)
 * ================================================================ */

static int float_is_nan(float x)
{
    uint32_t u;
    memcpy(&u, &x, 4);
    /* NaN: exponent == 0xFF, mantissa != 0 */
    return ((u & 0x7F800000U) == 0x7F800000U) && ((u & 0x007FFFFFU) != 0);
}

/* ================================================================
 *  6×6 matrix × 6×1 vector  (F = K × U)
 *
 *  When the decoupling matrix has not been calibrated (NaN),
 *  pass the filtered mV through unchanged so kg/N output
 *  equals mV — avoids NaN propagation.
 * ================================================================ */

void calib_matrix_multiply(const float u[6], float f[6])
{
    if (float_is_nan(g_matrix.m[0][0])) {
        /* Uncalibrated matrix → pass through mV directly */
        for (int i = 0; i < 6; i++) f[i] = u[i];
        return;
    }
    for (int i = 0; i < 6; i++) {
        f[i] = 0.0f;
        for (int j = 0; j < 6; j++) {
            f[i] += g_matrix.m[i][j] * u[j];
        }
    }
}

/* ================================================================
 *  Zero offset application
 *
 *  When zero has not been calibrated (NaN), skip subtraction
 *  so the output equals the input unchanged.
 * ================================================================ */

void calib_apply_zero(const float f[6], float f_out[6])
{
    if (float_is_nan(g_sensor.force_zero[0])) {
        /* Uncalibrated zero → pass through */
        for (int i = 0; i < 6; i++) f_out[i] = f[i];
        return;
    }
    for (int i = 0; i < 6; i++) {
        float z = g_sensor.force_zero[i];
        f_out[i] = f[i] - z;
    }
}

/* ================================================================
 *  Full pipeline
 * ================================================================ */

void calib_process_sample(const float raw_mv[6], float force[6])
{
    /* Step 1: push into 20-point filter (also collects zero if active) */
    calib_push_raw(raw_mv);

    /* Step 2: get filtered U */
    float u[6];
    calib_get_filtered_all(u);

    /* Step 3: F = K × U */
    float f[6];
    calib_matrix_multiply(u, f);

    /* Step 4: F_out = F - F_zero */
    calib_apply_zero(f, force);
}

/**
 * Full pipeline with filtered-mV output.
 * Same as calib_process_sample() but also saves the 20-pt filtered mV
 * into g_sensor.force_mv[] for format=0 (mV) output.
 */
void calib_process_sample_with_mv(const float raw_mv[6], float force[6])
{
    /* Step 1: push into 20-point filter (also collects zero if active) */
    calib_push_raw(raw_mv);

    /* Step 2: get filtered U — also save for mV-format output */
    float u[6];
    calib_get_filtered_all(u);
    for (int i = 0; i < 6; i++) {
        g_sensor.force_mv[i] = u[i];
    }

    /* Step 3: F = K × U */
    float f[6];
    calib_matrix_multiply(u, f);

    /* Step 4: F_out = F - F_zero */
    calib_apply_zero(f, force);
}

/* ================================================================
 *  Zero calibration state machine
 * ================================================================ */

void calib_zero_start(void)
{
    zcalib.idx    = 0;
    zcalib.count  = 0;
    zcalib.active = 1;
    zcalib.done   = 0;
    for (int i = 0; i < 6; i++) {
        zcalib.sum[i] = 0.0f;
    }
}

void calib_zero_cale(float adc_value[6])
{
    uint8_t i = 0, ch = 0;
    for (i = 0; i < 6; i++) {
        zcalib.sum[i] += adc_value[i];
    }

    zcalib.count++;
    if (zcalib.count >= SMOOTH_WINDOW) {
        for (ch = 0; ch < 6; ch++) {
            g_sensor.force_zero[ch] = zcalib.sum[ch] / (float)ZERO_SAMPLES;
        }

        zcalib.active = 0;
        zcalib.done   = 1;
    }
}

void calib_zero_over(void)
{
    zcalib.done   = 0;
}

int calib_zero_is_done(void)
{
    return zcalib.done ? 1 : 0;
}

void calib_zero_get_result(float zero[6])
{
    for (int i = 0; i < 6; i++) {
        zero[i] = g_sensor.force_zero[i];
    }
}

void calib_zero_cancel(void)
{
    zcalib.active = 0;
    zcalib.done   = 0;
    zcalib.count  = 0;
    for (int i = 0; i < 6; i++) {
        g_sensor.force_zero[i] = 0.0f;
        zcalib.sum[i]     = 0.0f;
    }
}
