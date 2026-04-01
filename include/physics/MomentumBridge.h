/**
 * @file MomentumBridge.h
 * @brief C ABI and Emscripten-compatible public interface for the physics engine.
 *
 * This header declares the exported C functions that allow external callers
 * (Python scripts, JavaScript via WebAssembly, or any C-compatible language)
 * to create and control Task objects without knowing their internal layout.
 *
 * All functions are wrapped with MOMENTUM_API which expands to the correct
 * export decoration for Windows (__declspec(dllexport)) or POSIX/Emscripten
 * (visibility("default")).
 *
 * The companion MomentumBridge.cpp provides the implementations through the
 * Bridge namespace and, when compiled with Emscripten, also registers all
 * functions with the EMSCRIPTEN_BINDINGS macro so they are callable from
 * JavaScript as Module.Task_Create(), etc.
 */
#pragma once

// Forward declaration so we don't need to include the full Task definition here
struct Task;

// Cross-platform export macros
#ifdef _WIN32
    #define MOMENTUM_API __declspec(dllexport)
#else
    #define MOMENTUM_API __attribute__((visibility("default")))
#endif

extern "C" {
    // -----------------------------------------------------------------------
    // Task Lifecycle
    // -----------------------------------------------------------------------

    /**
     * @brief Allocates and default-initialises a new Task on the heap.
     * @return Pointer to the newly created Task; the caller is responsible
     *         for eventually passing it to Task_Destroy().
     *
     * Changed from void* to Task* to satisfy Emscripten Bindings
     */
    MOMENTUM_API Task* Task_Create();

    /**
     * @brief Frees the heap memory owned by @p taskPtr.
     * @param taskPtr Pointer previously returned by Task_Create().
     *                Passing nullptr is safe and has no effect.
     */
    MOMENTUM_API void Task_Destroy(Task* taskPtr);

    // -----------------------------------------------------------------------
    // Task Configuration — setters for initial conditions
    // -----------------------------------------------------------------------

    /**
     * @brief Sets the 2-D world-space position of the task.
     * @param taskPtr  Target task. Passing nullptr is safe and has no effect.
     * @param x        New X position (metres).
     * @param y        New Y position (metres).
     */
    MOMENTUM_API void Task_SetPosition(Task* taskPtr, double x, double y);

    /**
     * @brief Sets the 2-D velocity of the task.
     * @param taskPtr  Target task. Passing nullptr is safe and has no effect.
     * @param vx       New X velocity component (m/s).
     * @param vy       New Y velocity component (m/s).
     */
    MOMENTUM_API void Task_SetVelocity(Task* taskPtr, double vx, double vy);

    /**
     * @brief Sets the inertial mass of the task.
     * @param taskPtr  Target task. Passing nullptr is safe and has no effect.
     * @param mass     Mass in kg (should be > 0 to avoid division-by-zero).
     */
    MOMENTUM_API void Task_SetMass(Task* taskPtr, double mass);

    /**
     * @brief Sets the Lorenz stress state of the task.
     * @param taskPtr  Target task. Passing nullptr is safe and has no effect.
     * @param sx       Initial Lorenz X component.
     * @param sy       Initial Lorenz Y component.
     * @param sz       Initial Lorenz Z component.
     */
    MOMENTUM_API void Task_SetStress(Task* taskPtr, double sx, double sy, double sz);

    // -----------------------------------------------------------------------
    // Task State Retrieval — getters for current simulation values
    // -----------------------------------------------------------------------

    /** @brief Returns the current X position of the task (metres). */
    MOMENTUM_API double Task_GetPositionX(Task* taskPtr);

    /** @brief Returns the current Y position of the task (metres). */
    MOMENTUM_API double Task_GetPositionY(Task* taskPtr);

    /** @brief Returns the current X velocity component (m/s). */
    MOMENTUM_API double Task_GetVelocityX(Task* taskPtr);

    /** @brief Returns the current Y velocity component (m/s). */
    MOMENTUM_API double Task_GetVelocityY(Task* taskPtr);

    /** @brief Returns the inertial mass of the task (kg). */
    MOMENTUM_API double Task_GetMass(Task* taskPtr);

    /** @brief Returns the current Lorenz X stress component. */
    MOMENTUM_API double Task_GetStressX(Task* taskPtr);

    /** @brief Returns the current Lorenz Y stress component. */
    MOMENTUM_API double Task_GetStressY(Task* taskPtr);

    /** @brief Returns the current Lorenz Z stress component. */
    MOMENTUM_API double Task_GetStressZ(Task* taskPtr);

    /**
     * @brief Returns the current Shannon entropy of the task's quantum state.
     *
     * Entropy is a combined value: the quantum contribution from the wavefunction
     * (updated by ThermodynamicsEngine) plus the chaos contribution from the
     * Lorenz displacement (accumulated by ChaosEngine).
     */
    MOMENTUM_API double Task_GetEntropy(Task* taskPtr);

    /**
     * @brief Returns the number of integration steps completed since task creation.
     */
    MOMENTUM_API int Task_GetStepCount(Task* taskPtr);

    // -----------------------------------------------------------------------
    // Force Application
    // -----------------------------------------------------------------------

    /**
     * @brief Applies an instantaneous impulse to the task's velocity.
     *
     * The velocity is updated as Δv = F / m.  The z-component of the force
     * is accepted for API symmetry but ignored because the simulation is 2-D.
     *
     * @param taskPtr  Target task.
     * @param fx       Force X component.
     * @param fy       Force Y component.
     * @param fz       Force Z component (ignored).
     */
    MOMENTUM_API void Task_ApplyForce(Task* taskPtr, double fx, double fy, double fz);

    // -----------------------------------------------------------------------
    // Engine Operations — single-step physics updates
    // -----------------------------------------------------------------------

    /**
     * @brief Runs one RK4 classical-mechanics integration step on the task.
     *
     * Equivalent to calling ClassicalEngine::integrateRK4() directly.
     * Use this for fine-grained control when the full SimulationEngine loop
     * is not needed.
     */
    MOMENTUM_API void Engine_IntegrateClassical(Task* taskPtr);

    /**
     * @brief Advances the Lorenz chaos attractor state by one time-step.
     *
     * Also increments task.stepCount so the step counter stays accurate
     * when using this function independently of Engine_IntegrateClassical.
     */
    MOMENTUM_API void Engine_UpdateChaos(Task* taskPtr);

    // -----------------------------------------------------------------------
    // Quantum Operations
    // -----------------------------------------------------------------------

    /**
     * @brief Returns the probability [0, 1] that the wavefunction will collapse.
     *
     * Delegates to QuantumEngine::calculateCollapseProbability().  High entropy
     * → high probability.  Safe to call every frame for threshold-based logic.
     */
    MOMENTUM_API double Task_GetCollapseProbability(Task* taskPtr);

    /**
     * @brief Collapses the wavefunction to its dominant basis state.
     *
     * Delegates to QuantumEngine::collapse() and then applies a small upward
     * velocity kick (visual feedback).  Resets entropy to 0.
     */
    MOMENTUM_API void Engine_PerformQuantumCollapse(Task* taskPtr);

    // -----------------------------------------------------------------------
    // State Serialization — JSON round-trip for save/restore
    // -----------------------------------------------------------------------

    /**
     * @brief Serialises the task's core state to a JSON string.
     *
     * The returned pointer is valid until the next call to State_Serialize on
     * the same thread (backed by a thread_local buffer).  Copy the string
     * before calling again if you need to retain it.
     *
     * @param taskPtr  Task to serialise.
     * @return Null-terminated JSON string, e.g.
     *         {"posX":0.0,"posY":0.0,"velX":1.0,...}
     */
    MOMENTUM_API const char* State_Serialize(Task* taskPtr);

    /**
     * @brief Deserialises a JSON string and updates matching fields in the task.
     *
     * Only fields present in @p json are updated; missing keys leave the
     * corresponding task fields unchanged.  This allows partial updates
     * without silently zeroing unspecified state.
     *
     * @param taskPtr  Task to update.
     * @param json     Null-terminated JSON string produced by State_Serialize.
     */
    MOMENTUM_API void State_Deserialize(Task* taskPtr, const char* json);
}