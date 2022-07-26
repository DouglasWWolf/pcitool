//=================================================================================================
// FpgaReg.cpp - Implements a class that manages FPGA registers
//=================================================================================================
#include <stdio.h>
#include <fstream>
#include <stdarg.h>
#include <vector>
#include <cstring>
#include "FpgaReg.h"
using namespace std;

// This is the userspace base address where AXI registers are mapped to
uint8_t* FpgaReg::userspaceBaseAddress_;

// This maps a REG_xxxx constant to an AXI address
map<fpgareg_t, int32_t> FpgaReg::regMap_;

const uint32_t UNMAPPED = 0xFFFFFFFF;

//=================================================================================================
// setUserspaceAddress() - Sets the base address (in user-space) where all AXI registers are 
//                         mapped to.
//=================================================================================================
void FpgaReg::setUserspaceAddr(uint8_t* userspaceAddress)
{    
    userspaceBaseAddress_= userspaceAddress;
}
//=================================================================================================


//=================================================================================================
// Constructor() 
//=================================================================================================
FpgaReg::FpgaReg(fpgareg_t regIndex)
{
    axiAddress_ = UNMAPPED;
    regIndex_   = regIndex;
    regValue_   = 0;
}
//=================================================================================================


//=================================================================================================
// axiAddress() - Returns the AXI address of this register
//=================================================================================================
uint32_t FpgaReg::axiAddress()
{
    // If we don't have an address yet, look it up
    if (axiAddress_ == UNMAPPED) axiAddress_ = regMap_[regIndex_];

    // Hand the caller the AXI address of this register
    return axiAddress_;
}
//=================================================================================================


//=================================================================================================
// read() - Reads this AXI register via the PCIe bus
//=================================================================================================
uint32_t FpgaReg::read()
{
    // Read the AXI register and save its value
    regValue_ =  *(uint32_t*)(userspaceBaseAddress_ + axiAddress()); 

    // Hand the saved value to the caller
    return regValue_;  
}
//=================================================================================================


