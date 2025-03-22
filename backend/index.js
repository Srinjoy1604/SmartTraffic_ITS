const express = require('express');
const mongoose = require('mongoose');
const dotenv = require('dotenv');
const cors = require('cors');

const gpsRoutes = require('./routes/gpsRoutes');

dotenv.config();
const app = express();


app.use(cors());
app.use(express.json());


app.use('/api', gpsRoutes);
app.get("/health", (req, res) => {
    res.send("Server ON");
});

mongoose.connect(process.env.MONGO_URI).then(() => console.log("✅ MongoDB Connected"))
  .catch(err => console.error("❌ MongoDB Error:", err));

// Start Server
const PORT = process.env.PORT || 5000;
app.listen(PORT, () => {
  console.log(`🚀 Server running at http://localhost:${PORT}`);
});
