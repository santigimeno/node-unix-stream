// -D_GNU_SOURCE makes SOCK_NONBLOCK etc. available on linux
#undef  _GNU_SOURCE
#define _GNU_SOURCE

#include <nan.h>
#if !NODE_VERSION_AT_LEAST(0, 9, 10)
#include <pipe_wrap.h>
#endif
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>

using namespace v8;
using namespace node;

namespace {

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
    Nan::HandleScope scope;

    int protocol;
    int domain;
    int type;
    int fd;

    assert(info.Length() == 3);

    domain    = info[0]->Int32Value();
    type      = info[1]->Int32Value();
    protocol  = info[2]->Int32Value();

#if defined(SOCK_NONBLOCK)
    type |= SOCK_NONBLOCK;
#endif
#if defined(SOCK_CLOEXEC)
    type |= SOCK_CLOEXEC;
#endif

    if ((fd = socket(domain, type, protocol)) == -1) {
        fd = -errno;
        goto out;
    }

#if !defined(SOCK_NONBLOCK)
    SetNonBlock(fd);
#endif
#if !defined(SOCK_CLOEXEC)
    SetCloExec(fd);
#endif

out:
    info.GetReturnValue().Set(fd);
}


NAN_METHOD(Bind) {
    Nan::HandleScope scope;

    sockaddr_un sun;
    int fd;
    int ret;

    assert(info.Length() == 2);

    fd = info[0]->Int32Value();
    String::Utf8Value path(info[1]);

    memset(&sun, 0, sizeof(sun));
    strncpy(sun.sun_path, *path, sizeof(sun.sun_path) - 1);
    sun.sun_family = AF_UNIX;

    if ((ret = bind(fd, reinterpret_cast<sockaddr*>(&sun), sizeof(sun))) == -1) {
        ret = -errno;
    }

    info.GetReturnValue().Set(ret);
}

NAN_METHOD(GetPeerName) {
    Nan::HandleScope scope;

    int fd;
    assert(info.Length() == 1);
#if NODE_VERSION_AT_LEAST(0, 9, 10)
    fd = info[0]->Int32Value();
#else
    Local<Object> obj = info[0]->ToObject();
    assert(obj->InternalFieldCount() > 0);
    PipeWrap* wrap = static_cast<PipeWrap*>(obj->GetPointerFromInternalField(0));
#if NODE_VERSION_AT_LEAST(0, 9, 4)
    fd = wrap->UVHandle()->io_watcher.fd;
#else
    fd = wrap->UVHandle()->fd;
#endif
#endif

    sockaddr_un sun;
    socklen_t addrlen = sizeof(sun);
    memset(&sun, 0, addrlen);
    if (getpeername(fd, reinterpret_cast<sockaddr*>(&sun), &addrlen) == -1) {
        info.GetReturnValue().Set(-errno);
        return;
    }

    /* If addrlen == 2 --> no path */
    if (addrlen == 2) {
        info.GetReturnValue().SetNull();
    } else {
        info.GetReturnValue().Set(Nan::New(sun.sun_path).ToLocalChecked());
    }
}

NAN_METHOD(GetSockName) {
    Nan::HandleScope scope;

    int fd;
    assert(info.Length() == 1);
#if NODE_VERSION_AT_LEAST(0, 9, 10)
    fd = info[0]->Int32Value();
#else
    Local<Object> obj = info[0]->ToObject();
    assert(obj->InternalFieldCount() > 0);
    PipeWrap* wrap = static_cast<PipeWrap*>(obj->GetPointerFromInternalField(0));
#if NODE_VERSION_AT_LEAST(0, 9, 4)
    fd = wrap->UVHandle()->io_watcher.fd;
#else
    fd = wrap->UVHandle()->fd;
#endif
#endif

    sockaddr_un sun;
    socklen_t addrlen = sizeof(sun);
    memset(&sun, 0, addrlen);
    if (getsockname(fd, reinterpret_cast<sockaddr*>(&sun), &addrlen) == -1) {
        info.GetReturnValue().Set(-errno);
        return;
    }

    /* If addrlen == 2 --> no path */
    if (addrlen == 2) {
        info.GetReturnValue().SetNull();
    } else {
        info.GetReturnValue().Set(Nan::New(sun.sun_path).ToLocalChecked());
    }
}

void Initialize(Local<Object> target) {
    target->Set(Nan::New("AF_UNIX").ToLocalChecked(),
                Nan::New(AF_UNIX));
    target->Set(Nan::New("SOCK_STREAM").ToLocalChecked(),
                Nan::New(SOCK_STREAM));
    target->Set(Nan::New("socket").ToLocalChecked(),
                Nan::New<FunctionTemplate>(Socket)->GetFunction());
    target->Set(Nan::New("bind").ToLocalChecked(),
                Nan::New<FunctionTemplate>(Bind)->GetFunction());
    target->Set(Nan::New("getpeername").ToLocalChecked(),
                Nan::New<FunctionTemplate>(GetPeerName)->GetFunction());
    target->Set(Nan::New("getsockname").ToLocalChecked(),
                Nan::New<FunctionTemplate>(GetSockName)->GetFunction());
}

}

NODE_MODULE(unix_stream, Initialize)
