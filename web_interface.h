#ifndef WEB_INTERFACE_H
#define WEB_INTERFACE_H

#include <Arduino.h>

static const char INDEX_HTML[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>ESP32-CAM Timelapse Studio</title>
    <style>
        :root {
            --bg-color: #0b0f19;
            --card-bg: rgba(23, 31, 51, 0.75);
            --card-border: rgba(255, 255, 255, 0.08);
            --primary: #3b82f6;
            --primary-hover: #2563eb;
            --accent: #10b981;
            --danger: #ef4444;
            --text-main: #f3f4f6;
            --text-sub: #9ca3af;
        }

        * {
            box-sizing: border-box;
            margin: 0;
            padding: 0;
            font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, Helvetica, Arial, sans-serif;
        }

        body {
            background-color: var(--bg-color);
            color: var(--text-main);
            min-height: 100vh;
            padding: 1.5rem;
            background-image: 
                radial-gradient(at 0% 0%, rgba(59, 130, 246, 0.15) 0px, transparent 50%),
                radial-gradient(at 100% 100%, rgba(16, 185, 129, 0.1) 0px, transparent 50%);
        }

        .header {
            display: flex;
            justify-content: space-between;
            align-items: center;
            max-width: 1200px;
            margin: 0 auto 1.5rem auto;
            padding-bottom: 1rem;
            border-bottom: 1px solid var(--card-border);
        }

        .header h1 {
            font-size: 1.6rem;
            font-weight: 700;
            background: linear-gradient(135deg, #60a5fa, #34d399);
            -webkit-background-clip: text;
            -webkit-text-fill-color: transparent;
        }

        .badge {
            background: rgba(59, 130, 246, 0.2);
            color: #60a5fa;
            padding: 0.25rem 0.75rem;
            border-radius: 9999px;
            font-size: 0.85rem;
            border: 1px solid rgba(59, 130, 246, 0.3);
        }

        .grid {
            display: grid;
            grid-template-columns: 1fr;
            gap: 1.5rem;
            max-width: 1200px;
            margin: 0 auto;
        }

        @media (min-width: 900px) {
            .grid {
                grid-template-columns: 1.2fr 0.8fr;
            }
        }

        .card {
            background: var(--card-bg);
            backdrop-filter: blur(12px);
            border: 1px solid var(--card-border);
            border-radius: 16px;
            padding: 1.5rem;
            box-shadow: 0 8px 32px 0 rgba(0, 0, 0, 0.37);
        }

        .card-title {
            font-size: 1.1rem;
            font-weight: 600;
            margin-bottom: 1rem;
            color: var(--text-main);
            display: flex;
            align-items: center;
            gap: 0.5rem;
        }

        .stream-viewport {
            width: 100%;
            height: 380px;
            background: #000;
            border-radius: 12px;
            overflow: hidden;
            display: flex;
            align-items: center;
            justify-content: center;
            position: relative;
            border: 1px solid var(--card-border);
        }

        .stream-viewport img {
            width: 100%;
            height: 100%;
            object-fit: contain;
        }

        .controls-grid {
            display: grid;
            grid-template-columns: repeat(auto-fit, minmax(140px, 1fr));
            gap: 0.75rem;
            margin-top: 1rem;
        }

        .btn {
            background: var(--primary);
            color: white;
            border: none;
            padding: 0.75rem 1rem;
            border-radius: 8px;
            font-weight: 600;
            cursor: pointer;
            transition: all 0.2s ease;
            display: flex;
            align-items: center;
            justify-content: center;
            gap: 0.5rem;
            font-size: 0.9rem;
        }

        .btn:hover {
            background: var(--primary-hover);
            transform: translateY(-1px);
        }

        .btn-success { background: var(--accent); }
        .btn-success:hover { background: #059669; }

        .btn-danger { background: var(--danger); }
        .btn-danger:hover { background: #dc2626; }

        .form-group {
            margin-bottom: 1rem;
        }

        .form-group label {
            display: block;
            font-size: 0.85rem;
            color: var(--text-sub);
            margin-bottom: 0.4rem;
        }

        select, input {
            width: 100%;
            background: rgba(15, 23, 42, 0.8);
            border: 1px solid var(--card-border);
            color: var(--text-main);
            padding: 0.6rem 0.8rem;
            border-radius: 8px;
            outline: none;
            font-size: 0.9rem;
        }

        select:focus, input:focus {
            border-color: var(--primary);
        }

        .status-table {
            width: 100%;
            border-collapse: collapse;
            font-size: 0.9rem;
        }

        .status-table td {
            padding: 0.5rem 0;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
        }

        .status-table td:last-child {
            text-align: right;
            font-weight: 600;
            color: #60a5fa;
        }

        .file-list {
            max-height: 250px;
            overflow-y: auto;
            margin-top: 1rem;
            border: 1px solid var(--card-border);
            border-radius: 8px;
        }

        .file-item {
            display: flex;
            justify-content: space-between;
            align-items: center;
            padding: 0.6rem 0.8rem;
            border-bottom: 1px solid rgba(255, 255, 255, 0.05);
            font-size: 0.85rem;
        }

        .file-item:last-child { border-bottom: none; }

        .file-actions {
            display: flex;
            gap: 0.4rem;
        }

        .btn-sm {
            padding: 0.25rem 0.5rem;
            font-size: 0.75rem;
            border-radius: 4px;
        }
    </style>
</head>
<body>
    <div class="header">
        <h1>📷 ESP32-CAM Timelapse Studio</h1>
        <span class="badge" id="statusBadge">Status: Ready</span>
    </div>

    <div class="grid">
        <!-- Left Column: Camera Preview -->
        <div class="card">
            <div class="card-title">Live Preview & Snapshot</div>
            <div class="stream-viewport">
                <img id="cameraFrame" src="/capture" alt="Camera Feed">
            </div>
            <div class="controls-grid">
                <button class="btn" onclick="refreshFrame()">Refresh Snapshot</button>
                <button class="btn btn-success" id="toggleTimelapseBtn" onclick="toggleTimelapse()">Start Timelapse</button>
                <button class="btn btn-danger" onclick="captureNow()">Capture Single</button>
            </div>
        </div>

        <!-- Right Column: Controls & System Info -->
        <div class="card">
            <div class="card-title">Timelapse Settings</div>
            
            <div class="form-group">
                <label for="intervalSelect">Capture Interval</label>
                <select id="intervalSelect" onchange="updateInterval()">
                    <option value="2">2 Seconds</option>
                    <option value="5" selected>5 Seconds</option>
                    <option value="10">10 Seconds</option>
                    <option value="30">30 Seconds</option>
                    <option value="60">1 Minute</option>
                    <option value="300">5 Minutes</option>
                    <option value="1800">30 Minutes</option>
                    <option value="3600">1 Hour</option>
                </select>
            </div>

            <div class="form-group">
                <label for="frameSizeSelect">Camera Resolution</label>
                <select id="frameSizeSelect" onchange="updateResolution()">
                    <option value="10">UXGA (1600x1200)</option>
                    <option value="9">SXGA (1280x1024)</option>
                    <option value="8">XGA (1024x768)</option>
                    <option value="7">SVGA (800x600)</option>
                    <option value="6" selected>VGA (640x480)</option>
                    <option value="5">CIF (400x296)</option>
                </select>
            </div>

            <div class="card-title" style="margin-top: 1.5rem;">System Information</div>
            <table class="status-table">
                <tr><td>Timelapse Active</td><td id="infoActive">No</td></tr>
                <tr><td>Capture Interval</td><td id="infoInterval">5s</td></tr>
                <tr><td>Photos Captured</td><td id="infoCount">0</td></tr>
                <tr><td>SD Space Free</td><td id="infoSDFree">Checking...</td></tr>
                <tr><td>Wi-Fi Signal (RSSI)</td><td id="infoRSSI">- dBm</td></tr>
            </table>

            <div class="card-title" style="margin-top: 1.5rem;">SD Card Files</div>
            <button class="btn btn-sm" style="width: 100%" onclick="loadFileList()">Refresh Files</button>
            <div class="file-list" id="fileList">
                <div class="file-item"><span>No files loaded</span></div>
            </div>
        </div>
    </div>

    <script>
        let isTimelapseRunning = false;

        function refreshFrame() {
            const img = document.getElementById('cameraFrame');
            img.src = '/capture?t=' + new Date().getTime();
        }

        function captureNow() {
            fetch('/snap').then(r => r.text()).then(msg => {
                alert(msg);
                refreshFrame();
                updateStatus();
            });
        }

        function toggleTimelapse() {
            const btn = document.getElementById('toggleTimelapseBtn');
            const endpoint = isTimelapseRunning ? '/stop' : '/start';
            fetch(endpoint)
                .then(r => r.json())
                .then(data => {
                    isTimelapseRunning = data.running;
                    btn.textContent = isTimelapseRunning ? 'Stop Timelapse' : 'Start Timelapse';
                    btn.className = isTimelapseRunning ? 'btn btn-danger' : 'btn btn-success';
                    document.getElementById('statusBadge').textContent = isTimelapseRunning ? 'Status: Recording' : 'Status: Ready';
                    updateStatus();
                });
        }

        function updateInterval() {
            const sec = document.getElementById('intervalSelect').value;
            fetch('/set_interval?val=' + sec);
        }

        function updateResolution() {
            const val = document.getElementById('frameSizeSelect').value;
            fetch('/set_framesize?val=' + val);
        }

        function updateStatus() {
            fetch('/status')
                .then(r => r.json())
                .then(data => {
                    isTimelapseRunning = data.running;
                    document.getElementById('infoActive').textContent = data.running ? 'Yes' : 'No';
                    document.getElementById('infoInterval').textContent = data.interval + 's';
                    document.getElementById('infoCount').textContent = data.count;
                    document.getElementById('infoSDFree').textContent = data.sdFree;
                    document.getElementById('infoRSSI').textContent = data.rssi + ' dBm';

                    const btn = document.getElementById('toggleTimelapseBtn');
                    btn.textContent = isTimelapseRunning ? 'Stop Timelapse' : 'Start Timelapse';
                    btn.className = isTimelapseRunning ? 'btn btn-danger' : 'btn btn-success';
                    document.getElementById('statusBadge').textContent = isTimelapseRunning ? 'Status: Recording' : 'Status: Ready';
                }).catch(e => console.error(e));
        }

        function loadFileList() {
            fetch('/list')
                .then(r => r.json())
                .then(files => {
                    const listEl = document.getElementById('fileList');
                    if (files.length === 0) {
                        listEl.innerHTML = '<div class="file-item"><span>No images stored</span></div>';
                        return;
                    }
                    listEl.innerHTML = files.map(f => `
                        <div class="file-item">
                            <span>${f.name} (${f.size})</span>
                            <div class="file-actions">
                                <a href="${f.path}" target="_blank" class="btn btn-sm">View</a>
                                <button onclick="deleteFile('${f.path}')" class="btn btn-sm btn-danger">Del</button>
                            </div>
                        </div>
                    `).join('');
                });
        }

        function deleteFile(path) {
            if (confirm('Delete file ' + path + '?')) {
                fetch('/delete?path=' + encodeURIComponent(path))
                    .then(r => r.text())
                    .then(res => {
                        alert(res);
                        loadFileList();
                    });
            }
        }

        setInterval(updateStatus, 3000);
        updateStatus();
    </script>
</body>
</html>
)rawliteral";

#endif // WEB_INTERFACE_H
