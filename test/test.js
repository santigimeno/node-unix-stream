var assert = require('assert');
var net = require('net');
var fs = require('fs');
var unix = require('../src/unix_stream');

describe('Echo server', function() {
    before(function(done) {
        if (fs.existsSync('/tmp/remote_path')) {
            fs.unlinkSync('/tmp/remote_path');
        }

        if (fs.existsSync('/tmp/local_path')) {
            fs.unlinkSync('/tmp/local_path');
        }

        this.server = net.createServer();
        this.server.listen('/tmp/remote_path', done);
        this.server.on('error', done);
    });

    it('should emit error if binding to an existing path', function(done) {
        fs.openSync('/tmp/local_ppp', 'w');
        var socket = unix.createSocket('/tmp/local_ppp');
        socket.on('error', function(err) {
            assert.equal(err.syscall, 'bind');
            done();
        });
    });

    it('should set the local and remote path correctly', function(done) {
        var self = this;
        this.server.once('connection', function(con) {
            assert.equal(con.path(), self.socket.remotePath);
            assert.equal(con.localPath, self.socket.remotePath);
            assert.equal(con.remotePath, self.socket.path());
            done();
        })
        
        this.socket = unix.createSocket('/tmp/local_path');
        this.socket.connect({ path : '/tmp/remote_path' });
    });

    it('should set the local and remote path correctly', function(done) {
        var self = this;
        this.server.once('connection', function(con) {
            assert.equal(con.path(), self.socket.remotePath);
            assert.equal(con.localPath, self.socket.remotePath);
            assert.equal(con.remotePath, null);
            assert.equal(self.socket.path(), null);
            assert.equal(self.socket.localPath, null);
            done();
        });

        this.socket = unix.createSocket();
        this.socket.connect({ path : '/tmp/remote_path' });
    });

    afterEach(function(done) {
        if (fs.existsSync('/tmp/local_path')) {
            fs.unlinkSync('/tmp/local_path');
        }

        if (this.socket) {
            this.socket.on('end', done);
            this.socket.end();
        } else {
            done();
        }
    });

    after(function(done) {
        this.server.close(done);
        if (fs.existsSync('/tmp/remote_path')) {
            fs.unlinkSync('/tmp/remote_path');
        }
    });
});

