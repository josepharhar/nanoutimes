const nanoutimes = require('nanoutimes');
const nanostat = require('nanostat');

BigInt.prototype.toJSON = function() {
  return this.toString();
}

const stats = nanostat.statSync('file.txt');
let atimeS = stats.atimeMs / 1000n;
let atimeNs = stats.atimeNs;
let mtimeS = stats.mtimeMs / 1000n;
let mtimeNs = stats.mtimeNs;

atimeS += 1n;
atimeNs += 202n;
mtimeS += 3n;
mtimeNs += 404n;

const retval = nanoutimes.utimesSync('file.txt', atimeS, atimeNs, mtimeS, mtimeNs);
console.log('nanoutimes.utimesSync() returned: ' + JSON.stringify(retval, null, 2));
