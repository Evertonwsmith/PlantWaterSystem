// index.h
const char index_html[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Plant Water System Controller</title>
    <style>
        * { box-sizing: border-box; margin: 0; padding: 0; }
        body { font-family: -apple-system, BlinkMacSystemFont, "Segoe UI", Roboto, sans-serif; background: #f5f5f5; min-height: 100vh; display: flex; flex-direction: column; align-items: center; padding: 16px; }
        .container { width: 100%; max-width: 800px; }
        h1 { text-align: center; font-size: 1.5rem; margin: 20px 0; color: #333; }
        .data-grid { display: flex; flex-direction: column; gap: 12px; margin-bottom: 24px; }
        .data-box { background: #fff; border-radius: 8px; padding: 20px; box-shadow: 0 1px 3px rgba(0,0,0,0.1); text-align: center; }
        .data-box .label { font-size: 0.85rem; color: #666; text-transform: uppercase; letter-spacing: 0.5px; margin-bottom: 6px; }
        .data-box .value { font-size: 1.8rem; font-weight: bold; color: #222; }
        .scheduler { background: #fff; border-radius: 8px; padding: 18px; margin-bottom: 20px; box-shadow: 0 1px 3px rgba(0,0,0,0.1); }
        .scheduler h2 { font-size: 1.1rem; margin-bottom: 6px; color: #333; }
        .scheduler-desc { font-size: 0.8rem; color: #888; margin-bottom: 14px; }
        .sched-row { display: flex; align-items: center; gap: 8px; margin-bottom: 10px; flex-wrap: wrap; }
        .sched-label { font-size: 0.9rem; font-weight: 600; color: #444; min-width: 110px; }
        .sched-time { padding: 6px 8px; font-size: 0.95rem; border: 1px solid #ccc; border-radius: 6px; flex: 1; min-width: 100px; }
        .btn-sched-toggle { padding: 8px 16px; font-size: 0.85rem; font-weight: 600; border: none; border-radius: 6px; cursor: pointer; color: #fff; background: #888; transition: background 0.2s, opacity 0.2s; white-space: nowrap; }
        .btn-sched-toggle.enabled { background: #c0392b; }
        .btn-sched-toggle:active { opacity: 0.7; }
        .btn-sched-delete { padding: 8px 12px; font-size: 0.85rem; font-weight: 600; border: none; border-radius: 6px; cursor: pointer; background: #e74c3c; color: #fff; transition: opacity 0.2s; white-space: nowrap; }
        .btn-sched-delete:active { opacity: 0.7; }
        .btn-add { padding: 8px 16px; font-size: 0.85rem; font-weight: 600; border: none; border-radius: 6px; cursor: pointer; background: #27ae60; color: #fff; transition: opacity 0.2s; white-space: nowrap; }
        .btn-add:active { opacity: 0.7; }
        .sched-status { font-size: 0.8rem; color: #777; font-style: italic; }
        .button-row { display: flex; flex-direction: column; gap: 10px; margin-bottom: 20px; }
        .btn { padding: 14px 0; font-size: 1.05rem; font-weight: 600; border: none; border-radius: 8px; cursor: pointer; color: #fff; transition: opacity 0.2s; width: 100%; }
        .btn:active { opacity: 0.7; }
        .btn-cmd1 { background: #2d7d46; }
        .log { background: #fff; border-radius: 8px; padding: 14px; font-size: 0.85rem; color: #555; box-shadow: 0 1px 3px rgba(0,0,0,0.1); min-height: 40px; word-break: break-word; }
        @media (min-width: 600px) {
            .data-grid { flex-direction: row; }
            .data-box { flex: 1; }
            .button-row { flex-direction: row; }
            .btn { flex: 1; }
            h1 { font-size: 1.8rem; }
        }
    </style>
</head>
<body>

    <div class="container">
        <h1>🌱 Plant Water Controller</h1>

        <div class="data-grid">
            <div class="data-box">
                <div class="label">Temperature</div>
                <div class="value" id="temperature">--</div>
            </div>
            <div class="data-box">
                <div class="label">Humidity</div>
                <div class="value" id="humidity">--</div>
            </div>
            <div class="data-box">
                <div class="label">Soil Moisture</div>
                <div class="value" id="moisture">--</div>
            </div>
        </div>

        <div class="button-row">
            <button class="btn btn-cmd1" id="btn-water">💧 Water Now</button>
        </div>

        <div class="scheduler">
            <h2>⏰ Water Schedule</h2>
            <p class="scheduler-desc">Set times for automatic watering (24h format HH:MM).</p>
            <div id="schedule-list"></div>
            <div style="display:flex; gap:8px; align-items:center; margin-top:10px; flex-wrap:wrap;">
                <button class="btn-add" id="btn-add-time">➕ Add Time</button>
                <span class="sched-status" id="sched-status"></span>
            </div>
        </div>

        <div class="log" id="log">Ready.</div>
    </div>

    <script>
        const BASE_URL = ''; 
        const logEl = document.getElementById('log');
        let schedule = [];
        let nextId = 1;

        function logMessage(text) {
            const now = new Date();
            const time = now.toLocaleTimeString();
            logEl.textContent = `[${time}] ${text}`;
        }

        function fetchData() {
            fetch(BASE_URL + '/temperature').then(r => r.text()).then(d => { document.getElementById('temperature').textContent = d; }).catch(() => {});
            fetch(BASE_URL + '/humidity').then(r => r.text()).then(d => { document.getElementById('humidity').textContent = d; }).catch(() => {});
            fetch(BASE_URL + '/moisture').then(r => r.text()).then(d => { document.getElementById('moisture').textContent = d; }).catch(() => {});
        }

        function sendCommand(endpoint, label) {
            logMessage(`Sending: ${label}...`);
            fetch(BASE_URL + endpoint, { method: 'POST' })
                .then(r => r.ok ? r.text() : Promise.reject(r.status))
                .then(d => logMessage(`${label} OK — ${d}`))
                .catch(e => logMessage(`${label} failed: ${e}`));
        }

        document.getElementById('btn-water').addEventListener('click', () => sendCommand('/water-now', 'Water Now'));

        // --- NEW SERVER-SIDE STORAGE FUNCTIONS ---
        
        function loadSchedule() {
            document.getElementById('schedule-list').innerHTML = 'Loading schedules from ESP32...';
            fetch(BASE_URL + '/api/schedule')
                .then(res => res.json())
                .then(data => {
                    schedule = Array.isArray(data) ? data : [];
                    nextId = schedule.length > 0 ? Math.max(...schedule.map(e => e.id || 0)) + 1 : 1;
                    // Fix entries from legacy formatting
                    schedule.forEach((e, i) => { if(!e.id) e.id = i + 1; });
                    renderSchedule();
                    updateSchedulerStatus();
                })
                .catch(() => {
                    logMessage("Failed to download schedule configurations from ESP32.");
                });
        }

        function saveSchedule() {
            document.getElementById('sched-status').textContent = 'Saving to ESP32...';
            // Clean dynamic framework properties before posting string payload
            const dynamicCleanPayload = schedule.map(({id, time, enabled}) => ({id, time, enabled}));

            fetch(BASE_URL + '/api/schedule', {
                method: 'POST',
                headers: { 'Content-Type': 'application/json' },
                body: JSON.stringify(dynamicCleanPayload)
            })
            .then(res => res.text())
            .then(() => { 
                document.getElementById('sched-status').textContent = 'Saved to hardware ✔️'; 
            })
            .catch(() => { 
                document.getElementById('sched-status').textContent = 'Error writing to ESP32'; 
            });
        }

        function renderSchedule() {
            const container = document.getElementById('schedule-list');
            container.innerHTML = '';

            schedule.forEach((entry, index) => {
                const row = document.createElement('div');
                row.className = 'sched-row';

                const label = document.createElement('span');
                label.className = 'sched-label';
                label.textContent = `#${index + 1}`;

                const timeInput = document.createElement('input');
                timeInput.type = 'time';
                timeInput.className = 'sched-time';
                timeInput.value = entry.time;
                timeInput.addEventListener('change', function () {
                    entry.time = this.value;
                    saveSchedule();
                    updateSchedulerStatus();
                });

                const toggle = document.createElement('button');
                toggle.className = 'btn-sched-toggle' + (entry.enabled ? ' enabled' : '');
                toggle.textContent = entry.enabled ? 'Disable' : 'Enable';
                toggle.addEventListener('click', function () {
                    entry.enabled = !entry.enabled;
                    saveSchedule();
                    renderSchedule();
                    updateSchedulerStatus();
                    logMessage(`Schedule #${index + 1} ${entry.enabled ? 'enabled' : 'disabled'}.`);
                });

                const delBtn = document.createElement('button');
                delBtn.className = 'btn-sched-delete';
                delBtn.textContent = '✕';
                delBtn.addEventListener('click', function () {
                    const idx = schedule.findIndex(e => e.id === entry.id);
                    if (idx > -1) {
                        schedule.splice(idx, 1);
                        saveSchedule();
                        renderSchedule();
                        updateSchedulerStatus();
                        logMessage(`Schedule slot #${index + 1} deleted.`);
                    }
                });

                row.appendChild(label);
                row.appendChild(timeInput);
                row.appendChild(toggle);
                row.appendChild(delBtn);
                container.appendChild(row);
            });
        }

        function updateSchedulerStatus() {
            const statusEl = document.getElementById('sched-status');
            const active = schedule.filter(e => e.enabled && e.time);
            if (active.length === 0) {
                statusEl.textContent = 'No active watering times.';
            } else {
                const parts = active.map(e => `#${schedule.indexOf(e) + 1} @ ${e.time}`);
                statusEl.textContent = 'Active: ' + parts.join(' | ');
            }
        }

        document.getElementById('btn-add-time').addEventListener('click', () => {
            schedule.push({ id: nextId++, time: '12:00', enabled: false });
            saveSchedule();
            renderSchedule();
            updateSchedulerStatus();
        });

        // Initialize execution setup
        fetchData();
        setInterval(fetchData, 5000);
        loadSchedule();
    </script>
</body>
</html>
)rawliteral";