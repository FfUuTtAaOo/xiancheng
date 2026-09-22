/***********************滤波算法**************************/
#include "filter.h"

// 全局浮点滤波器数组
FloatMovingAverageFilter float_filters[NUM_CHANNELS];

// 初始化所有浮点滤波器
void FloatFilter_Init(void) {
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        for (int i = 0; i < FILTER_WINDOW_SIZE; i++) {
            float_filters[ch].buffer[i] = 0.0f;
        }
        float_filters[ch].sum = 0.0f;
        float_filters[ch].index = 0;
        float_filters[ch].count = 0;
        float_filters[ch].initialized = 0;
    }
}

// 更新单个通道浮点滤波器
float FloatFilter_UpdateChannel(uint8_t channel, float new_sample) {
    if (channel >= NUM_CHANNELS) return 0.0f;

    FloatMovingAverageFilter* f = &float_filters[channel];

    // 首次采样初始化
    if (!f->initialized) {
        for (int i = 0; i < FILTER_WINDOW_SIZE; i++) {
            f->buffer[i] = new_sample;
        }
        f->sum = new_sample * FILTER_WINDOW_SIZE;
        f->count = FILTER_WINDOW_SIZE;
        f->initialized = 1;
        return new_sample;
    }

    // 减去最旧的值并添加新值
    f->sum -= f->buffer[f->index];
    f->buffer[f->index] = new_sample;
    f->sum += new_sample;

    // 更新环形缓冲区索引
    f->index = (f->index + 1) % FILTER_WINDOW_SIZE;

    // 计算浮点平均值
    return f->sum / FILTER_WINDOW_SIZE;
}

// 更新所有通道
void FloatFilter_UpdateAllChannels(float new_samples[NUM_CHANNELS]) {
    for (int ch = 0; ch < NUM_CHANNELS; ch++) {
        new_samples[ch] = FloatFilter_UpdateChannel(ch, new_samples[ch]);
    }
}

// 获取当前通道的平均值
float FloatFilter_GetCurrentAverage(uint8_t channel) {
    if (channel >= NUM_CHANNELS) return 0.0f;
    FloatMovingAverageFilter* f = &float_filters[channel];

    if (f->count == 0) return 0.0f;
    return f->sum / f->count;
}

// 重置指定通道的滤波器
void FloatFilter_ResetChannel(uint8_t channel) {
    if (channel >= NUM_CHANNELS) return;

    FloatMovingAverageFilter* f = &float_filters[channel];
    f->sum = 0.0f;
    f->index = 0;
    f->count = 0;
    f->initialized = 0;
}
