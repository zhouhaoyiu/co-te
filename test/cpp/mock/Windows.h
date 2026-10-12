#pragma once
#include <cstdint>
#include <vector>

using HANDLE = void*;
using WORD = std::uint16_t;
using DWORD = std::uint32_t;
constexpr int STD_OUTPUT_HANDLE = -11;
constexpr WORD FOREGROUND_BLUE = 0x0001;
constexpr WORD FOREGROUND_GREEN = 0x0002;
constexpr WORD FOREGROUND_RED = 0x0004;
constexpr WORD FOREGROUND_INTENSITY = 0x0008;
constexpr WORD COMMON_LVB_UNDERSCORE = 0x8000;
constexpr DWORD ENABLE_LVB_GRID_WORLDWIDE = 0x0010;
struct CONSOLE_SCREEN_BUFFER_INFO { WORD wAttributes; };

namespace fake_console {
inline WORD attributes = 0x24;
inline DWORD mode = 0x03;
inline std::vector<WORD> attribute_calls;
inline std::vector<DWORD> mode_calls;
inline bool fail_info = false;
inline bool fail_get_mode = false;
inline bool fail_set_mode = false;
inline bool fail_next_attribute = false;
}
inline HANDLE GetStdHandle(int) { return nullptr; }
inline int GetConsoleScreenBufferInfo(HANDLE, CONSOLE_SCREEN_BUFFER_INFO* info) {
  if (fake_console::fail_info) return 0;
  info->wAttributes = fake_console::attributes;
  return 1;
}
inline int GetConsoleMode(HANDLE, DWORD* mode) {
  if (fake_console::fail_get_mode) return 0;
  *mode = fake_console::mode;
  return 1;
}
inline int SetConsoleMode(HANDLE, DWORD mode) {
  fake_console::mode_calls.push_back(mode);
  if (fake_console::fail_set_mode) return 0;
  fake_console::mode = mode;
  return 1;
}
inline int SetConsoleTextAttribute(HANDLE, WORD attributes) {
  fake_console::attribute_calls.push_back(attributes);
  if (fake_console::fail_next_attribute) {
    fake_console::fail_next_attribute = false;
    return 0;
  }
  fake_console::attributes = attributes;
  return 1;
}
