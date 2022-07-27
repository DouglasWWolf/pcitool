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

// This maps a FLD_xxxx constant to a field-descriptor
std::map<fpgafld_t, FpgaReg::field_desc_t> FpgaReg::fldMap_;

// Indicates that we don't yet know the AXI address of a register
static const uint32_t UNMAPPED = 0xFFFFFFFF;



//=================================================================================================
// throw_runtime() - Throws a std::runtime_error exception
//=================================================================================================
static void throw_runtime(const char* fmt, ...)
{
    char message[1024];
    va_list ap;

    va_start(ap, fmt);
    vsprintf(message, fmt, ap);
    va_end(ap);

    throw runtime_error(message);
}
//=================================================================================================




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
    // If the registerMap has entries, look up this register-index, otherwise, mark
    // this register as unmapped.
    axiAddress_ = regMap_.empty() ? UNMAPPED : regMap_[regIndex];
    
    // Save the index of this register for posterity
    regIndex_   = regIndex;

    // We don't yet know the real value of this FPGA register
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
    // Read the AXI register from the FPGA and save its value
    regValue_ =  *(uint32_t*)(userspaceBaseAddress_ + axiAddress()); 

    // Hand the saved value to the caller
    return regValue_;  
}
//=================================================================================================



//=================================================================================================
// write() - Writes a value to the AXI register via the PCIe bus
//=================================================================================================
void FpgaReg::write(uint32_t value)
{
    // Save the value of the register
    regValue_ = value;

    // Write this value to the AXI register in the FPGA
    *(uint32_t*)(userspaceBaseAddress_ + axiAddress()) = regValue_; 
}
//=================================================================================================


//=================================================================================================
// flush() - Writes the current value to the AXI register via the PCIe bus
//=================================================================================================
void FpgaReg::flush()
{
    // Write this value to the AXI register in the FPGA
    *(uint32_t*)(userspaceBaseAddress_ + axiAddress()) = regValue_; 
}
//=================================================================================================





void FpgaReg::setField(fpgafld_t fieldIndex, uint32_t value, bool auto_flush)
{
    auto it = fldMap_.find(fieldIndex); 

    // If we can't find this field index, it's a problem
    if (it == fldMap_.end())
    {
        throw_runtime("Missing AXI field index %u", fieldIndex);
    }

    // Get a convenient reference to the field-descriptor that matches this index
    auto& fd = it->second;

    // If the AXI address of the field descriptor doesn't match this register, complain!
    if (fd.axiAddr != axiAddress())
    {
        throw_runtime("Field idx %i: axi address mistmatch", fieldIndex);        
    }

    printf("Before: regValue_ = 0x%x\n", regValue_);
    printf("bitpos=%u  width = %u\n", fd.bitPos, fd.width);

    // Mask off the appropriate bits from our current register value
    regValue_ &= ~(fd.mask << fd.bitPos);

    // And "or in" the value of this bit-field
    regValue_ |= (value << fd.bitPos) & fd.mask;

    if (auto_flush) flush();

    printf("After: regValue_ = 0x%x\n", regValue_);


}
