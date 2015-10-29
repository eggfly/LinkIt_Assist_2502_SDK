/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
  This example shows how to setup a BLE profile,
  specifically the DIS (Device Information Service) profile.
*/
#include "vmtype.h" 
#include "vmchset.h"
#include "vmstdlib.h"
#include "vmfs.h"
#include "vmlog.h" 
#include "vmwdt.h"
#include "vmcmd.h" 
#include "vmbt_gatt.h"
#include "vmbt_cm.h"
#include "vmtimer.h"
#include "ResID.h"
#include "DIS.h"

void vm_main(void)
{
    dis_check_bt_on_off();
}
