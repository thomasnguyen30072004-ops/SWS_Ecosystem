// ==================== MERGED MAIN.JS (single-file bundle) ====================
// This file combines config, state, charts, utils, firebase, history, sensors,
// control, auth, router, history-page and the main initialization.

// -------- config.js --------
const CONFIG = {
  API_BASE_URL: "http://localhost:5000",
  WS_URL: "ws://localhost:8080",
  DEMO_MODE: false,
  CHART_UPDATE_INTERVAL: 1000,
  TRASH_CHECK_INTERVAL: 2000,
  PING_INTERVAL: 3000,
  STATS_UPDATE_INTERVAL: 5000,
  SNAPSHOT_UPDATE_INTERVAL: 1000,
  PI_CAMERA_URL: "http://192.168.1.183:8085",
  MAX_CHART_POINTS: 30,
  MAX_LOG_ENTRIES: 50,
  NOTIFICATION_DURATION: 3000,
  DEFAULT_SPEED: 50,
  DEFAULT_MODE: "manual",
};

// -------- state.js --------
let appState = {
  isAuthenticated: false,
  currentUser: null,
  currentMode: "manual",
  isMoving: false,
  lastCommand: null,
  trashLevels: { inorganic: false, organic: true, recycle: true, other: false },
  imuData: { yaw: 0, pitch: 0, roll: 0, timestamp: Date.now() },
  statistics: { trips: 1245, totalTrash: 2847, battery: 87, temperature: 32 },
  wsConnected: false,
};

let imuChartData = { labels: [], yaw: [], pitch: [], roll: [] };

// -------- charts.js --------
let charts = { imuChart: null };
function setChart(name, instance) {
  charts[name] = instance;
}
function getChart(name) {
  return charts[name];
}

// -------- utils.js (no imports) --------
function addCommandLog(message, type = "info") {
  const logContent = document.getElementById("commandLog");
  if (!logContent) return;
  const logEntry = document.createElement("p");
  logEntry.className = "log-entry";
  if (type) logEntry.classList.add(type);
  const timestamp = new Date().toLocaleTimeString("vi-VN");
  logEntry.textContent = `[${timestamp}] ${message}`;
  logContent.insertBefore(logEntry, logContent.firstChild);
  while (logContent.children.length > CONFIG.MAX_LOG_ENTRIES) {
    logContent.removeChild(logContent.lastChild);
  }
}
function clearCommandLog() {
  const logContent = document.getElementById("commandLog");
  if (logContent)
    logContent.innerHTML = '<p class="log-entry">[Nhật ký đã được xóa]</p>';
}
function updateCurrentTime() {
  const now = new Date();
  const timeString = now.toLocaleTimeString("vi-VN");
  const timeElement = document.getElementById("currentTime");
  if (timeElement) timeElement.textContent = timeString;
}
function updatePing() {
  if (appState.isAuthenticated) {
    const ping = Math.floor(Math.random() * 50) + 5;
    const pingElement = document.getElementById("pingValue");
    if (pingElement) pingElement.textContent = ping + "ms";
  }
}
let lastSnapshotTs = 0;

async function updateLatestSnapshot() {
  const snapshotView = document.getElementById("snapshotView");
  const placeholder = document.getElementById("streamPlaceholder");

  if (!snapshotView || !placeholder) return;

  try {
    const infoResponse = await fetch(
      `${CONFIG.PI_CAMERA_URL}/latest_info?t=${Date.now()}`
    );

    if (!infoResponse.ok) {
      return;
    }

    const info = await infoResponse.json();

    if (!info.available) {
      snapshotView.style.display = "none";
      placeholder.style.display = "flex";
      return;
    }

    if (info.ts !== lastSnapshotTs) {
      lastSnapshotTs = info.ts;

      snapshotView.src = `${CONFIG.PI_CAMERA_URL}/latest.jpg?t=${Date.now()}`;
      snapshotView.style.display = "block";
      placeholder.style.display = "none";

      addCommandLog(`📸 Ảnh mới: ${info.label || "Không rõ nhãn"}`, "success");
      console.log("Latest snapshot updated:", info);
    }
  } catch (error) {
    console.log("Cannot connect to Pi camera server:", error);
  }
}
function updateStatistics() {
  if (CONFIG.DEMO_MODE && appState.isAuthenticated) {
    const tripCountEl = document.getElementById("tripCount");
    if (!tripCountEl) return;
    appState.statistics.trips =
      parseInt(tripCountEl.textContent) + Math.floor(Math.random() * 2);
    appState.statistics.totalTrash += Math.floor(Math.random() * 5);
    appState.statistics.battery -= Math.random() * 0.1;
    document.getElementById("tripCount").textContent =
      appState.statistics.trips.toLocaleString();
    document.getElementById("totalTrash").textContent =
      appState.statistics.totalTrash.toLocaleString() + " kg";
    document.getElementById("batteryLevel").textContent =
      Math.max(0, appState.statistics.battery.toFixed(1)) + "%";
    document.getElementById("temperature").textContent =
      (32 + Math.sin(Date.now() / 5000) * 3).toFixed(1) + "°C";
    // Update trash levels randomly for demo
    const newTrashLevels = {
      inorganic: Math.random() > 0.8,
      organic: Math.random() > 0.8,
      recycle: Math.random() > 0.8,
      other: Math.random() > 0.8,
    };
    updateTrashLevels(
      newTrashLevels.inorganic,
      newTrashLevels.organic,
      newTrashLevels.recycle,
      newTrashLevels.other,
    );
  }
}
function loadUserPreferences() {
  const speedSlider = document.getElementById("speedSlider");
  const modeSelect = document.getElementById("modeSelect");
  if (!speedSlider || !modeSelect) return;
  const savedSpeed = localStorage.getItem("robotSpeed");
  const savedMode = localStorage.getItem("robotMode");
  if (savedSpeed) {
    speedSlider.value = savedSpeed;
    document.getElementById("speedValue").textContent = savedSpeed + "%";
  }
  if (savedMode) modeSelect.value = savedMode;
}
function saveUserPreferences() {
  const speedSlider = document.getElementById("speedSlider");
  const modeSelect = document.getElementById("modeSelect");
  if (!speedSlider || !modeSelect) return;
  localStorage.setItem("robotSpeed", speedSlider.value);
  localStorage.setItem("robotMode", modeSelect.value);
}

