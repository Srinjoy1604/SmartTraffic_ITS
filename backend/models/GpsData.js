const mongoose = require('mongoose');

const gpsDataSchema = new mongoose.Schema({
  vehicleID: { type: String, required: true },
  latitude: { type: Number, required: true },
  longitude: { type: Number, required: true },
  speed: { type: Number },
  course: { type: Number },
  time: { type: String },
  createdAt: { type: Date, default: Date.now }
});

module.exports = mongoose.model('GpsData', gpsDataSchema);
