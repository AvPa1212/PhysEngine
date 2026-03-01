import React, { useRef, useEffect, useState } from 'react';
import * as THREE from 'three';

const QuantumTask = ({ engine, title, difficulty, onRemove }) => {
  const mountRef = useRef(null);
  const taskPtr = useRef(null);
  const [stats, setStats] = useState({ entropy: 0, chaos: 0 });

  useEffect(() => {
    // 1. Create the C++ Task instance
    taskPtr.current = engine.Task_Create();
    engine.Task_SetMass(taskPtr.current, difficulty);

    // 2. Setup Three.js for this specific task
    const scene = new THREE.Scene();
    const camera = new THREE.PerspectiveCamera(75, 1, 0.1, 100);
    const renderer = new THREE.WebGLRenderer({ alpha: true, antialias: true });
    renderer.setSize(200, 200);
    mountRef.current.appendChild(renderer.domElement);

    const geo = new THREE.SphereGeometry(1, 32, 32);
    const mat = new THREE.MeshPhongMaterial({ color: 0xff7700, emissive: 0xff3300 });
    const sphere = new THREE.Mesh(geo, mat);
    scene.add(sphere);
    scene.add(new THREE.PointLight(0xffffff, 1, 10));
    camera.position.z = 5;

    // 3. Animation Loop
    let frameId;
    const animate = () => {
      engine.Engine_UpdateChaos(taskPtr.current);
      
      const x = engine.Task_GetStressX(taskPtr.current);
      const y = engine.Task_GetStressY(taskPtr.current);
      const z = engine.Task_GetStressZ(taskPtr.current);
      const entropy = engine.Task_GetEntropy(taskPtr.current);

      // Map Physics to Visuals
      sphere.position.set(x * 0.1, y * 0.1, (z - 25) * 0.1);
      sphere.scale.setScalar(1 + entropy * 2); // Entropy makes task "grow" and become unstable
      
      setStats({ 
        entropy: entropy.toFixed(4), 
        chaos: Math.abs(x + y + z).toFixed(2) 
      });

      renderer.render(scene, camera);
      frameId = requestAnimationFrame(animate);

      // Auto-Collapse: If entropy is too high, the task "resets" its urgency
      if (entropy > 1.5) {
        engine.Engine_PerformQuantumCollapse(taskPtr.current);
      }
    };

    animate();
    return () => {
      cancelAnimationFrame(frameId);
      // We should ideally have a Task_Delete in the bridge to free memory
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