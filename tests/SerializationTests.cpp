#include "physics/MomentumBridge.h"
#include "physics/Task.hpp"
#include <iostream>
#include <iomanip>
#include <cmath>
#include <cstring>
#include <string>

// Test result tracking
struct TestResults {
    int total = 0;
    int passed = 0;
    int failed = 0;
    
    void recordPass(const std::string& testName) {
        passed++;
        total++;
        std::cout << "  \xE2\x9C\x93 [PASS] " << testName << std::endl;
    }
    
    void recordFail(const std::string& testName, const std::string& reason = "") {
        failed++;
        total++;
        std::cout << "  \xE2\x9C\x97 [FAIL] " << testName;
        if (!reason.empty()) {
            std::cout << " - " << reason;
        }
        std::cout << std::endl;
    }
    
    void printSummary() const {
        std::cout << "\n" << std::string(60, '=') << std::endl;
        std::cout << "SERIALIZATION TEST SUMMARY" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
        std::cout << "Total:  " << total << std::endl;
        std::cout << "Passed: " << std::string(5, ' ') << "\033[32m" << passed << "\033[0m" << std::endl;
        std::cout << "Failed: " << std::string(5, ' ') << (failed > 0 ? "\033[31m" : "") 
                  << failed << "\033[0m" << std::endl;
        std::cout << std::string(60, '=') << std::endl;
    }
};

void testSerializeRoundTrip(TestResults& results) {
    std::cout << "\n[TEST] Serialize/Deserialize Round-Trip" << std::endl;
    
    try {
        Task* task = Task_Create();
        Task_SetPosition(task, 3.14, 2.71);
        Task_SetVelocity(task, -1.5, 0.75);
        Task_SetMass(task, 7.5);
        Task_SetStress(task, 10.0, 20.0, 30.0);
        
        // Run a few chaos steps to get non-zero entropy and stepCount
        for (int i = 0; i < 10; ++i) {
            Engine_UpdateChaos(task);
        }
        
        // Serialize
        const char* json = State_Serialize(task);
        std::cout << "    Serialized: " << json << std::endl;
        
        if (json == nullptr || std::strlen(json) == 0) {
            results.recordFail("Serialize Round-Trip", "Serialization returned empty string");
            Task_Destroy(task);
            return;
        }
        
        // Store original values
        double origPosX = Task_GetPositionX(task);
        double origPosY = Task_GetPositionY(task);
        double origVelX = Task_GetVelocityX(task);
        double origVelY = Task_GetVelocityY(task);
        double origMass = Task_GetMass(task);
        double origStressX = Task_GetStressX(task);
        double origStressY = Task_GetStressY(task);
        double origStressZ = Task_GetStressZ(task);
        double origEntropy = Task_GetEntropy(task);
        int origStepCount = Task_GetStepCount(task);
        
        // Create a new task and deserialize into it
        Task* restored = Task_Create();
        State_Deserialize(restored, json);
        
        // Compare all fields
        bool allMatch = true;
        auto check = [&](const std::string& field, double orig, double rest) {
            if (std::abs(orig - rest) > 1e-10) {
                std::cout << "    MISMATCH " << field << ": " << orig << " vs " << rest << std::endl;
                allMatch = false;
            }
        };
        
        check("posX", origPosX, Task_GetPositionX(restored));
        check("posY", origPosY, Task_GetPositionY(restored));
        check("velX", origVelX, Task_GetVelocityX(restored));
        check("velY", origVelY, Task_GetVelocityY(restored));
        check("mass", origMass, Task_GetMass(restored));
        check("stressX", origStressX, Task_GetStressX(restored));
        check("stressY", origStressY, Task_GetStressY(restored));
        check("stressZ", origStressZ, Task_GetStressZ(restored));
        check("entropy", origEntropy, Task_GetEntropy(restored));
        
        if (origStepCount != Task_GetStepCount(restored)) {
            std::cout << "    MISMATCH stepCount: " << origStepCount 
                      << " vs " << Task_GetStepCount(restored) << std::endl;
            allMatch = false;
        }
        
        Task_Destroy(task);
        Task_Destroy(restored);
        
        if (allMatch) {
            results.recordPass("Serialize Round-Trip");
        } else {
            results.recordFail("Serialize Round-Trip", "Field mismatch after round-trip");
        }
        
    } catch (const std::exception& e) {
        results.recordFail("Serialize Round-Trip", std::string(e.what()));
    }
}

