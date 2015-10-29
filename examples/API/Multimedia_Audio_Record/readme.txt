This example will record an AMR file.

In this example, it will record a file(vm_audio_record_start) named audio.amr. The file will be stored either on the SD card or in the flash storage. During the recording, you can pause(vm_audio_record_pause), resume(vm_audio_record_resume), and stop(vm_audio_record_stop) the recording. 
When launching the example, you can send AT commands through the monitor tool to control the recording like below:

AT+[1000]Test01: begin record file
AT+[1000]Test02: pause record
AT+[1000]Test03: resume record
AT+[1000]Test04: stop record

After Test04 you will find a file named audio.amr in the flash storage or the SD card.
