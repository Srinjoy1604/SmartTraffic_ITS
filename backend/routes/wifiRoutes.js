const express = require('express');
const router = express.Router();
const wifiController = require('../controllers/wifiControllers');

module.exports = (wss) => {
  router.get('/', wifiController.serveReact);
  router.get('/scan', wifiController.scanDevices);
  router.post('/config', wifiController.submitConfig);
  router.post('/feedback', wifiController.handleFeedback(wss));
  return router;
};
