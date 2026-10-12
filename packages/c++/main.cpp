#include <iostream>
#include <string>
#include <stdexcept>
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

void red(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void green(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void blue(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void yellow(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void magenta(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_BLUE);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void cyan(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void white(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void black(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void gray(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void light_red(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_INTENSITY);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void light_green(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_INTENSITY);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void light_blue(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_INTENSITY);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void light_yellow(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_INTENSITY);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void light_magenta(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void light_cyan(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void light_white(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void light_black(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), 0 | FOREGROUND_INTENSITY);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

void light_gray(std::string message)
{
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE | FOREGROUND_INTENSITY);
  std::cout << message << std::endl;
  SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), FOREGROUND_BLUE | FOREGROUND_GREEN | FOREGROUND_RED);
}

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

  void print_style(const std::string &message, WORD attribute, bool enable_underline)
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
    if (!SetConsoleTextAttribute(handle, info.wAttributes | attribute))
      throw std::runtime_error("Cannot apply console style");
    std::cout << message << std::endl;
  }
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
  std::string reverse_message;
  for (int i = message.length() - 1; i >= 0; i--)
  {
    reverse_message += message[i];
  }
  std::cout << reverse_message << std::endl;
}
