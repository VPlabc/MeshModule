import React from "react";

const Dashboard = ({ children }) => (
  <div className="tab-content active" id="dashboard">
    <div style={{ display: "flex", gap: 20 }}>
      {/* Dashboard Card */}
      <div
        style={{
          flex: 2,
          border: "1px solid #ddd",
          padding: 20,
          borderRadius: 8,
          boxShadow: "0 2px 4px rgba(0, 0, 0, 0.1)",
          height: 750,
        }}
      >
        <div
          id="nodeDashboard"
          style={{
            display: "flex",
            flexWrap: "wrap",
            gap: 20,
          }}
        >
          {children}
        </div>
      </div>
    </div>
  </div>
);

export default Dashboard;