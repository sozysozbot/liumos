#include "liumos.h"

void LocalAPIC::Init(void) {
    uint64_t base_msr = ReadMSR(MSRIndex::kLocalAPICBase);
    PutStringAndHex("IA32_APIC_BASE MSR", base_msr);

    base_addr_ = (base_msr & ((1ULL << kMaxPhyAddr) - 1)) & ~0xfffULL;

    PutStringAndHex("Read 0xc000'0000", *reinterpret_cast<uint8_t *>(0xc000'0000));
    PutStringAndHex("Read SVR", ReadRegister(RegisterOffset::kSVR));
    PutStringAndHex("Read APIC ID Register", ReadRegister(RegisterOffset::kLocalAPICID));


    CPUID cpuid;
    ReadCPUID(&cpuid, kCPUIDIndexXTopology, 0);
    id_ = cpuid.edx;
    PutStringAndHex("APIC ID come from cpuid", cpuid.edx);
}

uint32_t LocalAPIC::ReadRegister(RegisterOffset offset) {
  volatile uint32_t *addr = reinterpret_cast<uint32_t *>(base_addr_ + static_cast<uint64_t>(offset));
  PutStringAndHex("LAPIC ID reg v2p", reinterpret_cast<IA_PML4 *>(ReadCR3())->v2p(reinterpret_cast<uint64_t>(addr)));
  return *addr;
}

void SendEndOfInterruptToLocalAPIC() {
  *(uint32_t*)(((ReadMSR(MSRIndex::kLocalAPICBase) &
                 ((1ULL << kMaxPhyAddr) - 1)) &
                ~0xfffULL) +
               0xB0) = 0;
}

static uint32_t ReadIOAPICRegister(uint64_t io_apic_base_addr,
                                   uint8_t reg_index) {
  *(uint32_t volatile*)(io_apic_base_addr) = (uint32_t)reg_index;
  return *(uint32_t volatile*)(io_apic_base_addr + 0x10);
}

static void WriteIOAPICRegister(uint64_t io_apic_base_addr,
                                uint8_t reg_index,
                                uint32_t value) {
  *(uint32_t volatile*)(io_apic_base_addr) = (uint32_t)reg_index;
  *(uint32_t volatile*)(io_apic_base_addr + 0x10) = value;
}

static uint64_t ReadIOAPICRedirectTableRegister(uint64_t io_apic_base_addr,
                                                uint8_t irq_index) {
  return (uint64_t)ReadIOAPICRegister(io_apic_base_addr, 0x10 + irq_index * 2) |
         ((uint64_t)ReadIOAPICRegister(io_apic_base_addr,
                                       0x10 + irq_index * 2 + 1)
          << 32);
}

static void WriteIOAPICRedirectTableRegister(uint64_t io_apic_base_addr,
                                             uint8_t irq_index,
                                             uint64_t value) {
  WriteIOAPICRegister(io_apic_base_addr, 0x10 + irq_index * 2, (uint32_t)value);
  WriteIOAPICRegister(io_apic_base_addr, 0x10 + irq_index * 2 + 1,
                      (uint32_t)(value >> 32));
}

static void SetInterruptRedirection(uint64_t local_apic_id,
                                    int from_irq_num,
                                    int to_vector_index) {
  uint64_t redirect_table =
      ReadIOAPICRedirectTableRegister(IO_APIC_BASE_ADDR, from_irq_num);
  redirect_table &= 0x00fffffffffe0000UL;
  redirect_table |= (local_apic_id << 56) | to_vector_index;
  WriteIOAPICRedirectTableRegister(IO_APIC_BASE_ADDR, from_irq_num,
                                   redirect_table);
}

void InitIOAPIC(uint64_t local_apic_id) {
  SetInterruptRedirection(local_apic_id, 2, 0x20);
  SetInterruptRedirection(local_apic_id, 1, 0x21);
}
