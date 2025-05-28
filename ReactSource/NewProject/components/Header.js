import React, { useState } from "react";

const Header = ({ onResetEsp, socketConnected, user = "Admin" }) => {
  const [dropdownOpen, setDropdownOpen] = useState(false);
  const [showChangePassword, setShowChangePassword] = useState(false);
  const [newPass, setNewPass] = useState("");
  const [confirmPass, setConfirmPass] = useState("");
  const [changePassError, setChangePassError] = useState("");

  // Dummy handlers for demo; replace with actual logic as needed
  const handleResetEsp = () => {
    if (onResetEsp) onResetEsp();
    // else: fetch('/reset-esp', { method: 'POST' })...
  };

  const handleLogout = () => {
    // Implement logout logic here
    window.location.reload();
  };

  const handleChangePassword = () => {
    if (!newPass || !confirmPass) {
      setChangePassError("Please enter both fields.");
      return;
    }
    if (newPass !== confirmPass) {
      setChangePassError("Passwords do not match.");
      return;
    }
    // Send password change command here (e.g., via WebSocket)
    setShowChangePassword(false);
    alert("Password change command sent.");
  };

  return (
    <div className="header" style={{ display: "flex", justifyContent: "space-between", alignItems: "center", backgroundColor: "#00101c", color: "white", padding: "10px 20px", width: "100%" }}>
      {/* Logo and Title */}
      <div style={{ display: "flex", alignItems: "center", gap: 16 }}>
        <svg xmlns="http://www.w3.org/2000/svg" width="110" height="26" viewBox="0 0 341.95 79.68">
          {/* ...SVG content omitted for brevity... */}
        </svg>
        <h1 style={{ margin: 0, fontSize: 24 }}>Wireless IoT Configuration</h1>
      </div>
      {/* Socket Status Icon */}
      <span id="socketStatusIcon" style={{ marginLeft: 8, display: "inline-flex", alignItems: "center" }}>
        <svg id="socketStatusSvg" width="16" height="16" viewBox="0 0 16 16" style={{ verticalAlign: "middle" }}>
          <circle cx="8" cy="8" r="7" fill={socketConnected ? "green" : "red"} stroke="#888" strokeWidth="1" />
        </svg>
      </span>
      {/* Reset ESP Button */}
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
        onClick={handleResetEsp}
        title="Reset ESP"
      >
        <svg xmlns="http://www.w3.org/2000/svg" height="18" width="18" viewBox="0 0 24 24" fill="none" stroke="white" strokeWidth="2" strokeLinecap="round" strokeLinejoin="round"><polyline points="1 4 1 10 7 10"></polyline><path d="M3.51 15a9 9 0 1 0 2.13-9.36L1 10"></path></svg>
      </button>
      {/* User Icon Dropdown */}
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
          onClick={() => setDropdownOpen((v) => !v)}
        >
          <svg xmlns="http://www.w3.org/2000/svg" height="28" width="28" viewBox="0 0 24 24" fill="white"><circle cx="12" cy="8" r="4"/><path d="M12 14c-4.418 0-8 1.79-8 4v2h16v-2c0-2.21-3.582-4-8-4z"/></svg>
          <span style={{ marginLeft: 8, fontSize: 16 }}>{user}</span>
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
              onClick={handleLogout}
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
              />
              <input
                id="confirmPassword"
                type="password"
                placeholder="Confirm New Password"
                style={{ width: "100%", padding: 8, border: "1px solid #ccc", borderRadius: 4 }}
                value={confirmPass}
                onChange={(e) => setConfirmPass(e.target.value)}
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
                onClick={handleChangePassword}
                style={{ padding: "6px 14px", background: "#2196F3", color: "white", border: "none", borderRadius: 4, cursor: "pointer" }}
              >
                Change
              </button>
            </div>
          </div>
        </div>
      )}
    </div>
  );
};

export default Header;