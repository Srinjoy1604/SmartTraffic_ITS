import React, { useEffect, useState } from 'react';
import DeviceSelector from '../components/DeviceSelector';
import WifiForm from '../components/WifiForm';
import FeedbackList from '../components/FeedbackList';

const Home = () => {
  const [devices, setDevices] = useState([]);
  const [selectedDevice, setSelectedDevice] = useState('');
  const [ssid, setSsid] = useState('');
  const [password, setPassword] = useState('');
  const [securityId, setSecurityId] = useState('');
  const [feedback, setFeedback] = useState([]);
//   const serverHost = 'your-domain.vercel.app'; // Replace with your domain

  const production=import.meta.env.VITE_PRODUCTION;
  const BASE_URL = (production=='true'?import.meta.env.VITE_BASE_URL_BACKEND:'http://localhost:8000');
  const wsProtocol = BASE_URL.startsWith('https') ? 'wss' : 'ws';
  const addFeedback = (message) => {
    setFeedback(prev => [...prev, {
      id: Date.now(),
      message,
      time: new Date().toLocaleTimeString(),
    }]);
  };

  useEffect(() => {
    const scanDevices = async () => {
      try {
        const res = await fetch(`${BASE_URL}/api/scan`);
        if (res.ok) {
          const data = await res.json();
          setDevices(data.map(d => d.deviceId));
        }
      } catch (err) {
        addFeedback('Failed to scan devices: ' + err.message);
      }
    };

    scanDevices();
    const interval = setInterval(scanDevices, 30000);
    return () => clearInterval(interval);
  }, []);

  const submitConfig = async () => {
    if (!selectedDevice || !ssid || !password || !securityId) {
      addFeedback('Please select a device and fill all fields!');
      return;
    }

    try {
      const res = await fetch(`${BASE_URL}/api/config`, {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ deviceId: selectedDevice, ssid, password, securityId }),
      });

      if (res.ok) {
        addFeedback('Configuration sent to device ' + selectedDevice + '! Waiting for response...');
      } else {
        addFeedback('Failed to send configuration.');
      }
    } catch (err) {
      addFeedback('Error: ' + err.message);
    }
  };

  useEffect(() => {
    const ws = new WebSocket(`${wsProtocol}://${new URL(BASE_URL).host}/feedback`);

    ws.onmessage = (event) => {
      const data = JSON.parse(event.data);
      if (data.deviceId === selectedDevice) {
        addFeedback(data.message);
      }
    };

    ws.onerror = () => {
      addFeedback('WebSocket connection failed. Polling instead...');
      const poll = setInterval(async () => {
        try {
          const res = await fetch(`${BASE_URL}/api/feedback?deviceId=${selectedDevice}`);
          if (res.ok) {
            const data = await res.json();
            data.forEach(msg => addFeedback(msg.message));
          }
        } catch (err) {
          addFeedback('Polling error: ' + err.message);
        }
      }, 5000);
      return () => clearInterval(poll);
    };

    return () => ws.close();
  }, [selectedDevice]);

  return (
    <div className="App">
      <h1>Configure ESP32 Device</h1>
      <DeviceSelector
        devices={devices}
        selectedDevice={selectedDevice}
        onChange={(e) => setSelectedDevice(e.target.value)}
      />
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
        selectedDevice={selectedDevice}
      />
    </div>
  );
};

export default Home;
