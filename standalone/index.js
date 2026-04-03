import * as THREE from 'three';
import { OrbitControls } from 'three/addons/controls/OrbitControls.js';

const MAX_POINTS = 1200;
const ENGINE_SCRIPT_URL = new URL('../web_dist/MomentumCore.js', import.meta.url);
const ENGINE_WASM_DIR_URL = new URL('../web_dist/', import.meta.url);

const state = {
    is3D: false,
    isPaused: false,
    ticks: 0,
    ticksSinceCollapse: 0,
    score: 0,
    xp: 0,
    level: 1,
    streak: 0,
    combo: 1,
    viewSwitchCount: 0,
    manualCollapseCount: 0,
    lastEntropy: 0,
    stabilityFrames: 0,
    lastCz: 0,
    observerJumpCounter: 0,
    unlocked: new Set(),
    missionComplete: {
        entropy: false,
        stability: false,
        observer: false
    },
    entropyHistory: [],
    pointHistory: []
};

let scene;
let camera;
let renderer;
let controls;
let particle;
let trail;
let shockwave;
let diagnosticFrame = 0;
let diagnosticLoopHandle = 0;
let appStarted = false;
const actionCooldowns = new Map();
let toastBurstCount = 0;
let toastBurstWindow = 0;

const elements = {
    container: document.getElementById('container'),
    canvas2d: document.getElementById('canvas2d'),
    flash: document.getElementById('flash'),
    status: document.getElementById('status'),
    stateTxt: document.getElementById('stateTxt'),
    entropy: document.getElementById('entropy'),
    timeSince: document.getElementById('timeSince'),
    scoreVal: document.getElementById('scoreVal'),
    probRing: document.getElementById('probRing'),
    prob: document.getElementById('prob'),
    chaosX: document.getElementById('chaosX'),
    chaosY: document.getElementById('chaosY'),
    chaosZ: document.getElementById('chaosZ'),
    streakVal: document.getElementById('streakVal'),
    comboVal: document.getElementById('comboVal'),
    xpFill: document.getElementById('xpFill'),
    xpValue: document.getElementById('xpValue'),
    levelBadge: document.getElementById('levelBadge'),
    waveEntropy: document.getElementById('waveEntropy'),
    sliderMass: document.getElementById('sliderMass'),
    sliderStressX: document.getElementById('sliderStressX'),
    massVal: document.getElementById('massVal'),
    stressVal: document.getElementById('stressVal'),
    btnToggle: document.getElementById('btnToggle'),
    btnPause: document.getElementById('btnPause'),
    btnClear: document.getElementById('btnClear'),
    btnCollapse: document.getElementById('btnCollapse'),
    toastRack: document.getElementById('toastRack'),
    bootBanner: document.getElementById('bootBanner'),
    bootTitle: document.getElementById('bootTitle'),
    bootMessage: document.getElementById('bootMessage')
};

const ctx2d = elements.canvas2d?.getContext('2d') ?? null;
let firstRuntimeFailureCaptured = false;
let debugConsoleDetails;

function resizeCanvas() {
    if (!elements.canvas2d) {
        return;
    }

    elements.canvas2d.width = window.innerWidth;
    elements.canvas2d.height = window.innerHeight;
}

function loadScriptOnce(url) {
    return new Promise((resolve, reject) => {
        const existing = Array.from(document.scripts).find((script) =>
            script.dataset.engineSrc === url || script.src === url
        );
        if (existing?.dataset.loaded === 'true') {
            resolve();
            return;
        }

        if (existing) {
            existing.addEventListener('load', resolve, { once: true });
            existing.addEventListener('error', () => reject(new Error(`Failed to load ${url}`)), { once: true });
            return;
        }

        const script = document.createElement('script');
        script.src = url;
        script.async = true;
        script.dataset.engineSrc = url;
        script.addEventListener('load', () => {
            script.dataset.loaded = 'true';
            resolve();
        }, { once: true });
        script.addEventListener('error', () => reject(new Error(`Failed to load ${url}`)), { once: true });
        document.head.appendChild(script);
    });
}

async function resolveEngineFactory() {
    if (typeof window.PhysEngine === 'function') {
        return window.PhysEngine;
    }

    await loadScriptOnce(ENGINE_SCRIPT_URL.href);
    return typeof window.PhysEngine === 'function' ? window.PhysEngine : null;
}

