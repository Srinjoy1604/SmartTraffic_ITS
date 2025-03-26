const mongoose = require('mongoose');

const gpsDataSchema = new mongoose.Schema({
  vehicleID: { type: String, required: true },
  latitude: { type: Number, required: true },
  longitude: { type: Number, required: true },
  speed: { type: Number },        // From NEO-6M
  course: { type: Number },       // From NEO-6M
  accelVelocity: { type: Number }, // From MPU6050
  accelCourse: { type: Number },   // From MPU6050
  time: { type: String },
  createdAt: { type: Date, default: Date.now }
});

module.exports = mongoose.model('GpsData', gpsDataSchema);