// -------- firebase.js (no imports) --------
function updateFirebase(command) {
  if (CONFIG.DEMO_MODE) {
    console.log("📡 [DEMO MODE] Firebase command:", command);
  }
}
function listenToSensors() {
  if (CONFIG.DEMO_MODE) {
    console.log("📡 [DEMO MODE] Listening to sensors...");
  }
}
function sendWebSocketMessage(message) {
  if (CONFIG.DEMO_MODE) {
    console.log("💬 [DEMO MODE] WebSocket:", message);
  }
}

// -------- history.js (no imports) --------
let historyData = {
  trips: [],
  activities: [],
  stats: { totalTrips: 0, totalDistance: 0, totalTrash: 0, totalTime: 0 },
};
function generateDemoHistory() {
  if (localStorage.getItem("historyDemoShown") === "1") return;
  historyData.trips = [
    {
      id: Date.now() + 1,
      startTime: new Date(Date.now() - 1000 * 60 * 30),
      endTime: new Date(Date.now() - 1000 * 60 * 15),
      distance: 3.4,
      trash: 5.8,
      status: "completed",
      mode: "auto",
      timestamp: new Date(Date.now() - 1000 * 60 * 15).toLocaleString("vi-VN"),
      time: new Date(Date.now() - 1000 * 60 * 15).toLocaleTimeString("vi-VN"),
    },
    {
      id: Date.now() + 2,
      startTime: new Date(Date.now() - 1000 * 60 * 60),
      endTime: new Date(Date.now() - 1000 * 60 * 45),
      distance: 2.1,
      trash: 3.2,
      status: "completed",
      mode: "manual",
      timestamp: new Date(Date.now() - 1000 * 60 * 45).toLocaleString("vi-VN"),
      time: new Date(Date.now() - 1000 * 60 * 45).toLocaleTimeString("vi-VN"),
    },
  ];
  historyData.activities = [
    {
      id: Date.now() + 10,
      description: "📤 Lệnh: Bắt đầu thu gom",
      type: "command",
      timestamp: new Date(Date.now() - 1000 * 60 * 40).toLocaleString("vi-VN"),
      time: new Date(Date.now() - 1000 * 60 * 40).toLocaleTimeString("vi-VN"),
    },
    {
      id: Date.now() + 11,
      description: "📌 Sự kiện: Pin còn 72%",
      type: "event",
      timestamp: new Date(Date.now() - 1000 * 60 * 35).toLocaleString("vi-VN"),
      time: new Date(Date.now() - 1000 * 60 * 35).toLocaleTimeString("vi-VN"),
    },
    {
      id: Date.now() + 12,
      description: "⚠️ Cảnh báo: Vùng thu gom cao điểm",
      type: "warning",
      timestamp: new Date(Date.now() - 1000 * 60 * 20).toLocaleString("vi-VN"),
      time: new Date(Date.now() - 1000 * 60 * 20).toLocaleTimeString("vi-VN"),
    },
  ];
  historyData.stats = {
    totalTrips: 2,
    totalDistance: 5.5,
    totalTrash: 9.0,
    totalTime: 2700,
  };
  try {
    localStorage.setItem("historyDemoShown", "1");
  } catch (e) {
    console.warn("Unable to persist demo history flag:", e);
  }
}
function renderHistory() {
  updateTripDisplay();
  updateActivityDisplay();
  updateStatsDisplay();
}
function initializeHistoryListeners() {
  const toggleControlBtn = document.getElementById("toggleControl");
  const toggleHistoryBtn = document.getElementById("toggleHistory");
  const controlSection = document.getElementById("controlSection");
  const historySection = document.getElementById("historySection");
  if (toggleControlBtn && controlSection) {
    toggleControlBtn.addEventListener("click", () => {
      controlSection.classList.add("active");
      if (historySection) historySection.classList.remove("active");
      toggleControlBtn.classList.add("active");
      if (toggleHistoryBtn) toggleHistoryBtn.classList.remove("active");
    });
  }
  if (toggleHistoryBtn && historySection) {
    toggleHistoryBtn.addEventListener("click", () => {
      historySection.classList.add("active");
      if (controlSection) controlSection.classList.remove("active");
      toggleHistoryBtn.classList.add("active");
      if (toggleControlBtn) toggleControlBtn.classList.remove("active");
    });
  }
  if (
    historyData.trips.length === 0 &&
    historyData.activities.length === 0 &&
    localStorage.getItem("historyDemoShown") !== "1"
  ) {
    generateDemoHistory();
  }
  renderHistory();
  const tabButtons = document.querySelectorAll(".history-tab-btn");
  const historyContents = document.querySelectorAll(".history-content");
  tabButtons.forEach((button) => {
    button.addEventListener("click", () => {
      const tabName = button.getAttribute("data-tab");
      tabButtons.forEach((btn) => btn.classList.remove("active"));
      historyContents.forEach((content) => content.classList.remove("active"));
      button.classList.add("active");
      const activeContent = document.getElementById(tabName + "Content");
      if (activeContent) activeContent.classList.add("active");
    });
  });
  const clearHistoryBtn = document.getElementById("clearHistoryBtn");
  const exportHistoryBtn = document.getElementById("exportHistoryBtn");
  if (clearHistoryBtn) clearHistoryBtn.addEventListener("click", clearHistory);
  if (exportHistoryBtn)
    exportHistoryBtn.addEventListener("click", exportHistoryAsCSV);
}
function addTrip(tripData) {
  const trip = {
    id: Date.now(),
    startTime: tripData.startTime || new Date(),
    endTime: tripData.endTime || new Date(),
    distance: tripData.distance || 0,
    trash: tripData.trash || 0,
    status: tripData.status || "completed",
    mode: tripData.mode || "manual",
    timestamp: new Date().toLocaleString("vi-VN"),
  };
  historyData.trips.unshift(trip);
  historyData.stats.totalTrips += 1;
  historyData.stats.totalDistance += trip.distance;
  historyData.stats.totalTrash += trip.trash;
  if (historyData.trips.length > 100) historyData.trips.pop();
  updateTripDisplay();
  updateStatsDisplay();
  console.log("✅ Trip added:", trip);
}
function addActivity(description, type = "command") {
  const activity = {
    id: Date.now(),
    description: description,
    type: type,
    timestamp: new Date().toLocaleString("vi-VN"),
    time: new Date().toLocaleTimeString("vi-VN"),
  };
  historyData.activities.unshift(activity);
  if (historyData.activities.length > 100) historyData.activities.pop();
  updateActivityDisplay();
  console.log("📝 Activity logged:", activity);
}
function updateTripDisplay() {
  const tripsList = document.getElementById("tripsList");
  if (!tripsList) return;
  if (historyData.trips.length === 0) {
    tripsList.innerHTML = `<div class="history-demo-card"> <div class="history-demo-title">Chưa có chuyến đi thực tế</div> <p class="history-demo-text">Đây là ví dụ về lịch sử chuyến đi khi hệ thống bắt đầu thu thập dữ liệu.</p> <div class="history-item success"> <div class="history-item-header"> <span class="history-item-title">✅ Tự động - Mã 001</span> <span class="history-item-time">09:12</span> </div> <div class="history-item-body"> <div class="history-detail"> <span class="history-detail-label">Khoảng Cách:</span> <span class="history-detail-value">3.40 km</span> </div> <div class="history-detail"> <span class="history-detail-label">Rác Thu Gom:</span> <span class="history-detail-value">5.8 kg</span> </div> <div class="history-detail"> <span class="history-detail-label">Thời Lượng:</span> <span class="history-detail-value">15m 20s</span> </div> </div> </div> <div class="history-demo-note">Dữ liệu mẫu sẽ được thay thế khi có chuyến đi thực tế.</div> </div>`;
    return;
  }
  tripsList.innerHTML = historyData.trips
    .map(
      (trip) =>
        `<div class="history-item ${trip.status === "failed" ? "error" : trip.status === "stopped" ? "warning" : "success"}"> <div class="history-item-header"> <span class="history-item-title"> ${trip.status === "completed" ? "✅" : trip.status === "failed" ? "❌" : "⏹️"} ${trip.mode === "auto" ? "🤖 Tự Động" : "🎮 Thủ Công"} </span> <span class="history-item-time">${trip.time}</span> </div> <div class="history-item-body"> <div class="history-detail"> <span class="history-detail-label">Khoảng Cách:</span> <span class="history-detail-value">${trip.distance.toFixed(2)} km</span> </div> <div class="history-detail"> <span class="history-detail-label">Rác Thu Gom:</span> <span class="history-detail-value">${trip.trash.toFixed(1)} kg</span> </div> <div class="history-detail"> <span class="history-detail-label">Thời Lượng:</span> <span class="history-detail-value">${getDuration(trip.startTime, trip.endTime)}</span> </div> </div> </div>`,
    )
    .join("");
}
function updateActivityDisplay() {
  const activitiesList = document.getElementById("activitiesList");
  if (!activitiesList) return;
  if (historyData.activities.length === 0) {
    activitiesList.innerHTML = `<div class="history-demo-card"> <div class="history-demo-title">Chưa có hoạt động ghi nhận</div> <p class="history-demo-text">Mọi lệnh, thông báo và cảnh báo sẽ xuất hiện tại đây sau khi robot hoạt động.</p> <div class="history-item command"> <div class="history-item-header"> <span class="history-item-title">📤 Lệnh: Thu gom tự động</span> <span class="history-item-time">09:14</span> </div> </div> <div class="history-item event"> <div class="history-item-header"> <span class="history-item-title">📌 Sự kiện: Pin còn 72%</span> <span class="history-item-time">09:20</span> </div> </div> <div class="history-demo-note">Dữ liệu mẫu sẽ được cập nhật sau khi hệ thống chạy thực tế.</div> </div>`;
    return;
  }
  activitiesList.innerHTML = historyData.activities
    .map(
      (activity) =>
        `<div class="history-item ${activity.type}"> <div class="history-item-header"> <span class="history-item-title"> ${getActivityIcon(activity.type)} ${activity.description} </span> <span class="history-item-time">${activity.time}</span> </div> </div>`,
    )
    .join("");
}
function updateStatsDisplay() {
  const statTripsEl = document.getElementById("statTotalTrips");
  if (!statTripsEl) return;
  if (historyData.stats.totalTrips === 0) {
    document.getElementById("statTotalTrips").textContent = "—";
    document.getElementById("statTotalDistance").textContent = "—";
    document.getElementById("statTotalTrash").textContent = "—";
    document.getElementById("statTotalTime").textContent = "—";
    const statsGrid = statTripsEl.closest(".stats-grid");
    if (statsGrid && !statsGrid.querySelector(".history-demo-note"))
      statsGrid.insertAdjacentHTML(
        "beforeend",
        '<div class="history-demo-note stats-note">Dữ liệu thực tế chưa có. Đây là vùng thống kê mẫu.</div>',
      );
    return;
  }
  const statsGrid = statTripsEl.closest(".stats-grid");
  if (statsGrid) {
    const demoNote = statsGrid.querySelector(".history-demo-note.stats-note");
    if (demoNote) demoNote.remove();
  }
  document.getElementById("statTotalTrips").textContent =
    historyData.stats.totalTrips;
  document.getElementById("statTotalDistance").textContent =
    historyData.stats.totalDistance.toFixed(2) + " km";
  document.getElementById("statTotalTrash").textContent =
    historyData.stats.totalTrash.toFixed(1) + " kg";
  const totalHours = (historyData.stats.totalTime / 3600).toFixed(1);
  document.getElementById("statTotalTime").textContent = totalHours + " h";
}
function getActivityIcon(type) {
  const icons = {
    command: "📤",
    event: "📌",
    warning: "⚠️",
    error: "❌",
    success: "✅",
  };
  return icons[type] || "📝";
}
function getDuration(startTime, endTime) {
  const diff = new Date(endTime) - new Date(startTime);
  const seconds = Math.floor(diff / 1000);
  const minutes = Math.floor(seconds / 60);
  const hours = Math.floor(minutes / 60);
  if (hours > 0) return `${hours}h ${minutes % 60}m`;
  else if (minutes > 0) return `${minutes}m ${seconds % 60}s`;
  else return `${seconds}s`;
}
function clearHistory() {
  if (confirm("⚠️ Xóa tất cả lịch sử? Thao tác này không thể hoàn tác!")) {
    historyData.trips = [];
    historyData.activities = [];
    historyData.stats = {
      totalTrips: 0,
      totalDistance: 0,
      totalTrash: 0,
      totalTime: 0,
    };
    updateTripDisplay();
    updateActivityDisplay();
    updateStatsDisplay();
    console.log("🗑️ History cleared");
  }
}
function exportHistoryAsCSV() {
  let csv =
    "Chuyến Đi,Khoảng Cách (km),Rác Thu Gom (kg),Thời Lượng,Trạng Thái\n";
  historyData.trips.forEach((trip) => {
    csv += `${trip.timestamp},${trip.distance.toFixed(2)},${trip.trash.toFixed(1)},${getDuration(trip.startTime, trip.endTime)},${trip.status}\n`;
  });
  const blob = new Blob([csv], { type: "text/csv" });
  const url = URL.createObjectURL(blob);
  const a = document.createElement("a");
  a.href = url;
  a.download = `history-${new Date().getTime()}.csv`;
  a.click();
  console.log("📥 History exported");
}
function getAllTrips() {
  return historyData.trips;
}
function getAllActivities() {
  return historyData.activities;
}
function getStatistics() {
  return historyData.stats;
}