function ensureDebugConsole() {
    if (document.getElementById('debugConsolePanel')) {
        debugConsoleDetails = document.getElementById('debugConsoleDetails');
        return;
    }

    const panel = document.createElement('section');
    panel.id = 'debugConsolePanel';
    panel.className = 'debug-console';
    panel.innerHTML = `
        <div class="debug-console__head">
            <strong>DEBUG CONSOLE (temporary)</strong>
            <span>Waiting for first runtime failure...</span>
        </div>
        <pre id="debugConsoleDetails" class="debug-console__body">No failures captured yet.</pre>
    `;
    document.body.appendChild(panel);
    debugConsoleDetails = document.getElementById('debugConsoleDetails');
}

function normalizeFailureDetails(label, errorLike, extra = {}) {
    const error = errorLike instanceof Error ? errorLike : null;
    const now = new Date().toISOString();

    const base = {
        label,
        time: now,
        message: error?.message || String(errorLike ?? 'Unknown error'),
        stack: error?.stack || '(no stack available)'
    };

    return {
        ...base,
        ...extra
    };
}

function captureFirstRuntimeFailure(label, errorLike, extra = {}) {
    if (firstRuntimeFailureCaptured) {
        return;
    }

    firstRuntimeFailureCaptured = true;
    ensureDebugConsole();

    const details = normalizeFailureDetails(label, errorLike, extra);
    if (debugConsoleDetails) {
        debugConsoleDetails.textContent = JSON.stringify(details, null, 2);
    }

    setBootState('ENGINE ERROR', `${details.label}: ${details.message}`, true);
}

function setBootState(title, message, isError = false) {
    if (!elements.bootBanner || !elements.bootTitle || !elements.bootMessage) {
        return;
    }

    elements.bootTitle.innerText = title;
    elements.bootMessage.innerText = message;
    elements.bootBanner.classList.toggle('error', isError);
}

const vertexShader = `
    varying vec3 vNormal;
    void main() {
        vNormal = normalize(normalMatrix * normal);
        gl_Position = projectionMatrix * modelViewMatrix * vec4(position, 1.0);
    }
`;

const fragmentShader = `
    varying vec3 vNormal;
    void main() {
        float intensity = pow(0.7 - dot(vNormal, vec3(0, 0, 1.0)), 3.0);
        gl_FragColor = vec4(1.0, 0.46, 0.0, 1.0) * intensity;
    }
`;

function showToast(message) {
    if (!elements.toastRack) {
        return;
    }

    const now = performance.now();
    if (now - toastBurstWindow > 1000) {
        toastBurstWindow = now;
        toastBurstCount = 0;
    }

    if (toastBurstCount >= 6) {
        return;
    }
    toastBurstCount += 1;

    const toast = document.createElement('div');
    toast.className = 'toast';
    toast.innerText = message;
    elements.toastRack.appendChild(toast);
    setTimeout(() => toast.remove(), 3700);
}

function unlockAchievement(id, text) {
    if (state.unlocked.has(id)) {
        return;
    }

    state.unlocked.add(id);
    const badge = document.getElementById(id);
    if (badge) {
        badge.classList.add('unlocked');
    }
    showToast(`Achievement unlocked: ${text}`);
}

function xpNeededForLevel(currentLevel) {
    return 120 + Math.floor((currentLevel - 1) * 45);
}

function renderGameHUD() {
    const needed = xpNeededForLevel(state.level);
    const ratio = Math.min(1, state.xp / needed);

    elements.xpFill.style.width = `${(ratio * 100).toFixed(1)}%`;
    elements.xpValue.innerText = `XP ${Math.floor(state.xp)} / ${needed}`;
    elements.levelBadge.innerText = `Level ${state.level}`;
    elements.scoreVal.innerText = state.score.toString();
    elements.streakVal.innerText = state.streak.toString();
    elements.comboVal.innerText = `x${state.combo.toFixed(1)}`;
}

function grantXP(amount, reason) {
    state.xp += amount;
    showToast(`+${amount} XP - ${reason}`);

    while (state.xp >= xpNeededForLevel(state.level)) {
        state.xp -= xpNeededForLevel(state.level);
        state.level += 1;
        showToast(`Level up! Now Level ${state.level}`);
    }

    renderGameHUD();
}

function addScore(points) {
    state.score += Math.round(points * state.combo);
}

