// Firebase initialization and helpers for bin states (Realtime Database)
// Uses Firebase v9 modular SDK via CDN (ES modules)
import { initializeApp } from "https://www.gstatic.com/firebasejs/9.22.1/firebase-app.js";
import {
  getDatabase,
  ref,
  set,
  onValue,
  update,
  get,
  child,
  remove,
} from "https://www.gstatic.com/firebasejs/9.22.1/firebase-database.js";

const firebaseConfig = {
  apiKey: "AIzaSyCvRqGdrR55xzVMr-JL0J0859rGjoGx5I0",
  authDomain: "datn-be262.firebaseapp.com",
  databaseURL: "https://datn-be262-default-rtdb.firebaseio.com",
  projectId: "datn-be262",
  storageBucket: "datn-be262.firebasestorage.app",
  messagingSenderId: "215392363207",
  appId: "1:215392363207:web:f3227ef9ed1e22c0be54e2",
  measurementId: "G-FTH3KHK5W9",
};

let db = null;

function initFirebase() {
  const app = initializeApp(firebaseConfig);
  db = getDatabase(app);
  // ensure the BIN node exists with boolean defaults
  ensureBinsExist();
  // Monitor connection state (.info/connected)
  try {
    const connRef = ref(db, ".info/connected");
    onValue(connRef, (snap) => {
      const el = document.getElementById("firebaseStatus");
      const connected = snap.exists() && snap.val() === true;
      if (el) {
        el.textContent = connected ? "Connected" : "Disconnected";
        el.classList.toggle("connected", connected);
        el.classList.toggle("disconnected", !connected);
      }
      console.log("Firebase connection:", connected);
    });
  } catch (e) {
    console.warn("Connection monitor init failed:", e);
  }
  console.log("Firebase initialized (Realtime DB)");
}

const BIN_KEYS = ["VC", "HC", "TC", "RK"]; // Vô cơ, Hữu cơ, Tái chế, Khác

function ensureBinsExist() {
  if (!db) return;
  const binsRef = ref(db, "Bin");
  // read existing BIN node and set boolean defaults for missing keys
  get(binsRef)
    .then((snap) => {
      const val = snap.exists() ? snap.val() : {};
      const updates = {};
      BIN_KEYS.forEach((k) => {
        if (!(k in val)) updates[k] = false;
      });
      if (Object.keys(updates).length) {
        update(binsRef, updates).catch((err) => console.error(err));
      }
    })
    .catch((err) => console.error(err));
}

function setBinState(binKey, booleanState) {
  if (!db) {
    console.warn("Firebase not initialized yet");
    return Promise.reject(new Error("Firebase not initialized"));
  }
  if (!BIN_KEYS.includes(binKey)) {
    return Promise.reject(new Error("Unknown bin key: " + binKey));
  }
  return set(ref(db, `Bin/${binKey}`), !!booleanState);
}

function onBinsChange(callback) {
  if (!db) return;
  const binsRef = ref(db, "Bin");
  onValue(binsRef, (snap) => {
    callback(snap.exists() ? snap.val() : {});
  });
}

function getBinsOnce() {
  if (!db) return Promise.reject(new Error("Firebase not initialized"));
  const binsRef = ref(db, "Bin");
  return get(binsRef).then((snap) => (snap.exists() ? snap.val() : {}));
}

// Expose a simple global API for existing non-module code (main.js etc.)
window.FirebaseService = {
  initFirebase,
  setBinState,
  onBinsChange,
  getBinsOnce,
  BIN_KEYS,
  migrateToBin,
};

// Auto-init when loaded in the page
try {
  initFirebase();
} catch (e) {
  console.warn("Firebase init failed:", e);
}

// Note: legacy 'bins' handling and migration helpers removed —
// app expects a `BIN` node created in Firebase with boolean values for keys:
// VC, HC, TC, RK. Create them manually or via your backend/ESP32.
// Migration helper: consolidate any variant nodes into `Bin`.
function migrateToBin(deleteSources = false) {
  if (!db) return Promise.reject(new Error("Firebase not initialized"));
  const candidates = ["BIN", "BIn", "bins", "Bin"];
  const targetRef = ref(db, "Bin");
  // read all candidate nodes and merge values
  const reads = candidates.map((c) =>
    get(ref(db, c)).then((s) => ({
      name: c,
      val: s.exists() ? s.val() : null,
    })),
  );
  return Promise.all(reads).then((results) => {
    const merged = {};
    results.forEach((r) => {
      if (!r.val) return;
      BIN_KEYS.forEach((k) => {
        if (k in r.val) merged[k] = !!r.val[k];
      });
    });
    // ensure defaults
    BIN_KEYS.forEach((k) => {
      if (!(k in merged)) merged[k] = false;
    });
    return update(targetRef, merged)
      .then(() => {
        if (!deleteSources) return { migrated: true, deleted: [] };
        // remove all source nodes except the canonical 'Bin'
        const removes = candidates
          .filter((c) => c !== "Bin")
          .map((c) =>
            remove(ref(db, c))
              .then(() => c)
              .catch(() => null),
          );
        return Promise.all(removes).then((deleted) => ({
          migrated: true,
          deleted: deleted.filter(Boolean),
        }));
      })
      .catch((err) => {
        console.error("migrateToBin failed", err);
        throw err;
      });
  });
}