// -------- sensors.js (no imports) --------
function updateIMUData(yaw, pitch, roll) {
  appState.imuData = { yaw, pitch, roll, timestamp: Date.now() };
  const elYaw = document.getElementById("yawValue");
  if (elYaw) elYaw.textContent = yaw.toFixed(2) + "°";
  const elPitch = document.getElementById("pitchValue");
  if (elPitch) elPitch.textContent = pitch.toFixed(2) + "°";
  const elRoll = document.getElementById("rollValue");
  if (elRoll) elRoll.textContent = roll.toFixed(2) + "°";
  const imuChart = getChart("imuChart");
  if (imuChart) {
    const maxDataPoints = CONFIG.MAX_CHART_POINTS;
    const now = new Date().toLocaleTimeString("vi-VN", {
      hour12: false,
      hour: "2-digit",
      minute: "2-digit",
      second: "2-digit",
    });
    imuChartData.labels.push(now);
    imuChartData.yaw.push(yaw);
    imuChartData.pitch.push(pitch);
    imuChartData.roll.push(roll);
    if (imuChartData.labels.length > maxDataPoints) {
      imuChartData.labels.shift();
      imuChartData.yaw.shift();
      imuChartData.pitch.shift();
      imuChartData.roll.shift();
    }
    imuChart.data.labels = imuChartData.labels;
    imuChart.data.datasets[0].data = imuChartData.yaw;
    imuChart.data.datasets[1].data = imuChartData.pitch;
    imuChart.data.datasets[2].data = imuChartData.roll;
    imuChart.update("none");
  }
}
function updateTrashLevels(inorganic, organic, recycle, other) {
  appState.trashLevels = { inorganic, organic, recycle, other };
  const levels = [
    { id: "inorganic-fill", value: inorganic, label: "Vô Cơ" },
    { id: "organic-fill", value: organic, label: "Hữu Cơ" },
    { id: "recycle-fill", value: recycle, label: "Tái Chế" },
    { id: "other-fill", value: other, label: "Khác" },
  ];
  const imageMap = {
    "inorganic-fill": { empty: "images/VC_0.png", full: "images/VC_100.png" },
    "organic-fill": { empty: "images/HC_0.png", full: "images/HC_100.png" },
    "recycle-fill": { empty: "images/TC_0.png", full: "images/TC_100.png" },
    "other-fill": { empty: "images/K_0.png", full: "images/K_100.png" },
  };

  levels.forEach((level) => {
    const element = document.getElementById(level.id);
    if (element) {
      const icon = element.querySelector(".trash-icon");
      const isFull = Boolean(level.value);
      if (isFull) {
        element.classList.add("full");
        if (icon && imageMap[level.id]) icon.src = imageMap[level.id].full;
        addCommandLog(`⚠️ ${level.label} đã đầy!`, "warning");
      } else {
        element.classList.remove("full");
        if (icon && imageMap[level.id]) icon.src = imageMap[level.id].empty;
      }
    }
  });
}
function updateBatteryStatus(percentage) {
  appState.statistics.battery = percentage;
  const batteryElement = document.getElementById("batteryLevel");
  if (batteryElement) batteryElement.textContent = percentage + "%";
  if (percentage < 20) addCommandLog(`🔋 Pin yếu! (${percentage}%)`, "warning");
}
function updateRobotSpeed(speed) {
  if (CONFIG.DEMO_MODE) {
    console.log("⚡ [DEMO MODE] Robot speed:", speed + "%");
  }
}
function handleModeChange(mode) {
  appState.currentMode = mode;
  const modeLabels = {
    auto: "🤖 Tự Động",
    manual: "🎮 Thủ Công",
    idle: "😴 Chờ",
  };
  addCommandLog(`🔄 Chế độ: ${modeLabels[mode]}`, "command");
  updateFirebase("MODE_CHANGE_" + mode.toUpperCase());
}
function setupModeSelector() {
  const modeSelect = document.getElementById("modeSelect");
  if (modeSelect)
    modeSelect.addEventListener("change", (e) => {
      handleModeChange(e.target.value);
    });
}

