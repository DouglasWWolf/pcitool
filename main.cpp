#include <unistd.h>
#include <stdio.h>
#include "PciDevice.h"

PciDevice pci;

int main()
{
   if (!pci.open(0x10ee, 0x903f, 1))
   {
      printf("Error : %s\n", pci.error());

   }
   
   uint32_t* control = (uint32_t*)pci.bar(0);

   while (true)
   {
      *control = 0xFFFFFFFF;
      usleep(500000);
      *control = 0;
      usleep(500000);
   }
}
