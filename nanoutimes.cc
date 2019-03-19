#include <node.h>

#include <errno.h>
#include <sys/stat.h>
#include <string.h>

#ifdef _WIN32

#include <windows.h>

#else  // _WIN32

#include <fcntl.h>
#include <sys/stat.h>

#endif  // _WIN32

#define NEW_STRING(str) \
  v8::String::NewFromUtf8(isolate, str, v8::NewStringType::kNormal) \
    .ToLocalChecked()

static void utimesSync(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();

  if (args.Length() < 5) {
    // TODO figure out what fs.utimes() does in this case
    isolate->ThrowException(v8::Exception::Error(NEW_STRING("args.Length() < 5")));
    return;
  }

  if (!args[0]->IsString()) {
    // TODO figure out what fs.utimes() does in this case
    isolate->ThrowException(v8::Exception::Error(NEW_STRING("!args[0]->IsString()")));
    return;
  }
  v8::Local<v8::String> filepath = v8::Local<v8::String>::Cast(args[0]);
  v8::String::Utf8Value utf8_filepath(isolate, filepath);
  char* char_filepath = *utf8_filepath;

  // TODO do i need any logic for this?
  // If the input numbers are less than zero, then don't change the time.

  if (!args[1]->IsBigInt()) {
    // TODO
    isolate->ThrowException(v8::Exception::Error(NEW_STRING("atimeS must be a BigInt")));
    return;
  }
  int64_t atimeS = v8::Local<v8::BigInt>::Cast(args[1])->Int64Value();

  if (!args[2]->IsBigInt()) {
    // TODO
    isolate->ThrowException(v8::Exception::Error(NEW_STRING("atimeNs must be a BigInt")));
    return;
  }
  int64_t atimeNs = v8::Local<v8::BigInt>::Cast(args[2])->Int64Value();

  if (!args[3]->IsBigInt()) {
    // TODO
    isolate->ThrowException(v8::Exception::Error(NEW_STRING("mtimeS must be a BigInt")));
    return;
  }
  int64_t mtimeS = v8::Local<v8::BigInt>::Cast(args[3])->Int64Value();

  if (!args[4]->IsBigInt()) {
    // TODO
    isolate->ThrowException(v8::Exception::Error(NEW_STRING("mtimeNs must be a BigInt")));
    return;
  }
  int64_t mtimeNs = v8::Local<v8::BigInt>::Cast(args[4])->Int64Value();

#ifdef _WIN32
  // TODO
#else  // _WIN32
  struct timespec times[2];
  memset(times, 0, sizeof(struct timespec) * 2);
  times[0].tv_sec = atimeS;
  times[0].tv_nsec = atimeNs;
  times[1].tv_sec = mtimeS;
  times[1].tv_nsec = mtimeNs;

  if (utimensat(AT_FDCWD, char_filepath, times, /* flags */ 0)) {
    std::string error_string =
      std::string("utimensat() failed. strerror: ") + strerror(errno);
    isolate->ThrowException(v8::Exception::Error(
          NEW_STRING(error_string.c_str())));
    return;
  }
#endif  // _WIN32
}

void Initialize(v8::Local<v8::Object> exports) {
  NODE_SET_METHOD(exports, "utimesSync", utimesSync);
}

NODE_MODULE(NODE_GYP_MODULE_NAME, Initialize);