void testSerializeNullSafety(TestResults& results) {
    std::cout << "\n[TEST] Serialize Null Safety" << std::endl;
    
    try {
        // Serialize null pointer
        const char* json = State_Serialize(nullptr);
        if (json != nullptr && std::strcmp(json, "{}") == 0) {
            std::cout << "    Null serialize returned: " << json << std::endl;
            results.recordPass("Serialize Null Safety");
        } else {
            results.recordFail("Serialize Null Safety", "Expected '{}' for null task");
        }
    } catch (const std::exception& e) {
        results.recordFail("Serialize Null Safety", std::string(e.what()));
    }
}

void testApplyForce(TestResults& results) {
    std::cout << "\n[TEST] Apply Force" << std::endl;
    
    try {
        Task* task = Task_Create();
        Task_SetMass(task, 2.0);
        
        double origVelX = Task_GetVelocityX(task);
        double origVelY = Task_GetVelocityY(task);
        
        // Apply force (fx=4, fy=6, fz=0)
        // Expected: velX += 4/2 = 2, velY += 6/2 = 3
        Task_ApplyForce(task, 4.0, 6.0, 0.0);
        
        double newVelX = Task_GetVelocityX(task);
        double newVelY = Task_GetVelocityY(task);
        
        std::cout << "    VelX: " << origVelX << " -> " << newVelX << std::endl;
        std::cout << "    VelY: " << origVelY << " -> " << newVelY << std::endl;
        
        bool xOk = std::abs(newVelX - (origVelX + 2.0)) < 1e-10;
        bool yOk = std::abs(newVelY - (origVelY + 3.0)) < 1e-10;
        
        Task_Destroy(task);
        
        if (xOk && yOk) {
            results.recordPass("Apply Force");
        } else {
            results.recordFail("Apply Force", "Velocity change does not match expected F/m");
        }
        
    } catch (const std::exception& e) {
        results.recordFail("Apply Force", std::string(e.what()));
    }
}

void testStepCount(TestResults& results) {
    std::cout << "\n[TEST] Step Count Tracking" << std::endl;
    
    try {
        Task* task = Task_Create();
        
        if (Task_GetStepCount(task) != 0) {
            results.recordFail("Step Count", "Initial step count not zero");
            Task_Destroy(task);
            return;
        }
        
        // Run 50 chaos steps
        for (int i = 0; i < 50; ++i) {
            Engine_UpdateChaos(task);
        }
        
        int count = Task_GetStepCount(task);
        std::cout << "    After 50 updates: stepCount = " << count << std::endl;
        
        Task_Destroy(task);
        
        if (count == 50) {
            results.recordPass("Step Count");
        } else {
            results.recordFail("Step Count", 
                "Expected 50, got " + std::to_string(count));
        }
        
    } catch (const std::exception& e) {
        results.recordFail("Step Count", std::string(e.what()));
    }
}

// -----------------------------------------------------------------------
// Task 9.3: Energy Serialization Tests
// Requirements: 18.1, 18.2, 18.3, 18.4
// -----------------------------------------------------------------------

void testEnergyFieldsInSerialization(TestResults& results) {
    std::cout << "\n[TEST] Energy Fields Present in Serialized JSON" << std::endl;

    try {
        Task* task = Task_Create();
        Task_SetPosition(task, 1.0, 5.0);
        Task_SetVelocity(task, 3.0, 4.0);
        Task_SetMass(task, 2.0);
        // Inject energy so fields are non-zero
        Energy_Inject(task, 50.0);

        const char* json = State_Serialize(task);
        std::string s(json);

        bool hasKE = s.find("\"kineticEnergy\"") != std::string::npos;
        bool hasPE = s.find("\"potentialEnergy\"") != std::string::npos;
        bool hasTE = s.find("\"totalEnergy\"") != std::string::npos;

        std::cout << "    JSON: " << s << std::endl;

        Task_Destroy(task);

        if (hasKE && hasPE && hasTE) {
            results.recordPass("Energy Fields in Serialization");
        } else {
            std::string reason;
            if (!hasKE) reason += "missing kineticEnergy; ";
            if (!hasPE) reason += "missing potentialEnergy; ";
            if (!hasTE) reason += "missing totalEnergy; ";
            results.recordFail("Energy Fields in Serialization", reason);
        }
    } catch (const std::exception& e) {
        results.recordFail("Energy Fields in Serialization", std::string(e.what()));
    }
}

