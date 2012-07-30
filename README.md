node-unix-stream
================

At the moment, node does not support unix stream connections from a socket bound to a local path. As stated in
https://github.com/joyent/node/issues/3705, it likely won't be supported as Windows doesn't have this feature.

This module adds this functionality and expands net.Socket to add a __remotePath__ getter and a __path()__ functions
that implement a similar functionality to __remoteAddress__ and __address()__ for unix stream sockets.


Example
=======
```
var unix = require('./src/unix_stream');

// Create Unix socket bound to /tmp/local_path
var socket = unix.createSocket('/tmp/local_path', function(con) {
    console.log("[Server] Client connected from: " + con.remotePath);
});

socket.on('error', function(err) {
    console.log(err);
});

// Connect to /tmp/remote_path
socket.connect({ path : '/tmp/remote_path' }, function() {
    console.log("[Client] Client bound to: " + this.path() + " connected to: " + this.remotePath);
});

socket.on('data', function(data) {
    console.log('Data received: ' + data);
});
```
