const devices = {};

exports.serveReact = (req, res) => {
  res.sendFile(__dirname + '/../public/index.html');
};

exports.submitConfig = (req, res) => {
  const { deviceId, ssid, password, securityId } = req.body;

  if (!deviceId || !ssid || !password || !securityId) {
    return res.status(400).json({ error: "Missing required fields" });
  }

  devices[deviceId] = { ssid, password, securityId, lastFeedback: null };
  res.json({ status: "success" });
};

exports.handleFeedback = (req, res) => {
  const { deviceId, message } = req.body;
  console.log(`Feedback from ${deviceId}: ${message}`);

  if (devices[deviceId]) {
    devices[deviceId].lastFeedback = message;
  }

  res.json({ status: "received" });
};

exports.getFeedback = (req, res) => {
  const { deviceId } = req.query;
  if (!deviceId || !devices[deviceId]) {
    return res.json([]);
  }
  const lastFeedback = devices[deviceId].lastFeedback || "Waiting for device response...";
  res.json([{ message: lastFeedback }]);
};