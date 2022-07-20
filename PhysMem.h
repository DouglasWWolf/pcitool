//=================================================================================================
// PhysMem.h - Defines a class that maps physical address space into user-space
//=================================================================================================
#pragma once
#include <stdint.h>
#include <stddef.h>

class PhysMem
{
public:

    // Constructor
    PhysMem() {userspaceAddr_ = nullptr; mappedSize_ = 0;}

    // No copy or assignment constructor - objects of this class can't be copied
    PhysMem (const PhysMem&) = delete;
    PhysMem& operator= (const PhysMem&) = delete;

    // Destructor, unmaps the memory space
    ~PhysMem() {unmap();}

    // Call this to map a region of physical address space into user-space
    bool    map(uint64_t physAddr, size_t size);

    // Unmaps the address space if one has been mapped
    void    unmap();

protected:

    // An ASCII error message if "open" fails
    char    errorMsg_[256];

    // If this is not null or (void*)(-1), it contains a pointer to the mapped addresses
    void*   userspaceAddr_;

    // This is the size of the address spaces that has been mapped into user-space
    size_t  mappedSize_;
};