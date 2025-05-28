import React, { useState } from "react";

const MeshConfig = () => {
  const [form, setForm] = useState({
    boardModel: "0",
    meshEnable: false,
    debug: false,
    loraEnb: false,
    buzzEnb: false,
    brokerAddress: "",
    wifiChannel: "1",
    nodeId: "",
    netId: "",
    meshrole: "Broker",
    macSlaves: "",
    classDataRequest: "A",
    communicationTime: "",
    dataVersion: "0",
  });
  const [responseMsg, setResponseMsg] = useState("");
  const [resetEspResponse, setResetEspResponse] = useState("");

  const handleChange = (e) => {
    const { name, value, type, checked } = e.target;
    setForm((prev) => ({
      ...prev,
      [name]: type === "checkbox" ? checked : value,
    }));
  };

  const handleSubmit = async () => {
    const meshConfig = {
      meshEnable: form.meshEnable,
      BrokerAddress: form.brokerAddress,
      wifiChannel: parseInt(form.wifiChannel),
      id: parseInt(form.nodeId),
      netId: parseInt(form.netId),
      role: form.meshrole,
      debug: form.debug,
      macSlaves: form.macSlaves.split(",").map((mac) => mac.trim()),
      dataVersion: parseInt(form.dataVersion),
      boardModel: form.boardModel,
      classDataRequest: form.classDataRequest,
      communicationTime: parseInt(form.communicationTime) || 0,
      loraEnb: form.loraEnb,
      buzzEnb: form.buzzEnb,
    };
    try {
      const res = await fetch("/save-mesh-config", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(meshConfig),
      });
      const data = await res.json();
      setResponseMsg(data.status || "Mesh configuration saved successfully!");
    } catch (err) {
      setResponseMsg("Error: " + err.message);
    }
  };

  // Optional: handle reset ESP
  const handleResetEsp = async () => {
    setResetEspResponse("Resetting...");
    try {
      const res = await fetch("/reset-esp", { method: "POST" });
      const data = await res.json();
      setResetEspResponse(data.status || "Reset command sent!");
    } catch (err) {
      setResetEspResponse("Reset failed: " + err);
    }
  };

  return (
    <div className="tab-content" id="meshConfig">
      <div style={{ display: "flex", gap: 20 }}>
        <div
          style={{
            flex: 1,
            border: "1px solid #ddd",
            padding: 20,
            borderRadius: 8,
            boxShadow: "0 2px 4px rgba(0, 0, 0, 0.1)",
          }}
        >
          <h2>Mesh Network Configuration</h2>
          <form
            id="meshConfigForm"
            onSubmit={(e) => {
              e.preventDefault();
              handleSubmit();
            }}
            autoComplete="off"
          >
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                Board Model:
              </label>
              <select
                name="boardModel"
                style={{ width: 200 }}
                value={form.boardModel}
                onChange={handleChange}
              >
                <option value="0">Board Custom Model</option>
                <option value="1">Board ModRTUMesh</option>
                <option value="2">Board 410WER</option>
                <option value="3">Board LklineGw</option>
                <option value="4">Board LklineNode</option>
                <option value="5">Board S3SDCWER</option>
              </select>
            </div>
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                Mesh enable:
              </label>
              <input
                type="checkbox"
                name="meshEnable"
                checked={form.meshEnable}
                onChange={handleChange}
              />
            </div>
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                Debug:
              </label>
              <input
                type="checkbox"
                name="debug"
                checked={form.debug}
                onChange={handleChange}
              />
            </div>
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                LoRa enable:
              </label>
              <input
                type="checkbox"
                name="loraEnb"
                checked={form.loraEnb}
                onChange={handleChange}
              />
            </div>
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                Buzzer enable:
              </label>
              <input
                type="checkbox"
                name="buzzEnb"
                checked={form.buzzEnb}
                onChange={handleChange}
              />
            </div>
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                Broker MAC Address:
              </label>
              <input
                type="text"
                name="brokerAddress"
                placeholder="FF:FF:FF:FF:FF:FF"
                style={{ width: 200 }}
                value={form.brokerAddress}
                onChange={handleChange}
              />
            </div>
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                WiFi Channel:
              </label>
              <select
                name="wifiChannel"
                style={{ width: 200 }}
                value={form.wifiChannel}
                onChange={handleChange}
              >
                {Array.from({ length: 12 }, (_, i) => (
                  <option key={i + 1} value={i + 1}>
                    Channel {i + 1}
                  </option>
                ))}
              </select>
            </div>
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                Node ID:
              </label>
              <input
                type="number"
                name="nodeId"
                placeholder="Enter Node ID"
                style={{ width: 200 }}
                value={form.nodeId}
                onChange={handleChange}
              />
            </div>
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                Network ID:
              </label>
              <input
                type="number"
                name="netId"
                placeholder="Enter Network ID"
                style={{ width: 200 }}
                value={form.netId}
                onChange={handleChange}
              />
            </div>
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                Role:
              </label>
              <select
                name="meshrole"
                style={{ width: 200 }}
                value={form.meshrole}
                onChange={handleChange}
              >
                <option value="Broker">Broker</option>
                <option value="Node">Node</option>
                <option value="Repeater">Repeater</option>
              </select>
            </div>
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                Slave MAC Addresses:
              </label>
              <input
                type="text"
                name="macSlaves"
                placeholder="Comma-separated MACs"
                style={{ width: 200 }}
                value={form.macSlaves}
                onChange={handleChange}
              />
            </div>
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                Class Data Request:
              </label>
              <select
                name="classDataRequest"
                style={{ width: 200 }}
                value={form.classDataRequest}
                onChange={handleChange}
              >
                <option value="A">Class A</option>
                <option value="B">Class B</option>
                <option value="C">Class C</option>
              </select>
            </div>
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                Communication Time (ClassB):
              </label>
              <input
                type="number"
                name="communicationTime"
                placeholder="Enter Time (ms)"
                style={{ width: 200 }}
                min={0}
                value={form.communicationTime}
                onChange={handleChange}
              />
            </div>
            <div style={{ marginBottom: 10 }}>
              <label style={{ display: "inline-block", width: 150 }}>
                Data Version:
              </label>
              <select
                name="dataVersion"
                style={{ width: 200 }}
                value={form.dataVersion}
                onChange={handleChange}
              >
                <option value="0">Lookline v1</option>
                <option value="1">Lookline v2</option>
                <option value="2">Modbus Register</option>
                <option value="3">Serial TTL</option>
              </select>
            </div>
            <button
              type="submit"
              style={{
                backgroundColor: "#2196F3",
                color: "white",
                marginBottom: 10,
              }}
            >
              Submit
            </button>
          </form>
          <p id="responseMessageMesh" style={{ marginTop: 20, color: "green" }}>
            {responseMsg}
          </p>
          <span
            id="resetEspResponse"
            style={{
              marginLeft: 10,
              color: "#2196F3",
              fontWeight: "bold",
              display: "inline-block",
            }}
          >
            {resetEspResponse}
          </span>
          <button
            type="button"
            style={{
              backgroundColor: "#ff9800",
              color: "white",
              padding: "6px 14px",
              border: "none",
              borderRadius: "4px",
              cursor: "pointer",
              marginTop: 10,
            }}
            onClick={handleResetEsp}
          >
            Reset ESP
          </button>
        </div>
      </div>
    </div>
  );
};

export default MeshConfig;