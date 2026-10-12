const segmenter = new Intl.Segmenter(undefined, { granularity: "grapheme" });

const style: IStyle = {
  // Print successive prefixes without splitting visible characters.
  oneByOne: (message: string) => {
    let prefix = "";
    for (const { segment } of segmenter.segment(message)) {
      prefix += segment;
      console.log("\x1b[0m%s\x1b[0m", prefix);
    }
  },
};

export interface IStyle {
  oneByOne: (message: string) => void;
}

export default style;
