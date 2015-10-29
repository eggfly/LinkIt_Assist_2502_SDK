/*
  This example code is in public domain.

  This example code is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
*/

/*
  This sample shows how to display the text on Nokia 5110.
  In this sample, it will show "Wish you prosperity!" on the screen, please connect the Nokia 5110 to Assist as below:
  DC  <--> GPIO 29
  CE  <--> GPIO 43
  RST <--> GPIO 19
  CLK <--> GPIO 27
  DIN <--> GPIO 28
*/

#include "vmtype.h" 
#include "vmboard.h"
#include "vmsystem.h"
#include "vmlog.h" 
#include "vmcmd.h" 
#include "vmdcl.h"
#include "vmdcl_gpio.h"
#include "vmthread.h"
#include "vmdcl_spi.h"

#include "ResID.h"
#include "DCL_SPI_Nokia5110.h"


/* PIN for Nokia5110 */
#define LCD_DC(a)        pcd8544_5110_dc(a)
#define LCD_CE(a)        pcd8544_5100_ce(a)
#define LCD_RST(a)       pcd_8544_5110_rst(a)

#define LCD_DC_PIN_NAME   29
#define LCD_CE_PIN_NAME   43
#define LCD_RST_PIN_NAME  19
#define LCD_SCLK_PIN_NAME 27
#define LCD_SDIN_PIN_NAME 28

#define HIGH              VM_TRUE
#define LOW               VM_FALSE


/* SPI handle */
VM_DCL_HANDLE g_spi_handle = VM_DCL_HANDLE_INVALID;

/* SPI data transfer buffer */
VM_READ_BUFFER* g_spi_read_data;
VM_WRITE_BUFFER* g_spi_write_data;


/* 12*12 Chinese character data structure */
typedef struct
{
   VMUCHAR Index[2];  /* 1 character contain two bytes */
   VMUCHAR Msk[24];   /* 1 character code contain 24 bytes */
}GB12_t;

/* Chinese character table */
GB12_t const g_gb_12_data[] =
{
"AA",{0x10,0x94,0x54,0xBF,0x14,0xD4,0x14,0x3F,0x54,0x94,0x90,0x00,0x01,0x00,0x02,0x01,0x04,0x07,0x01,0x02,0x01,0x02,0x00,0x00},
"BB",{0x82,0x82,0xBA,0xAA,0xEA,0xAF,0xAA,0xEA,0xBA,0x82,0x82,0x00,0x00,0x00,0x07,0x04,0x04,0x04,0x04,0x04,0x07,0x00,0x00,0x00},
"CC",{0x08,0x0E,0x88,0x78,0x2F,0xE8,0x28,0xA9,0x6A,0x08,0x08,0x00,0x04,0x02,0x05,0x04,0x02,0x02,0x01,0x02,0x02,0x04,0x04,0x00},
"DD",{0xFF,0x01,0xFD,0x01,0xFF,0x04,0x84,0x44,0x34,0xFF,0x04,0x00,0x04,0x02,0x01,0x01,0x02,0x05,0x00,0x04,0x04,0x07,0x00,0x00}
};