void testEnergyDeserializationRestoresValues(TestResults& results) {
    std::cout << "\n[TEST] Deserialization Restores Energy Values" << std::endl;

    try {
        Task* task = Task_Create();
        Task_SetPosition(task, 2.0, 3.0);
        Task_SetVelocity(task, 1.5, 2.5);
        Task_SetMass(task, 1.0);
        Energy_Inject(task, 30.0);

        double origKE = Energy_GetKinetic(task);
        double origPE = Energy_GetPotential(task);
        double origTE = Energy_GetTotal(task);

        const char* json = State_Serialize(task);

        Task* restored = Task_Create();
        State_Deserialize(restored, json);

        double restoredKE = Energy_GetKinetic(restored);
        double restoredPE = Energy_GetPotential(restored);
        double restoredTE = Energy_GetTotal(restored);

        std::cout << "    KE: " << origKE << " -> " << restoredKE << std::endl;
        std::cout << "    PE: " << origPE << " -> " << restoredPE << std::endl;
        std::cout << "    TE: " << origTE << " -> " << restoredTE << std::endl;

        Task_Destroy(task);
        Task_Destroy(restored);

        bool keOk = std::abs(origKE - restoredKE) < 1e-10;
        bool peOk = std::abs(origPE - restoredPE) < 1e-10;
        bool teOk = std::abs(origTE - restoredTE) < 1e-10;

        if (keOk && peOk && teOk) {
            results.recordPass("Deserialization Restores Energy Values");
        } else {
            std::string reason;
            if (!keOk) reason += "KE mismatch; ";
            if (!peOk) reason += "PE mismatch; ";
            if (!teOk) reason += "TE mismatch; ";
            results.recordFail("Deserialization Restores Energy Values", reason);
        }
    } catch (const std::exception& e) {
        results.recordFail("Deserialization Restores Energy Values", std::string(e.what()));
    }
}

void testInconsistentEnergyTriggersRecalculation(TestResults& results) {
    std::cout << "\n[TEST] Inconsistent Energy Triggers Recalculation" << std::endl;

    try {
        // Build JSON with inconsistent totalEnergy (KE + PE != totalEnergy)
        // position.y = 0 so PE = 0; velocity = (3,4) mass=2 => KE = 0.5*2*25 = 25
        // We set totalEnergy = 999 (inconsistent)
        std::string badJson =
            "{\"posX\":0,\"posY\":0,\"velX\":3,\"velY\":4,\"mass\":2,"
            "\"stressX\":1,\"stressY\":1,\"stressZ\":1,\"entropy\":0,\"stepCount\":0,"
            "\"kineticEnergy\":25,\"potentialEnergy\":0,\"totalEnergy\":999}";

        Task* task = Task_Create();
        State_Deserialize(task, badJson.c_str());

        double ke = Energy_GetKinetic(task);
        double pe = Energy_GetPotential(task);
        double te = Energy_GetTotal(task);

        std::cout << "    KE=" << ke << " PE=" << pe << " TE=" << te << std::endl;

        // After recalculation, totalEnergy should equal KE + PE (not 999)
        bool consistent = std::abs(te - (ke + pe)) < 1e-9;
        bool notBadValue = std::abs(te - 999.0) > 1e-6;

        Task_Destroy(task);

        if (consistent && notBadValue) {
            results.recordPass("Inconsistent Energy Triggers Recalculation");
        } else {
            std::string reason;
            if (!consistent) reason += "totalEnergy != KE+PE after recalc; ";
            if (!notBadValue) reason += "totalEnergy still has bad value 999; ";
            results.recordFail("Inconsistent Energy Triggers Recalculation", reason);
        }
    } catch (const std::exception& e) {
        results.recordFail("Inconsistent Energy Triggers Recalculation", std::string(e.what()));
    }
}

int main() {
    std::cout << std::string(60, '=') << std::endl;
    std::cout << "PhysEngine Serialization & API Test Suite" << std::endl;
    std::cout << std::string(60, '=') << std::endl;
    
    TestResults results;
    
    testSerializeRoundTrip(results);
    testSerializeNullSafety(results);
    testApplyForce(results);
    testStepCount(results);
    testEnergyFieldsInSerialization(results);
    testEnergyDeserializationRestoresValues(results);
    testInconsistentEnergyTriggersRecalculation(results);
    
    results.printSummary();
    
    if (results.failed == 0) {
        std::cout << "\n\xE2\x9C\x85 All serialization tests passed!" << std::endl;
        return 0;
    } else {
        std::cout << "\n\xE2\x9D\x8C Some serialization tests failed!" << std::endl;
        return 1;
    }
}
