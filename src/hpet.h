#pragma once
#include "generic.h"

class HPET {
 public:
  enum class TimerConfig : uint64_t {
    kUseLevelTriggeredInterrupt = 1 << 1,
    kEnable = 1 << 2,
    kUsePeriodicMode = 1 << 3,
    kSetComparatorValue = 1 << 6,
  };
  struct RegisterSpace;

  void Init(RegisterSpace* registers);
  void SetTimerMs(int timer_index,
                  uint64_t milliseconds,
                  HPET::TimerConfig flags);

  packed_struct RegisterSpace {
    uint64_t general_capabilities_and_id;
    uint64_t reserved00;
    enum class GeneralConfig : uint64_t {
      kEnable = 1 << 0,
      kUseLegacyReplacementRouting = 1 << 1,
      kOperating = 0b01,
      kNotOperating = 0b00,
    } general_configuration;
    uint64_t reserved01;
    uint64_t general_interrupt_status;
    uint64_t reserved02;
    uint64_t reserved03[24];
    uint64_t main_counter_value;
    uint64_t reserved04;
    packed_struct TimerRegister {
      TimerConfig configuration_and_capability;
      uint64_t comparator_value;
      uint64_t fsb_interrupt_route;
      uint64_t reserved;
    }
    timers[32];
  }
  *registers_;

 private:
  void PrintCapabilities(void);
  void PrintTimerConfig(int index);
  uint64_t femtosecond_per_count_;
  uint8_t last_index_of_timer_;
};

constexpr HPET::TimerConfig operator|=(HPET::TimerConfig& a,
                                       HPET::TimerConfig b) {
  a = static_cast<HPET::TimerConfig>(static_cast<uint64_t>(a) |
                                     static_cast<uint64_t>(b));
  return a;
}

constexpr HPET::TimerConfig operator&=(HPET::TimerConfig& a,
                                       HPET::TimerConfig b) {
  a = static_cast<HPET::TimerConfig>(static_cast<uint64_t>(a) &
                                     static_cast<uint64_t>(b));
  return a;
}

constexpr HPET::TimerConfig operator|(HPET::TimerConfig a,
                                      HPET::TimerConfig b) {
  return static_cast<HPET::TimerConfig>(static_cast<uint64_t>(a) |
                                        static_cast<uint64_t>(b));
}

constexpr HPET::TimerConfig operator&(HPET::TimerConfig a,
                                      HPET::TimerConfig b) {
  return static_cast<HPET::TimerConfig>(static_cast<uint64_t>(a) &
                                        static_cast<uint64_t>(b));
}

constexpr HPET::TimerConfig operator~(HPET::TimerConfig a) {
  return static_cast<HPET::TimerConfig>(~static_cast<uint64_t>(a));
}