function runWithCooldown(actionKey, cooldownMs, handler) {
    const now = performance.now();
    const lastRun = actionCooldowns.get(actionKey) ?? 0;
    if (now - lastRun < cooldownMs) {
        return false;
    }

    actionCooldowns.set(actionKey, now);
    handler();
    return true;
}

function updateQuestUI(elementId, percentage, textId) {
    const clamped = Math.max(0, Math.min(1, percentage));
    const quest = document.getElementById(elementId);
    if (!quest) {
        return;
    }

    const fill = quest.querySelector('.quest-fill');
    if (fill) {
        fill.style.width = `${(clamped * 100).toFixed(1)}%`;
    }
    quest.classList.toggle('complete', clamped >= 1);

    const text = document.getElementById(textId);
    if (text) {
        text.innerText = `${Math.floor(clamped * 100)}%`;
    }
}

function updateTaskVisualization(entropy) {
    if (state.ticks % 2 !== 0) {
        return;
    }

    state.entropyHistory.push(entropy);
    if (state.entropyHistory.length > 32) {
        state.entropyHistory.shift();
    }

    const maxRange = 2.2;
    const width = 300;
    const height = 80;
    const step = width / Math.max(1, state.entropyHistory.length - 1);
    let points = '';

    for (let i = 0; i < state.entropyHistory.length; i += 1) {
        const x = i * step;
        const y = height - Math.min(height - 4, (state.entropyHistory[i] / maxRange) * (height - 8)) - 4;
        points += `${x.toFixed(2)},${y.toFixed(2)} `;
    }

    elements.waveEntropy.setAttribute('points', points.trim() || '0,40 300,40');
}

function updateMissionProgress(entropy, czDelta) {
    const entropyProgress = Math.max(0, 1 - entropy / 1.15);
    updateQuestUI('questEntropy', entropyProgress, 'questEntropyText');

    if (Math.abs(entropy - state.lastEntropy) < 0.014) {
        state.stabilityFrames += 1;
    } else {
        state.stabilityFrames = Math.max(0, state.stabilityFrames - 1.5);
    }
    const stabilityProgress = Math.min(1, state.stabilityFrames / (60 * 12));
    updateQuestUI('questStability', stabilityProgress, 'questStabilityText');

    if (czDelta > 1.2) {
        state.observerJumpCounter += 1;
    }
    state.observerJumpCounter = Math.max(0, state.observerJumpCounter - 0.05);
    const observerProgress = Math.min(1, state.observerJumpCounter / 11);
    updateQuestUI('questObserver', observerProgress, 'questObserverText');

    if (!state.missionComplete.entropy && entropyProgress >= 1) {
        state.missionComplete.entropy = true;
        grantXP(45, 'Thermal Discipline complete');
        unlockAchievement('badgeMaster', 'Entropy Master');
    }

    if (!state.missionComplete.stability && stabilityProgress >= 1) {
        state.missionComplete.stability = true;
        grantXP(55, 'Stability Streak complete');
        unlockAchievement('badgeStable', 'Stability Keeper');
    }

    if (!state.missionComplete.observer && observerProgress >= 1) {
        state.missionComplete.observer = true;
        grantXP(35, 'Observer Jumps complete');
        unlockAchievement('badgeNavigator', 'Mode Navigator');
    }

    if (state.missionComplete.entropy && state.missionComplete.stability && state.missionComplete.observer) {
        unlockAchievement('badgeMaster', 'Entropy Master');
    }
}

function updateSliderReadouts() {
    if (!elements.sliderMass || !elements.sliderStressX || !elements.massVal || !elements.stressVal) {
        return;
    }

    const mass = elements.sliderMass.value;
    const stress = elements.sliderStressX.value;
    elements.massVal.innerText = Number(mass).toFixed(1);
    elements.stressVal.innerText = Number(stress).toFixed(1);
}

function has3DRenderer() {
    return Boolean(renderer && camera && controls && particle && trail && shockwave);
}

function stopDiagnosticLoop() {
    if (!diagnosticLoopHandle) {
        return;
    }

    cancelAnimationFrame(diagnosticLoopHandle);
    diagnosticLoopHandle = 0;
}

