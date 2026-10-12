export type ColorLevel = 0 | 1 | 2 | 3;
export type Color = string | readonly [number, number, number];
export interface FormatOptions {
  foreground?: Color;
  background?: Color;
  bold?: boolean;
  level?: ColorLevel;
}
export interface Palette {
  readonly name: string;
  readonly background: string;
  readonly text: string;
  readonly muted: string;
  readonly info: string;
  readonly success: string;
  readonly warn: string;
  readonly error: string;
  readonly accent: string;
}

// Terminal palette values; source URLs and licenses are in THIRD_PARTY_NOTICES.md.
export const palettes = Object.freeze({
  mocha: Object.freeze({
    name: "Catppuccin Mocha", background: "#1e1e2e", text: "#cdd6f4", muted: "#a6adc8",
    info: "#89b4fa", success: "#a6e3a1", warn: "#f9e2af", error: "#f38ba8", accent: "#94e2d5",
  }),
  "tokyo-night": Object.freeze({
    name: "Tokyo Night", background: "#1a1b26", text: "#c0caf5", muted: "#a9b1d6",
    info: "#7aa2f7", success: "#9ece6a", warn: "#e0af68", error: "#f7768e", accent: "#7dcfff",
  }),
  latte: Object.freeze({
    name: "Catppuccin Latte", background: "#eff1f5", text: "#4c4f69", muted: "#5c5f77",
    info: "#1e66f5", success: "#40a02b", warn: "#df8e1d", error: "#d20f39", accent: "#179299",
  }),
} satisfies Record<string, Palette>);
export type ThemeName = keyof typeof palettes;
export type ThemeRole = "text" | "muted" | "info" | "success" | "warn" | "error" | "accent";
export type Theme = Readonly<{ palette: Palette } & Record<ThemeRole, (message: string) => string>>;
export interface ThemeOptions {
  level?: ColorLevel;
  /** Apply the palette background to formatted text only. */
  background?: boolean;
}

type Runtime = {
  env?: Record<string, string | undefined>;
  stdout?: { isTTY?: boolean; getColorDepth?: () => number };
};
function detectLevel(): ColorLevel {
  const runtime = (globalThis as typeof globalThis & { process?: Runtime }).process;
  const env = runtime?.env ?? {};
  if (env.NO_COLOR !== undefined) return 0;
  if (env.FORCE_COLOR !== undefined) {
    if (env.FORCE_COLOR === "0" || env.FORCE_COLOR === "false") return 0;
    if (env.FORCE_COLOR === "3") return 3;
    if (env.FORCE_COLOR === "2") return 2;
    return 1;
  }
  if (!runtime?.stdout?.isTTY || env.TERM === "dumb") return 0;
  const depth = runtime.stdout.getColorDepth?.() ?? 4;
  return depth >= 24 ? 3 : depth >= 8 ? 2 : 1;
}
function levelOf(level?: ColorLevel): ColorLevel {
  const resolved = level ?? detectLevel();
  if (![0, 1, 2, 3].includes(resolved)) throw new RangeError("Color level must be 0, 1, 2 or 3");
  return resolved;
}
function channels(color: Color): readonly [number, number, number] {
  if (typeof color === "string") {
    if (!/^#[\da-f]{6}$/i.test(color)) throw new TypeError("Colors must use #RRGGBB or an RGB tuple");
    return [1, 3, 5].map(offset => parseInt(color.slice(offset, offset + 2), 16)) as [number, number, number];
  }
  if (!Array.isArray(color) || color.length !== 3 || color.some(value => !Number.isInteger(value) || value < 0 || value > 255)) {
    throw new RangeError("RGB channels must be integers from 0 to 255");
  }
  return color;
}
const ansiColors: readonly (readonly number[])[] = [
  [0, 0, 0], [128, 0, 0], [0, 128, 0], [128, 128, 0], [0, 0, 128], [128, 0, 128], [0, 128, 128], [192, 192, 192],
  [128, 128, 128], [255, 0, 0], [0, 255, 0], [255, 255, 0], [0, 0, 255], [255, 0, 255], [0, 255, 255], [255, 255, 255],
];
function indexedColor(index: number): readonly number[] {
  if (index < 16) return ansiColors[index];
  if (index >= 232) return [8 + (index - 232) * 10, 8 + (index - 232) * 10, 8 + (index - 232) * 10];
  const cube = [0, 95, 135, 175, 215, 255];
  const value = index - 16;
  return [cube[Math.floor(value / 36)], cube[Math.floor(value / 6) % 6], cube[value % 6]];
}
function colorCode(color: Color, background: boolean, level: ColorLevel): string {
  const rgb = channels(color);
  if (level === 3) return `${background ? 48 : 38};2;${rgb.join(";")}`;
  let best = 0;
  let distance = Infinity;
  for (let index = 0; index < (level === 2 ? 256 : 16); index++) {
    const candidate = indexedColor(index);
    const delta = rgb.reduce((sum, value, channel) => sum + (value - candidate[channel]) ** 2, 0);
    if (delta < distance) { distance = delta; best = index; }
  }
  if (level === 2) return `${background ? 48 : 38};5;${best}`;
  return String((background ? 40 : 30) + (best >= 8 ? 60 : 0) + best % 8);
}

/** Return styled text without writing to stdout. Explicit level overrides detection. */
export function format(message: string, options: FormatOptions = {}): string {
  if (typeof message !== "string") throw new TypeError("The message must be a string");
  const level = levelOf(options.level);
  const codes: string[] = [];
  if (options.foreground !== undefined) codes.push(colorCode(options.foreground, false, level));
  if (options.background !== undefined) codes.push(colorCode(options.background, true, level));
  if (options.bold) codes.push("1");
  if (!level || !message || !codes.length) return message;
  const open = `\x1b[${codes.join(";")}m`;
  // Inner format() calls reset their styles. Reapply the outer style after each reset.
  return open + message.replaceAll("\x1b[0m", "\x1b[0m" + open) + "\x1b[0m";
}

/** Construct pure formatters for one of the three built-in palettes. */
export function createTheme(name: ThemeName = "mocha", options: ThemeOptions = {}): Theme {
  if (!Object.hasOwn(palettes, name)) throw new RangeError(`Unknown theme: ${String(name)}`);
  const palette: Palette = palettes[name];
  const level = levelOf(options.level);
  const role = (foreground: string) => (message: string) => format(message, {
    foreground, background: options.background ? palette.background : undefined, level,
  });
  return Object.freeze({
    palette, text: role(palette.text), muted: role(palette.muted), info: role(palette.info),
    success: role(palette.success), warn: role(palette.warn), error: role(palette.error), accent: role(palette.accent),
  });
}
