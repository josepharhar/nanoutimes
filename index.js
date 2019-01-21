const internal = require('./build/Release/nanoutimes');

module.exports.utimesSync = internal.utimesSync;

module.exports.utimes = function(filepath, settings, callback) {
  try {
    callback(undefined, internal.utimesSync(filepath, settings));
  } catch (err) {
    callback(err);
  }
};