function startDiagnosticLoop(label = 'DIAGNOSTIC MODE') {
    if (!ctx2d || diagnosticLoopHandle) {
        return;
    }

    elements.canvas2d.style.display = 'block';
    if (renderer?.domElement) {
        renderer.domElement.style.display = 'none';
    }

    const renderDiagnosticFrame = () => {
        diagnosticLoopHandle = requestAnimationFrame(renderDiagnosticFrame);
        diagnosticFrame += 1;

        const width = elements.canvas2d.width || window.innerWidth;
        const height = elements.canvas2d.height || window.innerHeight;
        const cx = width / 2;
        const cy = height / 2;
        const radius = Math.min(width, height) * 0.18;
        const orbit = diagnosticFrame * 0.018;

        ctx2d.clearRect(0, 0, width, height);
        ctx2d.fillStyle = 'rgba(3, 8, 20, 0.28)';
        ctx2d.fillRect(0, 0, width, height);

        ctx2d.strokeStyle = 'rgba(63, 240, 255, 0.08)';
        ctx2d.lineWidth = 1;
        for (let x = 0; x < width; x += 48) {
            ctx2d.beginPath();
            ctx2d.moveTo(x, 0);
            ctx2d.lineTo(x, height);
            ctx2d.stroke();
        }
        for (let y = 0; y < height; y += 48) {
            ctx2d.beginPath();
            ctx2d.moveTo(0, y);
            ctx2d.lineTo(width, y);
            ctx2d.stroke();
        }

        ctx2d.beginPath();
        ctx2d.strokeStyle = 'rgba(255, 255, 255, 0.14)';
        ctx2d.arc(cx, cy, radius, 0, Math.PI * 2);
        ctx2d.stroke();

        const pulseX = cx + Math.cos(orbit) * radius;
        const pulseY = cy + Math.sin(orbit * 1.35) * radius * 0.6;
        ctx2d.beginPath();
        ctx2d.fillStyle = '#3ff0ff';
        ctx2d.shadowBlur = 18;
        ctx2d.shadowColor = '#3ff0ff';
        ctx2d.arc(pulseX, pulseY, 7, 0, Math.PI * 2);
        ctx2d.fill();
        ctx2d.shadowBlur = 0;

        ctx2d.fillStyle = 'rgba(236, 247, 255, 0.92)';
        ctx2d.font = '700 14px "Space Grotesk", sans-serif';
        ctx2d.fillText(label, 24, 34);
        ctx2d.font = '400 12px "Space Grotesk", sans-serif';
        ctx2d.fillStyle = 'rgba(158, 184, 208, 0.92)';
        ctx2d.fillText('Rendering fallback telemetry while the primary engine is unavailable.', 24, 56);
    };

    renderDiagnosticFrame();
}

function initGraphics() {
    resizeCanvas();
    if (!ctx2d) {
        throw new Error('2D canvas context is unavailable');
    }
    if (!elements.container) {
        throw new Error('Application container is unavailable');
    }

    try {
        scene = new THREE.Scene();
        camera = new THREE.PerspectiveCamera(60, window.innerWidth / window.innerHeight, 0.1, 1000);
        camera.position.set(0, 0, 80);

        renderer = new THREE.WebGLRenderer({ antialias: true, alpha: true });
        renderer.setSize(window.innerWidth, window.innerHeight);
        renderer.domElement.style.zIndex = '1';
        renderer.domElement.style.position = 'absolute';
        elements.container.appendChild(renderer.domElement);

        controls = new OrbitControls(camera, renderer.domElement);
        controls.enableDamping = true;
        controls.dampingFactor = 0.05;

        const particleGeometry = new THREE.SphereGeometry(1.5, 32, 32);
        const particleMaterial = new THREE.ShaderMaterial({
            vertexShader,
            fragmentShader,
            blending: THREE.AdditiveBlending,
            transparent: true
        });
        particle = new THREE.Mesh(particleGeometry, particleMaterial);
        scene.add(particle);

        const trailGeometry = new THREE.BufferGeometry();
        trailGeometry.setAttribute('position', new THREE.BufferAttribute(new Float32Array(MAX_POINTS * 3), 3));
        const trailMaterial = new THREE.LineBasicMaterial({
            color: 0xff7700,
            transparent: true,
            opacity: 0.8
        });
        trail = new THREE.Line(trailGeometry, trailMaterial);
        scene.add(trail);

        const shockwaveGeometry = new THREE.SphereGeometry(1, 32, 32);
        const shockwaveMaterial = new THREE.MeshBasicMaterial({
            color: 0xff3366,
            transparent: true,
            opacity: 0,
            wireframe: true
        });
        shockwave = new THREE.Mesh(shockwaveGeometry, shockwaveMaterial);
        scene.add(shockwave);
    } catch (graphicsError) {
        renderer = null;
        controls = null;
        particle = null;
        trail = null;
        shockwave = null;
        state.is3D = false;
        if (elements.btnToggle) {
            elements.btnToggle.disabled = true;
            elements.btnToggle.innerText = 'Mode: 2D Fallback';
        }
        console.error(graphicsError);
        setBootState('VISUAL DEGRADED', 'WebGL is unavailable. Continuing with the 2D diagnostic renderer.', true);
        startDiagnosticLoop('2D FALLBACK');
        return;
    }

    renderer.domElement.style.display = 'none';
    elements.canvas2d.style.display = 'block';
}

