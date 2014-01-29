// -D_GNU_SOURCE makes SOCK_NONBLOCK etc. available on linux
#undef  _GNU_SOURCE
#define _GNU_SOURCE

#include <nan.h>
#include <pipe_wrap.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

using namespace v8;
using namespace node;

namespace {

void SetErrno(int errorno) {
    // set errno in the global context, this is the technique
    // that node uses to propagate system level errors to JS land
    Context::GetCurrent()->Global()->Set(NanSymbol("errno"), Integer::New(errorno));
}

void SetNonBlock(int fd) {
    int flags;
	int r;

	flags = fcntl(fd, F_GETFL);
	assert(flags != -1);

	r = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
	assert(r != -1);
}


void SetCloExec(int fd) {
	int flags;
	int r;

	flags = fcntl(fd, F_GETFD);
	assert(flags != -1);

	r = fcntl(fd, F_SETFD, flags | FD_CLOEXEC);
	assert(r != -1);
}

NAN_METHOD(Socket) {
    NanScope();
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
    NanReturnValue(Integer::New(fd));
}


NAN_METHOD(Bind) {
    NanScope();
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

    NanReturnValue(Integer::New(ret));
}

NAN_METHOD(GetPeerName) {
    NanScope();
    assert(args.Length() == 1);
    Local<Object> obj = args[0]->ToObject();
    assert(obj->InternalFieldCount() > 0);
    sockaddr_un sun;
    socklen_t addrlen = sizeof(sun);
    memset(&sun, '\0', addrlen);
    PipeWrap* wrap = static_cast<PipeWrap*>(obj->GetPointerFromInternalField(0));
#if NODE_VERSION_AT_LEAST(0, 9, 4)
    int fd = wrap->UVHandle()->io_watcher.fd;
#else
    int fd = wrap->UVHandle()->fd;
#endif
    if (getpeername(fd, reinterpret_cast<sockaddr*>(&sun), &addrlen) == -1) {
        SetErrno(errno);
        return Null();
    }

    /* If addrlen == 2 --> no path */
    NanReturnValue(addrlen == 2 ? Null() : String::New(sun.sun_path));
}

NAN_METHOD(GetSockName) {
    NanScope();
    assert(args.Length() == 1);
    Local<Object> obj = args[0]->ToObject();
    assert(obj->InternalFieldCount() > 0);
    sockaddr_un sun;
    socklen_t addrlen = sizeof(sun);
    memset(&sun, '\0', addrlen);
    PipeWrap* wrap = static_cast<PipeWrap*>(obj->GetPointerFromInternalField(0));
#if NODE_VERSION_AT_LEAST(0, 9, 4)
    int fd = wrap->UVHandle()->io_watcher.fd;
#else
    int fd = wrap->UVHandle()->fd;
#endif
    if (getsockname(fd, reinterpret_cast<sockaddr*>(&sun), &addrlen) == -1) {
        SetErrno(errno);
        return Null();
    }

    /* If addrlen == 2 --> no path */
    NanReturnValue(addrlen == 2 ? Null() : String::New(sun.sun_path));
}

void Initialize(Handle<Object> target) {
    target->Set(NanSymbol("AF_UNIX"), Integer::New(AF_UNIX));
    target->Set(NanSymbol("SOCK_STREAM"), Integer::New(SOCK_STREAM));
    target->Set(NanSymbol("socket"), FunctionTemplate::New(Socket)->GetFunction());
    target->Set(NanSymbol("bind"), FunctionTemplate::New(Bind)->GetFunction());
    target->Set(NanSymbol("getpeername"), FunctionTemplate::New(GetPeerName)->GetFunction());
    target->Set(NanSymbol("getsockname"), FunctionTemplate::New(GetSockName)->GetFunction());
}

}

NODE_MODULE(unix_stream, Initialize)
