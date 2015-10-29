This example shows how to use firmware update API.

It uses vm_firmware_trigger_update() to trigger firmware update, calls vm_pwr_reboot() to reboot the board. After reboot, the bootloader will update the firmware and write result of the update process to file system. This example then read firmware update result from file system.

To use this example, put the firmware update package image.bin on C:\  drive. The image.bin will replace the board's old firmware.
