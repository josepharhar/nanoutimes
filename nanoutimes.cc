#include <node.h>

#include <errno.h>
#include <sys/stat.h>
#include <string.h>

#include <utility>
#include <memory>
#include <iostream>
#include <sstream>

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

static std::unique_ptr<int64_t> valueToNumber(v8::Local<v8::Value> value, v8::Isolate* isolate) {
  if (value->IsBigInt())
    return std::make_unique<int64_t>(v8::Local<v8::BigInt>::Cast(value)->Int64Value());

  // try to convert strings to numbers?
  // TODO this wont have enough capacity
  /*v8::MaybeLocal<v8::Number> maybe_number = value->ToNumber(isolate);
  if (maybe_number.IsEmpty())
    return nullptr;
  return std::make_unique<int64_t>(maybe_number.ToLocalChecked()->Value());*/
  if (value->IsNumber())
    return std::make_unique<int64_t>(v8::Local<v8::Number>::Cast(value)->Value());

  return nullptr;
}

static std::string valueTypeOf(v8::Local<v8::Value> value, v8::Isolate* isolate) {
  v8::Local<v8::String> local_type = value->TypeOf(isolate);
  v8::String::Utf8Value utf8value(local_type);
  const char* type_cstr = *utf8value ? *utf8value : "<string conversion failed>";
  return std::string(type_cstr);
}

static void utimesSync(
    const v8::FunctionCallbackInfo<v8::Value>& args) {
  v8::Isolate* isolate = args.GetIsolate();

  if (args.Length() != 5) {
    // TODO figure out what fs.utimes() does in this case
    std::stringstream sstream;
    sstream << "utimes requires 5 arguments. num provided: " << args.Length();
    isolate->ThrowException(v8::Exception::Error(NEW_STRING(sstream.str().c_str())));
    return;
  }

  if (!args[0]->IsString()) {
    // TODO figure out what fs.utimes() does in this case
    isolate->ThrowException(v8::Exception::Error(NEW_STRING("filepath must be a string")));
    return;
  }
  v8::Local<v8::String> filepath = v8::Local<v8::String>::Cast(args[0]);
  v8::String::Utf8Value utf8_filepath(isolate, filepath);
  char* utf8_char_filepath = *utf8_filepath;

  // if not present, then don't update atime
  std::unique_ptr<std::pair<int64_t, int64_t>> atimeSandNs;
  if (args[1]->ToBoolean(isolate)->IsTrue() || args[2]->ToBoolean(isolate)->IsTrue()) {
    std::unique_ptr<int64_t> atimeS = valueToNumber(args[1], isolate);
    if (!atimeS) {
      std::string type = std::string("atimeS could not be converted to a number. type: ") + valueTypeOf(args[1], isolate);
      isolate->ThrowException(v8::Exception::Error(NEW_STRING(type.c_str())));
      return;
    }

    std::unique_ptr<int64_t> atimeNs = valueToNumber(args[2], isolate);
    if (!atimeNs) {
      std::string type = std::string("atimeNs could not be converted to a number. type: ") + valueTypeOf(args[2], isolate);
      isolate->ThrowException(v8::Exception::Error(NEW_STRING(type.c_str())));
      return;
    }

    atimeSandNs = std::make_unique<std::pair<int64_t, int64_t>>(*atimeS, *atimeNs);
  }

  // if not present, then don't update mtime
  std::unique_ptr<std::pair<int64_t, int64_t>> mtimeSandNs;
  if (args[3]->ToBoolean(isolate)->IsTrue() || args[4]->ToBoolean(isolate)->IsTrue()) {
    std::unique_ptr<int64_t> mtimeS = valueToNumber(args[3], isolate);
    if (!mtimeS) {
      std::string type = std::string("mtimeS could not be converted to a number. type: ") + valueTypeOf(args[3], isolate);
      isolate->ThrowException(v8::Exception::Error(NEW_STRING(type.c_str())));
      return;
    }

    std::unique_ptr<int64_t> mtimeNs = valueToNumber(args[4], isolate);
    if (!mtimeNs) {
      std::string type = std::string("mtimeNs could not be converted to a number. type: ") + valueTypeOf(args[4], isolate);
      isolate->ThrowException(v8::Exception::Error(NEW_STRING(type.c_str())));
      return;
    }

    mtimeSandNs = std::make_unique<std::pair<int64_t, int64_t>>(*mtimeS, *mtimeNs);
  }

#ifdef _WIN32
  // https://docs.microsoft.com/en-us/windows/desktop/SysInfo/retrieving-the-last-write-time
  //HANDLE file_handle = CreateFile(utf8_char_filepath, FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  int wchars_num = MultiByteToWideChar(CP_UTF8, 0, utf8_char_filepath, -1, NULL, 0);
  std::vector<wchar_t> wstr;
  wstr.reserve(wchars_num);
  MultiByteToWideChar(CP_UTF8, 0, utf8_char_filepath, -1, wstr.data(), wchars_num);
  HANDLE file_handle = CreateFileW(wstr.data(), FILE_WRITE_ATTRIBUTES, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);
  if (file_handle == INVALID_HANDLE_VALUE) {
    isolate->ThrowException(v8::Exception::Error(NEW_STRING("failed to open file")));
    return;
  }

  // https://docs.microsoft.com/en-us/windows/desktop/SysInfo/changing-a-file-time-to-the-current-time
  FILETIME lpLastAccessTime, lpLastWriteTime;
  lpLastAccessTime.dwLowDateTime = 0;
  lpLastAccessTime.dwHighDateTime = 0;
  lpLastWriteTime.dwLowDateTime = 0;
  lpLastWriteTime.dwHighDateTime = 0;
  if (atimeSandNs)
    lpLastAccessTime = createFILETIME(atimeSandNs->first, atimeSandNs->second);
  if (mtimeSandNs)
    lpLastWriteTime = createFILETIME(mtimeSandNs->first, mtimeSandNs->second);
  DWORD retval = SetFileTime(
      file_handle,
      (LPFILETIME) NULL,
      /*atimeSandNs ? &lpLastAccessTime : NULL,
      mtimeSandNs ? &lpLastWriteTime : NULL);*/
      &lpLastAccessTime,
      &lpLastWriteTime);
  if (!retval) {
    // https://docs.microsoft.com/en-us/windows/desktop/debug/retrieving-the-last-error-code
    LPTSTR lpszFunction = "SetFileTime";
    LPVOID lpMsgBuf;
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
      + std::string(utf8_char_filepath)
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
  if (atimeSandNs) {
    times[0].tv_sec = atimeSandNs->first;
    times[0].tv_nsec = atimeSandNs->second;
  } else {
    times[0].tv_nsec = UTIME_OMIT;
  }
  if (mtimeSandNs) {
    times[1].tv_sec = mtimeSandNs->first;
    times[1].tv_nsec = mtimeSandNs->second;
  } else {
    times[1].tv_nsec = UTIME_OMIT;
  }

  if (utimensat(AT_FDCWD, utf8_char_filepath, times, /* flags */ 0)) {
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
