import React, { useEffect, useRef, useState } from "react";

// You should move CSS to App.css or use styled-components in real project
const styles = `
body {
    font-family: Arial, sans-serif;
    margin: 20px;
}
.header {
    display: flex;
    justify-content: space-between;
    align-items: center;
    background-color: #00101c;
    color: white;
    padding: 10px 20px;
    width: 100%;
}
.header h1 {
    margin: 0;
}
.tabs {
    display: flex;
    margin-top: 20px;
    border-bottom: 2px solid #ddd;
}
.tab {
    padding: 10px 20px;
    cursor: pointer;
    border: 1px solid #ddd;
    border-bottom: none;
    background-color: #f9f9f9;
}
.tab.active {
    background-color: white;
    font-weight: bold;
    border-top: 4px solid #00101c;
}
.tab-content {
    display: none;
    margin-top: 20px;
}
.tab-content.active {
    display: block;
}
table {
    width: 100%;
    border-collapse: collapse;
    margin-top: 20px;
}
table, th, td {
    border: 1px solid #ddd;
}
th, td {
    padding: 10px;
    text-align: left;
}
th {
    background-color: #f4f4f4;
}
button {
    padding: 5px 10px;
    background-color: #f44336;
    color: white;
    border: none;
    cursor: pointer;
}
button:hover {
    background-color: #d32f2f;
}
`;

