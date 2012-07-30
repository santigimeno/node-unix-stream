var assert = require('assert');
var net = require('net');
var fs = require('fs');
var unix = require('../src/unix_stream');

describe('Echo server', function() {

    it('client bound', function(done) {
        var socket;
        var server = net.createServer(function(con) {
            assert.equal(con.path(), socket.remotePath);
            assert.equal(con.remotePath, socket.path());
            done();
        });

        /* Unlink /tmp/tmp_server if exists, then listen */
        if (fs.existsSync('/tmp/remote_path')) fs.unlinkSync('/tmp/remote_path');
        server.listen('/tmp/remote_path', function() {
            /* Create Unix socket bound to /tmp/local_path */
            if (fs.existsSync('/tmp/local_path')) fs.unlinkSync('/tmp/local_path');
            socket = unix.createSocket('/tmp/local_path');
            /* Connect to /tmp/remote_path */
            socket.connect({ path : '/tmp/remote_path' });
        });
    });

    it('client not bound', function(done) {
        var socket;
        var server = net.createServer(function(con) {
            assert.equal(con.path(), socket.remotePath);
            assert.equal(con.remotePath, null);
            assert.equal(socket.path(), null);
            done();
        });

        /* Unlink /tmp/tmp_server if exists, then listen */
        if (fs.existsSync('/tmp/remote_path')) fs.unlinkSync('/tmp/remote_path');
        server.listen('/tmp/remote_path', function() {
            /* Create Unix socket bound to /tmp/local_path */
            if (fs.existsSync('/tmp/local_path')) fs.unlinkSync('/tmp/local_path');
            socket = unix.createSocket();
            /* Connect to /tmp/remote_path */
            socket.connect({ path : '/tmp/remote_path' });
        });
    });
});
