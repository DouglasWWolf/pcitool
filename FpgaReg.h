
//=================================================================================================
// FpgaReg.h - Defines a class that manages FPGA registers
//=================================================================================================
#pragma once
#include <stdint.h>
#include <map>
#include <string>



//=================================================================================================
// The AXI address of every FPGA register in the system
//=================================================================================================
enum fpgareg_t
{
    REG_PCIPROXY_ADDRH,
    REG_PCIPROXY_ADDRL,
    REG_PCIPROXY_DATA,
    REG_COUNT
};

enum fpgafld_t
{
    FLD_PCIPROXY_ADDRH_top,
    FLD_PCIPROXY_ADDRH_btm,
    FLD_PCIPROXY_ADDRH_mid,
    FLD_COUNT
};
//=================================================================================================



class FpgaReg
{
public:

    // Set the base address of the PCI region as mapped into user-space
    static void setUserspaceAddr(uint8_t* userspaceAddress);

    // Reads the file that defines the addresses and field info about AXI registers
    static void readDefinitions(std::string filename);

    // Constructor requires the AXI address of the register
    FpgaReg(fpgareg_t axiRegister);

    // Allow "regVariableName = <value>"
    FpgaReg&    operator=(uint32_t value) {write(value); return *this;};
    
    // Allow "uint32_t variableName = regVariableName"
    operator uint32_t() {return read();}

    // Reads the register (and internally saves the returned value);
    uint32_t    read();

    // Writes a value to the register
    void        write(uint32_t value);

    // After "setField()" operations, this will write the register to the FPGA
    void        flush();

    // Sets the value of a bit-field
    void        setField(fpgafld_t idx, uint32_t value, bool auto_flush=true);

    // Fetches the value of a bit-field
    uint32_t    getField(fpgafld_t idx, bool auto_read=true);

    // Returns the AXI address of this register
    uint32_t    axiAddress();


protected:

    // Field descriptor, describes a bit-field within a register
    struct field_desc_t {uint32_t axiAddr; uint32_t mask; uint32_t bitPos; uint32_t width;};

    // The base address of registers, as mapped into userspace
    static uint8_t* userspaceBaseAddress_;

    // This maps a REG_xxxx constant to an AXI address
    static std::map<fpgareg_t, int32_t> regMap_;

    // This maps a FLD_xxxx constant to a field-descriptor
    static std::map<fpgafld_t, field_desc_t> fldMap_;

    // The REG_xxxx constant that programmers use to identify a register
    fpgareg_t regIndex_;

    // This is the AXI address of this register
    uint32_t axiAddress_;

    // This is the value after the last read() or setField()
    uint32_t regValue_;

};