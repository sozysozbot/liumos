#pragma once
#include "ring_buffer.h"
#include "scheduler.h"

packed_struct TSS64 {
  uint32_t reserved0;
  uint64_t rsp[3];
  uint64_t ist[9];
  uint16_t reserved1;
  uint16_t io_map_base_addr;
};

extern Scheduler* scheduler;
extern RingBuffer<uint8_t, 16> keycode_buffer;
void InitIDT(void);
