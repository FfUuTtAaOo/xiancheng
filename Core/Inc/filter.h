#ifndef __FILTER_H__
#define __FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <math.h>

// 配置参数
#define FILTER_WINDOW_SIZE 30  // 滑动窗口大小
#define NUM_CHANNELS       6   // 通道数量
#define FLOAT_PRECISION    1e-6f // 浮点精度阈值

// 浮点滤波器结构体
typedef struct {
    float buffer[FILTER_WINDOW_SIZE];  // 浮点数据缓冲区
    float sum;                         // 当前窗口总和
    uint8_t index;                     // 当前写入位置
    uint8_t count;                     // 已填充数据计数
    uint8_t initialized;               // 初始化标志
} FloatMovingAverageFilter;

void FloatFilter_Init(void);
float FloatFilter_UpdateChannel(uint8_t channel, float new_sample);
void FloatFilter_UpdateAllChannels(float new_samples[NUM_CHANNELS]);
float FloatFilter_GetCurrentAverage(uint8_t channel);
void FloatFilter_ResetChannel(uint8_t channel);

#ifdef __cplusplus
}
#endif

#endif
