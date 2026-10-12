#include <iostream>
#include <string>
#include <stdexcept>
#include <vector>
#include <utility>
#include "vendor/utf8proc/utf8proc.h"
#include "Windows.h"
#include "main.h"

#ifndef CO_TE_NO_MAIN
int main()
{
  reverse("Hello World!");
  system("pause");
  return 0;
}

#endif

namespace
{
  // Restore the caller's colors and console mode even when output throws.
  struct ConsoleStyleGuard
  {
    HANDLE handle;
    WORD attributes;
    DWORD mode = 0;
    bool restore_mode = false;

    ~ConsoleStyleGuard()
    {
      SetConsoleTextAttribute(handle, attributes);
      if (restore_mode)
        SetConsoleMode(handle, mode);
    }
  };

  void print_style(const std::string &message, WORD attribute, bool enable_underline, WORD clear_attributes = 0)
  {
    HANDLE handle = GetStdHandle(STD_OUTPUT_HANDLE);
    CONSOLE_SCREEN_BUFFER_INFO info{};
    if (!GetConsoleScreenBufferInfo(handle, &info))
      throw std::runtime_error("Styling requires a Windows console screen buffer");

    ConsoleStyleGuard guard{handle, info.wAttributes};
    if (enable_underline)
    {
      if (!GetConsoleMode(handle, &guard.mode))
        throw std::runtime_error("Cannot read console mode for underline");
      if ((guard.mode & ENABLE_LVB_GRID_WORLDWIDE) == 0)
      {
        if (!SetConsoleMode(handle, guard.mode | ENABLE_LVB_GRID_WORLDWIDE))
          throw std::runtime_error("This console cannot enable underline attributes");
        guard.restore_mode = true;
      }
    }
    if (!SetConsoleTextAttribute(handle, (info.wAttributes & ~clear_attributes) | attribute))
      throw std::runtime_error("Cannot apply console style");
    std::cout << message << std::endl;
  }
}

void red(std::string message)
{
  print_style(message, FOREGROUND_RED, false, 0x000f);
}

void green(std::string message)
{
  print_style(message, FOREGROUND_GREEN, false, 0x000f);
}

void blue(std::string message)
{
  print_style(message, FOREGROUND_BLUE, false, 0x000f);
}

void yellow(std::string message)
{
  print_style(message, FOREGROUND_RED | FOREGROUND_GREEN, false, 0x000f);
}

void magenta(std::string message)
{
  print_style(message, FOREGROUND_RED | FOREGROUND_BLUE, false, 0x000f);
}

void cyan(std::string message)
{
  print_style(message, FOREGROUND_GREEN | FOREGROUND_BLUE, false, 0x000f);
}

void white(std::string message)
{
  print_style(message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, false, 0x000f);
}

void black(std::string message)
{
  print_style(message, 0, false, 0x000f);
}

void gray(std::string message)
{
  print_style(message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE, false, 0x000f);
}

void light_red(std::string message)
{
  print_style(message, FOREGROUND_RED | FOREGROUND_INTENSITY, false, 0x000f);
}

void light_green(std::string message)
{
  print_style(message, FOREGROUND_GREEN | FOREGROUND_INTENSITY, false, 0x000f);
}

void light_blue(std::string message)
{
  print_style(message, FOREGROUND_BLUE | FOREGROUND_INTENSITY, false, 0x000f);
}

void light_yellow(std::string message)
{
  print_style(message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY, false, 0x000f);
}

void light_magenta(std::string message)
{
  print_style(message, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY, false, 0x000f);
}

void light_cyan(std::string message)
{
  print_style(message, FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, false, 0x000f);
}

void light_white(std::string message)
{
  print_style(message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, false, 0x000f);
}

void light_black(std::string message)
{
  print_style(message, 0 | FOREGROUND_INTENSITY, false, 0x000f);
}

void light_gray(std::string message)
{
  print_style(message, FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY, false, 0x000f);
}

void bold(std::string message)
{
  // The classic Windows console represents emphasis with foreground intensity.
  print_style(message, FOREGROUND_INTENSITY, false);
}

void underline(std::string message)
{
  print_style(message, COMMON_LVB_UNDERSCORE, true);
}

void blink(std::string)
{
  throw std::logic_error("Blink is not supported by the Windows console backend");
}

void reverse(std::string message)
{
  std::vector<std::pair<std::size_t, std::size_t>> clusters;
  std::size_t start = 0;
  utf8proc_int32_t previous = 0;
  utf8proc_int32_t state = 0;
  for (std::size_t offset = 0; offset < message.size();)
  {
    utf8proc_int32_t codepoint = 0;
    const auto count = utf8proc_iterate(
        reinterpret_cast<const utf8proc_uint8_t *>(message.data() + offset),
        static_cast<utf8proc_ssize_t>(message.size() - offset), &codepoint);
    if (count < 0)
      throw std::invalid_argument("reverse requires valid UTF-8");
    if (offset > 0 && utf8proc_grapheme_break_stateful(previous, codepoint, &state))
    {
      clusters.emplace_back(start, offset - start);
      start = offset;
    }
    previous = codepoint;
    offset += static_cast<std::size_t>(count);
  }
  if (!message.empty()) clusters.emplace_back(start, message.size() - start);
  std::string reversed;
  reversed.reserve(message.size());
  for (auto cluster = clusters.rbegin(); cluster != clusters.rend(); ++cluster)
    reversed.append(message, cluster->first, cluster->second);
  std::cout << reversed << std::endl;
}
