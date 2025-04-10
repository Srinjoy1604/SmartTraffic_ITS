const express = require('express');
const mongoose = require('mongoose');
const dotenv = require('dotenv');
const cors = require('cors');
const gpsRoutes = require('./routes/gpsRoutes');
const setupWebSocket = require('./websocket');
dotenv.config();
const app = express();
// Start Server
const PORT = process.env.PORT || 8000;
const DevURL = process.env.FRONTEND_URL;
const server=app.listen(PORT, () => {
  console.log(`🚀 Server running at http://localhost:${PORT}`);
});
const wss = setupWebSocket(server);
const wifiRoutes = require('./routes/wifiRoutes')(wss);
app.use(cors({
  origin: [`${DevURL}`, "http://localhost:5173"], // Allow both origins
  credentials:true,            //access-control-allow-credentials:true
  optionSuccessStatus:200
}));
app.use(express.json());


app.use('/api', gpsRoutes);

app.use('/', wifiRoutes);
app.get("/health", (req, res) => {
    res.send("Server ON");
});

mongoose.connect(process.env.MONGO_URI).then(() => console.log("✅ MongoDB Connected"))
  .catch(err => console.error("❌ MongoDB Error:", err));



