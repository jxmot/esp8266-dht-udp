/* ************************************************************************ */
/*
    multi-udp.js - listens and waits for a UDP multi-cast.

*/
// an option argument can specify and alternative client configuration file. 
var clientCfgFile = process.argv[2];

// we're assuming that if there's an argument present then it's going to be
// "path + filename.ext". It's not checked for validity.
if((clientCfgFile === undefined) || (clientCfgFile === ''))
    clientCfgFile = './multi-udp-cfg.js';

// read the IP address and port # that we'll be using
const cfg = require(clientCfgFile);
const addr = cfg.addr;
const port = cfg.port;

// create a socket to listen on...
const dgram = require('dgram');
const client = dgram.createSocket('udp4');

client.on('listening', () => {
    client.setBroadcast(true)
    client.setMulticastTTL(128); 
    client.addMembership(addr);
    console.log('UDP Multi-Cast Client listening on ' + addr + ":" + port);
});

/*
    Reply Received Event Handler
*/
client.on('message', (payload, remote) => {
    // the correct way to extract a string from the payload is this - 
    var message = payload.filter(letter => letter !== 0);
    console.log(`multicast received : [${message}] from ${remote.address}:${remote.port}`);
});
// all intercaces
client.bind(port);
