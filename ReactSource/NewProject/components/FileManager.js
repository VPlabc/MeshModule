import React, { useEffect, useRef, useState } from "react";

const FileManager = () => {
  const [files, setFiles] = useState([]);
  const [uploading, setUploading] = useState(false);
  const fileInputRef = useRef();

  // Fetch file list from server
  const fetchFileList = async () => {
    try {
      const res = await fetch("/list-files");
      const data = await res.json();
      setFiles(data);
    } catch (err) {
      alert("Error fetching file list: " + err);
    }
  };

  useEffect(() => {
    fetchFileList();
  }, []);

  // Upload file handler
  const uploadFile = async () => {
    const file = fileInputRef.current.files[0];
    if (!file) {
      alert("Please select a file to upload.");
      return;
    }
    setUploading(true);
    const formData = new FormData();
    formData.append("file", file);
    try {
      const res = await fetch("/upload", {
        method: "POST",
        body: formData,
      });
      if (res.ok) {
        alert("File uploaded successfully.");
        fetchFileList();
        fileInputRef.current.value = "";
      } else {
        alert("Failed to upload file.");
      }
    } catch (err) {
      alert("Error uploading file: " + err);
    }
    setUploading(false);
  };

  // Delete file handler
  const deleteFile = async (fileName) => {
    if (!window.confirm(`Delete file "${fileName}"?`)) return;
    try {
      const res = await fetch(`/delete?file=${encodeURIComponent(fileName)}`, {
        method: "DELETE",
      });
      if (res.ok) {
        alert("File deleted successfully.");
        fetchFileList();
      } else {
        alert("Failed to delete file.");
      }
    } catch (err) {
      alert("Error deleting file: " + err);
    }
  };

  return (
    <div className="tab-content" id="fileManager">
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
            {files.length === 0 ? (
              <tr>
                <td colSpan={2}>No files found.</td>
              </tr>
            ) : (
              files.map((file) => (
                <tr key={file}>
                  <td>{file}</td>
                  <td>
                    <button
                      style={{
                        backgroundColor: "#f44336",
                        color: "white",
                        padding: "5px 10px",
                        border: "none",
                        borderRadius: "3px",
                        cursor: "pointer",
                      }}
                      onClick={() => deleteFile(file)}
                    >
                      Delete
                    </button>
                  </td>
                </tr>
              ))
            )}
          </tbody>
        </table>
      </div>
      <div className="upload-section" style={{ marginTop: 20 }}>
        <h3>Upload File</h3>
        <input type="file" ref={fileInputRef} />
        <button
          onClick={uploadFile}
          style={{
            backgroundColor: "#2196F3",
            color: "white",
            marginLeft: 8,
            padding: "5px 10px",
            border: "none",
            borderRadius: "3px",
            cursor: uploading ? "not-allowed" : "pointer",
          }}
          disabled={uploading}
        >
          {uploading ? "Uploading..." : "Upload"}
        </button>
      </div>
    </div>
  );
};

export default FileManager;