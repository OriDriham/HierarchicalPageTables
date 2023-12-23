# Virtual Memory System

This project implements a virtual memory system with hierarchical page tables.\
The system consists of several components, each implemented in a separate file.


# File Descriptions

### Makefile:
Contains build instructions using the GNU Make utility.\
Defines compilation variables and rules for building the project.
  
### MemoryConstants.h:
Header file defining constants and types related to the virtual memory system.\
Includes the definition of word_t, various address widths, and other constants.
  
### PhysicalMemory.cpp:
Implementation file for the physical memory module.\
Manages the reading, writing, eviction, and restoration of pages in physical memory.\
Utilizes a vector for RAM and an unordered map for the swap file.
  
### PhysicalMemory.h:
Header file declaring functions for interacting with the physical memory module.\
Includes functions for reading, writing, evicting, and restoring pages in physical memory.
  
### VirtualMemory.cpp:
Implementation file for the virtual memory module.\
Implements functions for initializing virtual memory, translating virtual
addresses to physical addresses, reading from virtual memory, and writing to
virtual memory.\
Utilizes the PhysicalMemory module for managing the underlying physical memory.
  
### VirtualMemory.h:
Header file declaring functions for interacting with the virtual memory module.\
Includes functions for initializing virtual memory, reading from virtual
memory, and writing to virtual memory.


# Usage

To build the project, use the provided Makefile.\
Open a terminal in the project directory and run the following commands:

```sh
make          # Build the project
make clean    # Clean up object files and executables
make depend   # Update dependencies
make tar      # Create a tarball of the project files
```

The virtual memory system can be utilized by including the necessary header
files and linking against the compiled library.

Example:\
You can write the following main function:

```sh
#include "VirtualMemory.h"

int main() {
    // Example usage of virtual memory functions
    VMinitialize();
    word_t value;
    int success = VMread(0, &value);
    // Perform other virtual memory operations...

    return 0;
}
```
You can also use the SimpleTest.cpp file I included.

Please refer to the individual source files for detailed comments and
documentation on each module's functionality.
