#include "Windows.h"
#include "../../packages/c++/main.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>

void check(bool condition, const std::string& message) {
  if (!condition) throw std::runtime_error(message);
}
struct Capture {
  std::ostringstream buffer;
  std::streambuf* old = std::cout.rdbuf(buffer.rdbuf());
  ~Capture() { std::cout.rdbuf(old); }
};
struct FailingBuffer : std::streambuf {
  std::streamsize xsputn(const char*, std::streamsize) override {
    throw std::runtime_error("simulated output failure");
  }
};

int main(int argc, char** argv) {
  using namespace fake_console;
  try {
    check(argc == 2, "Expected one test case name");
    const std::string test = argv[1];
    if (test == "bold") {
      Capture output;
      bold("bold text");
      check(output.buffer.str() == "bold text\n", "Bold must print the message once");
      check(attribute_calls == std::vector<WORD>{0x2c, 0x24}, "Bold must add intensity, preserve colors, and restore attributes");
      check(mode_calls.empty(), "Bold must not change console mode");
    } else if (test == "underline" || test == "underline-enabled") {
      if (test == "underline-enabled") mode = 0x13;
      const DWORD initial_mode = mode;
      Capture output;
      underline("underlined text");
      check(output.buffer.str() == "underlined text\n", "Underline must print the message once");
      check(attribute_calls == std::vector<WORD>{0x8024, 0x24}, "Underline must set COMMON_LVB_UNDERSCORE, not intensity");
      check(mode == initial_mode, "Underline must restore the initial mode");
      if (test == "underline") check(mode_calls == std::vector<DWORD>{0x13, 0x03}, "Enable LVB for every code page, then restore mode");
      else check(mode_calls.empty(), "Already-enabled underline mode must remain unchanged");
    } else if (test == "blink") {
      Capture output;
      bool rejected = false;
      try { blink("must not print"); } catch (const std::logic_error& error) {
        rejected = std::string(error.what()).find("not supported") != std::string::npos;
      }
      check(rejected, "Blink must explicitly report unsupported behavior");
      check(output.buffer.str().empty(), "Unsupported blink must not print misleading output");
      check(attribute_calls.empty() && mode_calls.empty(), "Unsupported blink must not change console state");
    } else if (test == "write-failure") {
      FailingBuffer failure;
      auto* old_buffer = std::cout.rdbuf(&failure);
      const auto old_exceptions = std::cout.exceptions();
      std::cout.exceptions(std::ios::badbit | std::ios::failbit);
      bool rejected = false;
      try { underline("failing write"); } catch (const std::exception&) { rejected = true; }
      std::cout.exceptions(std::ios::goodbit);
      std::cout.clear();
      std::cout.rdbuf(old_buffer);
      std::cout.exceptions(old_exceptions);
      check(rejected, "Output failure must propagate");
      check(attribute_calls == std::vector<WORD>{0x8024, 0x24}, "Output failure must restore attributes");
      check(mode_calls == std::vector<DWORD>{0x13, 0x03}, "Output failure must restore mode");
    } else {
      if (test == "info-failure") fail_info = true;
      else if (test == "get-mode-failure") fail_get_mode = true;
      else if (test == "set-mode-failure") fail_set_mode = true;
      else if (test == "attribute-failure") fail_next_attribute = true;
      else throw std::runtime_error("Unknown test case");
      Capture output;
      bool rejected = false;
      try { underline("must not print"); } catch (const std::runtime_error&) { rejected = true; }
      check(rejected, "A failed console API must be reported");
      check(output.buffer.str().empty(), "Style failure must occur before output");
      check(mode == 0x03, "API failure must leave the original console mode");
    }
    check(attributes == 0x24, "The caller's colors and background must be preserved");
    return 0;
  } catch (const std::exception& error) {
    std::cerr << error.what() << '\n';
    return 1;
  }
}