/* ASCII table */
VMUCHAR font6x8[][6] =
{
{ 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 },   /* sp */
{ 0x00, 0x00, 0x00, 0x2f, 0x00, 0x00 },   /* ! */
{ 0x00, 0x00, 0x07, 0x00, 0x07, 0x00 },   /* " */
{ 0x00, 0x14, 0x7f, 0x14, 0x7f, 0x14 },   /* # */
{ 0x00, 0x24, 0x2a, 0x7f, 0x2a, 0x12 },   /* $ */
{ 0x00, 0x62, 0x64, 0x08, 0x13, 0x23 },   /* % */
{ 0x00, 0x36, 0x49, 0x55, 0x22, 0x50 },   /* & */
{ 0x00, 0x00, 0x05, 0x03, 0x00, 0x00 },   /* ' */
{ 0x00, 0x00, 0x1c, 0x22, 0x41, 0x00 },   /* ( */
{ 0x00, 0x00, 0x41, 0x22, 0x1c, 0x00 },   /* ) */
{ 0x00, 0x14, 0x08, 0x3E, 0x08, 0x14 },   /* * */
{ 0x00, 0x08, 0x08, 0x3E, 0x08, 0x08 },   /* + */
{ 0x00, 0x00, 0x00, 0xA0, 0x60, 0x00 },   /* , */
{ 0x00, 0x08, 0x08, 0x08, 0x08, 0x08 },   /* - */
{ 0x00, 0x00, 0x60, 0x60, 0x00, 0x00 },   /* . */
{ 0x00, 0x20, 0x10, 0x08, 0x04, 0x02 },   /* / */
{ 0x00, 0x3E, 0x51, 0x49, 0x45, 0x3E },   /* 0 */
{ 0x00, 0x00, 0x42, 0x7F, 0x40, 0x00 },   /* 1 */
{ 0x00, 0x42, 0x61, 0x51, 0x49, 0x46 },   /* 2 */
{ 0x00, 0x21, 0x41, 0x45, 0x4B, 0x31 },   /* 3 */
{ 0x00, 0x18, 0x14, 0x12, 0x7F, 0x10 },   /* 4 */
{ 0x00, 0x27, 0x45, 0x45, 0x45, 0x39 },   /* 5 */
{ 0x00, 0x3C, 0x4A, 0x49, 0x49, 0x30 },   /* 6 */
{ 0x00, 0x01, 0x71, 0x09, 0x05, 0x03 },   /* 7 */
{ 0x00, 0x36, 0x49, 0x49, 0x49, 0x36 },   /* 8 */
{ 0x00, 0x06, 0x49, 0x49, 0x29, 0x1E },   /* 9 */
{ 0x00, 0x00, 0x36, 0x36, 0x00, 0x00 },   /* : */
{ 0x00, 0x00, 0x56, 0x36, 0x00, 0x00 },   /*    */
{ 0x00, 0x08, 0x14, 0x22, 0x41, 0x00 },   /* < */
{ 0x00, 0x14, 0x14, 0x14, 0x14, 0x14 },   /* = */
{ 0x00, 0x00, 0x41, 0x22, 0x14, 0x08 },   /* > */
{ 0x00, 0x02, 0x01, 0x51, 0x09, 0x06 },   /* ? */
{ 0x00, 0x32, 0x49, 0x59, 0x51, 0x3E },   /* @ */
{ 0x00, 0x7C, 0x12, 0x11, 0x12, 0x7C },   /* A */
{ 0x00, 0x7F, 0x49, 0x49, 0x49, 0x36 },   /* B */
{ 0x00, 0x3E, 0x41, 0x41, 0x41, 0x22 },   /* C */
{ 0x00, 0x7F, 0x41, 0x41, 0x22, 0x1C },   /* D */
{ 0x00, 0x7F, 0x49, 0x49, 0x49, 0x41 },   /* E */
{ 0x00, 0x7F, 0x09, 0x09, 0x09, 0x01 },   /* F */
{ 0x00, 0x3E, 0x41, 0x49, 0x49, 0x7A },   /* G */
{ 0x00, 0x7F, 0x08, 0x08, 0x08, 0x7F },   /* H */
{ 0x00, 0x00, 0x41, 0x7F, 0x41, 0x00 },   /* I */
{ 0x00, 0x20, 0x40, 0x41, 0x3F, 0x01 },   /* J */
{ 0x00, 0x7F, 0x08, 0x14, 0x22, 0x41 },   /* K */
{ 0x00, 0x7F, 0x40, 0x40, 0x40, 0x40 },   /* L */
{ 0x00, 0x7F, 0x02, 0x0C, 0x02, 0x7F },   /* M */
{ 0x00, 0x7F, 0x04, 0x08, 0x10, 0x7F },   /* N */
{ 0x00, 0x3E, 0x41, 0x41, 0x41, 0x3E },   /* O */
{ 0x00, 0x7F, 0x09, 0x09, 0x09, 0x06 },   /* P */
{ 0x00, 0x3E, 0x41, 0x51, 0x21, 0x5E },   /* Q */
{ 0x00, 0x7F, 0x09, 0x19, 0x29, 0x46 },   /* R */
{ 0x00, 0x46, 0x49, 0x49, 0x49, 0x31 },   /* S */
{ 0x00, 0x01, 0x01, 0x7F, 0x01, 0x01 },   /* T */
{ 0x00, 0x3F, 0x40, 0x40, 0x40, 0x3F },   /* U */
{ 0x00, 0x1F, 0x20, 0x40, 0x20, 0x1F },   /* V */
{ 0x00, 0x3F, 0x40, 0x38, 0x40, 0x3F },   /* W */
{ 0x00, 0x63, 0x14, 0x08, 0x14, 0x63 },   /* X */
{ 0x00, 0x07, 0x08, 0x70, 0x08, 0x07 },   /* Y */
{ 0x00, 0x61, 0x51, 0x49, 0x45, 0x43 },   /* Z */
{ 0x00, 0x00, 0x7F, 0x41, 0x41, 0x00 },   /* [ */
{ 0x00, 0x55, 0x2A, 0x55, 0x2A, 0x55 },   /* 55 */
{ 0x00, 0x00, 0x41, 0x41, 0x7F, 0x00 },   /* ] */
{ 0x00, 0x04, 0x02, 0x01, 0x02, 0x04 },   /* ^ */
{ 0x00, 0x40, 0x40, 0x40, 0x40, 0x40 },   /* _ */
{ 0x00, 0x00, 0x01, 0x02, 0x04, 0x00 },   /* ' */
{ 0x00, 0x20, 0x54, 0x54, 0x54, 0x78 },   /* a */
{ 0x00, 0x7F, 0x48, 0x44, 0x44, 0x38 },   /* b */
{ 0x00, 0x38, 0x44, 0x44, 0x44, 0x20 },   /* c */
{ 0x00, 0x38, 0x44, 0x44, 0x48, 0x7F },   /* d */
{ 0x00, 0x38, 0x54, 0x54, 0x54, 0x18 },   /* e */
{ 0x00, 0x08, 0x7E, 0x09, 0x01, 0x02 },   /* f */
{ 0x00, 0x18, 0xA4, 0xA4, 0xA4, 0x7C },   /* g */
{ 0x00, 0x7F, 0x08, 0x04, 0x04, 0x78 },   /* h */
{ 0x00, 0x00, 0x44, 0x7D, 0x40, 0x00 },   /* i */
{ 0x00, 0x40, 0x80, 0x84, 0x7D, 0x00 },   /* j */
{ 0x00, 0x7F, 0x10, 0x28, 0x44, 0x00 },   /* k */
{ 0x00, 0x00, 0x41, 0x7F, 0x40, 0x00 },   /* l */
{ 0x00, 0x7C, 0x04, 0x18, 0x04, 0x78 },   /* m */
{ 0x00, 0x7C, 0x08, 0x04, 0x04, 0x78 },   /* n */
{ 0x00, 0x38, 0x44, 0x44, 0x44, 0x38 },   /* o */
{ 0x00, 0xFC, 0x24, 0x24, 0x24, 0x18 },   /* p */
{ 0x00, 0x18, 0x24, 0x24, 0x18, 0xFC },   /* q */
{ 0x00, 0x7C, 0x08, 0x04, 0x04, 0x08 },   /* r */
{ 0x00, 0x48, 0x54, 0x54, 0x54, 0x20 },   /* s */
{ 0x00, 0x04, 0x3F, 0x44, 0x40, 0x20 },   /* t */
{ 0x00, 0x3C, 0x40, 0x40, 0x20, 0x7C },   /* u */
{ 0x00, 0x1C, 0x20, 0x40, 0x20, 0x1C },   /* v */
{ 0x00, 0x3C, 0x40, 0x30, 0x40, 0x3C },   /* w */
{ 0x00, 0x44, 0x28, 0x10, 0x28, 0x44 },   /* x */
{ 0x00, 0x1C, 0xA0, 0xA0, 0xA0, 0x7C },   /* y */
{ 0x00, 0x44, 0x64, 0x54, 0x4C, 0x44 },   /* z */
{ 0x14, 0x14, 0x14, 0x14, 0x14, 0x14 }    /* horizontal line */
};

