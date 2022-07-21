//=================================================================================================
// PciDevice.cpp - Implements a generic class for mapping PCIe devices into user-space
//=================================================================================================
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include <string.h>
#include <string>
#include <fstream>
#include "PhysMem.h"
using namespace std;

#define MALFORMED 0xFFFFFFFFFFFFFFFF

//=================================================================================================
// parseKMG() - Examines a string for a delimeter, and parses the integer immediately after 
//              the delimeter.  Looks for the character after the digits, expect to find a K, M,
//              or G (meaning Kilo, Mega, or Giga) and returns the parsed value.
//
//              Example:  4G = 0x1_0000_0000
//                        2K = 0x400
//                        3M = 0x30_0000
//
// If the delimieter is not found or the string is malformed in some way, returns -1
//=================================================================================================
static uint64_t parseKMG(const char delimeter, const char* ptr)
{
    // Look for the delimeter in the string the user gave us
    ptr = strchr(ptr, delimeter);

    // If the delimeter didn't exist, tell the caller
    if (ptr == nullptr) return MALFORMED;

    // Point to the character after the delimeter
    ++ptr;

    // Convert the ASCII digits that follow the delimeter to an integer
    int64_t value = strtol(ptr, 0, 0);

    // Skip over all of the ASCII digits
    while (*ptr >= '0' && *ptr <= '9') ++ptr;

    // Return the appropriate scaled integer value
    if (*ptr == 'K') return value * 1024;
    if (*ptr == 'M') return value * 1024 * 1024;
    if (*ptr == 'G') return value * 1024 * 1024 * 1024;

    // If we get here, there wasn't a K, M, or G after the numeric value
    return MALFORMED;
}
//=================================================================================================



//=================================================================================================
// map() - Maps the specified physical address into user-space
//
// Passed: physAddr = The physical address to map into user-space
//         size     = The size of the region to map, in bytes
//
// Returns: true on success, otherwise false
//=================================================================================================
bool PhysMem::map(uint64_t physAddr, size_t size)
{
    const char* filename = "/dev/mem";

    // These are the memory protection flags we'll use when mapping the device into memory
    const int protection = PROT_READ | PROT_WRITE;

    // Assume for a moment that this routine will succeed
    bool result = true;

    // Unmap any memory we may already have mapped
    unmap();

    // Open the /dev/mem device
    int fd = ::open(filename, O_RDWR| O_SYNC);

    // If that open failed, we're done here
    if (fd < 0)
    {
        sprintf(errorMsg_, "Can't open %s", filename);
        return false;        
    }

    // Map the memory
    void* ptr = mmap(0, size, protection, MAP_SHARED, fd, physAddr);

    // If mapping into user-space failed tell the caller
    if (ptr == MAP_FAILED)
    {
        result = false;
        sprintf(errorMsg_, "mmap failed");        
    }

    // Otherwise, that mapping succeeded.  Record the userspace address and region size
    else
    {
        userspaceAddr_ = ptr;        
        mappedSize_    = size;
    }


    // We're done with "/dev/mem"
    ::close(fd);

    // Tell the caller whether or not this succeeded
    return result;

}
//=================================================================================================


//=================================================================================================
// map() - Automatically maps the region defined with "memmap=" in /proc/cmdline
//=================================================================================================
bool PhysMem::map()
{
    string line;
    
    const char* filename = "/proc/cmdline";

    // Ensure that we don't have anything mapped
    unmap();

    // Open the specified file.  It will contain a line of ASCII data
    ifstream file(filename);

    // If we couldn't open the file, tell the caller
    if (!file.is_open())
    {
        sprintf(errorMsg_, "Can't open %s", filename);
        return false;        
    }
    
    // Fetch the first line of the file
    getline(file, line);

    // Look for "memmap=" in the command line
    const char* p = ::strstr(line.c_str(), "memmap=");

    // If we can't find "memmap=", something is awry
    if (p == nullptr)
    {
        sprintf(errorMsg_, "malformed %s", filename);
        return false;        
    }

    // Fetch the value after the '='
    uint64_t size = parseKMG('=', p);

    // Fetch the value after the '$'
    uint64_t physAddr = parseKMG('$', p);

    // If we couldn't parse one of those values, /proc/cmdline is malformed
    if (physAddr == MALFORMED || size == MALFORMED)
    {
        sprintf(errorMsg_, "malformed %s", filename);
        return false;        
    }

    // Now go map this physical address into user-space
    return map(physAddr, size);
}
//=================================================================================================


//=================================================================================================
// unmap() - Checks to see if physical address space has been mapped into user-space [i.e., there
//           was a succesfull call to 'map()'], and if so, unmaps it
//=================================================================================================
void PhysMem::unmap()
{
    // If we have a valid user-space address, we need to unmap that memory
    if (userspaceAddr_) munmap(userspaceAddr_, mappedSize_);

    // Indicate that we no longer have any memory mapped
    userspaceAddr_ = nullptr;
    mappedSize_    = 0;
}
//=================================================================================================