function triggerCollapseVisuals() {
    elements.flash.classList.add('flashing');
    setTimeout(() => elements.flash.classList.remove('flashing'), 50);

    if (state.is3D && shockwave) {
        shockwave.position.copy(particle.position);
        shockwave.scale.set(1, 1, 1);
        shockwave.material.opacity = 0.8;
    }

    state.ticksSinceCollapse = 0;
    state.streak = 0;
    state.combo = 1;
    renderGameHUD();
}

function resizeView() {
    resizeCanvas();

    if (!camera || !renderer) {
        return;
    }

    camera.aspect = window.innerWidth / window.innerHeight;
    camera.updateProjectionMatrix();
    renderer.setSize(window.innerWidth, window.innerHeight);
}

function setReadyState() {
    stopDiagnosticLoop();
    elements.status.innerText = 'SYSTEMS ONLINE';
    elements.status.style.background = 'rgba(63, 240, 255, 0.18)';
    elements.status.style.color = '#d6fcff';
    elements.stateTxt.innerText = 'RUNNING';
    unlockAchievement('badgeOnline', 'System Online');
    setBootState('SYSTEMS ONLINE', 'Simulation active. Use the canvas and controls below to manipulate task state.');
}

function setErrorState() {
    elements.status.innerText = 'CRITICAL ERROR';
    elements.status.style.background = 'rgba(255, 77, 141, 0.25)';
    elements.status.style.color = '#ffd7e7';
    elements.stateTxt.innerText = 'DEGRADED';
    startDiagnosticLoop('OFFLINE MODE');
    setBootState('CRITICAL ERROR', 'Simulation failed to start. See the console for details.', true);
}

function bindSliders(Module, task) {
    let pendingSliderFrame = 0;
    let pendingMassValue = null;
    let pendingStressXValue = null;

    function flushSliderUpdates() {
        pendingSliderFrame = 0;

        if (pendingMassValue !== null && Module.Task_SetMass) {
            Module.Task_SetMass(task, pendingMassValue);
        }

        if (pendingStressXValue !== null && Module.Task_SetStress) {
            const currentY = typeof Module.Task_GetStressY === 'function' ? Module.Task_GetStressY(task) : 0;
            const currentZ = typeof Module.Task_GetStressZ === 'function' ? Module.Task_GetStressZ(task) : 0;
            Module.Task_SetStress(task, pendingStressXValue, currentY, currentZ);
        }
    }

    function scheduleSliderFlush() {
        if (pendingSliderFrame !== 0) {
            return;
        }

        pendingSliderFrame = requestAnimationFrame(flushSliderUpdates);
    }

    elements.sliderMass.oninput = (event) => {
        updateSliderReadouts();
        pendingMassValue = parseFloat(event.target.value);
        scheduleSliderFlush();
    };

    elements.sliderStressX.oninput = (event) => {
        updateSliderReadouts();
        pendingStressXValue = parseFloat(event.target.value);
        scheduleSliderFlush();
    };
}

