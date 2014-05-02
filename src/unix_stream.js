var util = require('util');
var Socket = require('net').Socket;

var bindings = require('bindings')('unix_stream.node');

/* bindings from C++ */
var SOCK_STREAM = bindings.SOCK_STREAM;
var AF_UNIX = bindings.AF_UNIX;
var bind = bindings.bind;
var getpeername = bindings.getpeername;
var getsockname = bindings.getsockname;
var socket = bindings.socket;

function errnoException(errorno, syscall) {
    var e = new Error(syscall + ' ' + errorno);
    e.errno = e.code = errorno;
    e.syscall = syscall;
    return e;
}

exports.createSocket = function(local_path) {

    var fd;
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) === -1)
        throw errnoException(errno, 'socket');

    var s = new Socket(fd);
    if (local_path) {
        var r = bind(fd, local_path);
        if (r < 0) {
            s.destroy();
            process.nextTick(function() {
                s.emit('error', errnoException(r, 'bind'));
            });
        }

        s.local_path = local_path;
    }

    return s;
};

/* getter property to retrieve the remote path of a established connection*/
Object.defineProperty(Socket.prototype, "remotePath", {
    get : function remotePath() {
        var addr = getpeername(this._handle);
        if (addr < 0) {
            throw errnoException(addr, 'getpeername');
        }

        return addr;
    }
});

/* function to retrieve the path a unix socket is bound to */
Socket.prototype.path = function() {

    if (this.local_path) return this.local_path;
    var addr = getsockname(this._handle);
    if (addr < 0) {
         throw errnoException(addr, 'getsockname');
    } 	

    return addr;
}
