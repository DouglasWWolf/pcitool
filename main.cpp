#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <iostream>
#include "PciDevice.h"
#include "PhysMem.h"
#include "FpgaReg.h"
PciDevice pci;
PhysMem   mem;

#define ENTRIES (1024*1024)
uint32_t buffer[ENTRIES];

FpgaReg pciProxyAddrH(REG_PCIPROXY_ADDRH);

void process()
{
   for (int i=0;i<ENTRIES;++i) buffer[i] = i;

   pci.open(0x10ee, 0x903f);

   // Set the user-space address where AXI registers live
   FpgaReg::setUserspaceAddr(pci.resourceList()[0].baseAddr);


   uint32_t* dest = (uint32_t*)pci.resourceList()[2].baseAddr;
   
   *dest = 37;
   printf("dest = %i\n", *dest);
   exit(1);

   memcpy(dest, buffer, sizeof(buffer));
   exit(1);


   FpgaReg::readDefinitions("register.def");
   
   pciProxyAddrH = 0;
   pciProxyAddrH.setField(FLD_PCIPROXY_ADDRH_mid, 0xFFFFFFFF);
   uint32_t value = pciProxyAddrH;

   printf("AXI 0x%0x = 0x%x\n", pciProxyAddrH.axiAddress(), value);

   exit(1);

   if (!mem.map())
   {
      printf("Error : %s\n", mem.error());
      exit(1);
   }

   uint32_t* p = (uint32_t*)mem.vptr();

   *p = 0x1234F0F0;
   exit(1);


   #if 0
   auto bar = pci.resourceList();
   uint32_t* control = (uint32_t*)bar[0].baseAddr;

   while (true)
   {
      printf("on\n");
      *control = 0xFFFFFFFF;
      usleep(500000);
      
      printf("off\n");
      *control = 0;
      usleep(500000);
   }

   #endif



}


int main()
{
   try
   {
      process();
   }
   catch(const std::exception& e)
   {
      std::cerr << e.what() << '\n';
   }
}