void lcd_init(void);
void lcd_clear(void);
void l5110_all(void);
void l5110_spi_deinit(void);
void lcd_delay_ms(VMUINT t);
void lcd_set_xy(VMUINT8 X, VMUINT8 Y);
void lcd_write_byte(VMUINT8 DATA, VMUINT8 COMMAND);
void lcd_write_english_string(VMUINT8 X,VMUINT8 Y,VMCHAR *s);
void lcd_write_chinese(VMUINT8 x, VMUINT8 y, VMCHAR *hz);
void lcd_write_chinese_string(VMUINT8 x, VMUINT8 y, VMCHAR *string);
VMBOOL spi_write_register(VMUINT8 Addr, VMUINT8 Val);

/* system event handler */
void handle_sysevt(VMINT message, VMINT param);

/* entry function */
void vm_main(void) 
{
    /* register system events handler */
	vm_pmng_register_system_event_callback(handle_sysevt);
}


/* system event handler */
void handle_sysevt(VMINT message, VMINT param)
{
    switch (message)
    {
		case VM_EVENT_CREATE:
			l5110_all();
			break;

		case VM_EVENT_QUIT:
			l5110_spi_deinit();
			break;
    }
}


void lcd_delay_ms(VMUINT T)
{
	VMUINT MS = T;
	vm_thread_sleep(MS);
}


