/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
This example shows how to use the firmware update API.

It uses vm_firmware_trigger_update() to trigger a firmware update and calls vm_pwr_reboot() to reboot the board. 
After rebooting, the bootloader will update the firmware and write the result of the update process into the file system. 
This example then reads the firmware update result from file system.

To use this example, put the firmware update package image.bin onto the C:\ drive. The image.bin will then replace the board's old firmware.

*/
#include "vmtype.h" 
#include "vmlog.h"
#include "vmfs.h"
#include "vmsystem.h"
#include "vmfirmware.h"

VMINT firmware_read_update_status(){
    VMINT fd = -1;
    VMINT32 ret = 1;
    vm_firmware_update_status_t update_info;
    VMWCHAR path[100];
    VMUINT read = 0;
    vm_chset_ascii_to_ucs2(path, 100, "c:\\update_status");
    fd = vm_fs_open(path, VM_FS_MODE_READ,TRUE);
    if (fd >= 0){
        vm_fs_read(fd, (void *)&update_info, sizeof(update_info), &read);
        if (read == sizeof(update_info)){
            ret = update_info.error_code;	/* refer to VM_FIRMWARE_UPDATE_RESULT in vmfirmware.h*/
        }
        else{
            ret = 2;	/*incorrect file size */
        }
        vm_fs_close(fd);
    }
    else{
        ret = 1;	/* file could not be found */
    }
    return ret;
}


/* The callback to be invoked by the system engine. */
void handle_sysevt(VMINT message, VMINT param) {
    switch (message){
    case VM_EVENT_CREATE:
        break;
    case VM_EVENT_PAINT:{
        VMWCHAR path[100];
        VMINT fs = 0;
        VMCHAR flag = 0;
        VMCHAR write = 'a';
        VMINT write_size = 0;
        VMINT ret = 0;
        /* This file is a flag to not update the firmware after the firmware was updated and the board reboots.
           It's to prevent calling the update API in an endless loop. */
        vm_chset_ascii_to_ucs2(path, 100, "c:\\firmware.txt");
        fs = vm_fs_open(path,VM_FS_MODE_READ,FALSE);
        if(fs<0){
            fs = vm_fs_open(path,VM_FS_MODE_CREATE_ALWAYS_WRITE,FALSE);
        }
        if(fs>=0){
            vm_fs_read(fs,(void *)&flag,sizeof(VMCHAR),&write_size);
            if(flag==0){
                vm_fs_seek(fs,0,VM_FS_BASE_BEGINNING);
                vm_fs_write(fs,(void *)&write,sizeof(VMCHAR),&write_size);
            }
            vm_fs_close(fs);
            if(flag=='a'){
                vm_log_info("firmware:have firmware update");
                ret = firmware_read_update_status();
                vm_log_info("firmware:update result = %d ",ret);
                vm_fs_delete(path);
            }
            if(flag==0){
                vm_log_info("firmware update begin");
                vm_firmware_trigger_update();
                vm_log_info("firmware update end");
                vm_pwr_reboot();
            }
        }
    }
        break;
    default:
        break;
    }
}
/* Entry point */
void vm_main(void){
    /* register system events handler */
    vm_pmng_register_system_event_callback(handle_sysevt);
}
