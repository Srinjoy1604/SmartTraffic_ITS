import React from 'react';

const FeedbackList = ({ feedback, selectedDevice }) => (
  <div className="feedback">
    <h3>Feedback for Device {selectedDevice || 'None'}</h3>
    {feedback.map(item => (
      <p key={item.id}>{item.time}: {item.message}</p>
    ))}
  </div>
);

export default FeedbackList;