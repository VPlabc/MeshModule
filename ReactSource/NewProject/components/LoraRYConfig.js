import React, { useState } from "react";

const LoraRYConfig = () => {
  const [form, setForm] = useState({
    LoRaEnable: false,
    ryMode: "0",
    ryRxTime: "",
    ryLowSpeedTime: "",
    ryBand: "",
    ryPower: "",
    ryAddress: "",
    ryNetworkId: "",
  });
  const [responseMsg, setResponseMsg] = useState("");

  const handleChange = (e) => {
    const { name, value, type, checked } = e.target;
    setForm((prev) => ({
      ...prev,
      [name]: type === "checkbox" ? checked : value,
    }));
  };

  const handleSubmit = async () => {
    const loraRYConfig = {
      LoRaEnable: form.LoRaEnable,
      mode: parseInt(form.ryMode),
      rxTime: parseInt(form.ryRxTime),
      lowSpeedTime: parseInt(form.ryLowSpeedTime),
      band: parseInt(form.ryBand),
      power: parseInt(form.ryPower),
      address: parseInt(form.ryAddress),
      networkId: parseInt(form.ryNetworkId),
    };
    try {
      const res = await fetch("/configure-lora", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(loraRYConfig),
      });
      const data = await res.json();
      setResponseMsg(data.status || "LoRa RY configuration saved successfully!");
    } catch (err) {
      setResponseMsg("Error: " + err.message);
    }
  };

  return (
    <div className="tab-content" id="loraRYConfig">
      <h2>LoRa RY Module Configuration</h2>
      <form
        id="loraRYConfigForm"
        onSubmit={(e) => {
          e.preventDefault();
          handleSubmit();
        }}
        autoComplete="off"
      >
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>
            Enable LoRa:
          </label>
          <input
            type="checkbox"
            name="LoRaEnable"
            checked={form.LoRaEnable}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Mode:</label>
          <select
            name="ryMode"
            style={{ width: 200 }}
            value={form.ryMode}
            onChange={handleChange}
          >
            <option value="0">Transceiver Mode</option>
            <option value="1">Sleep Mode</option>
            <option value="2">Smart Power Saving Mode</option>
          </select>
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>
            RX Time (ms):
          </label>
          <input
            type="number"
            name="ryRxTime"
            placeholder="Enter RX Time"
            style={{ width: 200 }}
            min={30}
            max={60000}
            value={form.ryRxTime}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>
            Low Speed Time (ms):
          </label>
          <input
            type="number"
            name="ryLowSpeedTime"
            placeholder="Enter Low Speed Time"
            style={{ width: 200 }}
            min={30}
            max={60000}
            value={form.ryLowSpeedTime}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>
            Frequency Band:
          </label>
          <input
            type="number"
            name="ryBand"
            placeholder="Enter Frequency (Hz)"
            style={{ width: 200 }}
            min={490000000}
            max={915000000}
            value={form.ryBand}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>
            Power Level:
          </label>
          <input
            type="number"
            name="ryPower"
            placeholder="Enter Power (dBm)"
            style={{ width: 200 }}
            min={0}
            max={22}
            value={form.ryPower}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>
            Address:
          </label>
          <input
            type="number"
            name="ryAddress"
            placeholder="Enter Address"
            style={{ width: 200 }}
            min={0}
            max={65535}
            value={form.ryAddress}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>
            Network ID:
          </label>
          <input
            type="number"
            name="ryNetworkId"
            placeholder="Enter Network ID"
            style={{ width: 200 }}
            min={3}
            max={18}
            value={form.ryNetworkId}
            onChange={handleChange}
          />
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
      <p id="responseMessageRyConfig" style={{ marginTop: 20, color: "green" }}>
        {responseMsg}
      </p>
    </div>
  );
};

export default LoraRYConfig;