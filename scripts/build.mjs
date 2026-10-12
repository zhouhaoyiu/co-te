import { cpSync, readFileSync, rmSync } from 'node:fs';
import { createRequire } from 'node:module';
import { dirname, join } from 'node:path';
import { fileURLToPath } from 'node:url';
import { spawnSync } from 'node:child_process';

const root = fileURLToPath(new URL('../', import.meta.url));
const require = createRequire(import.meta.url);
const metadataPath = require.resolve('typescript/package.json');
const metadata = JSON.parse(readFileSync(metadataPath, 'utf8'));
const compiler = join(dirname(metadataPath), metadata.bin.tsc);
const packages = ['code-back', 'cote-font', 'code-style'];
rmSync(join(root, 'dist'), { recursive: true, force: true });
for (const name of packages) rmSync(join(root, 'packages', name, 'dist'), { recursive: true, force: true });
const result = spawnSync(process.execPath, [compiler, '-p', join(root, 'tsconfig.json')], { cwd: root, stdio: 'inherit' });
if (result.error) throw result.error;
if (result.status !== 0) process.exit(result.status ?? 1);
for (const name of packages) cpSync(join(root, 'dist/packages', name), join(root, 'packages', name, 'dist'), { recursive: true });
