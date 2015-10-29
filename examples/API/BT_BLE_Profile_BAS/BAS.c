/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
  This example shows how to setup a BLE profile, specifically the BAS (Battery Service) profile.
*/
#include "vmtype.h" 
#include "vmchset.h"
#include "vmstdlib.h"
#include "vmfs.h"
#include "vmlog.h" 
#include "vmwdt.h"
#include "vmcmd.h" 
#include "vmbt_gatt.h"
#include "vmtimer.h"
#include "ResID.h"
#include "BAS.h"

void vm_main(void)
{
    gatt_init();
}


