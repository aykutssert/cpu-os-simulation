#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <string>
#include <cstring>
#include <map>
#include <algorithm>
#include <cstdlib>

class GTUCPU {
private:
    std::vector<long long> memory;
    std::vector<std::string> instructions;
    std::map<long long, std::string> memoryInstructions;
    bool halted;
    int debugMode;
    bool kernelMode;
    
    
public:
    GTUCPU(int memorySize = 20000) : memory(memorySize, 0), halted(false), debugMode(0), 
                                     kernelMode(true) {
    }
    
    void setDebugMode(int mode) { debugMode = mode; }
    bool isHalted() const { return halted; }
    
    bool loadProgram(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) return false;
        
        std::string line;
        bool inDataSection = false;
        bool inInstructionSection = false;
        bool hasStackPointer = false;
        

        instructions.resize(1000);
        
        while (std::getline(file, line)) {
          
            line.erase(0, line.find_first_not_of(" \t"));
            line.erase(line.find_last_not_of(" \t") + 1);
            
            if (line.empty()) continue;
            
            if (line.find("Begin Data Section") != std::string::npos) {
                inDataSection = true;
                continue;
            }
            if (line.find("End Data Section") != std::string::npos) {
                inDataSection = false;
                continue;
            }
            if (line.find("Begin Instruction Section") != std::string::npos) {
                inInstructionSection = true;
                continue;
            }
            if (line.find("End Instruction Section") != std::string::npos) {
                inInstructionSection = false;
                continue;
            }
            
            if (inDataSection && !line.empty() && line[0] != '#') {
                std::istringstream iss(line);
                int address;
                long long value;
                
                if (iss >> address >> value) {
                    if (address >= 0 && address < (int)memory.size()) {
                        memory[address] = value;
                        if (address == 1) {
                            hasStackPointer = true;
                        }
                    }
                }
            }
            
            if (inInstructionSection && !line.empty() && line[0] != '#') {
                std::istringstream iss(line);
                int instructionIndex;
                if (iss >> instructionIndex) {
                    std::string rest;
                    std::getline(iss, rest);
                    
                    // Remove comment
                    size_t commentPos = rest.find('#');
                    if (commentPos != std::string::npos) {
                        rest = rest.substr(0, commentPos);
                    }
                    
                    // Trim whitespace
                    rest.erase(0, rest.find_first_not_of(" \t"));
                    rest.erase(rest.find_last_not_of(" \t") + 1);
                    
                    if (!rest.empty()) {
                        if (instructionIndex >= 1000) {
                            // Thread instruction
                            memoryInstructions[instructionIndex] = rest;
                        } else if (instructionIndex < (int)instructions.size()) {
                            // OS instruction
                            instructions[instructionIndex] = rest;
                        }
                    }
                }
            }
        }
        
        // If no stack pointer was set in data section, set default for OS programs
        if (!hasStackPointer && instructions[0].find("CALL") != std::string::npos) {
            // This looks like an OS program, set high SP
            memory[1] = 19999;
        }
        
