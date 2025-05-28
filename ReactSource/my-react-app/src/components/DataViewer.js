import React, { useState } from "react";

const DataViewer = () => {
  const [dataViewerText, setDataViewerText] = useState("");
  const [jsonMappingData, setJsonMappingData] = useState("");
  const [jsonMappingDatas, setJsonMappingDatas] = useState("");
  const [responseViewer, setResponseViewer] = useState("");
  const [responseMapping, setResponseMapping] = useState("");
  const [responseMappings, setResponseMappings] = useState("");

  // Save Data Viewer
  const saveDataViewer = async () => {
    try {
      const res = await fetch("/save-data-viewer", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify({ data: dataViewerText }),
      });
      const result = await res.json();
      setResponseViewer(result.status || "Data saved!");
    } catch (err) {
      setResponseViewer("Save failed: " + err);
    }
  };

  // Load Data Viewer
  const loadDataViewer = async () => {
    try {
      const res = await fetch("/load-data-viewer");
      const result = await res.json();
      setDataViewerText(result.data || "");
    } catch (err) {
      setResponseViewer("Load failed: " + err);
    }
  };

  // Save Mapping Data
  const saveDataMapping = async () => {
    try {
      const parsed = JSON.parse(jsonMappingDatas);
      const res = await fetch("/save-data-mapping", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(parsed),
      });
      const result = await res.json();
      setResponseMapping("Data mapping saved successfully!");
    } catch (err) {
      setResponseMapping("Error: " + err.message);
    }
  };

  // Load Mapping Data
  const loadDataMapping = async () => {
    try {
      const res = await fetch("/data-mapping");
      const data = await res.json();
      setJsonMappingDatas(JSON.stringify(data, null, 2));
    } catch (err) {
      setResponseMapping("Error loading data mapping: " + err);
    }
  };

  // Submit Mapping Data
  const submitMappingData = async () => {
    try {
      const parsed = JSON.parse(jsonMappingData);
      // You can POST to your API here if needed
      // await fetch('/submit-mapping-data', { ... });
      setResponseMappings("Mapping data submitted successfully!");
    } catch (err) {
      setResponseMappings("Invalid JSON format. Please correct it and try again.");
    }
  };

  return (
    <div className="tab-content" id="dataViewer">
      <h2>Data Viewer</h2>
      <textarea
        style={{ width: "100%", height: 200, resize: "vertical" }}
        placeholder="Enter your data here..."
        value={dataViewerText}
        onChange={e => setDataViewerText(e.target.value)}
      />
      <div style={{ marginTop: 10 }}>
        <button type="button" onClick={saveDataViewer}>Save</button>
        <button type="button" onClick={loadDataViewer}>Load</button>
        <p style={{ marginTop: 20, color: "green" }}>{responseViewer}</p>
      </div>
      <div>
        <label style={{ display: "block", marginBottom: 10 }}>JSON Mapping Data:</label>
        <textarea
          name="jsonMappingData"
          placeholder="Enter JSON data here"
          style={{ width: "100%", height: 150, resize: "vertical" }}
          value={jsonMappingData}
          onChange={e => setJsonMappingData(e.target.value)}
        />
        <textarea
          name="jsonMappingDatas"
          placeholder="Enter JSON data here"
          style={{ width: "100%", height: 150, resize: "vertical" }}
          value={jsonMappingDatas}
          onChange={e => setJsonMappingDatas(e.target.value)}
        />
        <button type="button" onClick={saveDataMapping}>Save Mapping Data</button>
        <button type="button" onClick={loadDataMapping}>load Mapping Data</button>
        <p style={{ marginTop: 20, color: "green" }}>{responseMapping}</p>
        <button type="button" onClick={submitMappingData}>Submit Mapping Data</button>
        <p style={{ marginTop: 20, color: "green" }}>{responseMappings}</p>
      </div>
    </div>
  );
};

export default DataViewer;