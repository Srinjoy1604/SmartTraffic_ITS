const GpsData = require('../models/GpsData');

const uploadGpsData = async (req, res) => {
  try {
    const { vehicleID, latitude, longitude, speed, course, accelVelocity, accelCourse, time } = req.body;

    if (!vehicleID || !latitude || !longitude || !time) {
      return res.status(400).json({ error: "Missing required fields." });
    }

    const newEntry = new GpsData({ 
      vehicleID, 
      latitude, 
      longitude, 
      speed, 
      course, 
      accelVelocity, 
      accelCourse, 
      time 
    });
    await newEntry.save();

    res.status(200).json({ message: "GPS data uploaded successfully." });
  } catch (error) {
    console.error("Upload Error:", error);
    res.status(500).json({ error: error.message || "Unknown server error" });
  }
};
const fetchData= async (req, res) => {
  try {
    const data = await GpsData.find().sort({ timestamp: -1 });
    res.json(data);
  } catch (err) {
    res.status(500).json({ error: err.message});
  }
}
module.exports={
  uploadGpsData,
  fetchData
};