const express = require('express');
const router = express.Router();
const { uploadGpsData,fetchData} = require('../controllers/gpsController');

router.post('/gps', uploadGpsData);
router.get('/fetchdata',fetchData);

module.exports = router;