function bindToolbar(Module, task) {
    elements.btnToggle.onclick = () => {
        if (!runWithCooldown('toggle-view', 180, () => {})) {
            return;
        }

        if (!has3DRenderer()) {
            state.is3D = false;
            elements.btnToggle.innerText = 'Mode: 2D Fallback';
            showToast('3D renderer unavailable on this device. Staying in 2D mode.');
            return;
        }

        state.is3D = !state.is3D;
        elements.btnToggle.innerText = state.is3D ? 'Mode: 3D Space' : 'Mode: 2D Radar';
        renderer.domElement.style.display = state.is3D ? 'block' : 'none';
        elements.canvas2d.style.display = state.is3D ? 'none' : 'block';
        state.viewSwitchCount += 1;
        addScore(8);
        grantXP(6, 'Mode shift');
        if (state.viewSwitchCount >= 5) {
            unlockAchievement('badgeNavigator', 'Mode Navigator');
        }
    };

    elements.btnPause.onclick = () => {
        if (!runWithCooldown('pause-toggle', 150, () => {})) {
            return;
        }

        state.isPaused = !state.isPaused;
        elements.btnPause.innerText = state.isPaused ? 'Resume' : 'Pause';
        elements.stateTxt.innerText = state.isPaused ? 'PAUSED' : 'RUNNING';
        elements.stateTxt.style.color = state.isPaused ? '#ffaa00' : '#3ff0ff';
    };

    elements.btnClear.onclick = () => {
        if (!runWithCooldown('clear-trails', 250, () => {})) {
            return;
        }

        state.pointHistory.length = 0;
        state.entropyHistory.length = 0;
        ctx2d.clearRect(0, 0, elements.canvas2d.width, elements.canvas2d.height);
        grantXP(4, 'Trail reset');
    };

    elements.btnCollapse.onclick = () => {
        if (!runWithCooldown('force-collapse', 450, () => {})) {
            return;
        }

        Module.Engine_PerformQuantumCollapse(task);
        triggerCollapseVisuals();
        state.manualCollapseCount += 1;
        addScore(15);
        grantXP(12, 'Manual collapse');
        if (state.manualCollapseCount >= 3) {
            unlockAchievement('badgeCollapse', 'Collapse Commander');
        }
    };
}

function render2D(cx, cy) {
    if (!ctx2d || !elements.canvas2d) {
        return;
    }

    ctx2d.fillStyle = 'rgba(5, 9, 20, 0.08)';
    ctx2d.fillRect(0, 0, elements.canvas2d.width, elements.canvas2d.height);

    const centerX = elements.canvas2d.width / 2;
    const centerY = elements.canvas2d.height / 2;
    const scale = 8;

    ctx2d.fillStyle = '#ff7700';
    ctx2d.shadowBlur = 10;
    ctx2d.shadowColor = '#ff7700';
    ctx2d.beginPath();
    ctx2d.arc(centerX + (cx * scale), centerY + (cy * scale), 2, 0, Math.PI * 2);
    ctx2d.fill();
    ctx2d.shadowBlur = 0;
}

function render3D(cx, cy, cz) {
    particle.position.set(cx, cy, cz - 25);

    if (state.ticks % 2 === 0) {
        state.pointHistory.push(cx, cy, cz - 25);
        if (state.pointHistory.length > MAX_POINTS * 3) {
            state.pointHistory.splice(0, 3);
        }

        const positionAttribute = trail.geometry.attributes.position;
        for (let i = 0; i < state.pointHistory.length / 3; i += 1) {
            positionAttribute.setXYZ(
                i,
                state.pointHistory[i * 3],
                state.pointHistory[i * 3 + 1],
                state.pointHistory[i * 3 + 2]
            );
        }
        positionAttribute.needsUpdate = true;
        trail.geometry.setDrawRange(0, state.pointHistory.length / 3);
    }

    renderer.render(scene, camera);
}

function updateTelemetry(entropy, probability, cx, cy, cz) {
    elements.probRing.style.setProperty('--p', Math.min(100, Math.max(0, probability)).toFixed(1));

    if (state.ticks % 3 !== 0) {
        return;
    }

    elements.entropy.innerText = entropy.toFixed(6);
    elements.prob.innerText = `${probability.toFixed(1)}%`;
    elements.timeSince.innerText = `${(state.ticksSinceCollapse / 60).toFixed(1)}s`;
    elements.chaosX.innerText = cx.toFixed(3);
    elements.chaosY.innerText = cy.toFixed(3);
    elements.chaosZ.innerText = cz.toFixed(3);
    renderGameHUD();
}

