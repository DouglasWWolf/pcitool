//=================================================================================================
// PciDevice.cpp - Implements a generic class for mapping PCIe devices into user-space
//=================================================================================================
#include <unistd.h>
#include <stdio.h>
#include <sys/mman.h>
#include <sys/fcntl.h>
#include "PhysMem.h"


bool PhysMem::map(uint64_t address, size_t size)
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


    // Tell the caller whether or not this succeeded
    return result;

}


//=================================================================================================
// unmap() - Checks to see if physical address space has been mapped into user-space [i.e., there
//           was a succesfull call to 'open()'], and if so, unmaps it
//=================================================================================================
void PhysMem::unmap()
{
    if (userspaceAddr_ && userspaceAddr_ != MAP_FAILED)
    {
        munmap(userspaceAddr_, mappedSize_);
        userspaceAddr_ = nullptr;
        mappedSize_    = 0;
    }
}
//=================================================================================================
