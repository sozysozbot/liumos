#pragma once

#include "acpi.h"
#include "asm.h"
#include "efi.h"
#include "generic.h"
#include "guid.h"

// @console.c
void ResetCursorPosition();
void EnableVideoModeForConsole();
void PutChar(char c);
void PutString(const char* s);
void PutChars(const char* s, int n);
void PutHex64(uint64_t value);
void PutHex64ZeroFilled(uint64_t value);
void PutStringAndHex(const char* s, uint64_t value);
void PutStringAndHex(const char* s, void* value);

// @draw.c
void DrawCharacter(char c, int px, int py);
void DrawRect(int px, int py, int w, int h, uint32_t col);
void BlockTransfer(int to_x, int to_y, int from_x, int from_y, int w, int h);

// @font.gen.c
extern uint8_t font[0x100][16];

// @gdt.c
class GDT {
 public:
  GDT() {
    ReadGDTR(&gdtr_);
    PutStringAndHex("GDT base", gdtr_.base);
    PutStringAndHex("GDT limit", gdtr_.limit);
    Print();
  }
  void Print(void);

 private:
  GDTR gdtr_;
};

// @generic.h

[[noreturn]] void Panic(const char* s);
void __assert(const char* expr_str, const char* file, int line);
#define assert(expr) \
  ((void)((expr) || (__assert(#expr, __FILE__, __LINE__), 0)))
inline void* operator new(size_t, void* where) {
  return where;
}

// @liumos.c
[[noreturn]] void Panic(const char* s);
void MainForBootProcessor(void* image_handle, EFISystemTable* system_table);

// @static.c
extern const GUID EFI_ACPITableGUID;
extern const GUID EFI_GraphicsOutputProtocolGUID;
