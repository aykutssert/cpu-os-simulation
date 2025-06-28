# GTU-C312 Operating System Simulator

A custom CPU simulator with a cooperative operating system implementation, demonstrating low-level systems programming, thread management, and assembly-like instruction execution.

## 🎯 Project Overview

This project implements a complete operating system simulation for the GTU-C312 architecture - a custom CPU with a minimalist instruction set. The system features cooperative multitasking, system call handling, memory protection, and comprehensive debugging capabilities.

### Key Features
- **Custom CPU Architecture**: GTU-C312 with memory-mapped registers and specialized instruction set
- **Cooperative Operating System**: Written entirely in GTU-C312 assembly language
- **Thread Management**: Round-robin scheduling with context switching and thread isolation
- **System Call Interface**: PRN (print), HLT (halt), and YIELD operations
- **Memory Protection**: User/kernel mode separation with access control
- **Debug Framework**: Multiple debug modes for system analysis and development

## 🏗️ Architecture

### CPU Specifications
- **Memory**: 20,000 words with dedicated regions for OS and threads
- **Registers**: Memory-mapped control registers (PC, SP, Thread ID, etc.)
- **Instruction Set**: 12 core instructions including arithmetic, control flow, and system calls
- **Memory Layout**:
  - `0-20`: OS registers and control flags
  - `21-999`: OS data and thread management tables
  - `1000-1999`: Thread 1 workspace (Bubble Sort)
  - `2000-2999`: Thread 2 workspace (Linear Search)
  - `3000-3999`: Thread 3 workspace (Sum Calculator)

### Instruction Set
```assembly
SET, CPY, CPYI, CPYI2    # Data movement
ADD, ADDI, SUBI          # Arithmetic operations
JIF                      # Conditional branching
CALL, RET                # Subroutine handling
PUSH, POP                # Stack operations
SYSCALL                  # System calls (PRN/HLT/YIELD)
USER                     # Kernel to user mode transition
```

## 🧵 Thread Implementations

### Thread 1: Bubble Sort
Sorts a 4-element array `[4,2,3,1]` using bubble sort algorithm with optimized passes and cooperative yielding.

### Thread 2: Linear Search
Searches for a target value in an array using linear search with early termination and index reporting.

### Thread 3: Sum Calculator
Computes the sum of integers from 1 to N using iterative accumulation with progress printing.

## 🚀 Getting Started

### Prerequisites
- C++ compiler (g++ recommended)
- Make utility

### Building and Running
```bash
# Compile the simulator
make

# Run with different debug modes
./Simulate os.txt -D 0    # Memory dump at end
./Simulate os.txt -D 1    # Step-by-step memory dumps
./Simulate os.txt -D 2    # Interactive step-through
./Simulate os.txt -D 3    # Thread table dumps on context switches
```

### Debug Modes
- **Mode 0**: Complete execution with final memory state
- **Mode 1**: Memory dump after each instruction
- **Mode 2**: Interactive mode with manual stepping
- **Mode 3**: Thread table monitoring during context switches

## 🔧 Technical Implementation

### CPU Simulator (main.cpp)
- `GTUCPU` class encapsulating the entire simulation
- Instruction parsing and execution engine
- System call handling and mode transitions
- Thread state management and scheduling
- Memory protection and access control

### Operating System (os.txt)
- Boot sequence and system initialization
- Round-robin thread scheduler
- System call dispatcher and handlers
- Thread creation and management routines
- Context switching mechanisms

### Key Algorithms
- **Cooperative Scheduling**: Round-robin with voluntary yielding
- **System Call Handling**: Kernel/user mode transitions with parameter passing
- **Memory Management**: Protected address spaces with violation detection
- **Thread Synchronization**: Blocking/unblocking mechanisms for I/O operations

## 📊 System Behavior

The simulator demonstrates:
- **Concurrent Execution**: Multiple threads running cooperatively
- **Resource Sharing**: Controlled access to system resources
- **Fault Tolerance**: Memory protection and error handling
- **Performance Monitoring**: Instruction counting and timing analysis

## 🎓 Learning Outcomes

This project provides hands-on experience with:
- **CPU Architecture Design**: Custom instruction sets and execution models
- **Operating System Concepts**: Process management, scheduling, and system calls
- **Assembly Programming**: Low-level algorithm implementation
- **Systems Simulation**: Building and debugging complex system software
- **Concurrent Programming**: Thread coordination and synchronization

## 📁 Project Structure

```
├── main.cpp              # CPU simulator implementation
├── os.txt                # Operating system in GTU-C312 assembly
├── makefile              # Build configuration
└── README.md             # This file
```

## 🔍 Example Output

The simulator produces detailed execution traces showing:
- Thread scheduling decisions
- System call invocations and responses
- Memory state changes during execution
- Final results from each thread's computation

---

**Course**: Operating Systems (GTU-C312)  
**Term**: Spring 2025  
**Skills Demonstrated**: CPU Simulation, OS Development, Assembly Programming, Thread Management, Systems Architecture