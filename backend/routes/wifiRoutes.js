const express = require('express');
const router = express.Router();
const wifiController = require('../controllers/wifiControllers');

module.exports = () => {
  router.get('/', wifiController.serveReact);
  router.post('/config', wifiController.submitConfig);
  router.post('/feedback', wifiController.handleFeedback);
  router.get('/feedback', wifiController.getFeedback);
  return router;
};