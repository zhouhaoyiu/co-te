import { after, before, test } from 'node:test';
import assert from 'node:assert/strict';
import { copyFileSync, existsSync, mkdirSync, mkdtempSync, readFileSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join } from 'node:path';
import { fileURLToPath, pathToFileURL } from 'node:url';
import { createRequire } from 'node:module';
import { spawnSync } from 'node:child_process';

const repo = fileURLToPath(new URL('../', import.meta.url));
const scratch = mkdtempSync(join(tmpdir(), 'co-te package tests-'));
const consumer = join(scratch, 'consumer');
const npmCLI = process.env.npm_execpath;
assert.ok(npmCLI?.endsWith('npm-cli.js'), 'Run this suite with npm test or npm run test:js');
const env = { ...process.env, npm_config_update_notifier: 'false' };
delete env.NODE_OPTIONS;
for (const name of Object.keys(env)) if (name.startsWith('GIT_')) delete env[name];
env.GIT_CONFIG_NOSYSTEM = '1';
env.GIT_CONFIG_GLOBAL = join(scratch, 'gitconfig');
writeFileSync(env.GIT_CONFIG_GLOBAL, '[user]\nname = Package Test\nemail = package@example.invalid\n[commit]\ngpgSign = false\n');
const packages = [
  { name: 'co-te', directory: repo },
  ...['code-back', 'cote-font', 'code-style'].map(name => ({ name, directory: join(repo, 'packages', name) })),
];
const archives = new Map();
function run(command, args, cwd = consumer, check = true) {
  const result = spawnSync(command, args, { cwd, env, encoding: 'utf8', timeout: 120000 });
  assert.ifError(result.error);
  if (check) assert.equal(result.status, 0, `${command}: ${result.stdout}\n${result.stderr}`);
  return result;
}
const npm = (args, cwd) => run(process.execPath, [npmCLI, ...args], cwd);
const node = source => run(process.execPath, ['--input-type=module', '-e', source]);
after(() => rmSync(scratch, { recursive: true, force: true }));

before(() => {
  mkdirSync(consumer);
  writeFileSync(join(consumer, 'package.json'), '{"private":true,"type":"module"}\n');
  // A clean checkout must pack successfully without a separate build command.
  rmSync(join(repo, 'dist'), { recursive: true, force: true });
  for (const item of packages) {
    const result = npm(['pack', '--json', '--ignore-scripts=false', '--pack-destination', scratch], item.directory);
    const [archive] = JSON.parse(result.stdout);
    archives.set(item.name, archive);
  }
  npm(['install', '--offline', '--ignore-scripts', '--no-audit', '--no-fund', ...[...archives.values()].map(a => join(scratch, a.filename))], consumer);
});

for (const { name } of packages) {
  test(`${name}: the installed tarball imports silently without a TypeScript loader`, () => {
    const result = node(`await import(${JSON.stringify(name)});`);
    assert.equal(result.stdout, '');
    assert.equal(result.stderr, '');
  });

  test(`${name}: the archive ships its JavaScript and type entry points`, () => {
    const names = archives.get(name).files.map(file => file.path);
    assert.ok(names.includes('dist/index.js'));
    assert.ok(names.includes('dist/index.d.ts'));
    assert.ok(!names.some(file => /\.tsx?$/.test(file) && !file.endsWith('.d.ts')));
    assert.ok(!names.some(file => file.endsWith('.exe')));
    const metadata = JSON.parse(readFileSync(join(consumer, 'node_modules', name, 'package.json'), 'utf8'));
    assert.ok(existsSync(join(consumer, 'node_modules', name, metadata.main)));
    assert.ok(existsSync(join(consumer, 'node_modules', name, metadata.types)));
    assert.equal(Object.keys(metadata.dependencies ?? {}).length, 0);
  });
}

test('root and standalone packages expose working color methods', () => {
  const result = node(`
    import co from 'co-te';
    import back from 'code-back';
    import font from 'cote-font';
    const calls = [];
    console.log = (...args) => calls.push(args);
    co.back.red('background');
    back.blue('blue');
    co.font.red('red');
    font.underline('underlined');
    process.stdout.write(JSON.stringify(calls));
  `);
  assert.deepEqual(JSON.parse(result.stdout), [
    ['\x1b[41m%s\x1b[0m', 'background'],
    ['\x1b[44m%s\x1b[0m', 'blue'],
    ['\x1b[31m%s\x1b[0m', 'red'],
    ['\x1b[4m%s\x1b[0m', 'underlined'],
  ]);
});