void pcd_8544_5110_rst(VMBOOL ENABLE)
{
	VM_DCL_HANDLE gpio_handle;

	gpio_handle = vm_dcl_open(VM_DCL_GPIO, LCD_RST_PIN_NAME);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);

	if(ENABLE)
	{
		vm_dcl_control(gpio_handle,VM_DCL_GPIO_COMMAND_WRITE_HIGH,0);
	}
	else
	{
		vm_dcl_control(gpio_handle,VM_DCL_GPIO_COMMAND_WRITE_LOW,0);
	}

	vm_dcl_close(gpio_handle);
}


void pcd8544_5100_ce(VMBOOL ENABLE)
{
	VM_DCL_HANDLE gpio_handle;

	gpio_handle = vm_dcl_open(VM_DCL_GPIO, LCD_CE_PIN_NAME);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);

	if(ENABLE)
	{
		vm_dcl_control(gpio_handle,VM_DCL_GPIO_COMMAND_WRITE_HIGH,0);
	}
	else
	{
		vm_dcl_control(gpio_handle,VM_DCL_GPIO_COMMAND_WRITE_LOW,0);
	}

	vm_dcl_close(gpio_handle);
}


void pcd8544_5110_dc(VMBOOL ENABLE)
{
	VM_DCL_HANDLE gpio_handle;

	gpio_handle = vm_dcl_open(VM_DCL_GPIO, LCD_DC_PIN_NAME);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_0, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);

	if(ENABLE)
	{
		vm_dcl_control(gpio_handle,VM_DCL_GPIO_COMMAND_WRITE_HIGH,0);
	}
	else
	{
		vm_dcl_control(gpio_handle,VM_DCL_GPIO_COMMAND_WRITE_LOW,0);
	}

	vm_dcl_close(gpio_handle);
}


