import React from 'react';

const WifiForm = ({ ssid, password, securityId, setSsid, setPassword, setSecurityId, onSubmit }) => (
  <div>
    <div className="form-group">
      <label>WiFi SSID:</label>
      <input type="text" value={ssid} onChange={(e) => setSsid(e.target.value)} placeholder="Enter SSID" />
    </div>
    <div className="form-group">
      <label>WiFi Password:</label>
      <input type="password" value={password} onChange={(e) => setPassword(e.target.value)} placeholder="Enter Password" />
    </div>
    <div className="form-group">
      <label>Security ID:</label>
      <input type="text" value={securityId} onChange={(e) => setSecurityId(e.target.value)} placeholder="Enter Device-Specific Security ID" />
    </div>
    <button onClick={onSubmit}>Submit</button>
  </div>
);

export default WifiForm;
