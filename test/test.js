var unix = require('../src/unix_stream.js');
var fs = require('fs');

var listening_listener = function() {
    console.log('listening in /tmp/remote_path');
    /* Create Unix socket bound to /tmp/local_path */
    if (fs.existsSync('/tmp/local_path')) fs.unlinkSync('/tmp/local_path');
    var socket = unix.createSocket('/tmp/local_path');
    socket.on('error', function(err) {
        console.log(err);
    });

    // Connect to /tmp/remote_path
    socket.connect({ path : '/tmp/remote_path' }, function() {
        // CONNECTED !!!
        console.log("CLIENT CONNECTED: " + this.getRemotePath().path);
    });

    socket.on('data', function(data) {
        console.log('Data received: ' + data);
    });
};

if (fs.existsSync('/tmp/remote_path')) fs.unlinkSync('/tmp/remote_path');
var server = unix.createServer('/tmp/remote_path', listening_listener, function(con) {
    console.log("CONNECTED TO SERVER FROM: " + con.getRemotePath());
});

server.on('error', function(err) {
    console.log(err);
});