// -D_GNU_SOURCE makes SOCK_NONBLOCK etc. available on linux
#undef  _GNU_SOURCE
#define _GNU_SOURCE

#include "uv.h"
#include "node.h"
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

using namespace v8;
using namespace node;

namespace {

Persistent<String> errno_symbol;

void SetErrno(int errorno) {
    // set errno in the global context, this is the technique
    // that node uses to propagate system level errors to JS land
    Context::GetCurrent()->Global()->Set(errno_symbol, Integer::New(errorno));
}

Handle<Value> Socket(const Arguments& args) {
    HandleScope scope;
    int protocol;
    int domain;
    int type;
    int fd;

    assert(args.Length() == 3);

    domain    = args[0]->Int32Value();
    type      = args[1]->Int32Value();
    protocol  = args[2]->Int32Value();

#if defined(SOCK_NONBLOCK)
    type |= SOCK_NONBLOCK;
#endif
#if defined(SOCK_CLOEXEC)
    type |= SOCK_CLOEXEC;
#endif

    if ((fd = socket(domain, type, protocol)) == -1) {
        SetErrno(errno);
        goto out;
    }

#if !defined(SOCK_NONBLOCK)
    SetNonBlock(fd);
#endif
#if !defined(SOCK_CLOEXEC)
    SetCloExec(fd);
#endif

out:
    return scope.Close(Integer::New(fd));
}


Handle<Value> Bind(const Arguments& args) {
    HandleScope scope;
    sockaddr_un sun;
    int fd;
    int ret;

    assert(args.Length() == 2);

    fd = args[0]->Int32Value();
    String::Utf8Value path(args[1]);

    strncpy(sun.sun_path, *path, sizeof(sun.sun_path) - 1);
    sun.sun_path[sizeof(sun.sun_path) - 1] = '\0';
    sun.sun_family = AF_UNIX;

    if ((ret = bind(fd, reinterpret_cast<sockaddr*>(&sun), sizeof(sun))) == -1) {
        SetErrno(errno);
    }

    return scope.Close(Integer::New(ret));
}

void Initialize(Handle<Object> target) {

    errno_symbol = Persistent<String>::New(String::NewSymbol("errno"));
    target->Set(String::NewSymbol("AF_UNIX"), Integer::New(AF_UNIX));
    target->Set(String::NewSymbol("SOCK_STREAM"), Integer::New(SOCK_STREAM));
    target->Set(String::NewSymbol("socket"), FunctionTemplate::New(Socket)->GetFunction());
    target->Set(String::NewSymbol("bind"), FunctionTemplate::New(Bind)->GetFunction());
}

}

NODE_MODULE(unix_stream, Initialize)
