import { after, before, test } from 'node:test';
import assert from 'node:assert/strict';
import { existsSync, mkdirSync, mkdtempSync, readFileSync, rmSync, writeFileSync } from 'node:fs';
import { tmpdir } from 'node:os';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { createRequire } from 'node:module';
import { spawnSync } from 'node:child_process';

const repo = fileURLToPath(new URL('../', import.meta.url));
const scratch = mkdtempSync(join(tmpdir(), 'co-te package tests-'));
const consumer = join(scratch, 'consumer');
const npmCLI = process.env.npm_execpath;
assert.ok(npmCLI?.endsWith('npm-cli.js'), 'Run this suite with npm test or npm run test:js');
const env = { ...process.env, npm_config_update_notifier: 'false' };
delete env.NODE_OPTIONS;
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
