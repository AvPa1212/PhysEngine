import { existsSync, mkdirSync, copyFileSync } from 'node:fs';
import { resolve, dirname } from 'node:path';
import { fileURLToPath } from 'node:url';

// Always resolve relative to this script's location, regardless of cwd.
// This is necessary because `npm --prefix momentum-ui run build:vercel`
// keeps process.cwd() at the repo root, not momentum-ui/.
const __dirname = dirname(fileURLToPath(import.meta.url));
const packageRoot = resolve(__dirname, '..');          // momentum-ui/
const repoRoot    = resolve(packageRoot, '..');        // repo root

const publicWebDist = resolve(packageRoot, 'public', 'web_dist');
const rootWebDist   = resolve(repoRoot, 'web_dist');

const required = ['MomentumCore.js', 'MomentumCore.wasm'];

mkdirSync(publicWebDist, { recursive: true });

for (const file of required) {
  const target = resolve(publicWebDist, file);
  if (existsSync(target)) continue;

  const fallback = resolve(rootWebDist, file);
  if (existsSync(fallback)) {
    copyFileSync(fallback, target);
    console.log(`Copied ${file} from root web_dist.`);
  }
}

const missing = required.filter((file) => !existsSync(resolve(publicWebDist, file)));
if (missing.length > 0) {
  console.error('\nVercel build blocked: missing WebAssembly artifacts.');
  console.error(`Expected in ${publicWebDist}: ${missing.join(', ')}`);
  console.error('Run build_web.sh locally and commit the files under momentum-ui/public/web_dist/.\n');
  process.exit(1);
}

console.log('WASM assets ready for Vercel build.');
