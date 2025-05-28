import React, { useState } from "react";

const LoraConfig = () => {
  const [form, setForm] = useState({
    address: "",
    uartSpeed: "1200_8N1",
    airRate: "0.3kbps",
    channel: "",
    transmissionMode: "fixed",
    pullUp: false,
    fec: false,
    power: "0",
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
    const loraConfig = {
      address: parseInt(form.address),
      uart_speed: form.uartSpeed,
      air_data_rate: form.airRate,
      channel: parseInt(form.channel),
      transmission_mode: form.transmissionMode,
      pull_up: form.pullUp,
      fec: form.fec,
      power: parseInt(form.power),
    };
    try {
      const res = await fetch("/configure-e32", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(loraConfig),
      });
      const data = await res.json();
      setResponseMsg(data.status || "LoRa E32 configuration saved successfully!");
    } catch (err) {
      setResponseMsg("Error: " + err.message);
    }
  };

  return (
    <div className="tab-content" id="loraConfig">
      <h2>LoRa E32 Configuration</h2>
      <form
        id="loraConfigForm"
        onSubmit={(e) => {
          e.preventDefault();
          handleSubmit();
        }}
        autoComplete="off"
      >
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Address:</label>
          <input
            type="number"
            name="address"
            placeholder="Enter Address"
            style={{ width: 200 }}
            min={0}
            max={65535}
            required
            value={form.address}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>UART Speed:</label>
          <select
            name="uartSpeed"
            style={{ width: 200 }}
            value={form.uartSpeed}
            onChange={handleChange}
          >
            <option value="1200_8N1">1200 8N1</option>
            <option value="2400_8N1">2400 8N1</option>
            <option value="4800_8N1">4800 8N1</option>
            <option value="9600_8N1">9600 8N1</option>
            <option value="19200_8N1">19200 8N1</option>
            <option value="38400_8N1">38400 8N1</option>
          </select>
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Air Data Rate:</label>
          <select
            name="airRate"
            style={{ width: 200 }}
            value={form.airRate}
            onChange={handleChange}
          >
            <option value="0.3kbps">0.3 kbps</option>
            <option value="1.2kbps">1.2 kbps</option>
            <option value="2.4kbps">2.4 kbps</option>
            <option value="4.8kbps">4.8 kbps</option>
            <option value="9.6kbps">9.6 kbps</option>
            <option value="19.2kbps">19.2 kbps</option>
          </select>
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Channel:</label>
          <input
            type="number"
            name="channel"
            placeholder="Enter Channel"
            style={{ width: 200 }}
            min={0}
            max={30}
            required
            value={form.channel}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Transmission Mode:</label>
          <select
            name="transmissionMode"
            style={{ width: 200 }}
            value={form.transmissionMode}
            onChange={handleChange}
          >
            <option value="fixed">Fixed</option>
            <option value="transparent">Transparent</option>
          </select>
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Pull Up:</label>
          <input
            type="checkbox"
            name="pullUp"
            checked={form.pullUp}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>FEC:</label>
          <input
            type="checkbox"
            name="fec"
            checked={form.fec}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Power Level:</label>
          <select
            name="power"
            style={{ width: 200 }}
            value={form.power}
            onChange={handleChange}
          >
            <option value="0">1W (30 dBm)</option>
            <option value="1">500mW (27 dBm)</option>
            <option value="2">250mW (24 dBm)</option>
            <option value="3">125mW (21 dBm)</option>
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
      <p id="responseMessageLora" style={{ marginTop: 20, color: "green" }}>
        {responseMsg}
      </p>
    </div>
  );
};

export default LoraConfig;