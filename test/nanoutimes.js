const nanoutimes = require('nanoutimes');

const args = process.argv.slice(2);
if (args.length < 5) {
  console.log('usage: nanoutimes filepath atimeS atimeNs mtimeS mtimeNs');
  return;
}

const retval = nanoutimes.utimesSync(args[0], Number(args[1]), Number(args[2]), Number(args[3]), Number(args[4]));
console.log('nanoutimes returned: ' + retval);
