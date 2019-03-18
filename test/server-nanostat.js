const fs = require('fs');
const assert = require('assert').strict;
const util = require('util');

const nanostat = require('nanostat');

BigInt.prototype.toJSON = function() {
  return this.toString();
}

// TODO get all these to pass on all platforms

function compareStats(fn, nstat, stat) {
  function assertEq(field) {
    assert.equal(nstat[field], stat[field],
      new Error(`${fn}: ${field} not equal.\n nstat: ${nstat[field]}\nfsstat: ${stat[field]}`));
  }

  assertEq('atimeMs');
  assertEq('mtimeMs');
  assertEq('ctimeMs');
  assertEq('birthtimeMs');

  /*assertEq('atimeNs');
  assertEq('mtimeNs');
  assertEq('ctimeNs');
  assertEq('birthtimeNs');*/

  assertEq('dev');
  assertEq('mode');
  assertEq('nlink');
  assertEq('uid');
  assertEq('gid');
  assertEq('rdev');
  assertEq('blksize');
  assertEq('ino');
  assertEq('size');
  assertEq('blocks');
}

async function compareErrors(fn, nstatFn, fsstatFn) {
  const nstatErr = await new Promise((resolve, reject) => {
    try {
      const nstat = nstatFn();
      reject(`${fn}: nstatFn succeeded when it should have failed: ${JSON.stringify(nstat, null, 2)}`);
    } catch (err) {
      resolve(err);
    }
  });
  const fsstatErr = await new Promise((resolve, reject) => {
    try {
      const fsstat = fsstatFn();
      reject(`${fn}: fsstatFn succeeded when it should have failed: ${JSON.stringify(fsstat, null, 2)}`);
    } catch (err) {
      resolve(err);
    }
  });
  assert.equal(nstatErr.message, fsstatErr.message,
    new Error(`${fn}: error: messages not equal.`
      + `\n nstat message: ${nstatErr.message}`
      + `\nfsstat message: ${fsstatErr.message}`
      + `\n nstat error: ${JSON.stringify(nstatErr, null, 2)}`
      + `\nfsstat error: ${JSON.stringify(fsstatErr, null, 2)}`));
}

(async () => {
  compareStats(
    'statSync',
    nanostat.statSync('package.json'),
    fs.statSync('package.json', {bigint: true}));
  compareStats(
    'lstatSync',
    nanostat.lstatSync('package.json'),
    fs.lstatSync('package.json', {bigint: true}));

  // TODO test error messages
  /*await compareErrors(
    'statSync',
    () => nanostat.statSync('does-not-exist.txt'),
    () => fs.statSync('does-not-exist.txt'));
  await compareErrors(
    'lstatSync',
    () => nanostat.lstatSync('does-not-exist.txt'),
    () => fs.lstatSync('does-not-exist.txt'));*/

  compareStats(
    'stat',
    await (util.promisify(fs.stat)('package.json', {bigint: true})),
    await (util.promisify(nanostat.stat)('package.json')));
  compareStats(
    'lstat',
    await (util.promisify(fs.lstat)('package.json', {bigint: true})),
    await (util.promisify(nanostat.lstat)('package.json')));

})().catch(err => {
  console.error('test failed with error:\n' + err);
  process.exit(1);
});
