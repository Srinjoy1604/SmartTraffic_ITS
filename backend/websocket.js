const WebSocket = require('ws');

function setupWebSocket(server) {
  const wss = new WebSocket.Server({ server });

  wss.on('connection', ws => {
    console.log('New WebSocket client connected');
  });

  return wss;
}

module.exports = setupWebSocket;
