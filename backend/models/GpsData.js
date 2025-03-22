const mongoose = require('mongoose');

const gpsDataSchema = new mongoose.Schema({
  vehicleID: { type: String, required: true },
  latitude: { type: String, required: true },
  longitude: { type: String, required: true },
  speed: { type: String },
  course: { type: String },
  time: { type: String },
  createdAt: { type: Date, default: Date.now }
});

module.exports = mongoose.model('GpsData', gpsDataSchema);