// -------- control.js (after sensors) --------
function setupJoypadControls() {
  const joypadButtons = document.querySelectorAll(".joypad-btn");
  joypadButtons.forEach((button) => {
    const command = button.getAttribute("data-command");
    button.addEventListener("mousedown", (e) => {
      e.preventDefault();
      button.classList.add("active");
      if (command != "STOP") sendCommand(command);
    });
    button.addEventListener("mouseup", (e) => {
      e.preventDefault();
      button.classList.remove("active");
      if (command != "STOP") sendCommand("STOP");
    });
    button.addEventListener("mouseleave", () => {
      button.classList.remove("active");
      if (command != "STOP") sendCommand("STOP");
    });
    button.addEventListener("touchstart", (e) => {
      e.preventDefault();
      button.classList.add("active");
      if (command != "STOP") sendCommand(command);
    });
    button.addEventListener("touchend", (e) => {
      e.preventDefault();
      button.classList.remove("active");
      if (command != "STOP") sendCommand("STOP");
    });
    if (command === "STOP") {
      button.addEventListener("click", (e) => {
        e.preventDefault();
        sendCommand("STOP");
        addCommandLog("⏹️ DỪNG NGAY LẬP TỨC", "warning");
      });
    }
  });
}
function sendCommand(command) {
  if (!appState.isAuthenticated) return;
  appState.lastCommand = command;
  const timestamp = new Date().toLocaleTimeString("vi-VN");
  const emoji = {
    FORWARD: "⬆️",
    BACKWARD: "⬇️",
    LEFT: "⬅️",
    RIGHT: "➡️",
    STOP: "⏹️",
    COLLECT: "🗑️",
    HOME: "🏠",
  };
  addCommandLog(
    `${emoji[command] || "📤"} ${command} [${timestamp}]`,
    "command",
  );
  addActivity(`${emoji[command] || "📤"} ${command}`, "command");
  updateFirebase(command);
  if (appState.wsConnected) {
    sendWebSocketMessage({
      type: "COMMAND",
      data: {
        command: command,
        timestamp: Date.now(),
        speed: document.getElementById("speedSlider")?.value || 50,
      },
    });
  }
  console.log("📤 Command sent:", command);
}
function setupSpeedControl() {
  const speedSlider = document.getElementById("speedSlider");
  if (speedSlider)
    speedSlider.addEventListener("input", (e) => {
      document.getElementById("speedValue").textContent = e.target.value + "%";
      updateRobotSpeed(e.target.value);
    });
}
function setupFunctionButtons() {
  const collectBtn = document.getElementById("btnCollect");
  const homeBtn = document.getElementById("btnHome");
  if (collectBtn)
    collectBtn.addEventListener("click", () => sendCommand("COLLECT"));
  if (homeBtn) homeBtn.addEventListener("click", () => sendCommand("HOME"));
}

