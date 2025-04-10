import React from 'react';

const DeviceSelector = ({ devices, selectedDevice, onChange }) => (
  <div className="form-group">
    <label>Select Device:</label>
    <select value={selectedDevice} onChange={onChange}>
      <option value="">Select a device</option>
      {devices.map(device => (
        <option key={device} value={device}>{device}</option>
      ))}
    </select>
  </div>
);

export default DeviceSelector;
