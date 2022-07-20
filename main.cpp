#include <unistd.h>
#include <stdio.h>
#include "PciDevice.h"

PciDevice pci;

int main()
{
   if (!pci.open(0x10ee, 0x903f))
   {
      printf("Error : %s\n", pci.error());

   }
   
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
}