// -------- auth.js (no imports) --------
function handleLogin(e) {
  e.preventDefault();
  const username = document.getElementById("username").value;
  const password = document.getElementById("password").value;
  if (username === "admin" && password === "12345") {
    authenticateUser(username);
  } else {
    showNotification("❌ Sai tài khoản hoặc mật khẩu!", "error");
    document.getElementById("password").value = "";
  }
}
function authenticateUser(username) {
  appState.isAuthenticated = true;
  appState.currentUser = username;
  const loginOverlay = document.getElementById("loginOverlay");
  if (loginOverlay) loginOverlay.classList.add("hidden");
  const userNameEl = document.getElementById("userName");
  if (userNameEl) userNameEl.textContent = username;
  // Ensure logout button is visible on any page after login
  initializeHistoryListeners();
  const logoutBtn = document.getElementById("logoutBtn");
  if (logoutBtn) {
    logoutBtn.style.display = "inline-flex";
    logoutBtn.textContent = `Đăng Xuất (${username})`;
  }
  // Persist session for this browser tab so navigation between pages keeps the session
  try {
    sessionStorage.setItem("isAuthenticated", "1");
    sessionStorage.setItem("currentUser", username);
  } catch (e) {
    console.warn("Could not persist session state:", e);
  }
  if (document.getElementById("commandLog")) {
    addCommandLog("✅ Đăng nhập thành công!", "success");
  }
  addActivity(`👤 Đăng nhập: ${username}`, "event");
  console.log("✅ User authenticated:", username);
  connectToBackend();
}
function handleLogout() {
  if (confirm("⚠️ Bạn chắc chắn muốn đăng xuất?")) {
    appState.isAuthenticated = false;
    appState.currentUser = null;
    const loginOverlay = document.getElementById("loginOverlay");
    if (loginOverlay) loginOverlay.classList.remove("hidden");
    const loginForm = document.getElementById("loginForm");
    if (loginForm) loginForm.reset();
    try {
      sessionStorage.removeItem("isAuthenticated");
      sessionStorage.removeItem("currentUser");
    } catch (e) {
      console.warn("Could not clear session state:", e);
    }
    // Hide logout button immediately on logout so menu updates
    const logoutBtn = document.getElementById("logoutBtn");
    if (logoutBtn) logoutBtn.style.display = "none";
    if (document.getElementById("commandLog")) {
      addCommandLog("🚪 Đã đăng xuất", "warning");
    }
    addActivity("🚪 Đã đăng xuất", "event");
    console.log("🚪 User logged out");
  }
}
function connectToBackend() {
  if (CONFIG.DEMO_MODE) {
    console.log("🔌 [DEMO MODE] Connecting to backend...");
    if (document.getElementById("commandLog")) {
      addCommandLog("🟢 Kết nối hệ thống thành công", "success");
    }
    appState.wsConnected = true;
    listenToSensors();
  }
}
function showNotification(message, type = "info") {
  const notification = document.createElement("div");
  notification.className = `notification notification-${type}`;
  notification.textContent = message;
  notification.style.cssText = `position: fixed; top: 20px; right: 20px; padding: 12px 16px; background: ${type === "error" ? "#ff4757" : "#51cf66"}; color: white; border-radius: 4px; font-size: 14px; z-index: 9999; animation: slideInRight 0.3s ease-out;`;
  document.body.appendChild(notification);
  setTimeout(() => {
    notification.style.animation = "slideOutRight 0.3s ease-out";
    setTimeout(() => notification.remove(), 300);
  }, 3000);
}

