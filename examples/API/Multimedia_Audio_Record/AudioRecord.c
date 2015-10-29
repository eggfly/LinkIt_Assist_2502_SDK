/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
This example will record an AMR file.

In this example, it will record a file(vm_audio_record_start) named audio.amr. The file will be stored either on the SD card or in
the flash storage. During the recording, you can pause(vm_audio_record_pause), resume(vm_audio_record_resume), and stop
(vm_audio_record_stop) the recording.
When launching the example, you can send AT commands through the monitor tool to control the recording like below:

AT+[1000]Test01: begin record file
AT+[1000]Test02: pause record
AT+[1000]Test03: resume record
AT+[1000]Test04: stop record

After Test04 you will find a file named audio.amr in the flash storage or the SD card.

*/

#include <stdio.h>
#include "vmtype.h"
#include "vmsystem.h"
#include "vmcmd.h"
#include "vmlog.h"
#include "vmfs.h"
#include "vmaudio_record.h"
#include "vmchset.h"

#define COMMAND_PORT  1000 /* AT command port */
#define MAX_NAME_LEN 260 /* Max length of file name */

/* The callback function of recording. */
void audio_record_callback(VM_AUDIO_RECORD_RESULT result, void* userdata)
{
  switch (result) 
  {
      case VM_AUDIO_RECORD_ERROR_NO_SPACE:
      case VM_AUDIO_RECORD_ERROR:
    	/* not enough space */
        vm_audio_record_stop();
        break;
      default:
        vm_log_info("callback result = %d", result);
        break;
  }
}

/* Record a file. */
void audio_record()
{
  VMINT drv ;
  VMCHAR file_name[MAX_NAME_LEN];
  VMWCHAR w_file_name[MAX_NAME_LEN];
  VMINT result;
  
  /* If there is a removable letter (SD Card) use it, otherwise stored it in the flash storage (vm_fs_get_internal_drive_letter).  */
  drv = vm_fs_get_removable_drive_letter();
  if(drv <0)
  {
    drv = vm_fs_get_internal_drive_letter();
    if(drv <0)
    {
      vm_log_fatal("not find driver");
      return ;
    }
  }
  sprintf(file_name, "%c:\\audio.amr", drv);
  vm_chset_ascii_to_ucs2(w_file_name, MAX_NAME_LEN, file_name);
  /* Record an AMR file and save it in the root directory */
  result = vm_audio_record_start(w_file_name ,VM_AUDIO_FORMAT_AMR, audio_record_callback, NULL);
  if(result == VM_SUCCESS)
  {
    vm_log_info("record success");
  }
  else
  {
    vm_log_error("record failed");
  }
}

/* AT command callback which will be invoked when you send AT commands from the monitor tool */
static void at_callback(vm_cmd_command_t *param, void *user_data)
{
  if(strcmp("Test01",(char*)param->command_buffer) == 0)
  {
    /* start recording when following command is received: AT+[1000]Test01 */
    audio_record();
  }
  else if(strcmp("Test02",(char*)param->command_buffer) == 0)
  {
    /* pause recording when following command is received: AT+[1000]Test02 */
    vm_audio_record_pause();
  }
  else if(strcmp("Test03",(char*)param->command_buffer) == 0)
  {
    /* resume recording when following command is received: AT+[1000]Test03 */
    vm_audio_record_resume();
  }
  else if(strcmp("Test04",(char*)param->command_buffer) == 0)
  {
    /* stop recording when following command is received: AT+[1000]Test04 */
    vm_audio_record_stop();
  }
}

/* The callback to be invoked by the system engine. */
void handle_sysevt(VMINT message, VMINT param) {
  switch (message) {
  case VM_EVENT_CREATE:
    vm_cmd_open_port(COMMAND_PORT, at_callback, NULL);
    break;
  case VM_EVENT_QUIT:
    vm_cmd_close_port(COMMAND_PORT);
    break;
  }
}

/* Entry point */
void vm_main(void) {
  vm_pmng_register_system_event_callback(handle_sysevt);
}