/* 5110 RESET control */
void lcd_reset(void)
{
	LCD_RST(HIGH);
	lcd_delay_ms(5);
	LCD_RST(LOW);
	lcd_delay_ms(10);
	LCD_RST(HIGH);
	lcd_delay_ms(5);
}

/* Initial nokia5110 LCD */
void lcd_init(void)
{
	lcd_reset();

	LCD_CE(LOW);             /* disable LCD */
	lcd_delay_ms(10);

	LCD_CE(HIGH);            /* enable LCD */
	lcd_delay_ms(10);

	lcd_write_byte(0x21, 0); /* ext cmd setup LCD mode */

	lcd_write_byte(0xc8, 0); /* setup offset voltage */

	lcd_write_byte(0x06, 0); /* temperature correction */

	lcd_write_byte(0x13, 0); /* 1:48 */

	lcd_write_byte(0x20, 0); /* base cmd */

	lcd_clear();             /* clear LCD */

	lcd_write_byte(0x0c, 0); /* setup display mode, normal display */

	LCD_CE(LOW);             /* disable LCD */
}

/* Clear LCD */
void lcd_clear(void)
{
	VMUINT i;

	lcd_write_byte(0x0c, 0);
	lcd_write_byte(0x80, 0);

	for (i=0; i<504; i++)
		lcd_write_byte(0, 1);
}


/* cursor positioning column / row. */
void lcd_set_xy(VMUINT8 X, VMUINT8 Y)
{
	lcd_write_byte(0x40 | Y, 0);/* column */
	lcd_write_byte(0x80 | X, 0);/* row */
}


/* Write 1 byte */
void LCD_WRITE_CHAR(VMUINT8 c)
{
	VMUINT8 line;
	c -= 32;

	for (line=0; line<6; line++)
		lcd_write_byte(font6x8[c][line], 1); /* from ACSII table read byte, then write to LCD. */
}


/* Write english string */
void lcd_write_english_string(VMUINT8 X,VMUINT8 Y,VMCHAR *s)
{
	lcd_set_xy(X,Y);/* cursor positioning */

	while (*s)
	{
		LCD_WRITE_CHAR(*s);
		s++;
	}
}


/* Write 1 chinese word */
void lcd_write_chinese(VMUINT8 x, VMUINT8 y, VMCHAR *hz)
{
	VMUINT8 k,i;

	for(k=0;k<sizeof(g_gb_12_data)/sizeof(g_gb_12_data[0]);k++)  /* find chinese table */
	{
		if(hz[0] == g_gb_12_data[k].Index[0] && hz[1] == g_gb_12_data[k].Index[1])
			break;
	}

	lcd_set_xy(x,y);    /* cursor positioning */

	for(i=0;i<12;i++)   /* write the first part word */
		lcd_write_byte(g_gb_12_data[k].Msk[i],1);

	lcd_set_xy(x,y+1);  /* cursor positioning next row */

	for(i=12;i<24;i++)  /* write the next section word */
		lcd_write_byte(g_gb_12_data[k].Msk[i],1);
}

/* Write chinese string */
void lcd_write_chinese_string(VMUINT8 x, VMUINT8 y, VMCHAR *string)
{
	VMUINT8 i=0;

	while(string[i])
	{
		lcd_write_chinese(x,y,&string[i]);
		x=x+12;
		i=i+2;
	}
}

/* Write 1 byte */
void lcd_write_byte(VMUINT8 DATA, VMUINT8 COMMAND)
{
	LCD_CE(LOW);

	if(COMMAND == 0)
		LCD_DC(LOW);
	else
		LCD_DC(HIGH);

	spi_write_register(DATA, 0);

	LCD_CE(HIGH);
}

/* 5110 entry */
void l5110_all(void)
{
	L5110_SPI_INIT();  /* initial SPI */
	lcd_init();        /* initial LCD */
	lcd_clear();

    lcd_write_english_string(0,0,"Wish you prosperity!");
    lcd_write_chinese_string(0,2,"AABBCCDD");
}