// -------- router.js (no exports) --------
const PAGES = { DASHBOARD: "dashboard", HISTORY: "history" };
function getCurrentPage() {
  return localStorage.getItem("currentPage") || PAGES.DASHBOARD;
}
function setCurrentPage(page) {
  localStorage.setItem("currentPage", page);
}
function navigateTo(page) {
  setCurrentPage(page);
  // compute target path
  const target = page === PAGES.HISTORY ? "history.html" : "index.html";
  // if we're already on the target, do nothing (prevents reload loops)
  const currentPath = window.location.pathname.split("/").pop();
  if (currentPath === target) {
    console.log(`➡️ Already on ${target}, skipping navigation`);
    return;
  }
  console.log(`➡️ Navigating to ${target}`);
  window.location.href = target;
}
function goBack() {
  const currentPage = getCurrentPage();
  if (currentPage === PAGES.HISTORY) navigateTo(PAGES.DASHBOARD);
  else navigateTo(PAGES.HISTORY);
}

// -------- history-page.js (no imports) --------
document.addEventListener("DOMContentLoaded", () => {
  const historyPageElement = document.getElementById("historyPage");
  if (!historyPageElement) return;
  setCurrentPage(PAGES.HISTORY);
  const loginForm = document.getElementById("loginForm");
  const loginOverlay = document.getElementById("loginOverlay");
  const logoutBtn = document.getElementById("logoutBtn");
  const userNameEl = document.getElementById("userName");
  if (loginForm) loginForm.addEventListener("submit", handleLogin);
  // restore session from sessionStorage (persists for the tab/session)
  const savedAuth = sessionStorage.getItem("isAuthenticated") === "1";
  if (savedAuth) {
    appState.isAuthenticated = true;
    appState.currentUser = localStorage.getItem("currentUser") || "Admin";
    if (loginOverlay) loginOverlay.classList.add("hidden");
    if (logoutBtn) logoutBtn.style.display = "inline-flex";
    if (userNameEl) userNameEl.textContent = appState.currentUser;
    initializeHistoryListeners();
  } else {
    if (loginOverlay) loginOverlay.classList.remove("hidden");
    if (logoutBtn) logoutBtn.style.display = "none";
    if (userNameEl) userNameEl.textContent = "Khách";
  }
  historyPageElement.style.visibility = "visible";
  const backBtn = document.getElementById("backToDashboardBtn");
  if (backBtn)
    backBtn.addEventListener("click", () => {
      navigateTo("dashboard");
    });
  // Change header nav button into a 'Back to Dashboard' on history page
  const headerNavBtn = document.getElementById("historyNavBtn");
  if (headerNavBtn) {
    headerNavBtn.textContent = "🔙 Dashboard";
    headerNavBtn.title = "Quay lại Dashboard";
    headerNavBtn.addEventListener("click", (e) => {
      e.preventDefault();
      navigateTo("dashboard");
    });
  }
  if (logoutBtn)
    logoutBtn.addEventListener("click", () => {
      handleLogout();
      navigateTo("dashboard");
    });
  // Wire header menu (hamburger) and theme toggle on history page
  const headerMenuBtn = document.getElementById("headerMenuBtn");
  const headerMenu = document.getElementById("headerMenu");
  if (headerMenuBtn && headerMenu) {
    headerMenuBtn.addEventListener("click", (e) => {
      e.stopPropagation();
      const open = headerMenu.classList.toggle("open");
      headerMenu.setAttribute("aria-hidden", !open);
    });
    document.addEventListener("click", (e) => {
      if (!headerMenu.contains(e.target) && !headerMenuBtn.contains(e.target)) {
        headerMenu.classList.remove("open");
        headerMenu.setAttribute("aria-hidden", "true");
      }
    });
    document.addEventListener("keydown", (e) => {
      if (e.key === "Escape") {
        headerMenu.classList.remove("open");
        headerMenu.setAttribute("aria-hidden", "true");
      }
    });
  }
  const themeToggleBtn = document.getElementById("themeToggleBtn");
  if (themeToggleBtn) {
    themeToggleBtn.addEventListener("click", () => {
      const isLight = document.body.classList.toggle("light-theme");
      try {
        localStorage.setItem("theme", isLight ? "light" : "dark");
      } catch (e) {
        console.warn("Could not persist theme:", e);
      }
    });
  }
});

