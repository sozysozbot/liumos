#include "liumos.h"

void GDT::Init() {
  descriptors_.null_segment = 0;
  descriptors_.kernel_code_segment = kDescBitTypeCode | kDescBitPresent |
                                 kCSDescBitLongMode | kCSDescBitReadable;
  descriptors_.kernel_data_segment =
      kDescBitTypeData | kDescBitPresent | kDSDescBitWritable;
  gdtr_.base = reinterpret_cast<uint64_t *>(&descriptors_);
  gdtr_.limit = sizeof(GDTDescriptors) - 1;
  WriteGDTR(&gdtr_);
  WriteCSSelector(kKernelCSSelector);
  WriteSSSelector(kKernelDSSelector);
  WriteDataAndExtraSegmentSelectors(kKernelDSSelector);
  PutStringAndHex("GDT base", gdtr_.base);
  PutStringAndHex("GDT limit", gdtr_.limit);
  Print();
}

void GDT::Print() {
  for (size_t i = 0; i < (gdtr_.limit + 1) / sizeof(uint64_t); i++) {
    PutStringAndHex("ent", gdtr_.base[i]);
  }
}
