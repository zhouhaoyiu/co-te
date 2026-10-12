# Colorful-Terminal

终端文字、背景颜色和逐字输出工具。根包 `co-te` 导出 `back`、`font`、`style`，三个子包也可独立使用。Node.js 要求为 22.12 或以上。

```js
import co from 'co-te';

co.font.red('红色文字');
co.back.blue('蓝色背景');
co.style.oneByOne('你好😀');
```

```js
import font from 'cote-font';
import back from 'code-back';
import style from 'code-style';
```

导入模块不会输出文字。`oneByOne` 同步逐行输出累积前缀，按字素簇分割；组合 emoji、肤色修饰符、旗帜和组合附加符不会被拆开。它没有计时或动画延迟。例如 `oneByOne('A😀B')` 输出 `A`、`A😀`、`A😀B` 三行。

## 三套主题

根包提供 `mocha`（Catppuccin Mocha）、`tokyo-night`（Tokyo Night）和 `latte`（Catppuccin Latte）。主题方法返回字符串，可以在同一行组合；现有 `font`、`back` 方法仍直接打印。

```js
import co, { createTheme, format, palettes } from 'co-te';

const theme = createTheme('mocha');
console.log(theme.success('✓ 完成') + ' ' + theme.muted('3 files'));
console.log(theme.info('提示'), theme.warn('警告'), theme.error('错误'));

// 指定 24 位颜色，适用于支持 true color 的终端。
const night = co.createTheme('tokyo-night', { level: 3, background: true });
console.log(night.text('Tokyo Night'));
console.log(format('RGB 文字', { foreground: [137, 180, 250], bold: true, level: 3 }));
console.log(palettes.latte.background); // #eff1f5
```

每个主题提供 `text`、`muted`、`info`、`success`、`warn`、`error` 和 `accent`。`background: true` 只给这段文字添加主题背景，不修改终端设置。颜色及上游许可证见 [第三方声明](THIRD_PARTY_NOTICES.md)。

`format(message, options)` 接受 `foreground`、`background`、`bold` 和 `level`；颜色用 `#RRGGBB` 或整数 RGB 三元组。嵌套 `format()` 时，内层结束后会恢复外层样式。非法颜色、主题名或颜色级别会抛出异常。

颜色级别为 `0`（无颜色）、`1`（16 色近似）、`2`（256 色近似）、`3`（24 位颜色）。未指定级别时，重定向输出及 `NO_COLOR` 会关闭颜色；`FORCE_COLOR=1/2/3` 可指定能力，其余情况按终端能力检测。`NO_COLOR` 优先于环境中的 `FORCE_COLOR`，显式传入 `level` 则覆盖自动检测。精确色值需要 24 位颜色终端，16 色结果仍受用户终端色板影响。上述检测作用于新格式化接口，旧打印方法保持原行为。

## 构建与打包

```sh
pnpm install --frozen-lockfile
pnpm run build
npm pack
```

也可在任一子包目录运行 `npm pack`。各包的 `prepack` 会自动构建 JavaScript 和 `.d.ts` 文件；生成文件在 `dist` 中，打包内容不依赖 TypeScript 运行时。安装生成的 `.tgz` 后即可使用标准 ESM `import`。

根包还支持 Git 依赖安装：`npm install github:zhouhaoyiu/co-te`。根包的 `prepare` 会在 Git 安装过程中编译源码，须启用安装脚本并能下载开发依赖；已构建的 `.tgz` 不需要消费者编译。

导入路径遵循 Node 的 ESM 文件规则，类型检查使用 `NodeNext`。源码开发工具由根工作区统一安装。

## 测试

```sh
pnpm run lint
pnpm run type-check
npm test
npm run test:cpp
```

JavaScript 测试会实际打包全部四个包，在临时项目中离线安装，验证静默导入、颜色输出、Unicode 文本和消费者的类型声明解析。另有空缓存 Git 安装测试，需要访问 npm registry，并验证三套主题、嵌套格式化和无颜色输出。测试不会发布到 npm。

C++ 测试需要 CMake 和 C++17 编译器。所有平台运行控制台 API 模拟测试；Windows 额外编译原始示例，并用实际 Windows 控制台 API 检查输出时的属性及输出后的状态恢复。GitHub Actions 在 Linux/Windows、Node 22/26 上运行这些检查。

## C++ 控制台样式

`packages/c++` 使用 Windows 控制台后端：

- `bold` 增加前景亮度，保留原有前景色和背景色。经典控制台的这个属性不保证改变字体字重。
- `underline` 设置 `COMMON_LVB_UNDERSCORE`，临时启用 `ENABLE_LVB_GRID_WORLDWIDE`，使下划线属性能用于各代码页。
- 这两个函数在输出后恢复原来的颜色和控制台模式，输出抛出异常时也会执行恢复。控制台 API 失败会抛出 `std::runtime_error`；重定向到非控制台句柄时不承诺样式输出。
- 所有普通色和亮色函数只替换前景色位，保留背景及其他属性，输出后恢复调用前的完整属性。输出异常同样触发恢复。
- `blink` 明确抛出 `std::logic_error`，说明此 Windows 后端不支持闪烁，且不输出文字或修改控制台状态。
- `reverse` 接受 UTF-8，按字素簇倒序，保留中文、组合附加符、旗帜、肤色及 ZWJ emoji。非法 UTF-8 会在任何输出前抛出 `std::invalid_argument`；空字符串输出一个换行。终端显示 UTF-8 字节仍需要合适的代码页和字体。

字素簇分割使用仓库内固定版本的 utf8proc。C++17 编译时应链接它；在 Windows 上可直接使用提供的 CMake 项目：

```sh
cmake -S packages/c++ -B .build/console
cmake --build .build/console --config Release
```

集成到其他 CMake 项目时，添加该目录并链接 `cote_console`，utf8proc 会一并链接。Node.js 包的主题功能与这个 Windows 控制台后端是独立接口。

属性定义参见 [Microsoft 控制台字符属性](https://learn.microsoft.com/en-us/windows/console/console-screen-buffers#character-attributes) 和 [SetConsoleMode](https://learn.microsoft.com/en-us/windows/console/setconsolemode)。