/* 5110 SPI initial */
VMBOOL L5110_SPI_INIT(void)
{
	VM_DCL_STATUS status;
	VM_DCL_HANDLE gpio_handle;
	vm_dcl_spi_config_parameter_t conf_data;

	/* SPI_MOSI gpio config */
	gpio_handle = vm_dcl_open(VM_DCL_GPIO, LCD_SDIN_PIN_NAME);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_4, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);

	/* SPI_SCL gpio config */
	gpio_handle = vm_dcl_open(VM_DCL_GPIO, LCD_SCLK_PIN_NAME);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_MODE_4, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_ENABLE_PULL, NULL);
	vm_dcl_control(gpio_handle, VM_DCL_GPIO_COMMAND_SET_PULL_HIGH, NULL);

	if (VM_DCL_HANDLE_INVALID == g_spi_handle)
	{
		vm_log_info("[5110.vxp][L5110_SPI_INIT] g_spi_handle is null");
		g_spi_handle = vm_dcl_open(VM_DCL_SPI_PORT1, 0);
	}

	if (VM_DCL_HANDLE_INVALID == g_spi_handle)
	{
		vm_log_info("[5110.vxp][L5110_SPI_INIT] g_spi_handle invalid");
		return VM_FALSE;
	}

	conf_data.clock_high_time = 5;
	conf_data.clock_low_time = 5;
	conf_data.cs_hold_time = 15;
	conf_data.cs_idle_time = 15;
	conf_data.cs_setup_time= 15;
	conf_data.clock_polarity = VM_DCL_SPI_CLOCK_POLARITY_0;
	conf_data.clock_phase = VM_DCL_SPI_CLOCK_PHASE_0;
	conf_data.rx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
	conf_data.tx_endian = VM_DCL_SPI_ENDIAN_LITTLE;
	conf_data.rx_msbf = VM_DCL_SPI_MSB_FIRST;
	conf_data.tx_msbf = VM_DCL_SPI_MSB_FIRST;

	status = vm_dcl_control(g_spi_handle,VM_DCL_SPI_CONTROL_SET_CONFIG_PARAMETER,(void *)&conf_data);

	g_spi_write_data = (VM_WRITE_BUFFER*)vm_malloc_dma(sizeof(VM_WRITE_BUFFER));
	g_spi_read_data = (VM_READ_BUFFER*)vm_malloc_dma(sizeof(VM_READ_BUFFER));

	if (VM_DCL_STATUS_OK != status)
	{
		vm_log_info("[5110.vxp][L5110_SPI_INIT] spi ioctl set config para fail. status: %d", status);
		return VM_FALSE;
	}

	return VM_TRUE;
}

/* 5110 SPI deinit */
void l5110_spi_deinit(void)
{
	if (g_spi_handle != VM_DCL_HANDLE_INVALID)
	{
		vm_dcl_close(g_spi_handle);
	}
}

/* 5110 SPI write */
VMBOOL spi_write_register(VMUINT8 Addr, VMUINT8 Val)
{

	vm_dcl_spi_control_read_write_t write_and_read;
	VM_DCL_STATUS status;

	g_spi_write_data->write_buffer[0] = Addr;

	write_and_read.read_data_ptr= g_spi_read_data->read_buffer;
	write_and_read.read_data_length=0;

	write_and_read.write_data_ptr= g_spi_write_data->write_buffer;
	write_and_read.write_data_length=1;

	vm_dcl_control(g_spi_handle,VM_DCL_SPI_CONTROL_WRITE_AND_READ,(void *)&write_and_read);

	if (VM_DCL_STATUS_OK != status)
	{
		vm_log_info("5110 spi_write_register: status = %d", status);
		return VM_FALSE;
	}

	return VM_TRUE;
}
