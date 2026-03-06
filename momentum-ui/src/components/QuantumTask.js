import React, { useRef, useEffect, useState } from 'react';
import * as THREE from 'three';

// Entropy threshold that triggers an automatic Quantum Collapse.
const ENTROPY_COLLAPSE_THRESHOLD = 1.5;

const QuantumTask = ({ engine, title, difficulty, onRemove }) => {
  const mountRef = useRef(null);
  const taskPtr = useRef(null);
  const [stats, setStats] = useState({ entropy: 0, chaos: 0 });

  useEffect(() => {
    // 1. Create the C++ Task instance.
    taskPtr.current = engine.Task_Create();
    engine.Task_SetMass(taskPtr.current, difficulty);

    // 2. Determine canvas dimensions from the parent container.
    const container = mountRef.current;
    const width = container.clientWidth || 300;
    const height = 200;

    // 3. Setup Three.js scene for this specific task.
    const scene = new THREE.Scene();
    const camera = new THREE.PerspectiveCamera(75, width / height, 0.1, 100);
    const renderer = new THREE.WebGLRenderer({ alpha: true, antialias: true });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(width, height);
    container.appendChild(renderer.domElement);

    const geo = new THREE.SphereGeometry(1, 32, 32);
    const mat = new THREE.MeshPhongMaterial({ color: 0xff7700, emissive: 0xff3300 });
    const sphere = new THREE.Mesh(geo, mat);
    scene.add(sphere);

    const light = new THREE.PointLight(0xffffff, 1, 10);
    scene.add(light);
    camera.position.z = 5;

    // 4. Resize handler – keeps the canvas in sync with its container.
    // ResizeObserver fires only when this container's size changes, so with N tasks
    // a window resize triggers exactly N targeted callbacks instead of N window listeners.
    const resizeObserver = new ResizeObserver((entries) => {
      const w = entries[0].contentRect.width || 300;
      renderer.setSize(w, height);
      camera.aspect = w / height;
      camera.updateProjectionMatrix();
    });
    resizeObserver.observe(container);

    // 5. Animation loop.
    let frameId;
    const animate = () => {
      frameId = requestAnimationFrame(animate);

      engine.Engine_UpdateChaos(taskPtr.current);

      const x = engine.Task_GetStressX(taskPtr.current);
      const y = engine.Task_GetStressY(taskPtr.current);
      const z = engine.Task_GetStressZ(taskPtr.current);
      const entropy = engine.Task_GetEntropy(taskPtr.current);

      // Map physics values to visuals.
      sphere.position.set(x * 0.1, y * 0.1, (z - 25) * 0.1);
      // Higher entropy → larger, more unstable sphere.
      sphere.scale.setScalar(1 + entropy * 2);

      setStats({
        entropy: entropy.toFixed(4),
        chaos: Math.abs(x + y + z).toFixed(2)
      });

      // Auto-Collapse: reset urgency when entropy exceeds the safe limit.
      if (entropy > ENTROPY_COLLAPSE_THRESHOLD) {
        engine.Engine_PerformQuantumCollapse(taskPtr.current);
      }

      renderer.render(scene, camera);
    };

    animate();

    // 6. Cleanup on unmount: cancel RAF, dispose Three.js resources, free C++ memory.
    return () => {
      cancelAnimationFrame(frameId);
      resizeObserver.disconnect();

      // Dispose Three.js GPU resources.
      geo.dispose();
      mat.dispose();
      renderer.dispose();
      if (container.contains(renderer.domElement)) {
        container.removeChild(renderer.domElement);
      }

      // Free the WASM-side Task allocation.
      if (taskPtr.current !== null) {
        engine.Task_Destroy(taskPtr.current);
        taskPtr.current = null;
      }
    };
  }, [engine, difficulty]);

  return (
    <div className="task-card">
      <div className="task-viz" ref={mountRef}></div>
      <div className="task-info">
        <h3>{title}</h3>
        <p>Entropy: <span className="neon-text">{stats.entropy}</span></p>
        <p>Chaos Level: <span className="orange-text">{stats.chaos}</span></p>
        <button onClick={() => engine.Engine_PerformQuantumCollapse(taskPtr.current)}>
          Force Collapse
        </button>
        <button className="done-btn" onClick={onRemove}>Complete Task</button>
      </div>
    </div>
  );
};

export default QuantumTask;