// -------- Original main.js initialization adapted (no imports) --------
document.addEventListener("DOMContentLoaded", () => {
  console.log("🚀 Dashboard initialized");
  // restore session from sessionStorage (persists for the tab/session)
  const savedAuth = sessionStorage.getItem("isAuthenticated") === "1";
  // theme restore: apply saved theme (localStorage) if present
  try {
    const savedTheme = localStorage.getItem("theme");
    if (savedTheme === "light") document.body.classList.add("light-theme");
  } catch (e) {
    console.warn("Could not read theme setting:", e);
  }
  const currentPage = getCurrentPage();
  if (currentPage === PAGES.HISTORY) {
    navigateTo(PAGES.HISTORY);
    return;
  }
  setCurrentPage(PAGES.DASHBOARD);
  const loginOverlay = document.getElementById("loginOverlay");
  const logoutBtn = document.getElementById("logoutBtn");
  const userNameEl = document.getElementById("userName");
  if (savedAuth) {
    appState.isAuthenticated = true;
    appState.currentUser = localStorage.getItem("currentUser") || "Admin";
    if (loginOverlay) loginOverlay.classList.add("hidden");
    // username display moved into logout menu; do not set visible header text
    if (logoutBtn) {
      logoutBtn.style.display = "inline-flex";
      logoutBtn.textContent = `Đăng Xuất (${appState.currentUser})`;
    }
  } else {
    // ensure login overlay is visible and header shows guest state
    if (loginOverlay) loginOverlay.classList.remove("hidden");
    if (logoutBtn) logoutBtn.style.display = "none";
    // keep header userName empty when not authenticated
  }
  // Header nav button on dashboard: open history
  const headerNavBtn = document.getElementById("historyNavBtn");
  if (headerNavBtn) {
    headerNavBtn.textContent = "📊 Lịch Sử";
    headerNavBtn.title = "Mở trang Lịch Sử";
    headerNavBtn.addEventListener("click", (e) => {
      e.preventDefault();
      navigateTo(PAGES.HISTORY);
    });
  }
  initializeEventListeners();
  initializeHistoryListeners();
  initializeIMUChart();
  updateTrashLevels(
    appState.trashLevels.inorganic,
    appState.trashLevels.organic,
    appState.trashLevels.recycle,
    appState.trashLevels.other,
  );
  startAutoUpdates();
  loadUserPreferences();
  // If FirebaseService is available, use realtime data instead of demo/random
  if (window.FirebaseService) {
    try {
      CONFIG.DEMO_MODE = false;
      // initialize UI from a one-time snapshot
      window.FirebaseService.getBinsOnce()
        .then((bins) => {
          const inorganic = !!bins.VC;
          const organic = !!bins.HC;
          const recycle = !!bins.TC;
          const other = !!bins.RK;
          updateTrashLevels(inorganic, organic, recycle, other);
        })
        .catch((e) => console.warn("Firebase getBinsOnce failed", e));
      // subscribe to realtime changes
      window.FirebaseService.onBinsChange((bins) => {
        const inorganic = !!bins.VC;
        const organic = !!bins.HC;
        const recycle = !!bins.TC;
        const other = !!bins.RK;
        updateTrashLevels(inorganic, organic, recycle, other);
      });
      console.log("🔌 Using Firebase realtime Bin (ESP32 source)");
    } catch (e) {
      console.warn("FirebaseService hookup failed", e);
    }
  }
  // wire theme toggle button
  const themeToggleBtn = document.getElementById("themeToggleBtn");
  if (themeToggleBtn) {
    themeToggleBtn.addEventListener("click", () => {
      const isLight = document.body.classList.toggle("light-theme");
      try {
        localStorage.setItem("theme", isLight ? "light" : "dark");
      } catch (e) {
        console.warn("Could not persist theme:", e);
      }
    });
  }
  // Header menu (hamburger) behavior
  const headerMenuBtn = document.getElementById("headerMenuBtn");
  const headerMenu = document.getElementById("headerMenu");
  if (headerMenuBtn && headerMenu) {
    headerMenuBtn.addEventListener("click", (e) => {
      e.stopPropagation();
      const open = headerMenu.classList.toggle("open");
      headerMenu.setAttribute("aria-hidden", !open);
    });
    // close when clicking outside
    document.addEventListener("click", (e) => {
      if (!headerMenu.contains(e.target) && !headerMenuBtn.contains(e.target)) {
        headerMenu.classList.remove("open");
        headerMenu.setAttribute("aria-hidden", "true");
      }
    });
    // close on Escape
    document.addEventListener("keydown", (e) => {
      if (e.key === "Escape") {
        headerMenu.classList.remove("open");
        headerMenu.setAttribute("aria-hidden", "true");
      }
    });
  }
});

