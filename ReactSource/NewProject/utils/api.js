// WebSocket API
export let websocket = null;
export let gateway = `ws://${window.location.hostname}/ws`;

export function initWebSocket(onOpen, onClose, onMessage) {
    websocket = new WebSocket(gateway);
    websocket.onopen = onOpen;
    websocket.onclose = onClose;
    websocket.onmessage = onMessage;
}

export function sendWebSocketCommand(cmd) {
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(JSON.stringify(cmd));
        return true;
    }
    return false;
}

// WiFi & MQTT
export function loadWiFiMQTTConfig() {
    return fetch('/load-wifi-mqtt-config/')
        .then(response => response.json());
}

export function submitWiFiMQTTConfig(config) {
    return fetch('/save-wifi-mqtt-config/', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
    }).then(response => response.json());
}

// Ethernet
export function loadEthernetConfig() {
    return fetch('/load-ethernet-config')
        .then(response => response.json());
}

export function saveEthernetConfig(config) {
    return fetch('/save-ethernet-config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
    }).then(response => response.json());
}

// Mesh Network
export function loadMeshConfig() {
    return fetch('/load-mesh-config')
        .then(response => response.json());
}

export function submitMeshConfig(config) {
    return fetch('/save-mesh-config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
    }).then(response => response.json());
}

// Modbus
export function loadModbusConfig() {
    return fetch('/load-modbus-config')
        .then(response => response.json());
}

export function submitModbusConfig(config) {
    return fetch('/modbus-config', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
    }).then(response => response.json());
}

// Mapping Data
export function loadDataMapping() {
    return fetch('/data-mapping')
        .then(response => response.json());
}

export function saveDataMapping(data) {
    return fetch('/save-data-mapping', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(data)
    }).then(response => response.json());
}

// Data Viewer
export function saveDataViewer(data) {
    return fetch('/save-data-viewer', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify({ data })
    }).then(response => response.json());
}

export function loadDataViewer() {
    return fetch('/load-data-viewer')
        .then(response => response.json());
}

// File Manager
export function fetchFileList() {
    return fetch('/list-files')
        .then(response => response.json());
}

export function uploadFile(file) {
    const formData = new FormData();
    formData.append('file', file);
    return fetch('/upload', {
        method: 'POST',
        body: formData
    });
}

export function deleteFile(fileName) {
    return fetch(`/delete?file=${encodeURIComponent(fileName)}`, {
        method: 'DELETE'
    });
}

// Data Blocks
export function fetchDataBlocks() {
    return fetch('/datablocks').then(r => r.json());
}

export function saveDataBlocks(payload) {
    return fetch('/save-datablocks', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(payload)
    }).then(res => res.json());
}

// LoRa RY
export function loadLoRaRYConfig() {
    return fetch('/load-lora-config')
        .then(response => response.json());
}

export function submitLoRaRYConfig(config) {
    return fetch('/configure-lora', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
    }).then(response => response.json());
}

// LoRa E32
export function submitLoRaConfig(config) {
    return fetch('/configure-e32', {
        method: 'POST',
        headers: { 'Content-Type': 'application/json' },
        body: JSON.stringify(config)
    }).then(response => response.json());
}

// Reset ESP
export function resetEsp() {
    return fetch('/reset-esp', { method: 'POST' })
        .then(res => res.json());
}