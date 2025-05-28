import React, { useState, useEffect, useRef } from "react";
import "./styles/App.css"; // Copy CSS từ <style> vào đây

function App() {
  // State cho tab
  const [activeTab, setActiveTab] = useState("dashboard");
  // State cho WebSocket
  const [socketConnected, setSocketConnected] = useState(false);
  const websocket = useRef(null);

  // Các state khác cho form, data, modal...
  // ...

  // WebSocket logic
  useEffect(() => {
    const gateway = `ws://${window.location.hostname}/ws`;
    websocket.current = new window.WebSocket(gateway);

    websocket.current.onopen = () => {
      setSocketConnected(true);
      websocket.current.send(JSON.stringify({ type: "subscribe", topic: "nodeDashboard" }));
      // Gửi ping định kỳ
      websocket.current.pingInterval = setInterval(() => {
        if (websocket.current.readyState === 1) websocket.current.send("ping");
      }, 20000);
    };

    websocket.current.onclose = () => {
      setSocketConnected(false);
      setTimeout(() => {
        // reconnect
        websocket.current = null;
        // Gọi lại useEffect
        setSocketConnected(false);
      }, 2000);
    };

    websocket.current.onmessage = (event) => {
      // Xử lý dữ liệu nhận được
      // ...
    };

    return () => {
      if (websocket.current) {
        clearInterval(websocket.current.pingInterval);
        websocket.current.close();
      }
    };
  }, []);

  // Tab switching
  const showTab = (tabId) => setActiveTab(tabId);

  return (
    <div>
      {/* Header */}
      <div className="header">
        {/* SVG Logo */}
        {/* ...SVG code... */}
        <h1>Wireless IoT Configuration</h1>
        {/* Socket status, Reset button, User dropdown, Modal... */}
      </div>

      {/* Tabs */}
      <div className="tabs">
        <div className={`tab ${activeTab === "dashboard" ? "active" : ""}`} onClick={() => showTab("dashboard")}>Dashboard</div>
        <div className={`tab ${activeTab === "meshConfig" ? "active" : ""}`} onClick={() => showTab("meshConfig")}>Mesh Network</div>
        <div className={`tab ${activeTab === "modbusConfig" ? "active" : ""}`} onClick={() => showTab("modbusConfig")}>Modbus Configuration</div>
        <div className={`tab ${activeTab === "wifiConfig" ? "active" : ""}`} onClick={() => showTab("wifiConfig")}>WiFi Configuration</div>
        <div className={`tab ${activeTab === "mappingData" ? "active" : ""}`} onClick={() => showTab("mappingData")}>Mapping Data</div>
        <div className={`tab ${activeTab === "fileManager" ? "active" : ""}`} onClick={() => showTab("fileManager")}>File Manager</div>
        <div className={`tab ${activeTab === "dataViewer" ? "active" : ""}`} onClick={() => showTab("dataViewer")}>Data Viewer</div>
      </div>

      {/* Tab Contents */}
      <div className={`tab-content ${activeTab === "dashboard" ? "active" : ""}`}>
        {/* Dashboard content here */}
      </div>
      <div className={`tab-content ${activeTab === "meshConfig" ? "active" : ""}`}>
        {/* MeshConfig form here */}
      </div>
      <div className={`tab-content ${activeTab === "modbusConfig" ? "active" : ""}`}>
        {/* ModbusConfig form here */}
      </div>
      <div className={`tab-content ${activeTab === "wifiConfig" ? "active" : ""}`}>
        {/* WiFiConfig form here */}
      </div>
      <div className={`tab-content ${activeTab === "mappingData" ? "active" : ""}`}>
        {/* MappingData content here */}
      </div>
      <div className={`tab-content ${activeTab === "fileManager" ? "active" : ""}`}>
        {/* FileManager content here */}
      </div>
      <div className={`tab-content ${activeTab === "dataViewer" ? "active" : ""}`}>
        {/* DataViewer content here */}
      </div>
      {/* Modal, Dropdown, ... */}
    </div>
  );
}

export default App;