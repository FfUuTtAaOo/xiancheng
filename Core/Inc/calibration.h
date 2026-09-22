#ifndef __CALIBRATION_H__
#define __CALIBRATION_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

/* ================================================================
 *  Sensor calibration pipeline
 *
 *  Data flow:
 *    raw mV[6] → 20-pt MA filter → U[6]
 *              → matrix multiply  → F[6] = K × U
 *              → zero subtract    → F_out[6] = F - F_zero
 *
 *  Zero calibration:
 *    ZERO cmd → collect 20 F samples → average → F_zero
 *    UNZERO   → set F_zero = [0,0,0,0,0,0]
 * ================================================================ */

/**
 * Initialise the smoothing filter and zero-collection state.
 * Must be called once at boot, after flash_load_all().
 */
void calib_init(void);

/**
 * Push a new raw-mV sample from ADC (6 channels).
 * Updates the 20-point moving-average filter internally.
 * If zero calibration is active, also collects the decoupled force sample.
 *
 * @param raw_mv  Pointer to float[6] — raw mV from 6 ADC channels
 */
void calib_push_raw(const float raw_mv[6]);

/**
 * Get the current 20-point filtered mV value for one channel.
 * Returns 0.0 if no samples have been pushed yet.
 */
float calib_get_filtered(int channel);

/**
 * Get all 6 filtered mV values: U[0..5].
 */
void calib_get_filtered_all(float u[6]);

/**
 * Compute F = K × U (6×6 matrix × 6×1 vector).
 * Uses g_matrix.m[6][6] via extern.
 */
void calib_matrix_multiply(const float u[6], float f[6]);

/**
 * Apply zero offset: f_out[i] = f[i] - force_zero[i].
 * If no zero is set, force_zero is all zeros → no change.
 */
void calib_apply_zero(const float f[6], float f_out[6]);

/**
 * The full pipeline for one sample: raw → filtered → decoupled → zero-corrected.
 * @param raw_mv  float[6] raw ADC mV
 * @param force   float[6] output — final force/torque values (N/Nm)
 */
void calib_process_sample(const float raw_mv[6], float force[6]);

/**
 * Full pipeline + save filtered mV into g_sensor.force_mv[] (for format=0).
 * Same as calib_process_sample() but also exposes the 20-pt filtered mV.
 * @param raw_mv  float[6] raw ADC mV
 * @param force   float[6] output — final force/torque values (N/Nm)
 */
void calib_process_sample_with_mv(const float raw_mv[6], float force[6]);

/* ---- Zero calibration state machine ---- */

/**
 * Start zero calibration: begin collecting 20 decoupled force samples.
 * Non-blocking — collection happens via calib_push_raw() calls.
 */
void calib_zero_start(void);

void calib_zero_cale(float adc_value[6]);
void calib_zero_over(void);

/**
 * Check whether zero calibration collection has finished.
 * @return 0 = still collecting, 1 = done (result ready in force_zero)
 */
int  calib_zero_is_done(void);

/**
 * Retrieve the computed zero offsets after collection is done.
 * @param zero  float[6] output buffer
 */
void calib_zero_get_result(float zero[6]);

/**
 * Cancel zero: set all offsets to 0.0 immediately.
 */
void calib_zero_cancel(void);

#ifdef __cplusplus
}
#endif

#endif /* __CALIBRATION_H__ */
