/* eslint-disable no-unused-vars */

const style: IStyle = {
  // 一个一个地出现
  oneByOne: (message: string) => {
    let _message = "";
    for (let i = 0; i < message.length; i++) {
      _message += message[i];
      console.log("\x1b[0m%s\x1b[0m", _message);
    }
  },
};

interface IStyle {
  oneByOne: (message: string) => void;
}
style.oneByOne("hello world");
export default style;