for (const [message, prefixes] of [
  ['abc', ['a', 'ab', 'abc']],
  ['😀', ['😀']],
  ['A😀B', ['A', 'A😀', 'A😀B']],
  ['👨‍👩‍👧‍👦!', ['👨‍👩‍👧‍👦', '👨‍👩‍👧‍👦!']],
  ['👍🏽', ['👍🏽']],
  ['🇨🇳', ['🇨🇳']],
  ['e\u0301', ['e\u0301']],
  ['你好', ['你', '你好']],
  ['', []],
]) {
  test(`oneByOne preserves complete graphemes: ${JSON.stringify(message)}`, () => {
    for (const name of ['co-te', 'code-style']) {
      const result = node(`
        import api from ${JSON.stringify(name)};
        const style = ${name === 'co-te' ? 'api.style' : 'api'};
        style.oneByOne(${JSON.stringify(message)});
      `);
      assert.equal(result.stdout, prefixes.map(prefix => `\x1b[0m${prefix}\x1b[0m\n`).join(''));
      assert.ok(!result.stdout.includes('\ufffd'));
      assert.equal(result.stderr, '');
    }
  });
}

test('an independent TypeScript consumer resolves the published declarations', () => {
  writeFileSync(join(consumer, 'usage.mts'), `
    import co from 'co-te';
    import back from 'code-back';
    import font from 'cote-font';
    import style from 'code-style';
    co.style.oneByOne('😀');
    back.red('text');
    font.underline('text');
    style.oneByOne('text');
    co.createTheme('mocha', { level: 3 }).success('ready');
    co.format('text', { foreground: [1, 2, 3], level: 3 });
    // @ts-expect-error unknown palettes must be rejected
    co.createTheme('unknown');
    // @ts-expect-error message must be a string
    co.font.red(123);
    // @ts-expect-error unknown colors must not silently become any
    back.missing('text');
  `);
  const require = createRequire(import.meta.url);
  const metadataPath = require.resolve('typescript/package.json');
  const metadata = JSON.parse(readFileSync(metadataPath, 'utf8'));
  run(process.execPath, [join(dirname(metadataPath), metadata.bin.tsc), '--noEmit', '--strict', '--target', 'ES2022', '--module', 'NodeNext', '--moduleResolution', 'NodeNext', 'usage.mts']);
});

test('a fresh Git dependency installation builds loadable JavaScript and types', () => {
  const source = join(scratch, 'git-source');
  const installed = join(scratch, 'git-consumer');
  mkdirSync(source);
  mkdirSync(installed);
  // Include the current working source, so this regression runs before committing a fix.
  const files = run('git', ['ls-files', '--cached', '--others', '--exclude-standard', '-z'], repo).stdout.split('\0').filter(Boolean);
  for (const file of files) {
    const destination = join(source, file);
    mkdirSync(dirname(destination), { recursive: true });
    copyFileSync(join(repo, file), destination);
  }
  run('git', ['init'], source);
  run('git', ['add', '.'], source);
  run('git', ['-c', 'core.hooksPath=empty-hooks', 'commit', '-m', 'package fixture'], source);
  const commit = run('git', ['rev-parse', 'HEAD'], source).stdout.trim();
  writeFileSync(join(installed, 'package.json'), '{"private":true,"type":"module"}\n');
  npm(['install', '--cache', join(scratch, 'fresh-cache'), '--ignore-scripts=false', '--no-audit', '--no-fund', `git+${pathToFileURL(source).href}#${commit}`], installed);
  const result = run(process.execPath, ['--input-type=module', '-e', "import co from 'co-te'; if (co.createTheme('mocha', {level:0}).success('ready') !== 'ready') throw Error('broken API')"], installed);
  assert.equal(result.stdout, '');
  assert.equal(result.stderr, '');
  assert.ok(existsSync(join(installed, 'node_modules/co-te/dist/index.d.ts')));
});

