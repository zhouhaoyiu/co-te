#include <Windows.h>
#include "../../packages/c++/main.h"
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>

void check(bool condition, const char* message) {
  if (!condition) throw std::runtime_error(message);
}
struct Observation { WORD attributes; DWORD mode; };
struct ConsoleCapture : std::streambuf {
  HANDLE handle;
  std::vector<Observation> observations;
  std::string text;
  explicit ConsoleCapture(HANDLE output) : handle(output) {}
  void observe() {
    CONSOLE_SCREEN_BUFFER_INFO info{};
    DWORD mode = 0;
    check(GetConsoleScreenBufferInfo(handle, &info) != 0, "Cannot inspect console attributes");
    check(GetConsoleMode(handle, &mode) != 0, "Cannot inspect console mode");
    observations.push_back({info.wAttributes, mode});
  }
  std::streamsize xsputn(const char* value, std::streamsize size) override {
    observe();
    text.append(value, static_cast<std::size_t>(size));
    return size;
  }
  int overflow(int value) override {
    if (value != traits_type::eof()) {
      observe();
      text.push_back(static_cast<char>(value));
    }
    return traits_type::not_eof(value);
  }
};

int main() {
  // The hosted Windows runner may launch tests without an attached console.
  if (!AttachConsole(ATTACH_PARENT_PROCESS)) {
    const DWORD error = GetLastError();
    if (error != ERROR_ACCESS_DENIED && !AllocConsole()) return 1;
  }
  const HANDLE original = GetStdHandle(STD_OUTPUT_HANDLE);
  HANDLE buffer = CreateConsoleScreenBuffer(GENERIC_READ | GENERIC_WRITE,
      FILE_SHARE_READ | FILE_SHARE_WRITE, nullptr, CONSOLE_TEXTMODE_BUFFER, nullptr);
  if (buffer == INVALID_HANDLE_VALUE) return 1;
  auto* original_stream = std::cout.rdbuf();
  int result = 0;
  try {
    check(SetStdHandle(STD_OUTPUT_HANDLE, buffer) != 0, "Cannot set test output handle");
    constexpr WORD attributes = BACKGROUND_GREEN | FOREGROUND_RED;
    constexpr DWORD mode = ENABLE_PROCESSED_OUTPUT | ENABLE_WRAP_AT_EOL_OUTPUT;
    check(SetConsoleTextAttribute(buffer, attributes) != 0, "Cannot initialize attributes");
    check(SetConsoleMode(buffer, mode) != 0, "Cannot initialize console mode");
    for (const bool is_underline : {false, true}) {
      ConsoleCapture capture(buffer);
      std::cout.rdbuf(&capture);
      if (is_underline) underline("native text"); else bold("native text");
      std::cout.rdbuf(original_stream);
      check(capture.text == "native text\n", "Native style changed the message");
      check(!capture.observations.empty(), "No output observations");
      for (const auto& seen : capture.observations) {
        const WORD expected = attributes | (is_underline ? COMMON_LVB_UNDERSCORE : FOREGROUND_INTENSITY);
        check(seen.attributes == expected, "Wrong native attributes during output");
        check(seen.mode == (is_underline ? mode | ENABLE_LVB_GRID_WORLDWIDE : mode), "Wrong native console mode during output");
      }
      CONSOLE_SCREEN_BUFFER_INFO restored{};
      DWORD restored_mode = 0;
      check(GetConsoleScreenBufferInfo(buffer, &restored) != 0 && restored.wAttributes == attributes, "Native attributes not restored");
      check(GetConsoleMode(buffer, &restored_mode) != 0 && restored_mode == mode, "Native mode not restored");
    }
    ConsoleCapture capture(buffer);
    std::cout.rdbuf(&capture);
    bool rejected = false;
    try { blink("unsupported"); } catch (const std::logic_error&) { rejected = true; }
    std::cout.rdbuf(original_stream);
    check(rejected && capture.text.empty(), "Blink must reject without output");
  } catch (const std::exception& error) {
    std::cout.rdbuf(original_stream);
    std::cerr << error.what() << '\n';
    result = 1;
  }
  SetStdHandle(STD_OUTPUT_HANDLE, original);
  CloseHandle(buffer);
  return result;
}
