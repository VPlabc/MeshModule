import React, { useEffect, useRef, useState } from "react";

const typeOptions = [
  { value: 0, label: "Coil", min: 0, max: 1 },
  { value: 1, label: "Word", min: 0, max: 65535 },
  { value: 2, label: "DWord", min: 0, max: 4294967295 },
];

const MappingData = ({ nodeData = [], onSendEdit }) => {
  const [editType, setEditType] = useState(0);
  const [editValue, setEditValue] = useState("");
  const [editAddress, setEditAddress] = useState("");
  const [addressOptions, setAddressOptions] = useState([]);
  const [tables, setTables] = useState([]);
  const nodeDataTablesRef = useRef();

  // Update address options when nodeData changes
  useEffect(() => {
    if (nodeData && nodeData.length > 0) {
      // Assume nodeData is an array of objects with index or address
      const options = [];
      nodeData.forEach((node) => {
        if (node.data && Array.isArray(node.data)) {
          node.data.forEach((_, idx) => {
            options.push({ value: idx, label: idx });
          });
        }
      });
      setAddressOptions(options);
      if (options.length > 0) setEditAddress(options[0].value);
    }
  }, [nodeData]);

  // Render node data tables
  useEffect(() => {
    if (!nodeDataTablesRef.current) return;
    // Example: nodeData = [{ nodeId, data: [...], keys: [...], type: [...] }]
    setTables(
      nodeData.map((node) => (
        <div key={node.nodeId} style={{ marginBottom: 20 }}>
          <h3>Node {node.nodeId} Data</h3>
          <table style={{ width: "100%", borderCollapse: "collapse" }}>
            <thead>
              <tr>
                <th>Index</th>
                <th>Key</th>
                <th>Type</th>
                <th>Data</th>
              </tr>
            </thead>
            <tbody>
              {node.data &&
                node.data.map((val, idx) => (
                  <tr key={idx}>
                    <td>{idx}</td>
                    <td>{node.keys && node.keys[idx] ? node.keys[idx] : "-"}</td>
                    <td>{node.type && node.type[idx] ? node.type[idx] : "-"}</td>
                    <td>{val}</td>
                  </tr>
                ))}
            </tbody>
          </table>
        </div>
      ))
    );
  }, [nodeData]);

  // Handle type change
  const handleTypeChange = (e) => {
    const val = parseInt(e.target.value, 10);
    setEditType(val);
    setEditValue("");
  };

  // Handle send edit command
  const handleSend = () => {
    if (editValue === "" || editAddress === "") {
      alert("Please enter value and select address.");
      return;
    }
    if (onSendEdit) {
      onSendEdit({
        type: editType,
        value: editValue,
        address: editAddress,
      });
    }
  };

  const typeObj = typeOptions.find((t) => t.value === Number(editType));

  return (
    <div className="tab-content" id="mappingData">
      <h2>Mapping Data</h2>
      <div style={{ marginBottom: 20 }}>
        <select
          id="edit_type"
          style={{ width: 80 }}
          title="Select type"
          value={editType}
          onChange={handleTypeChange}
        >
          {typeOptions.map((t) => (
            <option key={t.value} value={t.value}>
              {t.label}
            </option>
          ))}
        </select>
        <input
          type="number"
          id="edit_value"
          placeholder="value"
          style={{ width: 100 }}
          min={typeObj?.min}
          max={typeObj?.max}
          value={editValue}
          onChange={(e) => setEditValue(e.target.value)}
        />
        <select
          id="edit_address"
          style={{ width: 90 }}
          value={editAddress}
          onChange={(e) => setEditAddress(e.target.value)}
        >
          {addressOptions.map((opt) => (
            <option key={opt.value} value={opt.value}>
              {opt.label}
            </option>
          ))}
        </select>
        <button type="button" onClick={handleSend}>
          Send
        </button>
      </div>
      <div style={{ marginBottom: 20 }}>
        <div id="nodeDataTables" ref={nodeDataTablesRef}>
          {tables}
        </div>
      </div>
    </div>
  );
};

export default MappingData;