var util = require('util');
var net = require('net');
var Socket = require('net').Socket;

/* Make sure we choose the correct build directory */
var directory = process.config.target_defaults.default_configuration === 'Debug' ? 'Debug' : 'Release';
var bindings = require(__dirname + '/../build/' + directory + '/unix_stream.node');

var SOCK_STREAM  = bindings.SOCK_STREAM;
var AF_UNIX     = bindings.AF_UNIX;

var bind = bindings.bind;
var socket = bindings.socket;
var getpeername = bindings.getpeername;

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

    s.fd = fd;
    return s;
};

exports.createServer = function(local_path, listening_listener, connect_listener) {

    var fd;
    if ((fd = socket(AF_UNIX, SOCK_STREAM, 0)) === -1)
        throw errnoException(errno, 'socket');

    var server = net.createServer(function(con) {
        con.fd = fd;
        connect_listener(con);
    });

    if (local_path) {
        if (bind(fd, local_path) == -1) {
            process.nextTick(function() {
                server.emit('error', errnoException(errno, 'bind'));
            });
        }
    }

    server.listen({ fd : fd }, listening_listener);
    return server;
}

Socket.prototype.getRemotePath = function() {

    return getpeername(this.fd);
}