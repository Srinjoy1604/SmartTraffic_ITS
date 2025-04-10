const express = require('express');
const router = express.Router();
const wifiController = require('../controllers/wifiControllers');

module.exports = (wss) => {
  router.get('/', wifiController.serveReact);
  router.get('/api/scan', wifiController.scanDevices);
  router.post('/api/config', wifiController.submitConfig);
  router.post('/api/feedback', wifiController.handleFeedback(wss));
  return router;
};