        return true;
    }
    
    void execute() {
        if (halted) return;
        
        // Handle thread-based blocking
        if(memory[1020] > 0) {
            memory[1020]--;
            if(memory[1020] == 0 && memory[34] == 3) {
                setThreadState(1, 1); // Thread 1 ready
                memory[6] = 1; // Set context switch flag
            }
        }
        if(memory[2020] > 0) {
            memory[2020]--;
            if(memory[2020] == 0 && memory[44] == 3) {
                setThreadState(2, 1); // Thread 2 ready
                memory[6] = 1; // Set context switch flag
            }
        }
        if(memory[3020] > 0) {
            memory[3020]--;
            if(memory[3020] == 0 && memory[54] == 3) {
                setThreadState(3, 1); // Thread 3 ready
                memory[6] = 1; // Set context switch flag
            }
        }
        
     
        
        long long pc = memory[0];
        std::string instruction;
        
        // Get instruction
        if (pc >= 1000) {
            // Thread instruction from memory
            auto it = memoryInstructions.find(pc);
            if (it != memoryInstructions.end()) {
                instruction = it->second;
            } else {
                if (debugMode >= 1) {
                    std::cerr << "No thread instruction at address " << pc << std::endl;
                }
                // Thread ended, switch back to OS
                if (!kernelMode) {
                    int currentThread = memory[4];
                    if (currentThread != 0) {
                        setThreadState(currentThread, 0); // mark as inactive
                    }
                    
                    kernelMode = true;
                    memory[4] = 0; // Switch to OS thread
                    memory[0] = 100; // Return to scheduler
                    memory[6] = 1; // Set context switch flag
                    return;
                }
                // Check if all threads are done before halting
                bool anyActive = false;
                for (int i = 1; i <= 3; i++) { // Check threads 1-3
                    if (getThreadState(i) != 0) { // 0 = inactive
                        anyActive = true;
                        break;
                    }
                }
                if (!anyActive) {
                    
                        std::cerr << "All threads are inactive, halting CPU" << std::endl;
                    
                    halted = true;
                } else {
                    // Jump back to scheduler
                    memory[0] = 100;
                }
                return;
            }
        } else {
            // OS instruction from array
            if (pc < 0 || pc >= (long long)instructions.size() || instructions[pc].empty()) {
                if (debugMode >= 1) {
                    std::cerr << "No OS instruction at PC: " << pc << std::endl;
                }
                // Check if all threads are done
                bool anyActive = false;
                for (int i = 1; i <= 10; i++) {
                    if (isThreadActive(i)) {
                        anyActive = true;
                        break;
                    }
                }
                if (!anyActive) {
                    halted = true;
                } else {
                    // Jump back to scheduler
                    memory[0] = 100;
                }
                return;
            }
            instruction = instructions[pc];
        }
        
        // Debug mode checks
       bool isContextSwitchOrSyscall = false;
        if (debugMode == 3) {
            std::string trimmed = instruction;
            trimmed.erase(0, trimmed.find_first_not_of(" \t"));
            if (trimmed.find("SYSCALL") == 0 || trimmed.find("USER") == 0) {
                isContextSwitchOrSyscall = true;
            }
        
        }
        
        executeInstruction(instruction);
        
        memory[3]++; // instruction count
        
        // Update current thread's used time
        updateCurrentThreadUsedTime();
        
         if (debugMode == 1 || debugMode == 2) {
            dumpMemory();
        }

        if (debugMode == 2) {
            std::cout << "Press Enter...";
            std::cin.get();
        }
        
        if (debugMode == 3 && isContextSwitchOrSyscall ) {
                dumpThreadTable();           
        }

        bool allTerminated = true;
        for (int i = 1; i <= 3; ++i) {
            if (getThreadState(i) != 0) { // 0 = inactive
                allTerminated = false;
                break;
            }
        }
        if (allTerminated) {
            halted = true;
        }
    }
    
    void updateCurrentThreadUsedTime() {
        int currentThread = memory[4];
        if (currentThread >= 0 && currentThread <= 10) { // Include OS (thread 0)
            int base = getThreadTableBase(currentThread);
            if (base != -1 && memory[base + 1] > 0) { // If start time is set
                memory[base + 2] = memory[3] - memory[base + 1];
            }
        }
    }
    
    // Thread table functions according to OS code layout
    int getThreadTableBase(int threadId) {
        if (threadId == 0) {
            return 21; // OS thread at 21-30
        } else if (threadId >= 1 && threadId <= 10) {
            return 21 + (threadId * 10); // Thread i at 21 + i*10
        }
        return -1; // Invalid thread
    }
    
    void setThreadState(int threadId, int state) {
        int base = getThreadTableBase(threadId);
        if (base != -1) {
            memory[base + 3] = state; // State is at offset 3
        }
    }
    
    int getThreadState(int threadId) {
        int base = getThreadTableBase(threadId);
        if (base != -1) {
            return memory[base + 3]; // State is at offset 3
        }
        return 0; // inactive
    }
    
    bool isThreadActive(int threadId) {
        return getThreadState(threadId) != 0;
    }
    
   void dumpMemory() {
    std::cerr << "=== Memory Dump ===" << std::endl;
    std::cerr << "PC: " << memory[0] << ", SP: " << memory[1] 
              << ", Instructions: " << memory[3] << ", Mode: " << (kernelMode ? "KERNEL" : "USER") << std::endl;

    std::cerr << "Current Thread: " << memory[4] << ", Next Thread: " << memory[9] << std::endl;
    std::cerr << "Context Switch Flag: " << memory[6] << ", Syscall Type: " << memory[7] << std::endl;

    std::cerr << "Active threads: ";
    for (int i = 0; i <= 10; i++) {
        if (isThreadActive(i)) {
            std::cerr << i << "(" << getThreadState(i) << ") ";
        }
    }
    std::cerr << std::endl;

    for (int i = 0; i < std::min(100, (int)memory.size()); i++) {
        if (memory[i] != 0) {
            std::cerr << "Memory[" << i << "] = " << memory[i] << std::endl;
        }
    }


    if (isThreadActive(1)) {
        std::cerr << "Thread 1 Counter = " << memory[1001] << std::endl;
    }
    if (isThreadActive(2)) {
        std::cerr << "Thread 2 Found: ";
      
                std::cerr <<  memory[2008] << " ";
      
        std::cerr << std::endl;
    }
    if (isThreadActive(3)) {
        std::cerr << "Thread 3 Sum = " << memory[3001] << std::endl;
    }

    std::cerr << "===================" << std::endl;
}

    
    void dumpThreadTable() {
        std::cerr << "=== Thread Table Dump ===" << std::endl;
        std::cerr << "Current Mode: " << (kernelMode ? "KERNEL" : "USER") << std::endl;
        std::cerr << "Total Instructions: " << memory[3] << std::endl;
        
        // Display threads according to OS layout
        for (int i = 0; i < 4; i++) {
            int base = getThreadTableBase(i);
            if (base != -1) {
                std::string threadName = (i == 0) ? "OS" : "Thread " + std::to_string(i);
                std::cerr << threadName << ": ID=" << memory[base] 
                          << " StartTime=" << memory[base+1]
                          << " UsedTime=" << memory[base+2]
                          << " State=" << memory[base+3] 
                          << " (0=inactive, 1=ready, 2=running, 3=blocked)"
                          << " PC=" << memory[base+4] 
                          << " SP=" << memory[base+5] << std::endl;
            }
        }
        
        // Display scheduler state
        std::cerr << "Current Running Thread: " << memory[4] << std::endl;
        std::cerr << "Next Thread to Schedule: " << memory[9] << std::endl;
        std::cerr << "Context Switch Flag: " << memory[6] << std::endl;
        std::cerr << "Syscall Type: " << memory[7] << std::endl;
        std::cerr << "=========================" << std::endl;
    }

