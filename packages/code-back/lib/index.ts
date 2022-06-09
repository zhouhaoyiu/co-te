/* eslint-disable no-unused-vars */

// 改变不同背景颜色
const back: IBack = {
  red: (message: string) => {
    console.log("\x1b[41m%s\x1b[0m", message);
  },
  yellow: (message: string) => {
    console.log("\x1b[43m%s\x1b[0m", message);
  },
  blue: (message: string) => {
    console.log("\x1b[44m%s\x1b[0m", message);
  },
  green: (message: string) => {
    console.log("\x1b[42m%s\x1b[0m", message);
  },
  magenta: (message: string) => {
    console.log("\x1b[45m%s\x1b[0m", message);
  },
  cyan: (message: string) => {
    console.log("\x1b[46m%s\x1b[0m", message);
  },
  white: (message: string) => {
    console.log("\x1b[47m%s\x1b[0m", message);
  },
  gray: (message: string) => {
    console.log("\x1b[100m%s\x1b[0m", message);
  },
  black: (message: string) => {
    console.log("\x1b[40m%s\x1b[0m", message);
  },
  default: (message: string) => {
    console.log("\x1b[0m%s\x1b[0m", message);
  },
};

interface IBack {
  red: (message: string) => void;
  yellow: (message: string) => void;
  blue: (message: string) => void;
  green: (message: string) => void;
  magenta: (message: string) => void;
  cyan: (message: string) => void;
  white: (message: string) => void;
  gray: (message: string) => void;
  black: (message: string) => void;
  default: (message: string) => void;
}

export default back;