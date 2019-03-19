#include <node.h>

#include <errno.h>
#include <sys/stat.h>
#include <string.h>

#ifdef _WIN32

#include <windows.h>
#include <strsafe.h>

FILETIME createFILETIME(uint64_t seconds, uint64_t nanoseconds) {
  //uint64_t hunnaNs = (seconds * 10000000) + (nanoseconds / 100);
  uint64_t hunnaNs = ((seconds + 11644473600) * 10000000) + nanoseconds / 100;

  ULARGE_INTEGER temp;
  temp.QuadPart = hunnaNs;

  FILETIME filetime;
  filetime.dwHighDateTime = temp.HighPart;
  filetime.dwLowDateTime = temp.LowPart;
  return filetime;
}

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
  // https://docs.microsoft.com/en-us/windows/desktop/SysInfo/retrieving-the-last-write-time
  HANDLE file_handle = CreateFile(char_filepath, FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  if (file_handle == INVALID_HANDLE_VALUE) {
    isolate->ThrowException(v8::Exception::Error(NEW_STRING("failed to open file")));
    return;
  }

  // https://docs.microsoft.com/en-us/windows/desktop/SysInfo/changing-a-file-time-to-the-current-time
  FILETIME lpLastAccessTime, lpLastWriteTime;
  lpLastAccessTime = createFILETIME(atimeS, atimeNs);
  lpLastWriteTime = createFILETIME(mtimeS, mtimeNs);
  DWORD retval = SetFileTime(file_handle, (LPFILETIME) NULL, &lpLastAccessTime, &lpLastWriteTime);
  if (!retval) {
    // https://docs.microsoft.com/en-us/windows/desktop/debug/retrieving-the-last-error-code
    LPTSTR lpszFunction = "SetFileTime";
    LPVOID lpMsgBuf, lpDisplayBuf;
    DWORD dw = GetLastError();
    FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | 
        FORMAT_MESSAGE_FROM_SYSTEM |
        FORMAT_MESSAGE_IGNORE_INSERTS,
        NULL,
        dw,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
        (LPTSTR) &lpMsgBuf,
        0, NULL );

    std::string error_string = std::string("SetFileTime(\"")
      + std::string(char_filepath)
      + std::string("\") failed with error ")
      + std::to_string(dw)
      + std::string(": ")
      + std::string((TCHAR*)lpMsgBuf);
    isolate->ThrowException(v8::Exception::Error(NEW_STRING(error_string.c_str())));
    return;
  }
  CloseHandle(file_handle);

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
