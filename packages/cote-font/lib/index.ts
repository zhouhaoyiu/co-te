#!/usr/bin/env node
/* eslint-disable no-unused-vars */
const font: IFont = {
  red: (message: string) => {
    console.log("\x1b[31m%s\x1b[0m", message);
  },
  yellow: (message: string) => {
    console.log("\x1b[33m%s\x1b[0m", message);
  },
  blue: (message: string) => {
    console.log("\x1b[34m%s\x1b[0m", message);
  },
  green: (message: string) => {
    console.log("\x1b[32m%s\x1b[0m", message);
  },
  magenta: (message: string) => {
    console.log("\x1b[35m%s\x1b[0m", message);
  },
  cyan: (message: string) => {
    console.log("\x1b[36m%s\x1b[0m", message);
  },
  white: (message: string) => {
    console.log("\x1b[37m%s\x1b[0m", message);
  },
  gray: (message: string) => {
    console.log("\x1b[90m%s\x1b[0m", message);
  },
  black: (message: string) => {
    console.log("\x1b[30m%s\x1b[0m", message);
    return this;
  },
  // 加粗
  bold: (message: string) => {
    console.log("\x1b[1m%s\x1b[0m", message);
  },
  // 下划线
  underline: (message: string) => {
    console.log("\x1b[4m%s\x1b[0m", message);
  },
  // 反显
  inverse: (message: string) => {
    console.log("\x1b[7m%s\x1b[0m", message);
  },
  // 闪烁
  strikethrough: (message: string) => {
    console.log("\x1b[9m%s\x1b[0m", message);
  },
  // 清除
  clear: function () {
    console.clear();
  },
  // 换行
  newline: function () {
    console.log("\n");
  },
  default: (message: string) => {
    console.log("\x1b[0m%s\x1b[0m", message);
  },
};

// 下划线
interface IFont {
  red: (message: string) => void;
  yellow: (message: string) => void;
  blue: (message: string) => void;
  green: (message: string) => void;
  magenta: (message: string) => void;
  cyan: (message: string) => void;
  white: (message: string) => void;
  gray: (message: string) => void;
  black: (message: string) => void;
  bold: (message: string) => void;
  underline: (message: string) => void;
  inverse: (message: string) => void;
  strikethrough: (message: string) => void;
  clear: () => void;
  newline: () => void;
  default: (message: string) => void;
}
export default font;
