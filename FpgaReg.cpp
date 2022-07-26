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
map<fpgareg_t, int32_t> regMap_;


//=================================================================================================
// setUserspaceAddress() - Sets the base address (in user-space) where all AXI registers are 
//                         mapped to.
//=================================================================================================
void FpgaReg::setUserspaceAddr(uint8_t* userspaceAddress)
{    
    userspaceBaseAddress_= userspaceAddress;
}
//=================================================================================================

