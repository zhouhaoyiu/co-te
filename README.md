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

## 构建与打包

```sh
pnpm install --frozen-lockfile
pnpm run build
npm pack
```

也可在任一子包目录运行 `npm pack`。各包的 `prepack` 会自动构建 JavaScript 和 `.d.ts` 文件；生成文件在 `dist` 中，打包内容不依赖 TypeScript 运行时。安装生成的 `.tgz` 后即可使用标准 ESM `import`。

导入路径遵循 Node 的 ESM 文件规则，类型检查使用 `NodeNext`。源码开发工具由根工作区统一安装。

## 测试

```sh
pnpm run lint
pnpm run type-check
npm test
npm run test:cpp
```

JavaScript 测试会实际打包全部四个包，在临时项目中离线安装，验证静默导入、颜色输出、Unicode 文本和消费者的类型声明解析。测试不会发布到 npm。

C++ 测试需要 CMake 和 C++17 编译器。所有平台运行控制台 API 模拟测试；Windows 额外编译原始示例，并用实际 Windows 控制台 API 检查输出时的属性及输出后的状态恢复。GitHub Actions 在 Linux/Windows、Node 22/26 上运行这些检查。

## C++ 控制台样式

`packages/c++` 使用 Windows 控制台后端：

- `bold` 增加前景亮度，保留原有前景色和背景色。经典控制台的这个属性不保证改变字体字重。
- `underline` 设置 `COMMON_LVB_UNDERSCORE`，临时启用 `ENABLE_LVB_GRID_WORLDWIDE`，使下划线属性能用于各代码页。
- 这两个函数在输出后恢复原来的颜色和控制台模式，输出抛出异常时也会执行恢复。控制台 API 失败会抛出 `std::runtime_error`；重定向到非控制台句柄时不承诺样式输出。
- `blink` 明确抛出 `std::logic_error`，说明此 Windows 后端不支持闪烁，且不输出文字或修改控制台状态。

属性定义参见 [Microsoft 控制台字符属性](https://learn.microsoft.com/en-us/windows/console/console-screen-buffers#character-attributes) 和 [SetConsoleMode](https://learn.microsoft.com/en-us/windows/console/setconsolemode)。
