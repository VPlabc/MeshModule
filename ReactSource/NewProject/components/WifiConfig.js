Chắc chắn rồi! Tuy nhiên, bạn chưa cung cấp mã nguồn cụ thể nào để tôi có thể chuyển đổi sang React. Nếu bạn có một đoạn mã hoặc một ý tưởng cụ thể mà bạn muốn chuyển đổi sang React, hãy chia sẻ với tôi, và tôi sẽ giúp bạn viết lại nó bằng React.import React, { useEffect, useState } from "react";

const initialState = {
  wifiMode: "STA",
  ssid: "",
  password: "",
  ethDhcp: false,
  ethIp: "",
  ethSubnet: "",
  ethGateway: "",
  ethDns: "",
  mqttEnable: false,
  mqttHost: "",
  mqttPort: "",
  mqttUser: "",
  mqttPass: "",
  conId: "",
  topicPush: "",
  topicSub: "",
  mqttKeepAlive: "",
  mqttCleanSession: false,
  mqttQos: "0",
  mqttRetain: false,
  mqttLwtTopic: "",
  mqttLwtMessage: "",
  mqttLwtQos: "0",
  mqttLwtRetain: false,
  mqttLwtEnabled: false,
  timezone: "0",
};

const WifiMqttConfig = () => {
  const [form, setForm] = useState(initialState);
  const [responseMsg, setResponseMsg] = useState("");
  const [responseEther, setResponseEther] = useState("");

  useEffect(() => {
    // Load WiFi/MQTT config
    fetch("/load-wifi-mqtt-config/")
      .then((res) => res.json())
      .then((data) => setForm((prev) => ({ ...prev, ...data })))
      .catch(() => {});
    // Load Ethernet config
    fetch("/load-ethernet-config")
      .then((res) => res.json())
      .then((data) =>
        setForm((prev) => ({
          ...prev,
          ethDhcp: data.dhcp || false,
          ethIp: data.ip || "",
          ethSubnet: data.subnet || "",
          ethGateway: data.gateway || "",
          ethDns: data.dns || "",
        }))
      )
      .catch(() => {});
  }, []);

  const handleChange = (e) => {
    const { name, value, type, checked } = e.target;
    setForm((prev) => ({
      ...prev,
      [name]: type === "checkbox" ? checked : value,
    }));
  };

  const saveEthernetConfig = () => {
    const config = {
      dhcp: form.ethDhcp,
      ip: form.ethIp,
      subnet: form.ethSubnet,
      gateway: form.ethGateway,
      dns: form.ethDns,
    };
    fetch("/save-ethernet-config", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(config),
    })
      .then((res) => res.json())
      .then((data) => setResponseEther(data.status || "Ethernet config saved!"))
      .catch((err) => setResponseEther("Save failed: " + err));
  };

  const loadEthernetConfig = () => {
    fetch("/load-ethernet-config")
      .then((res) => res.json())
      .then((data) =>
        setForm((prev) => ({
          ...prev,
          ethDhcp: data.dhcp || false,
          ethIp: data.ip || "",
          ethSubnet: data.subnet || "",
          ethGateway: data.gateway || "",
          ethDns: data.dns || "",
        }))
      )
      .catch(() => {});
  };

  const handleSubmit = (e) => {
    e.preventDefault();
    const wifiConfig = {
      wifiMode: form.wifiMode,
      ssid: form.ssid,
      password: form.password,
    };
    const mqttConfig = {
      mqttEnable: form.mqttEnable,
      mqttHost: form.mqttHost,
      mqttPort: parseInt(form.mqttPort),
      mqttUser: form.mqttUser,
      mqttPass: form.mqttPass,
      conId: form.conId,
      topicPush: form.topicPush,
      topicSub: form.topicSub,
      mqttKeepAlive: parseInt(form.mqttKeepAlive),
      mqttCleanSession: form.mqttCleanSession,
      mqttQos: parseInt(form.mqttQos),
      mqttRetain: form.mqttRetain,
      mqttLwtTopic: form.mqttLwtTopic,
      mqttLwtMessage: form.mqttLwtMessage,
      mqttLwtQos: parseInt(form.mqttLwtQos),
      mqttLwtRetain: form.mqttLwtRetain,
      mqttLwtEnabled: form.mqttLwtEnabled,
      timezone: form.timezone,
    };
    const combinedConfig = { ...wifiConfig, ...mqttConfig };
    fetch("/save-wifi-mqtt-config/", {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify(combinedConfig),
    })
      .then((res) => res.json())
      .then((data) =>
        setResponseMsg(data.status || "Configuration saved successfully!")
      )
      .catch((err) => setResponseMsg("Error: " + err));
  };

  return (
    <div className="tab-content" id="wifiConfig">
      <h2>WiFi Configuration</h2>
      <form id="wifiConfigForm" onSubmit={handleSubmit} autoComplete="off">
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>WiFi Mode:</label>
          <select
            name="wifiMode"
            style={{ width: 200 }}
            value={form.wifiMode}
            onChange={handleChange}
          >
            <option value="STA">Station (STA)</option>
            <option value="AP">Access Point (AP)</option>
          </select>
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>SSID:</label>
          <input
            type="text"
            name="ssid"
            placeholder="Enter SSID"
            style={{ width: 200 }}
            value={form.ssid}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Password:</label>
          <input
            type="password"
            name="password"
            placeholder="Enter Password"
            style={{ width: 200 }}
            value={form.password}
            onChange={handleChange}
          />
        </div>
        <h2>Ethernet Settings</h2>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>DHCP Enable:</label>
          <input
            type="checkbox"
            name="ethDhcp"
            checked={form.ethDhcp}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>IP Address:</label>
          <input
            type="text"
            name="ethIp"
            placeholder="192.168.1.100"
            style={{ width: 200 }}
            value={form.ethIp}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Subnet Mask:</label>
          <input
            type="text"
            name="ethSubnet"
            placeholder="255.255.255.0"
            style={{ width: 200 }}
            value={form.ethSubnet}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Gateway:</label>
          <input
            type="text"
            name="ethGateway"
            placeholder="192.168.1.1"
            style={{ width: 200 }}
            value={form.ethGateway}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>DNS:</label>
          <input
            type="text"
            name="ethDns"
            placeholder="8.8.8.8"
            style={{ width: 200 }}
            value={form.ethDns}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <button type="button" onClick={saveEthernetConfig}>
            Save Ethernet Settings
          </button>
          <button type="button" onClick={loadEthernetConfig}>
            Reload Ethernet Settings
          </button>
          <p id="responseMessageEther" style={{ marginTop: 20, color: "green" }}>
            {responseEther}
          </p>
        </div>
        <hr style={{ margin: "20px 0", border: "1px solid #ddd" }} />
        <h2>MQTT Configuration</h2>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Enable MQTT:</label>
          <input
            type="checkbox"
            name="mqttEnable"
            checked={form.mqttEnable}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>MQTT Host:</label>
          <input
            type="text"
            name="mqttHost"
            placeholder="Enter MQTT Host"
            style={{ width: 200 }}
            value={form.mqttHost}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>MQTT Port:</label>
          <input
            type="number"
            name="mqttPort"
            placeholder="Enter MQTT Port"
            style={{ width: 200 }}
            value={form.mqttPort}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>MQTT User:</label>
          <input
            type="text"
            name="mqttUser"
            placeholder="Enter MQTT User"
            style={{ width: 200 }}
            value={form.mqttUser}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>MQTT Password:</label>
          <input
            type="password"
            name="mqttPass"
            placeholder="Enter MQTT Password"
            style={{ width: 200 }}
            value={form.mqttPass}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Connection ID:</label>
          <input
            type="text"
            name="conId"
            placeholder="Enter Connection ID"
            style={{ width: 200 }}
            value={form.conId}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Topic Push:</label>
          <input
            type="text"
            name="topicPush"
            placeholder="Enter Topic Push"
            style={{ width: 200 }}
            value={form.topicPush}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Topic Subscribe:</label>
          <input
            type="text"
            name="topicSub"
            placeholder="Enter Topic Subscribe"
            style={{ width: 200 }}
            value={form.topicSub}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>MQTT Keep Alive:</label>
          <input
            type="number"
            name="mqttKeepAlive"
            placeholder="Enter Keep Alive"
            style={{ width: 200 }}
            value={form.mqttKeepAlive}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Clean Session:</label>
          <input
            type="checkbox"
            name="mqttCleanSession"
            checked={form.mqttCleanSession}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>MQTT QoS:</label>
          <select
            name="mqttQos"
            style={{ width: 200 }}
            value={form.mqttQos}
            onChange={handleChange}
          >
            <option value="0">QoS 0</option>
            <option value="1">QoS 1</option>
            <option value="2">QoS 2</option>
          </select>
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>MQTT Retain:</label>
          <input
            type="checkbox"
            name="mqttRetain"
            checked={form.mqttRetain}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>LWT Topic:</label>
          <input
            type="text"
            name="mqttLwtTopic"
            placeholder="Enter LWT Topic"
            style={{ width: 200 }}
            value={form.mqttLwtTopic}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>LWT Message:</label>
          <input
            type="text"
            name="mqttLwtMessage"
            placeholder="Enter LWT Message"
            style={{ width: 200 }}
            value={form.mqttLwtMessage}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>LWT QoS:</label>
          <select
            name="mqttLwtQos"
            style={{ width: 200 }}
            value={form.mqttLwtQos}
            onChange={handleChange}
          >
            <option value="0">QoS 0</option>
            <option value="1">QoS 1</option>
            <option value="2">QoS 2</option>
          </select>
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>LWT Retain:</label>
          <input
            type="checkbox"
            name="mqttLwtRetain"
            checked={form.mqttLwtRetain}
            onChange={handleChange}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>LWT Enabled:</label>
          <input
            type="checkbox"
            name="mqttLwtEnabled"
            checked={form.mqttLwtEnabled}
            onChange={handleChange}
          />
        </div>
        <hr style={{ margin: "20px 0", border: "1px solid #ddd" }} />
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Timezone:</label>
          <select
            name="timezone"
            style={{ width: 200 }}
            value={form.timezone}
            onChange={handleChange}
          >
            <option value="0">UTC</option>
            <option value="1">UTC+1</option>
            <option value="2">UTC+2</option>
            <option value="3">UTC+3</option>
            <option value="4">UTC+4</option>
            <option value="5">UTC+5</option>
            <option value="6">UTC+6</option>
            <option value="7">UTC+7</option>
            <option value="8">UTC+8</option>
            <option value="9">UTC+9</option>
            <option value="10">UTC+10</option>
            <option value="11">UTC+11</option>
            <option value="12">UTC+12</option>
            <option value="-1">UTC-1</option>
            <option value="-2">UTC-2</option>
            <option value="-3">UTC-3</option>
            <option value="-4">UTC-4</option>
            <option value="-5">UTC-5</option>
            <option value="-6">UTC-6</option>
            <option value="-7">UTC-7</option>
            <option value="-8">UTC-8</option>
            <option value="-9">UTC-9</option>
            <option value="-10">UTC-10</option>
            <option value="-11">UTC-11</option>
            <option value="-12">UTC-12</option>
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
          Update
        </button>
      </form>
      <p id="responseMessageWifi" style={{ marginTop: 20, color: "green" }}>
        {responseMsg}
      </p>
    </div>
  );
};

export default WifiMqttConfig;