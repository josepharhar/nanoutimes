# nanoutimes

nanoutimes is an npm package which provides utimes() functionality with higher precision than fs.utimes(), including the full nanosecond precision of the underlying utimes() call in C.

Hopefully this functionality will be upstreamed to nodejs's builtin fs package, in which case this package will become a polyfill for it.

# Usage

nanoutimes is intended to mirror fs.utimes() and similar functions as closely as possible.

```javascript
const nanoutimes = require('nanoutimes');
const atimeS = 1548106885;
const atimeNs = 269349603;
const mtimeS = 1548106885;
const mtimeNs = 269349603;
nanoutimes.utimesSync('file.txt', atimeS, atimeNs, mtimeS, mtimeNs);
```
