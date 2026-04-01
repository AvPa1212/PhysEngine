/**
 * QuantumTask.tsx
 *
 * React component that visualises a single physics Task using Three.js.
 *
 * Each QuantumTask:
 *  1. Creates a C++ Task instance via the Emscripten-compiled WASM engine.
 *  2. Mounts a 200×200 Three.js WebGL canvas to display the task as a sphere.
 *  3. Runs an animation loop that calls Engine_UpdateChaos every frame,
 *     reads the Lorenz stress components, and maps them to the sphere's
 *     3-D position.  Entropy is mapped to the sphere's scale — a larger,
 *     more "swollen" sphere indicates a more disordered (higher entropy) task.
 *  4. Auto-collapses the wavefunction when entropy exceeds 1.5 nats, which
 *     resets entropy to 0 and produces a visual velocity kick.
 *
 * Props:
 *  - engine     — the loaded Emscripten module (returned by useMomentum).
 *  - title      — display name shown below the visualisation.
 *  - difficulty — mapped directly to the task's mass; higher difficulty
 *                 (more mass) means the task is harder to accelerate.
 *  - onRemove   — callback invoked when the user marks the task as complete.
 */
import React, { useRef, useEffect, useState } from 'react';
import * as THREE from 'three';

const QuantumTask = ({ engine, title, difficulty, onRemove }) => {
  /** DOM node that the Three.js renderer canvas will be appended to. */
  const mountRef = useRef(null);

  /** Opaque pointer to the C++ Task object managed by the WASM engine. */
  const taskPtr = useRef(null);

  /** Live physics readouts displayed in the info panel. */
  const [stats, setStats] = useState({ entropy: 0, chaos: 0 });

  useEffect(() => {
    // 1. Create the C++ Task instance and set its mass from the difficulty prop.
    taskPtr.current = engine.Task_Create();
    engine.Task_SetMass(taskPtr.current, difficulty);

    // 2. Setup Three.js for this specific task
    const scene = new THREE.Scene();
    // Perspective camera with a 75° field of view, 1:1 aspect ratio, and
    // near/far clipping planes at 0.1 and 100 units respectively.
    const camera = new THREE.PerspectiveCamera(75, 1, 0.1, 100);
    const renderer = new THREE.WebGLRenderer({ alpha: true, antialias: true });
    renderer.setSize(200, 200);
    mountRef.current.appendChild(renderer.domElement);

    // Sphere geometry: radius 1, 32 horizontal and 32 vertical segments.
    const geo = new THREE.SphereGeometry(1, 32, 32);
    // Phong material with an orange base and red emissive glow.
    const mat = new THREE.MeshPhongMaterial({ color: 0xff7700, emissive: 0xff3300 });
    const sphere = new THREE.Mesh(geo, mat);
    scene.add(sphere);
    // Point light positioned at the origin to illuminate the sphere.
    scene.add(new THREE.PointLight(0xffffff, 1, 10));
    camera.position.z = 5; // Pull the camera back so the sphere is in view.

    // 3. Animation Loop — called by requestAnimationFrame each display frame.
    let frameId;
    const animate = () => {
      // Advance the Lorenz chaos state by one fixed time-step.
      engine.Engine_UpdateChaos(taskPtr.current);
      
      // Read the updated Lorenz stress components (unbounded chaotic trajectory).
      const x = engine.Task_GetStressX(taskPtr.current);
      const y = engine.Task_GetStressY(taskPtr.current);
      const z = engine.Task_GetStressZ(taskPtr.current);
      const entropy = engine.Task_GetEntropy(taskPtr.current);

      // Map Physics to Visuals:
      // Scale the raw Lorenz coordinates down by 0.1 so they fit in view.
      // Subtract 25 from z to centre the attractor (its z mean is ~25).
      sphere.position.set(x * 0.1, y * 0.1, (z - 25) * 0.1);
      // Entropy makes task "grow" and become unstable — a scale of 1 + 2·S
      // makes high-entropy tasks visually prominent.
      sphere.scale.setScalar(1 + entropy * 2);
      
      // Update the React state for the info panel display.
      setStats({ 
        entropy: entropy.toFixed(4), 
        chaos: Math.abs(x + y + z).toFixed(2) 
      });

      renderer.render(scene, camera);
      // Schedule the next frame; store the ID so we can cancel on unmount.
      frameId = requestAnimationFrame(animate);

      // Auto-Collapse: If entropy is too high, the task "resets" its urgency.
      // This triggers QuantumEngine::collapse(), zeroing entropy and applying
      // a velocity kick that is visible as a sudden scale-down.
      if (entropy > 1.5) {
        engine.Engine_PerformQuantumCollapse(taskPtr.current);
      }
    };

    // Start the animation loop immediately after setup.
    animate();

    // Cleanup: free all resources when the component unmounts or the effect
    // re-runs (e.g. difficulty change) to prevent memory leaks and duplicate canvases.
    return () => {
      // Stop the render loop before disposing anything it touches.
      cancelAnimationFrame(frameId);

      // Free the C++ Task object allocated in the WASM heap.
      if (taskPtr.current) {
        engine.Task_Destroy(taskPtr.current);
        taskPtr.current = null;
      }

      // Release GPU/CPU memory held by Three.js objects.
      geo.dispose();
      mat.dispose();
      renderer.dispose();

      // Remove the canvas that was appended to the mount node.
      if (mountRef.current && renderer.domElement.parentNode === mountRef.current) {
        mountRef.current.removeChild(renderer.domElement);
      }
    };
  }, [engine, difficulty]); // Re-run setup if the engine module or difficulty changes.

  return (
    <div className="task-card">
      {/* Three.js canvas is injected here by the effect above. */}
      <div className="task-viz" ref={mountRef}></div>
      <div className="task-info">
        <h3>{title}</h3>
        {/* Live entropy readout — high values indicate imminent collapse. */}
        <p>Entropy: <span className="neon-text">{stats.entropy}</span></p>
        {/* Chaos level: L1 norm of the Lorenz state vector, roughly proportional
            to the distance from the attractor's centroid. */}
        <p>Chaos Level: <span className="orange-text">{stats.chaos}</span></p>
        {/* Manual collapse button — triggers wavefunction reduction immediately. */}
        <button onClick={() => engine.Engine_PerformQuantumCollapse(taskPtr.current)}>
          Force Collapse
        </button>
        {/* Complete button — removes this task card from the parent list. */}
        <button className="done-btn" onClick={onRemove}>Complete Task</button>
      </div>
    </div>
  );
};

export default QuantumTask;