function App() {
    // Tabs
    const [activeTab, setActiveTab] = useState("dashboard");

    // WebSocket
    const [socketConnected, setSocketConnected] = useState(false);
    const wsRef = useRef(null);

    // Dropdown
    const [dropdownOpen, setDropdownOpen] = useState(false);

    // Modal
    const [showChangePassword, setShowChangePassword] = useState(false);
    const [newPass, setNewPass] = useState("");
    const [confirmPass, setConfirmPass] = useState("");
    const [changePassError, setChangePassError] = useState("");

    // Data Viewer
    const [dataViewer, setDataViewer] = useState("");
    const [mappingData, setMappingData] = useState("");
    const [mappingDatas, setMappingDatas] = useState("");
    const [responseMessageMapping, setResponseMessageMapping] = useState("");
    const [responseMessageViewer, setResponseMessageViewer] = useState("");

    // File Manager
    const [fileList, setFileList] = useState([]);
    const fileInputRef = useRef();

    // Mesh Config
    const [meshConfig, setMeshConfig] = useState({});
    // ...other configs as needed

    // Socket status icon
    const socketStatusRef = useRef();

    // --- WebSocket logic ---
    useEffect(() => {
        let ws;
        function connect() {
            ws = new window.WebSocket(`ws://${window.location.hostname}/ws`);
            wsRef.current = ws;
            ws.onopen = () => {
                setSocketConnected(true);
                if (socketStatusRef.current) {
                    socketStatusRef.current.setAttribute("fill", "green");
                }
                ws.send(JSON.stringify({ type: "subscribe", topic: "nodeDashboard" }));
            };
            ws.onclose = () => {
                setSocketConnected(false);
                if (socketStatusRef.current) {
                    socketStatusRef.current.setAttribute("fill", "red");
                }
                setTimeout(connect, 2000);
            };
            ws.onmessage = (event) => {
                // handle dashboard data here if needed
            };
        }
        connect();
        return () => ws && ws.close();
    }, []);

    // --- Dropdown logic ---
    useEffect(() => {
        function handleClick(e) {
            setDropdownOpen(false);
        }
        if (dropdownOpen) {
            document.addEventListener("click", handleClick);
        }
        return () => document.removeEventListener("click", handleClick);
    }, [dropdownOpen]);

    // --- File List ---
    useEffect(() => {
        fetchFileList();
    }, []);

    function fetchFileList() {
        fetch("/list-files")
            .then((res) => res.json())
            .then(setFileList)
            .catch(() => setFileList([]));
    }

    // --- File Upload ---
    function uploadFile() {
        const file = fileInputRef.current.files[0];
        if (!file) {
            alert("Please select a file to upload.");
            return;
        }
        const formData = new FormData();
        formData.append("file", file);
        fetch("/upload", { method: "POST", body: formData })
            .then((res) => {
                if (res.ok) {
                    alert("File uploaded successfully.");
                    fetchFileList();
                } else {
                    alert("Failed to upload file.");
                }
            });
    }

    function deleteFile(fileName) {
        fetch(`/delete?file=${encodeURIComponent(fileName)}`, { method: "DELETE" })
            .then((res) => {
                if (res.ok) {
                    alert("File deleted successfully.");
                    fetchFileList();
                } else {
                    alert("Failed to delete file.");
                }
            });
    }

    // --- Data Viewer ---
    function saveDataViewer() {
        fetch("/save-data-viewer", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ data: dataViewer }),
        })
            .then((res) => res.json())
            .then((res) => setResponseMessageViewer(res.status || "Data saved!"))
            .catch((err) => setResponseMessageViewer("Save failed: " + err));
    }
    function loadDataViewer() {
        fetch("/load-data-viewer")
            .then((res) => res.json())
            .then((res) => setDataViewer(res.data || ""))
            .catch((err) => setResponseMessageViewer("Load failed: " + err));
    }

    // --- Mapping Data ---
    function saveDataMapping() {
        try {
            const parsedData = JSON.parse(mappingDatas);
            fetch("/save-data-mapping", {
                method: "POST",
                headers: { "Content-Type": "application/json" },
                body: JSON.stringify(parsedData),
            })
                .then((res) => res.json())
                .then(() => setResponseMessageMapping("Data mapping saved successfully!"))
                .catch((err) => setResponseMessageMapping("Error: " + err.message));
        } catch (error) {
            alert("Invalid JSON format. Please correct it and try again.");
        }
    }
    function loadDataMapping() {
        fetch("/data-mapping")
            .then((res) => res.json())
            .then((data) => setMappingDatas(JSON.stringify(data, null, 2)));
    }

    // --- Change Password Modal ---
    function submitChangePassword() {
        if (!newPass || !confirmPass) {
            setChangePassError("Please enter both fields.");
            return;
        }
        if (newPass !== confirmPass) {
            setChangePassError("Passwords do not match.");
            return;
        }
        if (wsRef.current && wsRef.current.readyState === 1) {
            wsRef.current.send(JSON.stringify({ cmnd: "changePassword", data: newPass }));
            setShowChangePassword(false);
            alert("Password change command sent.");
        } else {
            setChangePassError("WebSocket not connected.");
        }
    }

    // --- Logout ---
    function logoutUser() {
        if (wsRef.current && wsRef.current.readyState === 1) {
            wsRef.current.send(JSON.stringify({ cmnd: "logout" }));
        }
        setTimeout(() => window.location.reload(), 500);
    }

    // --- Reset ESP ---
    function resetEsp() {
        fetch("/reset-esp", { method: "POST" })
            .then((res) => res.json())
            .then((data) => alert(data.status || "Reset command sent!"))
            .catch((err) => alert("Reset failed: " + err));
    }

    // --- Tab content renderers ---
    function renderTabContent() {
        switch (activeTab) {
            case "dashboard":
                return (
                    <div className="tab-content active">
                        <div style={{ display: "flex", gap: 20 }}>
                            <div style={{ flex: 2, border: "1px solid #ddd", padding: 20, borderRadius: 8, boxShadow: "0 2px 4px rgba(0,0,0,0.1)", height: 750 }}>
                                <div id="nodeDashboard" style={{ display: "flex", flexWrap: "wrap", gap: 20 }}></div>
                            </div>
                        </div>
                    </div>
                );
            case "fileManager":
                return (
                    <div className="tab-content active">
                        <h2>File Manager</h2>
                        <div className="file-list">
                            <h3>Uploaded Files</h3>
                            <table>
                                <thead>
                                    <tr>
                                        <th>File Name</th>
                                        <th>Action</th>
                                    </tr>
                                </thead>
                                <tbody>
                                    {fileList.map((file) => (
                                        <tr key={file}>
                                            <td>{file}</td>
                                            <td>
                                                <button onClick={() => deleteFile(file)}>Delete</button>
                                            </td>
                                        </tr>
                                    ))}
                                </tbody>
                            </table>
                        </div>
                        <div className="upload-section">
                            <h3>Upload File</h3>
                            <input type="file" ref={fileInputRef} />
                            <button onClick={uploadFile}>Upload</button>
                        </div>
                    </div>
                );
            case "dataViewer":
                return (
                    <div className="tab-content active">
                        <h2>Data Viewer</h2>
                        <textarea
                            style={{ width: "100%", height: 200, resize: "vertical" }}
                            placeholder="Enter your data here..."
                            value={dataViewer}
                            onChange={(e) => setDataViewer(e.target.value)}
                        />
                        <div style={{ marginTop: 10 }}>
                            <button onClick={saveDataViewer}>Save</button>
                            <button onClick={loadDataViewer}>Load</button>
                            <p style={{ marginTop: 20, color: "green" }}>{responseMessageViewer}</p>
                        </div>
                        <div>
                            <label style={{ display: "block", marginBottom: 10 }}>JSON Mapping Data:</label>
                            <textarea
                                value={mappingData}
                                onChange={(e) => setMappingData(e.target.value)}
                                placeholder="Enter JSON data here"
                                style={{ width: "100%", height: 150, resize: "vertical" }}
                            />
                            <textarea
                                value={mappingDatas}
                                onChange={(e) => setMappingDatas(e.target.value)}
                                placeholder="Enter JSON data here"
                                style={{ width: "100%", height: 150, resize: "vertical" }}
                            />
                            <button onClick={saveDataMapping}>Save Mapping Data</button>
                            <button onClick={loadDataMapping}>Load Mapping Data</button>
                            <p style={{ marginTop: 20, color: "green" }}>{responseMessageMapping}</p>
                        </div>
                    </div>
                );
            // Add other tab contents here...
            default:
                return <div className="tab-content active">Not implemented yet.</div>;
        }
    }

    // --- Main render ---
    return (
        <div>
            <style>{styles}</style>
            <div className="header">
                {/* SVG logo and title */}
                <svg xmlns="http://www.w3.org/2000/svg" width="110" height="26" viewBox="0 0 341.95 79.68">
                    {/* ...SVG content omitted for brevity... */}
                </svg>
                <h1>Wireless IoT Configuration</h1>
                <div style={{ position: "relative", display: "flex" }}>
                    <span id="socketStatusIcon" style={{ marginLeft: 8, display: "inline-flex", alignItems: "center" }}>
                        <svg id="socketStatusSvg" width="16" height="16" viewBox="0 0 16 16" style={{ verticalAlign: "middle" }}>
                            <circle ref={socketStatusRef} cx="8" cy="8" r="7" fill={socketConnected ? "green" : "red"} stroke="#888" strokeWidth="1" />
                        </svg>
                    </span>
                </div>
                <div style={{ position: "relative", display: "flex" }}>
                    <button
                        id="resetEspBtn"
                        style={{
                            backgroundColor: "#ff9800",
                            color: "white",
                            padding: "6px 14px",
                            border: "none",
                            borderRadius: 4,
                            cursor: "pointer",
                            display: "flex",
                            alignItems: "center",
                            gap: 6,
                            marginBottom: 10,
                        }}
                        onClick={resetEsp}
                        title="Reset ESP"
                    >
                        <svg xmlns="http://www.w3.org/2000/svg" height="18" width="18" viewBox="0 0 24 24" fill="none" stroke="white" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><polyline points="1 4 1 10 7 10"></polyline><path d="M3.51 15a9 9 0 1 0 2.13-9.36L1 10"></path></svg>
                    </button>
                </div>
                {/* User Dropdown */}
                <div style={{ position: "relative" }}>
                    <button
                        id="userDropdownBtn"
                        style={{
                            background: "none",
                            border: "none",
                            cursor: "pointer",
                            color: "white",
                            fontSize: 22,
                            display: "flex",
                            alignItems: "center",
                        }}
                        onClick={(e) => {
                            e.stopPropagation();
                            setDropdownOpen((v) => !v);
                        }}
                    >
                        <svg xmlns="http://www.w3.org/2000/svg" height="28" width="28" viewBox="0 0 24 24" fill="white"><circle cx="12" cy="8" r="4"/><path d="M12 14c-4.418 0-8 1.79-8 4v2h16v-2c0-2.21-3.582-4-8-4z"/></svg>
                        <span style={{ marginLeft: 8, fontSize: 16 }}>Admin</span>
                        <svg style={{ marginLeft: 4 }} width="16" height="16" fill="white" viewBox="0 0 16 16"><path d="M4.646 6.646a.5.5 0 0 1 .708 0L8 9.293l2.646-2.647a.5.5 0 0 1 .708.708l-3 3a.5.5 0 0 1-.708 0l-3-3a.5.5 0 0 1 0-.708z"/></svg>
                    </button>
                    {dropdownOpen && (
                        <div
                            id="userDropdownMenu"
                            style={{
                                position: "absolute",
                                right: 0,
                                top: 40,
                                background: "#646464",
                                color: "#222",
                                minWidth: 160,
                                boxShadow: "0 2px 8px rgba(0,0,0,0.15)",
                                borderRadius: 6,
                                zIndex: 1000,
                            }}
                        >
                            <button
                                onClick={() => {
                                    setShowChangePassword(true);
                                    setDropdownOpen(false);
                                }}
                                style={{
                                    width: "100%",
                                    padding: "10px 16px",
                                    background: "none",
                                    border: "none",
                                    textAlign: "left",
                                    cursor: "pointer",
                                    fontSize: 15,
                                }}
                            >
                                Change Password
                            </button>
                            <button
                                onClick={logoutUser}
                                style={{
                                    width: "100%",
                                    padding: "10px 16px",
                                    background: "none",
                                    border: "none",
                                    textAlign: "left",
                                    cursor: "pointer",
                                    fontSize: 15,
                                }}
                            >
                                Logout
                            </button>
                        </div>
                    )}
                </div>
                {/* Change Password Modal */}
                {showChangePassword && (
                    <div
                        id="changePasswordModal"
                        style={{
                            display: "flex",
                            position: "fixed",
                            top: 0,
                            left: 0,
                            width: "100vw",
                            height: "100vh",
                            background: "rgba(0,0,0,0.3)",
                            zIndex: 2000,
                            alignItems: "center",
                            justifyContent: "center",
                        }}
                    >
                        <div
                            style={{
                                background: "#cdcdcd",
                                padding: "28px 24px 18px 24px",
                                borderRadius: 8,
                                minWidth: 320,
                                maxWidth: "90vw",
                                boxShadow: "0 4px 16px rgba(0,0,0,0.18)",
                                position: "relative",
                            }}
                        >
                            <h3 style={{ marginTop: 0 }}>Change Password</h3>
                            <div style={{ marginBottom: 12 }}>
                                <input
                                    id="newPassword"
                                    type="password"
                                    placeholder="New Password"
                                    style={{ width: "100%", padding: 8, marginBottom: 8, border: "1px solid #ccc", borderRadius: 4 }}
                                    value={newPass}
                                    onChange={(e) => setNewPass(e.target.value)}
                                    onKeyDown={(e) => e.key === "Enter" && submitChangePassword()}
                                />
                                <input
                                    id="confirmPassword"
                                    type="password"
                                    placeholder="Confirm New Password"
                                    style={{ width: "100%", padding: 8, border: "1px solid #ccc", borderRadius: 4 }}
                                    value={confirmPass}
                                    onChange={(e) => setConfirmPass(e.target.value)}
                                    onKeyDown={(e) => e.key === "Enter" && submitChangePassword()}
                                />
                            </div>
                            <div id="changePasswordError" style={{ color: "red", fontSize: 13, minHeight: 18 }}>
                                {changePassError}
                            </div>
                            <div style={{ marginTop: 10, display: "flex", justifyContent: "flex-end", gap: 8 }}>
                                <button
                                    onClick={() => setShowChangePassword(false)}
                                    style={{ padding: "6px 14px", background: "#ff9f29", border: "none", borderRadius: 4, cursor: "pointer" }}
                                >
                                    Cancel
                                </button>
                                <button
                                    onClick={submitChangePassword}
                                    style={{ padding: "6px 14px", background: "#2196F3", color: "white", border: "none", borderRadius: 4, cursor: "pointer" }}
                                >
                                    Change
                                </button>
                            </div>
                        </div>
                    </div>
                )}
            </div>
            {/* Tabs */}
            <div className="tabs">
                <div className={`tab${activeTab === "dashboard" ? " active" : ""}`} onClick={() => setActiveTab("dashboard")}>Dashboard</div>
                <div className={`tab${activeTab === "meshConfig" ? " active" : ""}`} onClick={() => setActiveTab("meshConfig")}>Mesh Network</div>
                <div className={`tab${activeTab === "modbusConfig" ? " active" : ""}`} onClick={() => setActiveTab("modbusConfig")}>Modbus Configuration</div>
                <div className={`tab${activeTab === "wifiConfig" ? " active" : ""}`} onClick={() => setActiveTab("wifiConfig")}>WiFi Configuration</div>
                <div className={`tab${activeTab === "mappingData" ? " active" : ""}`} onClick={() => setActiveTab("mappingData")}>Mapping Data</div>
                {/* <div className={`tab${activeTab === "loraConfig" ? " active" : ""}`} onClick={() => setActiveTab("loraConfig")}>LoRa E32 Configuration</div>
                <div className={`tab${activeTab === "loraRYConfig" ? " active" : ""}`} onClick={() => setActiveTab("loraRYConfig")}>LoRa RY Configuration</div> */}
                <div className={`tab${activeTab === "fileManager" ? " active" : ""}`} onClick={() => setActiveTab("fileManager")}>File Manager</div>
                <div className={`tab${activeTab === "dataViewer" ? " active" : ""}`} onClick={() => setActiveTab("dataViewer")}>Data Viewer</div>
            </div>
            {/* Tab Content */}
            {renderTabContent()}
        </div>
    );
}

export default App;