function startUpdateLoop(Module, task) {
    function update() {
        requestAnimationFrame(update);

        if (state.is3D && controls) {
            controls.update();
        }

        if (shockwave && shockwave.material.opacity > 0) {
            shockwave.scale.addScalar(1.5);
            shockwave.material.opacity -= 0.02;
        }

        if (state.isPaused) {
            return;
        }

        Module.Engine_UpdateChaos(task);
        state.ticks += 1;
        state.ticksSinceCollapse += 1;

        const entropy = Module.Task_GetEntropy(task);
        const probability = entropy * 72;

        if (probability >= 100.0) {
            Module.Engine_PerformQuantumCollapse(task);
            triggerCollapseVisuals();
        }

        const cx = Module.Task_GetStressX(task);
        const cy = typeof Module.Task_GetStressY === 'function'
            ? Module.Task_GetStressY(task)
            : Module.Task_GetPositionX(task);
        const cz = typeof Module.Task_GetStressZ === 'function'
            ? Module.Task_GetStressZ(task)
            : (entropy * 10);
        const czDelta = Math.abs(cz - state.lastCz);

        updateTaskVisualization(entropy);
        updateMissionProgress(entropy, czDelta);

        if (Math.abs(entropy - state.lastEntropy) < 0.02) {
            state.streak += 1;
            state.combo = Math.min(4, 1 + state.streak / 150);
        } else {
            state.streak = Math.max(0, state.streak - 3);
            state.combo = Math.max(1, state.combo - 0.015);
        }

        addScore(1.8);
        if (state.ticks % 120 === 0) {
            grantXP(2, 'Sustained simulation');
        }

        updateTelemetry(entropy, probability, cx, cy, cz);

        state.lastEntropy = entropy;
        state.lastCz = cz;

        if (state.is3D && has3DRenderer()) {
            render3D(cx, cy, cz);
        } else {
            state.is3D = false;
            render2D(cx, cy);
        }
    }

    update();
}

async function initializeEngine() {
    const engineFactory = await resolveEngineFactory();
    if (!engineFactory) {
        captureFirstRuntimeFailure('MissingEngineFactory', 'window.PhysEngine is undefined');
        setBootState('OFFLINE MODE', 'Physics engine script was not found. The UI remains visible, but simulation data is unavailable.', true);
        startDiagnosticLoop('OFFLINE MODE');
        return;
    }

    try {
        setBootState('BOOTING', 'Graphics online. Connecting to physics engine...');
        const Module = await engineFactory({
            locateFile: (path) => (path.endsWith('.wasm') ? new URL(path, ENGINE_WASM_DIR_URL).href : path)
        });

        if (typeof Module.Task_Create !== 'function') {
            throw new Error('Task_Create is unavailable on the physics module');
        }

        if (Module.Task_Create.toString().includes('UnboundTypeError')) {
            await new Promise((resolve) => setTimeout(resolve, 100));
        }

        setReadyState();

        const task = Module.Task_Create();
        updateSliderReadouts();
        renderGameHUD();

        bindSliders(Module, task);
        bindToolbar(Module, task);
        startUpdateLoop(Module, task);
    } catch (error) {
        console.error(error);
        captureFirstRuntimeFailure('InitializeEngineCatch', error);
        setErrorState();
    }
}

async function initializeApp() {
    if (appStarted) {
        return;
    }
    appStarted = true;

    try {
        initGraphics();
    } catch (graphicsError) {
        console.error(graphicsError);
        captureFirstRuntimeFailure('InitGraphicsCatch', graphicsError);
        setBootState('VISUAL ERROR', 'Could not initialize graphics. The simulation is running in diagnostic mode.', true);
        setErrorState();
        return;
    }

    setBootState('BOOTING', 'Graphics online. Initializing simulation...');
    await initializeEngine();
}

window.addEventListener('error', (event) => {
    console.error(event.error || event.message);
    captureFirstRuntimeFailure('WindowError', event.error || event.message, {
        source: event.filename || '(unknown source)',
        line: event.lineno || 0,
        column: event.colno || 0
    });
    setBootState('ENGINE ERROR', event.message || 'Unexpected runtime failure', true);
});

window.addEventListener('unhandledrejection', (event) => {
    console.error(event.reason);
    captureFirstRuntimeFailure('UnhandledRejection', event.reason, {
        reasonType: typeof event.reason
    });
    setBootState('ENGINE ERROR', event.reason?.message || String(event.reason), true);
});

if (document.readyState === 'loading') {
    document.addEventListener('DOMContentLoaded', initializeApp, { once: true });
} else {
    initializeApp();
}
window.addEventListener('resize', resizeView);
