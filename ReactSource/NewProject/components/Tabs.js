import React, { useState } from "react";

// Import your tab content components here
// import Dashboard from "./Dashboard";
import MeshConfig from "./MeshConfig";
import ModbusConfig from "./ModbusConfig";
import WifiMqttConfig from "./WifiMqttConfig";
import MappingData from "./MappingData";
// import LoraConfig from "./LoraConfig";
// import LoraRYConfig from "./LoraRYConfig";
// import FileManager from "./FileManager";
// import DataViewer from "./DataViewer";

const tabList = [
  { id: "dashboard", label: "Dashboard" },
  { id: "meshConfig", label: "Mesh Network" },
  { id: "modbusConfig", label: "Modbus Configuration" },
  { id: "wifiConfig", label: "WiFi Configuration" },
  { id: "mappingData", label: "Mapping Data" },
  // { id: "loraConfig", label: "LoRa E32 Configuration" },
  // { id: "loraRYConfig", label: "LoRa RY Configuration" },
  // { id: "fileManager", label: "File Manager" },
  // { id: "dataViewer", label: "Data Viewer" },
];

function Tabs() {
  const [activeTab, setActiveTab] = useState("dashboard");

  const renderTabContent = () => {
    switch (activeTab) {
      // case "dashboard":
      //   return <Dashboard />;
      case "meshConfig":
        return <MeshConfig />;
      case "modbusConfig":
        return <ModbusConfig />;
      case "wifiConfig":
        return <WifiMqttConfig />;
      case "mappingData":
        return <MappingData />;
      // case "loraConfig":
      //   return <LoraConfig />;
      // case "loraRYConfig":
      //   return <LoraRYConfig />;
      // case "fileManager":
      //   return <FileManager />;
      // case "dataViewer":
      //   return <DataViewer />;
      default:
        return <div style={{ padding: 40 }}>Select a tab to view content.</div>;
    }
  };

  return (
    <div>
      <div className="tabs">
        {tabList.map((tab) => (
          <div
            key={tab.id}
            className={`tab${activeTab === tab.id ? " active" : ""}`}
            onClick={() => setActiveTab(tab.id)}
            style={{
              display: "inline-block",
              padding: "10px 20px",
              cursor: "pointer",
              borderBottom: activeTab === tab.id ? "2px solid #2196F3" : "none",
              fontWeight: activeTab === tab.id ? "bold" : "normal",
            }}
          >
            {tab.label}
          </div>
        ))}
      </div>
      <div>{renderTabContent()}</div>
    </div>
  );
}

export default Tabs;