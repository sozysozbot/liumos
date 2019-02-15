#include "liumos.h"

static uint32_t ReadIOAPICRegister(uint8_t reg_index) {
  *reinterpret_cast<volatile uint32_t*>(kIOAPICRegIndexAddr) = reg_index;
  return *reinterpret_cast<volatile uint32_t*>(kIOAPICRegDataAddr);
}

static void WriteIOAPICRegister(uint8_t reg_index, uint32_t value) {
  *reinterpret_cast<volatile uint32_t*>(kIOAPICRegIndexAddr) = reg_index;
  *reinterpret_cast<volatile uint32_t*>(kIOAPICRegDataAddr) = value;
}

static uint64_t ReadIOAPICRedirectTableRegister(uint8_t irq_index) {
  uint64_t data = (uint64_t)ReadIOAPICRegister(0x10 + irq_index * 2);
  data |= (uint64_t)ReadIOAPICRegister(0x10 + irq_index * 2 + 1) << 32;
  return data;
}

static void WriteIOAPICRedirectTableRegister(uint8_t irq_index,
                                             uint64_t value) {
  WriteIOAPICRegister(0x10 + irq_index * 2, (uint32_t)value);
  WriteIOAPICRegister(0x10 + irq_index * 2 + 1, (uint32_t)(value >> 32));
}

static void SetInterruptRedirection(uint64_t local_apic_id,
                                    int from_irq_num,
                                    int to_vector_index) {
  uint64_t redirect_table = ReadIOAPICRedirectTableRegister(from_irq_num);
  redirect_table &= 0x00fffffffffe0000UL;
  redirect_table |= (local_apic_id << 56) | to_vector_index;
  WriteIOAPICRedirectTableRegister(from_irq_num, redirect_table);
}

void LocalAPIC::Init() {
  uint64_t base_msr = ReadMSR(MSRIndex::kLocalAPICBase);
  base_addr_ = (base_msr & ((1ULL << kMaxPhyAddr) - 1)) & ~0xfffULL;
  PutStringAndHex("LAPIC base addr", base_addr_);
  CPUID cpuid;
  ReadCPUID(&cpuid, kCPUIDIndexXTopology, 0);
  id_ = cpuid.edx;
}

void SendEndOfInterruptToLocalAPIC() {
  PutStringAndHex("IOREDTBL[0x02]", ReadIOAPICRedirectTableRegister(2));
  PutStringAndHex("ISR[0x20]", static_cast<int>(bsp_local_apic->ReadISRBit(0x20)));
  PutStringAndHex("EOI reg addr", reinterpret_cast<uint64_t>(bsp_local_apic->GetRegisterAddr(0xB0)));
  *bsp_local_apic->GetRegisterAddr(0xB0) = 1;
  FlushCacheLine(bsp_local_apic->GetRegisterAddr(0xB0));
  PutStringAndHex("ISR[0x20]", static_cast<int>(bsp_local_apic->ReadISRBit(0x20)));
  PutStringAndHex("IOREDTBL[0x02]", ReadIOAPICRedirectTableRegister(2));
}

void InitIOAPIC(uint64_t local_apic_id) {
  PutStringAndHex("IOAPIC RegIndex", kIOAPICRegIndexAddr);
  *reinterpret_cast<volatile uint32_t*>(0xfec0'0000) = 0;
  PutStringAndHex("IOAPIC ID", ReadIOAPICRegister(0));
  PutStringAndHex("IOAPIC VER", ReadIOAPICRegister(1));
  //SetInterruptRedirection(local_apic_id, 2, 0x20);
  SetInterruptRedirection(local_apic_id, 1, 0x21);
}