function initializeEventListeners() {
  const loginForm = document.getElementById("loginForm");
  if (loginForm) loginForm.addEventListener("submit", handleLogin);
  const logoutBtn = document.getElementById("logoutBtn");
  if (logoutBtn) logoutBtn.addEventListener("click", handleLogout);
  const historyNavBtn = document.getElementById("historyNavBtn");
  if (historyNavBtn)
    historyNavBtn.addEventListener("click", () => {
      navigateTo("history");
    });
  setupJoypadControls();
  setupSpeedControl();
  setupModeSelector();
  setupFunctionButtons();
  const clearLogBtn = document.getElementById("clearLogBtn");
  if (clearLogBtn) clearLogBtn.addEventListener("click", clearCommandLog);
}

function initializeIMUChart() {
  const ctx = document.getElementById("imuChart");
  if (!ctx) return;
  const imuChart = new Chart(ctx, {
    type: "line",
    data: {
      labels: [],
      datasets: [
        {
          label: "Yaw (Z)",
          data: [],
          borderColor: "#00d4ff",
          backgroundColor: "rgba(0,212,255,0.1)",
          borderWidth: 2,
          tension: 0.4,
          pointRadius: 0,
          pointHoverRadius: 4,
          fill: true,
        },
        {
          label: "Pitch (Y)",
          data: [],
          borderColor: "#51cf66",
          backgroundColor: "rgba(81,207,102,0.1)",
          borderWidth: 2,
          tension: 0.4,
          pointRadius: 0,
          pointHoverRadius: 4,
          fill: true,
        },
        {
          label: "Roll (X)",
          data: [],
          borderColor: "#ff6b6b",
          backgroundColor: "rgba(255,107,107,0.1)",
          borderWidth: 2,
          tension: 0.4,
          pointRadius: 0,
          pointHoverRadius: 4,
          fill: true,
        },
      ],
    },
    options: {
      responsive: true,
      maintainAspectRatio: false,
      interaction: { mode: "index", intersect: false },
      plugins: {
        legend: {
          display: true,
          labels: {
            color: "#a8b2c1",
            boxWidth: 12,
            padding: 12,
            font: { size: 11, weight: "bold" },
          },
        },
      },
      scales: {
        y: {
          beginAtZero: true,
          max: 180,
          min: -180,
          ticks: { color: "#6b7386", font: { size: 10 } },
          grid: { color: "rgba(42,63,95,0.2)", drawBorder: false },
        },
        x: {
          ticks: { color: "#6b7386", font: { size: 10 } },
          grid: { display: false, drawBorder: false },
        },
      },
    },
  });
  setChart("imuChart", imuChart);
}

function startAutoUpdates() {
  setInterval(updateCurrentTime, 1000);

  setInterval(() => {
    if (appState.isAuthenticated && CONFIG.DEMO_MODE) {
      const yaw = Math.sin(Date.now() / 2000) * 45;
      const pitch = Math.cos(Date.now() / 2500) * 30;
      const roll = Math.sin(Date.now() / 1800) * 20;
      updateIMUData(yaw, pitch, roll);
    }
  }, CONFIG.CHART_UPDATE_INTERVAL);

  setInterval(updatePing, CONFIG.PING_INTERVAL);
  setInterval(updateStatistics, CONFIG.STATS_UPDATE_INTERVAL);

  updateLatestSnapshot();
  setInterval(updateLatestSnapshot, CONFIG.SNAPSHOT_UPDATE_INTERVAL);
}

window.addEventListener("error", (event) => {
  console.error("❌ Runtime error:", event.error);
  addCommandLog("❌ Lỗi: " + (event.error?.message || event.message), "error");
});

document.addEventListener("keydown", (e) => {
  if (!appState.isAuthenticated) return;
  if (e.key === "ArrowUp" && !e.repeat) {
    e.preventDefault();
    document.getElementById("btnForward")?.classList.add("active");
    sendCommand("FORWARD");
  }
  if (e.key === "ArrowDown" && !e.repeat) {
    e.preventDefault();
    document.getElementById("btnBackward")?.classList.add("active");
    sendCommand("BACKWARD");
  }
  if (e.key === "ArrowLeft" && !e.repeat) {
    e.preventDefault();
    document.getElementById("btnLeft")?.classList.add("active");
    sendCommand("LEFT");
  }
  if (e.key === "ArrowRight" && !e.repeat) {
    e.preventDefault();
    document.getElementById("btnRight")?.classList.add("active");
    sendCommand("RIGHT");
  }
  if (e.code === "Space") {
    e.preventDefault();
    sendCommand("STOP");
  }
});

document.addEventListener("keyup", (e) => {
  if (!appState.isAuthenticated) return;
  if (e.key === "ArrowUp") {
    document.getElementById("btnForward")?.classList.remove("active");
    sendCommand("STOP");
  }
  if (e.key === "ArrowDown") {
    document.getElementById("btnBackward")?.classList.remove("active");
    sendCommand("STOP");
  }
  if (e.key === "ArrowLeft") {
    document.getElementById("btnLeft")?.classList.remove("active");
    sendCommand("STOP");
  }
  if (e.key === "ArrowRight") {
    document.getElementById("btnRight")?.classList.remove("active");
    sendCommand("STOP");
  }
});

window.addEventListener("beforeunload", () => {
  if (appState.isAuthenticated) saveUserPreferences();
});

console.log("✅ Bundled main.js loaded");
