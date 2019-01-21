# nanoutimes

nanoutimes is an npm package which provides utimes() functionality with higher precision than fs.utimes(), including the full nanosecond precision of the underlying utimes() call in C.

Hopefully this functionality will be upstreamed to nodejs's builtin fs package, in which case this package will become a polyfill for it.

# Usage

nanoutimes is intended to mirror fs.utimes() and similar functions as closely as possible.

```javascript
const nanoutimes = require('nanoutimes');
nanoutimes.utimesSync('file.txt', {
  mtimeS: 1548106885,
  mtimeNs: 269349603,
  atimeS: 1548106885,
  atimeNs: 269349603
});
```
