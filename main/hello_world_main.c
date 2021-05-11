/* Hello World Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include "esp_task_wdt.h"

#include "i2s_parallel.h"

void app_main(void)
{
    // DMA data buffer
    uint16_t total_count = 100; // Seems to be irrelevant, it happens at 20 and at 2000
    uint16_t buffer_size = total_count * sizeof(uint16_t);
    uint16_t *buffer = (uint16_t*)heap_caps_malloc(buffer_size, MALLOC_CAP_DMA);

    uint16_t pattern_a = 0b10;
    uint16_t pattern_b = 0b01;

    for (int i = 0; i < total_count; i++) {
        buffer[i] = pattern_a;
    }
    uint8_t b_count = 3; // <<-- this value is important!
    // 1 or 2, everything is fine
    // After that, it seems that every _odd_ value is bugged, but every _even_ one is fine
    // See the `scope caps` directory
    for (int i = 0; i < b_count; i++) {
        buffer[total_count - (i + 1)] = pattern_b;
    }

    // And of course the descriptor
    lldesc_t *desc = (lldesc_t*)heap_caps_malloc(sizeof(lldesc_t), MALLOC_CAP_DMA);
    desc->buf          = (uint8_t*)buffer;
    desc->size         = buffer_size;
    desc->length       = buffer_size;
    desc->offset       = 0;
    desc->qe.stqe_next = desc;
    desc->eof          = 1;
    desc->sosf         = 0;
    desc->owner        = 1;

    // Hook up our I2S stuff
    // Wiring just happens to be what I had hooked up
    i2s_parallel_config_t conf = {
        .gpio_clk = 33,
        .gpios_bus = {16, 25, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1},
        .sample_width = I2S_PARALLEL_WIDTH_16,
        .sample_rate = 500000
    };
    i2s_parallel_driver_install(I2S_NUM_0, &conf, false, NULL, NULL);
    i2s_parallel_send_dma(I2S_NUM_0, desc);

    while (1) {
        esp_task_wdt_reset();
    }
}
