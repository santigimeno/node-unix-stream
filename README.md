node-unix-stream
================

Add bind support to client node unix stream sockets.
(*It only works on Linux flavoured systems*)


Example
=======
```
var unix = require('./src/unix_stream');

// Create Unix socket bound to /tmp/local_path
var socket = unix.createSocket('/tmp/local_path');

socket.on('error', function(err) {
    console.log(err);
});

// Connect to /tmp/remote_path
socket.connect({ path : '/tmp/remote_path' }, function() {
    // CONNECTED !!!
});

socket.on('data', function(data) {
    console.log('Data received: ' + data);
});
```