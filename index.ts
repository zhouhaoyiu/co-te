import back from "./packages/code-back/index.js";
import font from "./packages/cote-font/index.js";
import style from "./packages/code-style/index.js";
import { createTheme, format, palettes } from "./themes.js";

export { createTheme, format, palettes } from "./themes.js";
export type { Color, ColorLevel, FormatOptions, Palette, Theme, ThemeName, ThemeOptions, ThemeRole } from "./themes.js";

export default {
  back,
  font,
  style,
  createTheme,
  format,
  palettes,
};
