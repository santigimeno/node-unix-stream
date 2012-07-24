var util = require('util');
var Socket = require('net').Socket;

/* Make sure we choose the correct build directory */
var directory = process.config.default_configuration === 'Debug' ? 'Debug' : 'Release';
var bindings = require(__dirname + '/../build/' + directory + '/unix_stream.node');

var SOCK_STREAM  = bindings.SOCK_STREAM;
var AF_UNIX     = bindings.AF_UNIX;

var bind = bindings.bind;
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
