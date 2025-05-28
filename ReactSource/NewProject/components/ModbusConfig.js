import React, { useState } from "react";

const defaultRow = { tag: "", offset: "", value: "", type: "" };

const ModbusConfig = () => {
  // Main form state
  const [role, setRole] = useState("master");
  const [com, setCom] = useState("TCP/IP");
  const [slaveId, setSlaveId] = useState("");
  const [slaveIp, setSlaveIp] = useState("");
  const [responseMsg, setResponseMsg] = useState("");

  // Data block state
  const [dataBlocks, setDataBlocks] = useState([]);
  const [blockInput, setBlockInput] = useState({
    offset: "",
    tagFrom: "",
    valueFrom: "",
    typeFrom: "",
    amount: "",
  });

  // Table rows state
  const [rows, setRows] = useState([{ ...defaultRow }]);

  // Handle form submit
  const handleSubmit = async () => {
    const config = {
      role,
      Com: com,
      id: parseInt(slaveId),
      slaveip: slaveIp.split(",").map((ip) => ip.trim()),
      Tag: rows.map((r) => parseInt(r.tag)),
      Offset: rows.map((r) => parseInt(r.offset)),
      Value: rows.map((r) => parseInt(r.value)),
      Type: rows.map((r) => parseInt(r.type)),
    };
    try {
      const res = await fetch("/modbus-config", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(config),
      });
      const data = await res.json();
      setResponseMsg(data.status || "Configuration saved successfully!");
    } catch (err) {
      setResponseMsg("Error: " + err.message);
    }
  };

  // Handle add row
  const addRow = () => {
    if (rows.length >= 120) {
      alert("Maximum of 120 rows allowed.");
      return;
    }
    setRows([...rows, { ...defaultRow }]);
  };

  // Handle delete row
  const deleteRow = (idx) => {
    setRows(rows.filter((_, i) => i !== idx));
  };

  // Handle row change
  const handleRowChange = (idx, field, value) => {
    setRows(
      rows.map((row, i) =>
        i === idx ? { ...row, [field]: value } : row
      )
    );
  };

  // Handle data block input change
  const handleBlockInputChange = (e) => {
    setBlockInput({ ...blockInput, [e.target.name]: e.target.value });
  };

  // Add data block
  const addDataBlock = () => {
    const { offset, tagFrom, valueFrom, typeFrom, amount } = blockInput;
    if (
      !tagFrom ||
      !valueFrom ||
      !typeFrom ||
      !amount ||
      parseInt(amount) <= 0
    ) {
      alert("Please enter all From values and a valid amount.");
      return;
    }
    setDataBlocks([
      ...dataBlocks,
      {
        offset: parseInt(offset),
        tagFrom: parseInt(tagFrom),
        valueFrom: parseInt(valueFrom),
        typeFrom: parseInt(typeFrom),
        amount: parseInt(amount),
      },
    ]);
    setBlockInput({
      offset: "",
      tagFrom: "",
      valueFrom: "",
      typeFrom: "",
      amount: "",
    });
  };

  // Remove data block
  const removeDataBlock = (idx) => {
    setDataBlocks(dataBlocks.filter((_, i) => i !== idx));
  };

  // Send data blocks
  const sendDataBlocks = async () => {
    try {
      const payload = { block: dataBlocks };
      const res = await fetch("/save-datablocks", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload),
      });
      const data = await res.json();
      alert(data.status || "Data blocks sent!");
    } catch (err) {
      alert("Send failed: " + err);
    }
  };

  return (
    <div className="tab-content" id="modbusConfig">
      <h2>Modbus Configuration</h2>
      <form
        id="modbusConfigForm"
        onSubmit={(e) => {
          e.preventDefault();
          handleSubmit();
        }}
        autoComplete="off"
      >
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>Role:</label>
          <select
            style={{ width: 200 }}
            value={role}
            onChange={(e) => setRole(e.target.value)}
          >
            <option value="master">Master</option>
            <option value="slave">Slave</option>
          </select>
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>
            Communication (Com):
          </label>
          <select
            style={{ width: 200 }}
            value={com}
            onChange={(e) => setCom(e.target.value)}
          >
            <option value="TCP/IP">TCP/IP</option>
            <option value="RS485">RS485</option>
          </select>
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>
            Modbus Slave ID:
          </label>
          <input
            type="number"
            min={1}
            max={255}
            required
            style={{ width: 200 }}
            value={slaveId}
            onChange={(e) => setSlaveId(e.target.value)}
          />
        </div>
        <div style={{ marginBottom: 10 }}>
          <label style={{ display: "inline-block", width: 150 }}>
            Slave IP:
          </label>
          <input
            type="text"
            placeholder="192,168,1,100"
            required
            style={{ width: 200 }}
            value={slaveIp}
            onChange={(e) => setSlaveIp(e.target.value)}
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
          <i className="fa fa-paper-plane" /> Submit
        </button>
      </form>
      <p id="responseMessage" style={{ marginTop: 20, color: "green" }}>
        {responseMsg}
      </p>

      <div
        style={{
          marginTop: 10,
          display: "flex",
          gap: 10,
          alignItems: "flex-end",
        }}
      >
        <div>
          <label>Create Data Block</label>
          <br />
          <input
            type="number"
            name="offset"
            placeholder="Address offset"
            style={{ width: 70 }}
            value={blockInput.offset}
            onChange={handleBlockInputChange}
          />
          <input
            type="number"
            name="tagFrom"
            placeholder="Tag From"
            style={{ width: 70 }}
            value={blockInput.tagFrom}
            onChange={handleBlockInputChange}
          />
          <input
            type="number"
            name="valueFrom"
            placeholder="Value From"
            style={{ width: 70 }}
            value={blockInput.valueFrom}
            onChange={handleBlockInputChange}
          />
          <input
            type="number"
            name="typeFrom"
            placeholder="Type From"
            style={{ width: 70 }}
            value={blockInput.typeFrom}
            onChange={handleBlockInputChange}
          />
          <input
            type="number"
            name="amount"
            placeholder="Amount"
            style={{ width: 70 }}
            value={blockInput.amount}
            onChange={handleBlockInputChange}
          />
        </div>
        <button type="button" onClick={addDataBlock}>
          Add Data Block
        </button>
        <button type="button" onClick={sendDataBlocks}>
          Send Data Blocks
        </button>
      </div>
      <div
        id="dataBlockCards"
        style={{
          marginTop: 15,
          display: "flex",
          flexWrap: "wrap",
          gap: 10,
        }}
      >
        {dataBlocks.map((block, idx) => (
          <div
            key={idx}
            style={{
              border: "1px solid #aaa",
              borderRadius: 6,
              padding: 10,
              background: "#f9f9f9",
              minWidth: 180,
            }}
          >
            <strong>Block {idx + 1}</strong>
            <br />
            Address offset: {block.offset}
            <br />
            Tag From: {block.tagFrom}
            <br />
            Value From: {block.valueFrom}
            <br />
            Type From: {block.typeFrom}
            <br />
            Amount: {block.amount}
            <br />
            <button
              type="button"
              style={{
                marginTop: 5,
                background: "#f44336",
                color: "white",
                border: "none",
                padding: "2px 8px",
                borderRadius: 3,
              }}
              onClick={() => removeDataBlock(idx)}
            >
              Delete
            </button>
          </div>
        ))}
      </div>

      <button type="button" onClick={addRow} style={{ marginTop: 10 }}>
        Add Row
      </button>

      <div style={{ marginBottom: 10 }}>
        <table id="tagsTable">
          <thead>
            <tr>
              <th>Tag</th>
              <th>Offset</th>
              <th>Value</th>
              <th>Type</th>
              <th>Action</th>
            </tr>
          </thead>
          <tbody>
            {rows.map((row, idx) => (
              <tr key={idx}>
                <td>
                  <input
                    type="number"
                    name="tags[]"
                    placeholder="1101"
                    required
                    value={row.tag}
                    onChange={(e) =>
                      handleRowChange(idx, "tag", e.target.value)
                    }
                  />
                </td>
                <td>
                  <input
                    type="number"
                    name="Offset[]"
                    placeholder="0"
                    required
                    value={row.offset}
                    onChange={(e) =>
                      handleRowChange(idx, "offset", e.target.value)
                    }
                  />
                </td>
                <td>
                  <input
                    type="number"
                    name="values[]"
                    placeholder="164"
                    required
                    value={row.value}
                    onChange={(e) =>
                      handleRowChange(idx, "value", e.target.value)
                    }
                  />
                </td>
                <td>
                  <input
                    type="number"
                    name="types[]"
                    placeholder="2"
                    required
                    value={row.type}
                    onChange={(e) =>
                      handleRowChange(idx, "type", e.target.value)
                    }
                  />
                </td>
                <td>
                  <button
                    type="button"
                    onClick={() => deleteRow(idx)}
                    style={{
                      background: "#f44336",
                      color: "white",
                      border: "none",
                      borderRadius: 3,
                      padding: "2px 8px",
                    }}
                  >
                    Delete
                  </button>
                </td>
              </tr>
            ))}
          </tbody>
        </table>
      </div>
    </div>
  );
};

export default ModbusConfig;