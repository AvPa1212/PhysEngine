import { existsSync, mkdirSync, copyFileSync } from 'node:fs';
import { resolve } from 'node:path';

const projectRoot = resolve(process.cwd());
const publicWebDist = resolve(projectRoot, 'public', 'web_dist');
const rootWebDist = resolve(projectRoot, '..', 'web_dist');

const required = ['MomentumCore.js', 'MomentumCore.wasm'];

mkdirSync(publicWebDist, { recursive: true });

for (const file of required) {
  const target = resolve(publicWebDist, file);
  if (existsSync(target)) continue;

  const fallback = resolve(rootWebDist, file);
  if (existsSync(fallback)) {
    copyFileSync(fallback, target);
  }
}

const missing = required.filter((file) => !existsSync(resolve(publicWebDist, file)));
if (missing.length > 0) {
  console.error('\nVercel build blocked: missing WebAssembly artifacts.');
  console.error(`Expected in public/web_dist: ${missing.join(', ')}`);
  console.error('Run deploy_web.py or build_web.sh locally, then commit the generated files under momentum-ui/public/web_dist/.\n');
  process.exit(1);
}

console.log('WASM assets ready for Vercel build.');
