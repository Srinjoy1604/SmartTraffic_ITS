import React, { useEffect, useState } from 'react';
import DeviceSelector from '../components/DeviceSelector'; // Optional if keeping dropdown
import WifiForm from '../components/WifiForm';
import FeedbackList from '../components/FeedbackList';

const Home = () => {
  const [vehicleId, setVehicleId] = useState(''); // Changed from selectedDevice
  const [ssid, setSsid] = useState('');
  const [password, setPassword] = useState('');
  const [securityId, setSecurityId] = useState('');
  const [feedback, setFeedback] = useState([]);

  const production = import.meta.env.VITE_PRODUCTION;
  const BASE_URL = (production === 'true' ? import.meta.env.VITE_BASE_URL_BACKEND : 'http://localhost:8000');

  const addFeedback = (message) => {
    setFeedback(prev => [...prev, {
      id: Date.now(),
      message,
      time: new Date().toLocaleTimeString(),
    }]);
  };

  const submitConfig = async () => {
    if (!vehicleId || !ssid || !password || !securityId) {
      addFeedback('Please enter a vehicle ID and fill all fields!');
      return;
    }

    try {
      const res = await fetch(`${BASE_URL}/connect/config`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ deviceId: vehicleId, ssid, password, securityId }),
      });

      if (res.ok) {
        addFeedback('Configuration sent to device ' + vehicleId + '! Waiting for response...');
      } else {
        addFeedback('Failed to send configuration.');
      }
    } catch (err) {
      addFeedback('Error: ' + err.message);
    }
  };

  // Polling for feedback
  useEffect(() => {
    const pollFeedback = setInterval(async () => {
      if (vehicleId) {
        try {
          const res = await fetch(`${BASE_URL}/connect/feedback?deviceId=${vehicleId}`);
          if (res.ok) {
            const data = await res.json();
            data.forEach(msg => addFeedback(msg.message));
          }
        } catch (err) {
          addFeedback('Polling error: ' + err.message);
        }
      }
    }, 5000); // Poll every 5 seconds
    return () => clearInterval(pollFeedback);
  }, [vehicleId]);

  return (
    <div className="App">
      <h1>Configure ESP32 Device</h1>
      {/* Replace DeviceSelector with manual input */}
      <div className="form-group">
        <label>Vehicle ID:</label>
        <input
          type="text"
          value={vehicleId}
          onChange={(e) => setVehicleId(e.target.value)}
          placeholder="e.g., 0000"
        />
      </div>
      <WifiForm
        ssid={ssid}
        password={password}
        securityId={securityId}
        setSsid={setSsid}
        setPassword={setPassword}
        setSecurityId={setSecurityId}
        onSubmit={submitConfig}
      />
      <FeedbackList
        feedback={feedback}
        selectedDevice={vehicleId} // Pass vehicleId instead of selectedDevice
      />
    </div>
  );
};

export default Home;