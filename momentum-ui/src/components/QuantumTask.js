import React, { useRef, useEffect, useState, useCallback } from 'react';
import * as THREE from 'three';

/**
 * QuantumTask – Renders a single physics task as a Three.js sphere and drives
 * its state from a physics Web Worker.
 *
 * Props:
 *   taskId      – Unique identifier for this task (string)
 *   title       – Display title
 *   difficulty  – Mass / difficulty value
 *   taskState   – { stressX, stressY, stressZ, entropy, ... } from the worker
 *   onRemove    – Callback to remove this task
 *   onApplyForce– (taskId, fx, fy, fz) send force to worker
 *   onCollapse  – (taskId) request quantum collapse in worker
 *   onCreateTask– (taskId, opts) create C++ Task in worker
 *   onDestroyTask–(taskId) destroy C++ Task in worker (1:1 lifecycle)
 */
const QuantumTask = ({
  taskId,
  title,
  difficulty,
  taskState,
  onRemove,
  onApplyForce,
  onCollapse,
  onCreateTask,
  onDestroyTask,
}) => {
  const mountRef = useRef(null);
  const threeRef = useRef(null);
  const [isDragging, setIsDragging] = useState(false);
  const dragStartRef = useRef(null);

  // ── 1. Lifecycle: create / destroy C++ Task in the worker ──────────────
  useEffect(() => {
    onCreateTask(taskId, { mass: difficulty });
    return () => {
      onDestroyTask(taskId);
    };
    // taskId and difficulty are stable for the lifetime of this component.
    // eslint-disable-next-line react-hooks/exhaustive-deps
  }, [taskId, difficulty]);

  // ── 2. Three.js scene (runs entirely on the main thread) ──────────────
  useEffect(() => {
    const container = mountRef.current;
    if (!container) return;

    const width = container.clientWidth || 300;
    const height = 200;

    const scene = new THREE.Scene();
    const camera = new THREE.PerspectiveCamera(75, width / height, 0.1, 100);
    const renderer = new THREE.WebGLRenderer({ alpha: true, antialias: true });
    renderer.setPixelRatio(window.devicePixelRatio);
    renderer.setSize(width, height);
    container.appendChild(renderer.domElement);

    const geo = new THREE.SphereGeometry(1, 32, 32);
    const mat = new THREE.MeshPhongMaterial({
      color: 0xff7700,
      emissive: 0xff3300,
    });
    const sphere = new THREE.Mesh(geo, mat);
    scene.add(sphere);

    const light = new THREE.PointLight(0xffffff, 1, 10);
    scene.add(light);
    camera.position.z = 5;

    const resizeObserver = new ResizeObserver((entries) => {
      const w = entries[0].contentRect.width || 300;
      renderer.setSize(w, height);
      camera.aspect = w / height;
      camera.updateProjectionMatrix();
    });
    resizeObserver.observe(container);

    // Store refs for the render loop and state-update effect.
    threeRef.current = { sphere, scene, camera, renderer };

    // Pure render loop – no physics calls, just redraw.
    let frameId;
    const animate = () => {
      frameId = requestAnimationFrame(animate);
      renderer.render(scene, camera);
    };
    animate();

    return () => {
      cancelAnimationFrame(frameId);
      resizeObserver.disconnect();
      geo.dispose();
      mat.dispose();
      renderer.dispose();
      if (container.contains(renderer.domElement)) {
        container.removeChild(renderer.domElement);
      }
      threeRef.current = null;
    };
  }, []);

  // ── 3. Map worker state → Three.js visuals ────────────────────────────
  useEffect(() => {
    if (!taskState || !threeRef.current) return;
    const { sphere } = threeRef.current;
    sphere.position.set(
      taskState.stressX * 0.1,
      taskState.stressY * 0.1,
      (taskState.stressZ - 25) * 0.1
    );
    sphere.scale.setScalar(1 + taskState.entropy * 2);
  }, [taskState]);

  // ── 4. Interactive Dynamics – pointer drag → normalised force ─────────
  // Scale factor converting normalised drag deltas to physics force magnitude.
  const FORCE_SCALE = 10;

  const handlePointerDown = useCallback((e) => {
    setIsDragging(true);
    const rect = e.currentTarget.getBoundingClientRect();
    dragStartRef.current = {
      x: ((e.clientX - rect.left) / rect.width) * 2 - 1,
      y: -(((e.clientY - rect.top) / rect.height) * 2 - 1),
    };
    e.currentTarget.setPointerCapture(e.pointerId);
  }, []);

  const handlePointerMove = useCallback(
    (e) => {
      if (!isDragging || !dragStartRef.current) return;
      const rect = e.currentTarget.getBoundingClientRect();
      const curX = ((e.clientX - rect.left) / rect.width) * 2 - 1;
      const curY = -(((e.clientY - rect.top) / rect.height) * 2 - 1);

      const fx = (curX - dragStartRef.current.x) * FORCE_SCALE;
      const fy = (curY - dragStartRef.current.y) * FORCE_SCALE;
      const fz = 0;

      onApplyForce(taskId, fx, fy, fz);
      dragStartRef.current = { x: curX, y: curY };
    },
    [isDragging, taskId, onApplyForce]
  );

  const handlePointerUp = useCallback((e) => {
    setIsDragging(false);
    dragStartRef.current = null;
    if (e?.currentTarget?.releasePointerCapture && e?.pointerId != null) {
      try { e.currentTarget.releasePointerCapture(e.pointerId); } catch { /* already released */ }
    }
  }, []);

  const handlePointerCancel = useCallback((e) => {
    setIsDragging(false);
    dragStartRef.current = null;
    if (e?.currentTarget?.releasePointerCapture && e?.pointerId != null) {
      try { e.currentTarget.releasePointerCapture(e.pointerId); } catch { /* already released */ }
    }
  }, []);

  // ── Derived display values ─────────────────────────────────────────────
  const entropy = taskState?.entropy?.toFixed(4) ?? '0.0000';
  const chaos = taskState
    ? Math.abs(
        taskState.stressX + taskState.stressY + taskState.stressZ
      ).toFixed(2)
    : '0.00';

  return (
    <div className="task-card">
      <div
        className="task-viz"
        ref={mountRef}
        onPointerDown={handlePointerDown}
        onPointerMove={handlePointerMove}
        onPointerUp={handlePointerUp}
        onPointerCancel={handlePointerCancel}
        style={{ touchAction: 'none' }}
      ></div>
      <div className="task-info">
        <h3>{title}</h3>
        <p>
          Entropy: <span className="neon-text">{entropy}</span>
        </p>
        <p>
          Chaos Level: <span className="orange-text">{chaos}</span>
        </p>
        <button onClick={() => onCollapse(taskId)}>Force Collapse</button>
        <button className="done-btn" onClick={onRemove}>
          Complete Task
        </button>
      </div>
    </div>
  );
};

export default QuantumTask;