for (const [name, info, success, warn, error, background] of [
  ['mocha', [137,180,250], [166,227,161], [249,226,175], [243,139,168], [30,30,46]],
  ['tokyo-night', [122,162,247], [158,206,106], [224,175,104], [247,118,142], [26,27,38]],
  ['latte', [30,102,245], [64,160,43], [223,142,29], [210,15,57], [239,241,245]],
]) {
  test(`${name}: semantic colors use the published palette and return composable strings`, () => {
    const result = node(`
      import co, {createTheme, palettes} from 'co-te';
      const t=createTheme(${JSON.stringify(name)}, {level:3});
      const bg=co.createTheme(${JSON.stringify(name)}, {level:3,background:true});
      process.stdout.write(JSON.stringify([t.info('提示😀'),t.success('ok'),t.warn('warning'),t.error('error'),bg.success('ok'),Object.isFrozen(palettes[${JSON.stringify(name)}])]));
    `);
    assert.deepEqual(JSON.parse(result.stdout), [
      `\x1b[38;2;${info.join(';')}m提示😀\x1b[0m`,
      `\x1b[38;2;${success.join(';')}mok\x1b[0m`,
      `\x1b[38;2;${warn.join(';')}mwarning\x1b[0m`,
      `\x1b[38;2;${error.join(';')}merror\x1b[0m`,
      `\x1b[38;2;${success.join(';')};48;2;${background.join(';')}mok\x1b[0m`, true,
    ]);
  });
}

test('format supports RGB, nesting and deterministic 16/256-color fallbacks', () => {
  const result = node(`
    import {format} from 'co-te';
    const inner=format('inner', {foreground:'#00ff00',level:3});
    process.stdout.write(JSON.stringify([
      format('red',{foreground:[255,0,0],background:'#000000',bold:true,level:3}),
      format('outer '+inner+' tail',{foreground:'#ff0000',level:3}),
      format('x',{foreground:'#ff0000',level:1}),
      format('x',{foreground:'#5f87af',level:2}),
      format('你好😀',{foreground:'#ff0000',level:0}),
      format('',{foreground:'#ff0000',level:3})
    ]));
  `);
  assert.deepEqual(JSON.parse(result.stdout), [
    '\x1b[38;2;255;0;0;48;2;0;0;0;1mred\x1b[0m',
    '\x1b[38;2;255;0;0mouter \x1b[38;2;0;255;0minner\x1b[0m\x1b[38;2;255;0;0m tail\x1b[0m',
    '\x1b[91mx\x1b[0m', '\x1b[38;5;67mx\x1b[0m', '你好😀', '',
  ]);
});

test('theme formatters are silent and disable color when stdout is redirected', () => {
  const result = node("import {createTheme} from 'co-te'; createTheme('latte',{level:3}).warn('must not print');");
  assert.equal(result.stdout, '');
  const childEnv = {...env};
  delete childEnv.FORCE_COLOR;
  delete childEnv.NO_COLOR;
  const source = "import {createTheme} from 'co-te'; process.stdout.write(createTheme('mocha').success('plain'));";
  for (const extra of [{}, {NO_COLOR:'1',FORCE_COLOR:'3'}]) {
    const output = spawnSync(process.execPath, ['--input-type=module','-e',source], {cwd:consumer,env:{...childEnv,...extra},encoding:'utf8'});
    assert.equal(output.status, 0, output.stderr);
    assert.equal(output.stdout, 'plain');
  }
});

test('invalid themes, RGB channels, hex values and levels fail explicitly', () => {
  const result = node(`
    import assert from 'node:assert/strict';
    import {format,createTheme} from 'co-te';
    for(const name of ['unknown','__proto__','toString']) assert.throws(()=>createTheme(name),RangeError);
    for(const foreground of ['red','#12345',[256,0,0],[-1,0,0],[1.5,0,0],[1,2],null]) assert.throws(()=>format('x',{foreground,level:3}));
    assert.throws(()=>format('x',{level:4}),RangeError);
    assert.throws(()=>format(42),TypeError);
  `);
  assert.equal(result.stdout, '');
});
