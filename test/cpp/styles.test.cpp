#include "Windows.h"
#include "../../packages/c++/main.h"
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <string>
#include <utility>

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
    } else if (test == "colors") {
      const std::vector<std::pair<void (*)(std::string), WORD>> colors = {
        {red, 4}, {green, 2}, {blue, 1}, {yellow, 6}, {magenta, 5}, {cyan, 3},
        {white, 7}, {black, 0}, {gray, 7}, {light_red, 12}, {light_green, 10},
        {light_blue, 9}, {light_yellow, 14}, {light_magenta, 13}, {light_cyan, 11},
        {light_white, 15}, {light_black, 8}, {light_gray, 15}
      };
      for (const auto& color : colors) {
        attributes = 0x80bc;
        attribute_calls.clear();
        Capture output;
        color.first("color text");
        check(output.buffer.str() == "color text\n", "Colors must print exactly once");
        check(attribute_calls == std::vector<WORD>{static_cast<WORD>(0x80b0 | color.second), 0x80bc},
              "Colors must preserve background/style bits and restore the exact initial attributes");
        check(attributes == 0x80bc && mode_calls.empty(), "Color call leaked console state");
      }
      attributes = 0x24;
    } else if (test == "reverse") {
      const std::vector<std::pair<std::string, std::string>> examples = {
        {"", ""}, {"abc", "cba"}, {u8"A😀中", u8"中😀A"},
        {u8"a👨‍👩‍👧‍👦b", u8"b👨‍👩‍👧‍👦a"}, {u8"a👍🏽b", u8"b👍🏽a"},
        {u8"🇨🇳🇺🇸!", u8"!🇺🇸🇨🇳"}, {u8"e\u0301x", u8"xe\u0301"},
        {u8"a👩🏽‍💻b", u8"b👩🏽‍💻a"}, {"a\r\nb", "b\r\na"},
        {std::string("a\0b", 3), std::string("b\0a", 3)}
      };
      for (const auto& example : examples) {
        Capture output;
        reverse(example.first);
        check(output.buffer.str() == example.second + "\n", "reverse split or damaged a grapheme cluster");
      }
    } else if (test == "reverse-invalid") {
      for (const std::string input : {"a\xc0\xaf", "\xed\xa0\x80", "\xf4\x90\x80\x80", "\xf0\x9f", "\x80"}) {
        Capture output;
        bool rejected = false;
        try { reverse(input); } catch (const std::invalid_argument&) { rejected = true; }
        check(rejected && output.buffer.str().empty(), "Invalid UTF-8 must fail before any output");
      }
    } else if (test == "color-api-failure") {
      for (const bool info_failure : {true, false}) {
        fail_info = info_failure;
        fail_next_attribute = !info_failure;
        Capture output;
        bool rejected = false;
        try { red("must not print"); } catch (const std::runtime_error&) { rejected = true; }
        check(rejected && output.buffer.str().empty(), "Color API failures must be reported before output");
        check(attributes == 0x24, "Color API failure leaked attributes");
      }
    } else if (test == "write-failure" || test == "color-write-failure") {
      FailingBuffer failure;
      auto* old_buffer = std::cout.rdbuf(&failure);
      const auto old_exceptions = std::cout.exceptions();
      std::cout.exceptions(std::ios::badbit | std::ios::failbit);
      bool rejected = false;
      try {
        if (test == "color-write-failure") blue("failing write");
        else underline("failing write");
      } catch (const std::exception&) { rejected = true; }
      std::cout.exceptions(std::ios::goodbit);
      std::cout.clear();
      std::cout.rdbuf(old_buffer);
      std::cout.exceptions(old_exceptions);
      check(rejected, "Output failure must propagate");
      if (test == "color-write-failure") {
        check(attribute_calls == std::vector<WORD>{0x21, 0x24}, "Color output failure must restore attributes");
        check(mode_calls.empty(), "Color output failure must preserve mode");
      } else {
        check(attribute_calls == std::vector<WORD>{0x8024, 0x24}, "Output failure must restore attributes");
        check(mode_calls == std::vector<DWORD>{0x13, 0x03}, "Output failure must restore mode");
      }
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
