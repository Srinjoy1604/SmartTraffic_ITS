const express = require('express');
const router = express.Router();
const { uploadGpsData } = require('../controllers/gpsController');

router.post('/gps', uploadGpsData);

module.exports = router;
