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
        console.log("[Client] Client bound to: " + this.localPath + " connected to: " + this.remotePath);
    });

    socket.on('data', function(data) {
        console.log('Data received by client: ' + data);
		socket.end('pong');
    });
});
