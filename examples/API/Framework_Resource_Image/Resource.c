/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
  This example will show an image in the screen.

  In this example, it will init resource using vm_res_init , then load an image 
  using vm_res_get_image and show it in the screen, after show it, using
  vm_res_delete to release the image resource, finally using vm_res_release to release all
  Launch application, it will show image in the screen
*/

#include "vmtype.h" 
#include "vmsystem.h"
#include "vmchset.h" 
#include "vmstdlib.h" 
#include "vmlog.h" 
#include "vmres.h" 
#include "vmgraphic.h" 
#include "vmgraphic_font.h" 
#include "vmgraphic_image.h"
#include "vmdcl.h"
#include "vmdcl_pwm.h"
#include "ResID.h"

#if defined(__HDK_LINKIT_ASSIST_2502__)
#define VM_PIN_LCD_BG  VM_PIN_P1
#else
#error "Board not support"
#endif


/* control lcd backlight level. */
void lcd_backlight_level(VMUINT32 ulValue) {

	VM_DCL_HANDLE pwm_handle;
	VM_DCL_HANDLE gpio_handle;

	vm_dcl_pwm_set_clock_t pwm_clock;
	vm_dcl_pwm_set_counter_threshold_t pwm_config_adv;

	vm_dcl_config_pin_mode(VM_PIN_LCD_BG, VM_DCL_PIN_MODE_PWM);

	pwm_handle = vm_dcl_open(PIN2PWM(VM_PIN_LCD_BG),vm_dcl_get_owner_id());
	vm_dcl_control(pwm_handle,VM_PWM_CMD_START,0);
	pwm_config_adv.counter = 100;
	pwm_config_adv.threshold = ulValue;
	pwm_clock.source_clock = 0;
	pwm_clock.source_clock_division =3;
	vm_dcl_control(pwm_handle,VM_PWM_CMD_SET_CLOCK,(void *)(&pwm_clock));
	vm_dcl_control(pwm_handle,VM_PWM_CMD_SET_COUNTER_AND_THRESHOLD,(void *)(&pwm_config_adv));
	vm_dcl_close(pwm_handle);
}

/* Draw image to screen. */
static void image_draw(void) {

  void* data_ptr;
  VMUINT32 size;
  vm_graphic_frame_t frame;
  vm_graphic_frame_t* frame_blt_group[1];

  vm_graphic_point_t frame_position[1] = {0, 0};
  /* load image into memory */
  data_ptr = vm_res_get_image(ID_IMAGE_01, &size);
  if(data_ptr == NULL)
  {
    vm_log_info("get image failed");
    return;
  }
  /* draw memory image */
  frame.width = 240;
  frame.height = 240;
  frame.color_format = VM_GRAPHIC_COLOR_FORMAT_16_BIT;
  frame.buffer = (VMUINT8*)vm_malloc_dma(frame.width * frame.height * 2);
  frame.buffer_length = (frame.width * frame.height * 2);
  frame_blt_group[0] = &frame;
  vm_graphic_draw_solid_rectangle(&frame, 0,0, 240,240);
  vm_graphic_draw_image_memory(&frame,0,0,data_ptr,size,0);
  vm_graphic_blt_frame((const vm_graphic_frame_t**)frame_blt_group, frame_position, 1);
  /* delete image */
  vm_res_delete(ID_IMAGE_01);
}

/* The callback to be invoked by the system engine. */
void handle_sysevt(VMINT message, VMINT param) {
  switch (message) {
  case VM_EVENT_CREATE:
    vm_res_init(0); /* init resource with English language */
    break;
  case VM_EVENT_PAINT:
    image_draw();
    break;
  case VM_EVENT_QUIT:
    vm_res_release(); /* release resource */
    break;
  }
}

/* Entry point */
void vm_main(void){
  lcd_backlight_level(100); /* open backlight */
  vm_pmng_register_system_event_callback(handle_sysevt);
  lcd_st7789s_init();
}
