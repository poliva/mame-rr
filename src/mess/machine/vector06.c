/***************************************************************************

        Vector06c driver by Miodrag Milanovic

        10/07/2008 Preliminary driver.

****************************************************************************/


#include "includes/vector06.h"


READ8_MEMBER( vector06_state::vector06_8255_portb_r )
{
	UINT8 key = 0xff;
	if (BIT(m_keyboard_mask, 0)) key &= input_port_read(machine(),"LINE0");
	if (BIT(m_keyboard_mask, 1)) key &= input_port_read(machine(),"LINE1");
	if (BIT(m_keyboard_mask, 2)) key &= input_port_read(machine(),"LINE2");
	if (BIT(m_keyboard_mask, 3)) key &= input_port_read(machine(),"LINE3");
	if (BIT(m_keyboard_mask, 4)) key &= input_port_read(machine(),"LINE4");
	if (BIT(m_keyboard_mask, 5)) key &= input_port_read(machine(),"LINE5");
	if (BIT(m_keyboard_mask, 6)) key &= input_port_read(machine(),"LINE6");
	if (BIT(m_keyboard_mask, 7)) key &= input_port_read(machine(),"LINE7");
	return key;
}

READ8_MEMBER( vector06_state::vector06_8255_portc_r )
{
	UINT8 ret = input_port_read(machine(), "LINE8");

	if (m_cass->input() > 0)
		ret |= 0x10;

	return ret;
}

WRITE8_MEMBER( vector06_state::vector06_8255_porta_w )
{
	m_keyboard_mask = data ^ 0xff;
}

void vector06_state::vector06_set_video_mode(int width)
{
	rectangle visarea;

	visarea.min_x = 0;
	visarea.min_y = 0;
	visarea.max_x = width+64-1;
	visarea.max_y = 256+64-1;
	machine().primary_screen->configure(width+64, 256+64, visarea, machine().primary_screen->frame_period().attoseconds);
}

WRITE8_MEMBER( vector06_state::vector06_8255_portb_w )
{
	m_color_index = data & 0x0f;
	if ((data & 0x10) != m_video_mode)
	{
		m_video_mode = data & 0x10;
		vector06_set_video_mode((m_video_mode==0x10) ? 512 : 256);
	}
}

WRITE8_MEMBER( vector06_state::vector06_color_set )
{
	UINT8 r = (data & 7) << 5;
	UINT8 g = ((data >> 3) & 7) << 5;
	UINT8 b = ((data >>6) & 3) << 6;
	palette_set_color( machine(), m_color_index, MAKE_RGB(r,g,b) );
}


READ8_MEMBER( vector06_state::vector06_romdisk_portb_r )
{
	UINT8 *romdisk = machine().region("maincpu")->base() + 0x18000;
	UINT16 addr = (m_romdisk_msb | m_romdisk_lsb) & 0x7fff;
	return romdisk[addr];
}

WRITE8_MEMBER( vector06_state::vector06_romdisk_porta_w )
{
	m_romdisk_lsb = data;
}

WRITE8_MEMBER( vector06_state::vector06_romdisk_portc_w )
{
	m_romdisk_msb = data << 8;
}

I8255A_INTERFACE( vector06_ppi8255_2_interface )
{
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(vector06_state, vector06_romdisk_porta_w),
	DEVCB_DRIVER_MEMBER(vector06_state, vector06_romdisk_portb_r),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(vector06_state, vector06_romdisk_portc_w)
};


I8255A_INTERFACE( vector06_ppi8255_interface )
{
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(vector06_state, vector06_8255_porta_w),
	DEVCB_DRIVER_MEMBER(vector06_state, vector06_8255_portb_r),
	DEVCB_DRIVER_MEMBER(vector06_state, vector06_8255_portb_w),
	DEVCB_DRIVER_MEMBER(vector06_state, vector06_8255_portc_r),
	DEVCB_NULL
};

READ8_MEMBER( vector06_state::vector06_8255_1_r )
{
	return m_ppi->read(space, offset^3);
}

WRITE8_MEMBER( vector06_state::vector06_8255_1_w )
{
	m_ppi->write(space, offset^3, data);
}

READ8_MEMBER( vector06_state::vector06_8255_2_r )
{
	return m_ppi2->read(space, offset^3);
}

WRITE8_MEMBER( vector06_state::vector06_8255_2_w )
{
	m_ppi2->write(space, offset^3, data);
}

INTERRUPT_GEN( vector06_interrupt )
{
	vector06_state *state = device->machine().driver_data<vector06_state>();
	state->m_vblank_state++;
	if (state->m_vblank_state>1) state->m_vblank_state=0;
	device_set_input_line(device,0,state->m_vblank_state ? HOLD_LINE : CLEAR_LINE);

}

static IRQ_CALLBACK( vector06_irq_callback )
{
	// Interupt is RST 7
	return 0xff;
}

static TIMER_CALLBACK( reset_check_callback )
{
	UINT8 val = input_port_read(machine, "RESET");

	if (BIT(val, 0))
	{
		memory_set_bankptr(machine, "bank1", machine.region("maincpu")->base() + 0x10000);
		machine.device("maincpu")->reset();
	}

	if (BIT(val, 1))
	{
		memory_set_bankptr(machine, "bank1", ram_get_ptr(machine.device(RAM_TAG)) + 0x0000);
		machine.device("maincpu")->reset();
	}
}

WRITE8_MEMBER( vector06_state::vector06_disc_w )
{
// something here needs to turn the motor on

	wd17xx_set_side (m_fdc,BIT(data, 2) ^ 1);
	wd17xx_set_drive(m_fdc,BIT(data, 0));
}

MACHINE_START( vector06 )
{
	machine.scheduler().timer_pulse(attotime::from_hz(50), FUNC(reset_check_callback));
}

MACHINE_RESET( vector06 )
{
	vector06_state *state = machine.driver_data<vector06_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	device_set_irq_callback(machine.device("maincpu"), vector06_irq_callback);
	space->install_read_bank (0x0000, 0x7fff, "bank1");
	space->install_write_bank(0x0000, 0x7fff, "bank2");
	space->install_read_bank (0x8000, 0xffff, "bank3");
	space->install_write_bank(0x8000, 0xffff, "bank4");

	memory_set_bankptr(machine, "bank1", machine.region("maincpu")->base() + 0x10000);
	memory_set_bankptr(machine, "bank2", ram_get_ptr(machine.device(RAM_TAG)) + 0x0000);
	memory_set_bankptr(machine, "bank3", ram_get_ptr(machine.device(RAM_TAG)) + 0x8000);
	memory_set_bankptr(machine, "bank4", ram_get_ptr(machine.device(RAM_TAG)) + 0x8000);

	state->m_keyboard_mask = 0;
	state->m_color_index = 0;
	state->m_video_mode = 0;
}
