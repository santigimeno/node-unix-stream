var util = require('util');
var net = require('net');
var Socket = require('net').Socket;

/* Make sure we choose the correct build directory */
var directory = process.config.target_defaults.default_configuration === 'Debug' ? 'Debug' : 'Release';
var bindings = require(__dirname + '/../build/' + directory + '/unix_stream.node');

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
        if (bind(fd, local_path) == -1) {
            process.nextTick(function() {
                s.emit('error', errnoException(errno, 'bind'));
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
        if (!addr && (typeof errno !== 'undefined'))
            this.emit('error', errnoException(errno, 'getpeername'));
        return addr;
    }
});

/* function to retrieve the path a unix socket is bound to */
Socket.prototype.path = function() {

    if (this.local_path) return this.local_path;
    var addr = getsockname(this._handle);
    if (!addr && (typeof errno !== 'undefined'))
        this.emit('error', errnoException(errno, 'getsockname'));
    return addr;
}
