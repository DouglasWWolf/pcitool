//=================================================================================================
// PciDevice.cpp - Implements a generic class for mapping PCIe devices into user-space
//=================================================================================================
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include "PhysMem.h"


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

    // If mapping into user-space faile,d tell the caller
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
// unmap() - Checks to see if physical address space has been mapped into user-space [i.e., there
//           was a succesfull call to 'map()'], and if so, unmaps it
//=================================================================================================
void PhysMem::unmap()
{
    // If we have a valid user-space address, we need to unmap that memory
    if (userspaceAddr_)
    {
        munmap(userspaceAddr_, mappedSize_);
    }

    // Indicate that we no longer have any memory mapped
    userspaceAddr_ = nullptr;
    mappedSize_    = 0;
}
//=================================================================================================
