[![Build Status](https://secure.travis-ci.org/santigimeno/node-unix-stream.png?branch=master)](http://travis-ci.org/santigimeno/node-unix-stream)

node-unix-stream
================

At the moment, node does not support unix stream connections from a socket bound to a local path. As stated in
https://github.com/joyent/node/issues/3705, it likely won't be supported as Windows doesn't have this feature.

This module adds this functionality and expands net.Socket to add a __remotePath__ getter and a __path()__ functions
that implement a similar functionality to __remoteAddress__ and __address()__ for unix stream sockets.


Example
=======
```
var net = require('net');
var fs = require('fs');
var unix = require('../src/unix_stream');

// Create Unix socket Server bound to /tmp/remote_path
if (fs.existsSync('/tmp/remote_path')) {
    fs.unlinkSync('/tmp/remote_path');
}

var server = net.createServer(function(con) {
    console.log("[Server] Client connected from: " + con.remotePath);
    con.write('ping');
    con.on('data', function(data) {
        console.log('Data received by server: ' + data);
    });

    con.on('end', function() {
        server.close();
    });
});

server.listen('/tmp/remote_path', function() {
    // Create Unix socket bound to /tmp/local_path
    if (fs.existsSync('/tmp/local_path')) {
        fs.unlinkSync('/tmp/local_path');
    }

    var socket = unix.createSocket('/tmp/local_path');
    socket.on('error', function(err) {
        console.log(err);
    });

    // Connect to /tmp/remote_path
    socket.connect({ path : '/tmp/remote_path' }, function() {
        console.log("[Client] Client bound to: " + this.path() + " connected to: " + this.remotePath);
    });

    socket.on('data', function(data) {
        console.log('Data received by client: ' + data);
        socket.end('pong');
    });
});
```


[![Bitdeli Badge](https://d2weczhvl823v0.cloudfront.net/santigimeno/node-unix-stream/trend.png)](https://bitdeli.com/free "Bitdeli Badge")