private:
    bool isValidMemoryAccess(int address) {
        if (address < 0 || address >= (int)memory.size()) {
            if (debugMode >= 1) {
                std::cerr << "Memory bounds violation: address " << address << std::endl;
            }
            return false;
        }
        
        // In user mode, can only access memory >= 1000 and registers 0-20
        if (!kernelMode && address < 1000 && address > 20) {
            if (debugMode >= 1) {
                std::cerr << "Memory access violation: User mode cannot access address " << address << std::endl;
            }
            
            // Shutdown violating thread
            int currentThread = memory[4];
            if (currentThread != 0) {
                setThreadState(currentThread, 0); // inactive
            }
            
            kernelMode = true;
            memory[4] = 0; // Switch to OS
            memory[0] = 100; // Return to scheduler
            memory[6] = 1; // Set context switch flag
            return false;
        }
        
        return true;
    }
    
    void executeInstruction(const std::string& instruction) {
        std::vector<std::string> tokens = tokenize(instruction);
        if (tokens.empty()) {
            memory[0]++;
            return;
        }
        
        std::string cmd = tokens[0];
        
        if (debugMode == 1 || debugMode == 2) {
            std::cerr << "Executing at PC=" << memory[0] << " (Mode: " << (kernelMode ? "KERNEL" : "USER") 
                      << ", Thread: " << memory[4] << "): " << instruction << std::endl;
        }

        if (cmd == "SET" && tokens.size() >= 3) {
            long long value = std::stoll(tokens[1]);
            int address = std::stoi(tokens[2]);
            if (isValidMemoryAccess(address)) {
                memory[address] = value;
                if (address == 0) {
                    std::cerr << "Setting PC tooooooooo " << value << std::endl;
                    // Setting PC - don't increment
                    return;
                }
            }
            memory[0]++;
        }
        else if (cmd == "CPY" && tokens.size() >= 3) {
            int source = std::stoi(tokens[1]);
            int dest = std::stoi(tokens[2]);
            if (isValidMemoryAccess(source) && isValidMemoryAccess(dest)) {
                memory[dest] = memory[source];
            }
            memory[0]++;
        }
        else if (cmd == "CPYI" && tokens.size() >= 3) {
            int ptrAddr = std::stoi(tokens[1]);
            int dest = std::stoi(tokens[2]);
            if (isValidMemoryAccess(ptrAddr) && isValidMemoryAccess(dest)) {
                int sourceAddr = (int)memory[ptrAddr];
                if (isValidMemoryAccess(sourceAddr)) {
                    memory[dest] = memory[sourceAddr];
                }
            }
            memory[0]++;
        }
        else if (cmd == "CPYI2" && tokens.size() >= 3) {
            int ptr1Addr = std::stoi(tokens[1]);
            int ptr2Addr = std::stoi(tokens[2]);
            if (isValidMemoryAccess(ptr1Addr) && isValidMemoryAccess(ptr2Addr)) {
                int sourceAddr = (int)memory[ptr1Addr];
                int destAddr = (int)memory[ptr2Addr];
                if (isValidMemoryAccess(sourceAddr) && isValidMemoryAccess(destAddr)) {
                    memory[destAddr] = memory[sourceAddr];
                }
            }
            memory[0]++;
        }
        else if (cmd == "ADD" && tokens.size() >= 3) {
            int address = std::stoi(tokens[1]);
            long long value = std::stoll(tokens[2]);
            if (isValidMemoryAccess(address)) {
                memory[address] += value;
            }
            memory[0]++;
        }
        else if (cmd == "ADDI" && tokens.size() >= 3) {
            int A1 = std::stoi(tokens[1]);
            int A2 = std::stoi(tokens[2]);
            if (isValidMemoryAccess(A1) && isValidMemoryAccess(A2)) {
                memory[A1] += memory[A2];
            }
            memory[0]++;
        }
        else if (cmd == "SUBI" && tokens.size() >= 3) {
            int operand1 = std::stoi(tokens[1]);
            int operand2 = std::stoi(tokens[2]);
            if (isValidMemoryAccess(operand1) && isValidMemoryAccess(operand2)) {
                memory[operand2] = memory[operand1] - memory[operand2];
            }
            memory[0]++;
        }
        else if (cmd == "JIF" && tokens.size() >= 3) {
            int conditionAddr = std::stoi(tokens[1]);
            int jumpTarget = std::stoi(tokens[2]);
            if (isValidMemoryAccess(conditionAddr)) {
                if (memory[conditionAddr] <= 0) {
                    memory[0] = jumpTarget;
                } else {
                    memory[0]++;
                }
            } else {
                memory[0]++;
            }
        }
        else if (cmd == "PUSH" && tokens.size() >= 2) {
            int addr = std::stoi(tokens[1]);
            if (isValidMemoryAccess(addr)) {
                long long sp = memory[1];
                if (sp > 21 && isValidMemoryAccess(sp)) {
                    memory[sp] = memory[addr];
                    memory[1] = sp - 1;
                }
            }
            memory[0]++;
        }
        else if (cmd == "POP" && tokens.size() >= 2) {
            int addr = std::stoi(tokens[1]);
            if (isValidMemoryAccess(addr)) {
                long long sp = memory[1] + 1;
                if (sp < (long long)memory.size() && isValidMemoryAccess(sp)) {
                    memory[addr] = memory[sp];
                    memory[1] = sp;
                }
            }
            memory[0]++;
        }
        else if (cmd == "CALL" && tokens.size() >= 2) {
            int instructionAddr = std::stoi(tokens[1]);
            long long sp = memory[1];
            if (sp > 21 && isValidMemoryAccess(sp)) {
                memory[sp] = memory[0] + 1; // Save return address
                memory[1] = sp - 1;
                memory[0] = instructionAddr;
            } else {
                memory[0]++;
            }
        }
        else if (cmd == "RET") {
            long long sp = memory[1] + 1;
            if (sp < (long long)memory.size() && isValidMemoryAccess(sp)) {
                memory[0] = memory[sp]; // Return to saved address
                memory[1] = sp;
            } else {
                memory[0]++;
            }
        }
        else if (cmd == "USER" && tokens.size() >= 2) {
            int addressPtr = std::stoi(tokens[1]);
            if (isValidMemoryAccess(addressPtr)) {
                // Save current PC before switching to user mode
                memory[10] = memory[0] + 1;
                
                kernelMode = false;
                long long jumpAddress = memory[addressPtr];
                
                // Update thread start time if this is first run
                int currentThread = memory[4];
                if (currentThread > 0 && currentThread <= 10) {
                    int base = getThreadTableBase(currentThread);
                    if (base != -1 && memory[base + 1] == 0) {
                        memory[base + 1] = memory[3]; // Set start time
                    }
                }
                
                // Simply jump to the address - OS has already set up context
                memory[0] = jumpAddress;
                
                if (debugMode >= 1) {
                    std::cerr << "USER: Switching to user mode, Thread=" << currentThread 
                              << ", PC=" << jumpAddress << std::endl;
                }
            } else {
                memory[0]++;
            }
        }
        else if (cmd == "SYSCALL" && tokens.size() >= 2) {

            
            if (tokens[1] == "PRN" && tokens.size() >= 3) {
                int param = std::stoi(tokens[2]);

                // } else 
                if (kernelMode) {
                    // OS is handling the PRN syscal
                
                    
                } else {
                    // User mode calling PRN - switch to OS
                    memory[7] = 1; // PRN syscall type
                    memory[8] = param; // PRN parameter
                    memory[11] = memory[4]; // Save current thread ID
                    memory[10] = memory[0] + 1; // Save return address
                    
                    if (isValidMemoryAccess(param)) {
                    std::cout << "PRN from thread " << memory[4] << ": " << memory[param] << std::endl;
                    }

                    int currentThread = memory[4];
                    if (currentThread != 0) {
                        setThreadState(currentThread, 3); // blocked
                         if(currentThread == 1) memory[1020] = 100;
                        else if(currentThread == 2) memory[2020] = 100;
                        else if(currentThread == 3) memory[3020] = 100;
                    }

                    // Switch to OS mode
                    kernelMode = true;
                    memory[4] = 0; // Switch to OS thread
                    memory[6] = 1; // Set context switch flag  
                    memory[0] = 25; // Jump to OS syscall handler
                return; // Don't increment PC
                }
                
                memory[0]++;
            }
            else if (tokens[1] == "HLT") {
                // Set syscall parameters for OS
                memory[7] = 2; // HLT syscall type
                memory[8] = memory[4]; // Which thread is calling HLT
                memory[11] = memory[4]; // Save current thread ID
                
                // Switch to OS mode
                kernelMode = true;
                memory[4] = 0;
                memory[6] = 1; // Set context switch flag  
                memory[0] = 25; // Jump to OS syscall handler
            }
            else if (tokens[1] == "YIELD") {
                // Set syscall parameters for OS
                memory[7] = 3; // YIELD syscall type
                memory[8] = memory[0] + 1; // Save return address
                memory[10] = memory[0] + 1; // Save return address
                  memory[11] = memory[4]; // Save current thread ID
                
                // Switch to OS mode
                kernelMode = true;
                memory[4] = 0;
                memory[6] = 1; // Set context switch flag
                memory[0] = 25; // Jump to OS syscall handler
            }
            else {
                memory[0]++;
            }
        }
        else if (cmd == "HLT") {
            halted = true;
        }
        else {
            // Unknown instruction - just advance PC
            if (debugMode >= 1) {
                std::cerr << "Unknown instruction: " << cmd << std::endl;
            }
            memory[0]++;
        }
    }
    
    std::vector<std::string> tokenize(const std::string& str) {
        std::vector<std::string> tokens;
        std::istringstream iss(str);
        std::string token;
        
        while (iss >> token) {
            if (token[0] == '#') break;
            // Remove commas that might be present
            if (token.back() == ',') {
                token.pop_back();
            }
            tokens.push_back(token);
        }
        return tokens;
    }
};

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <filename> [-D <debug_mode>]" << std::endl;
        std::cout << "Debug modes:" << std::endl;
        std::cout << "  0: Memory dump after halt" << std::endl;
        std::cout << "  1: Memory dump after each instruction" << std::endl;
        std::cout << "  2: Memory dump + wait for keypress after each instruction" << std::endl;
        std::cout << "  3: Thread table dump after context switches/syscalls" << std::endl;
        return 1;
    }
    
    GTUCPU cpu;
    int debugMode = 0;
    
    // Parse command line arguments
    for (int i = 2; i < argc; i++) {
        if (strcmp(argv[i], "-D") == 0 && i + 1 < argc) {
            debugMode = std::stoi(argv[i + 1]);
            break;
        }
    }
    
    cpu.setDebugMode(debugMode);
    
    if (!cpu.loadProgram(argv[1])) {
        std::cerr << "Error: Cannot load program file: " << argv[1] << std::endl;
        return 1;
    }
    
    std::cerr << "Starting CPU execution..." << std::endl;
    
    // Main execution loop
    while (!cpu.isHalted()) {
        cpu.execute();
    }
    
    // Debug mode 0: dump memory after halt
    if (debugMode == 0) {
        cpu.dumpMemory();
    }
    
    std::cerr << "Program execution completed." << std::endl;
    return 0;
}

    