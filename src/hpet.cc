#include "liumos.h"

using TimerConfig = HPET::TimerConfig;
using TimerRegister = HPET::RegisterSpace::TimerRegister;
using GeneralConfig = HPET::RegisterSpace::GeneralConfig;

constexpr GeneralConfig operator|=(GeneralConfig& a, GeneralConfig b) {
  a = static_cast<GeneralConfig>(static_cast<uint64_t>(a) |
                                 static_cast<uint64_t>(b));
  return a;
}
constexpr GeneralConfig operator&=(GeneralConfig& a, GeneralConfig b) {
  a = static_cast<GeneralConfig>(static_cast<uint64_t>(a) &
                                 static_cast<uint64_t>(b));
  return a;
}
constexpr GeneralConfig operator~(GeneralConfig a) {
  return static_cast<GeneralConfig>(~static_cast<uint64_t>(a));
}

void HPET::PrintCapabilities(){
  uint64_t gconfig = static_cast<uint64_t>(registers_->general_capabilities_and_id);
  PutStringAndHex("Supported HPET timers", last_index_of_timer_ + 1);
  PutStringAndHex("Legacy replacement routing cap", (gconfig >> 15) & 1);
  PutString(((gconfig >> 13) & 1) ? "64bits\n" : "32bits\n");
  for(int i = 0; i <= last_index_of_timer_; i++){
    uint64_t config = static_cast<uint64_t>(registers_->timers[i].configuration_and_capability);
    PutHex64(i);
    PutString(" cap: int_route=");
    for(int i = 63; i >= 32; i--) PutChar(((config >> i) & 1 ) ? '1' : '0');
    PutChar(' ');
    if(config & (1 << 5)) PutString("64");
    else PutString("32");
    PutString("bits ");
    if(static_cast<uint64_t>(config) & (1 << 15)) PutString("FSB ");
    if(static_cast<uint64_t>(config) & (1 << 4)) PutString("periodic");
    PutChar('\n');
  }
}

void HPET::Init(HPET::RegisterSpace* registers) {
  registers_ = registers;
  femtosecond_per_count_ = registers->general_capabilities_and_id >> 32;
  last_index_of_timer_ = (registers->general_capabilities_and_id >> 8) & 0b11111; 
  for(int i = 0; i <= last_index_of_timer_; i++){
    TimerConfig config = registers->timers[i].configuration_and_capability;
    config &= ~TimerConfig::kEnable;
    registers->timers[i].configuration_and_capability = config;
  }
  GeneralConfig general_config = registers->general_configuration;
  general_config &= ~GeneralConfig::kUseLegacyReplacementRouting;
  general_config &= ~GeneralConfig::kEnable;
  registers->general_configuration = general_config;
  PrintCapabilities();
}

void HPET::PrintTimerConfig(int index){
  TimerRegister* entry = &registers_->timers[index];
  int64_t config = static_cast<uint64_t>(entry->configuration_and_capability);
  PutStringAndHex("HPET Timer #", index);
  PutStringAndBoolNames("Enabled", (config >> 2) & 1, "True", "False");
  PutStringAndBoolNames("Trigger Mode", (config >> 1) & 1, "Edge", "Level");
  PutStringAndBoolNames("Type", (config >> 3) & 1, "Periodic", "Single");
  PutStringAndBoolNames("Operating bit width", (config >> 3) & 1, "32", "64");
  
}

void HPET::SetTimerMs(int timer_index,
                      uint64_t milliseconds,
                      TimerConfig flags) {
  uint64_t count = 1e12 * milliseconds / femtosecond_per_count_;
  TimerRegister* entry = &registers_->timers[timer_index];
  TimerConfig config = entry->configuration_and_capability;
  TimerConfig mask = TimerConfig::kEnable | TimerConfig::kUsePeriodicMode;
  registers_->general_configuration = GeneralConfig::kNotOperating;
  registers_->main_counter_value = 0;
  config &= ~mask;
  config |= mask & flags;
  config |= TimerConfig::kSetComparatorValue;
  entry->configuration_and_capability = config;
  entry->comparator_value = count;
  registers_->general_configuration = GeneralConfig::kOperating;
  PrintTimerConfig(timer_index);
}
