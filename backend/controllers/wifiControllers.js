const wifi = require('node-wifi');

wifi.init({ iface: null }); // Use default interface

// In-memory storage for device configs
const devices = {};

exports.serveReact = (req, res) => {
  res.sendFile(__dirname + '/../public/index.html');
};

exports.scanDevices = (req, res) => {
  wifi.scan((err, networks) => {
    if (err) {
      console.error(err);
      return res.status(500).json({ error: "Failed to scan networks" });
    }

    const esp32Devices = networks
      .filter(net => net.ssid.startsWith('ESP32_'))
      .map(net => ({ ssid: net.ssid, deviceId: net.ssid.replace('ESP32_', '') }));

    res.json(esp32Devices);
  });
};

exports.submitConfig = (req, res) => {
  const { deviceId, ssid, password, securityId } = req.body;

  if (!deviceId || !ssid || !password || !securityId) {
    return res.status(400).json({ error: "Missing required fields" });
  }

  devices[deviceId] = { ssid, password, securityId };
  res.json({ status: "success" });
};

exports.handleFeedback = (wss) => (req, res) => {
  const { deviceId, message } = req.body;
  console.log(`Feedback from ${deviceId}: ${message}`);

  wss.clients.forEach(client => {
    if (client.readyState === 1) {
      client.send(JSON.stringify({ deviceId, message }));
    }
  });

  res.json({ status: "received" });
};
