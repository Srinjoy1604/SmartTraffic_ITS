const express = require('express');
const mongoose = require('mongoose');
const dotenv = require('dotenv');
const cors = require('cors');
const gpsRoutes = require('./routes/gpsRoutes');
const wifiRoutes = require('./routes/wifiRoutes');

dotenv.config();
const app = express();

// Start Server
const PORT = process.env.PORT || 8000;
const DevURL = process.env.FRONTEND_URL;
app.listen(PORT, () => {
  console.log(`🚀 Server running at http://localhost:${PORT}`);
});

app.use(cors({
  origin: [`${DevURL}`, "http://localhost:5173","https://iot-project-interfacefrontend.vercel.app"],
  credentials: true,
  optionSuccessStatus: 200
}));
app.use(express.json());

app.use('/api', gpsRoutes);
app.use('/connect', wifiRoutes());

app.get("/health", (req, res) => {
  res.send("Server ON");
});

mongoose.connect(process.env.MONGO_URI)
  .then(() => console.log("✅ MongoDB Connected"))
  .catch(err => console.error("❌ MongoDB Error:", err));

module.exports = app; // For Vercel