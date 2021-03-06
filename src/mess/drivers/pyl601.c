/***************************************************************************

        Pyldin-601

        12/05/2009 Skeleton driver.

****************************************************************************/

#include "emu.h"
#include "cpu/m6800/m6800.h"
#include "video/mc6845.h"
#include "imagedev/flopdrv.h"
#include "formats/basicdsk.h"
#include "machine/upd765.h"
#include "machine/ram.h"


class pyl601_state : public driver_device
{
public:
	pyl601_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	UINT8 m_rom_page;
	UINT32 m_vdisk_addr;
	UINT8 m_key_code;
	UINT8 m_keyboard_clk;
	UINT8 m_video_mode;
	UINT8 m_tick50_mark;
	UINT8 m_floppy_ctrl;
};



static READ8_HANDLER (rom_page_r)
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	return state->m_rom_page;
}

static WRITE8_HANDLER (rom_page_w)
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	state->m_rom_page =data;
	if (data & 8)
	{
		int chip = (data >> 4) % 5;
		int page = data & 7;
		memory_set_bankptr(space->machine(), "bank2", space->machine().region("romdisk")->base() + chip*0x10000 + page * 0x2000);
	}
	else
	{
		memory_set_bankptr(space->machine(), "bank2", ram_get_ptr(space->machine().device(RAM_TAG)) + 0xc000);
	}
}


static WRITE8_HANDLER (vdisk_page_w)
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	state->m_vdisk_addr = (state->m_vdisk_addr & 0x0ffff) | ((data & 0x0f)<<16);
}

static WRITE8_HANDLER (vdisk_h_w)
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	state->m_vdisk_addr = (state->m_vdisk_addr & 0xf00ff) | (data<<8);
}

static WRITE8_HANDLER (vdisk_l_w)
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	state->m_vdisk_addr = (state->m_vdisk_addr & 0xfff00) | data;
}

static WRITE8_HANDLER (vdisk_data_w)
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	ram_get_ptr(space->machine().device(RAM_TAG))[0x10000 + (state->m_vdisk_addr & 0x7ffff)] = data;
	state->m_vdisk_addr++;
	state->m_vdisk_addr&=0x7ffff;
}

static READ8_HANDLER (vdisk_data_r)
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	UINT8 retVal = ram_get_ptr(space->machine().device(RAM_TAG))[0x10000 + (state->m_vdisk_addr & 0x7ffff)];
	state->m_vdisk_addr++;
	state->m_vdisk_addr &= 0x7ffff;
	return retVal;
}

static UINT8 selectedline(UINT16 data)
{
	UINT8 i;
	for(i = 0; i < 16; i++)
	{
		if (BIT(data, i))
		{
			return i;
		}
	}
	return 0;
}

static READ8_HANDLER ( keyboard_r )
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	return state->m_key_code;
}

static READ8_HANDLER ( keycheck_r )
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	UINT8 retVal = 0x3f;
	UINT8 *keyboard = space->machine().region("keyboard")->base();
	UINT16 row1 = input_port_read(space->machine(), "ROW1");
	UINT16 row2 = input_port_read(space->machine(), "ROW2");
	UINT16 row3 = input_port_read(space->machine(), "ROW3");
	UINT16 row4 = input_port_read(space->machine(), "ROW4");
	UINT16 row5 = input_port_read(space->machine(), "ROW5");
	UINT16 all = row1 | row2 | row3 | row4 | row5;
	UINT16 addr = (input_port_read(space->machine(), "SHIFT") & 1) | (input_port_read(space->machine(), "CTRL") & 1) << 1;
	if (all != 0xff)
	{
		addr |= selectedline(all) << 2;

		addr |=  ((row5 == 0x00) ? 1 : 0) << 6;
		addr |=  ((row4 == 0x00) ? 1 : 0) << 7;
		addr |=  ((row3 == 0x00) ? 1 : 0) << 8;
		addr |=  ((row2 == 0x00) ? 1 : 0) << 9;
		addr |=  ((row1 == 0x00) ? 1 : 0) << 10;

		state->m_key_code = keyboard[addr];
		state->m_keyboard_clk = ~state->m_keyboard_clk;

		if (state->m_keyboard_clk)
			retVal |= 0x80;
	}
	return retVal;
}


static WRITE8_HANDLER (video_mode_w)
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	state->m_video_mode = data;
}
static READ8_HANDLER (video_mode_r)
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	return state->m_video_mode;
}

static READ8_HANDLER (timer_r)
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	UINT8 retVal= state->m_tick50_mark | 0x37;
	state->m_tick50_mark = 0;
	return retVal;
}

static WRITE8_HANDLER (speaker_w)
{
}

static WRITE8_HANDLER (led_w)
{
//  UINT8 caps_led = BIT(data,4);
}

INLINE device_t *get_floppy_image(running_machine &machine, int drive)
{
	return floppy_get_device(machine, drive);
}

static UPD765_GET_IMAGE( pyldin_upd765_get_image )
{
	return get_floppy_image(device->machine(), (floppy_index & 1)^1);
}
static WRITE8_HANDLER( floppy_w )
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	// bit 0 is reset (if zero)
	// bit 1 is TC state
	// bit 2 is drive selected
	// bit 3 is motor state
	device_t *floppy = space->machine().device("upd765");
	if (BIT(data,0)==0) {
		//reset
		upd765_reset(floppy,0);
	}
	floppy_mon_w(get_floppy_image(space->machine(), BIT(data,2)), !BIT(data, 3));

	floppy_drive_set_ready_state(get_floppy_image(space->machine(), 0), BIT(data,2), 0);

	upd765_tc_w(floppy, BIT(data,1));

	state->m_floppy_ctrl = data;
}
static READ8_HANDLER (floppy_r)
{
	pyl601_state *state = space->machine().driver_data<pyl601_state>();
	return state->m_floppy_ctrl;
}

static const struct upd765_interface pyldin_upd765_interface =
{
	DEVCB_NULL,					/* interrupt */
	DEVCB_NULL,					/* DMA request */
	pyldin_upd765_get_image,	/* image lookup */
	UPD765_RDY_PIN_CONNECTED,	/* ready pin */
	{FLOPPY_0,FLOPPY_1, NULL, NULL}
};

static ADDRESS_MAP_START(pyl601_mem, AS_PROGRAM, 8)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE( 0x0000, 0xbfff ) AM_RAMBANK("bank1")
	AM_RANGE( 0xc000, 0xdfff ) AM_RAMBANK("bank2")
	AM_RANGE( 0xe000, 0xe5ff ) AM_RAMBANK("bank3")
	AM_RANGE( 0xe600, 0xe600 ) AM_DEVWRITE_MODERN("crtc", mc6845_device, address_w)
	AM_RANGE( 0xe601, 0xe601 ) AM_DEVREADWRITE_MODERN("crtc", mc6845_device, register_r, register_w)
	AM_RANGE( 0xe604, 0xe604 ) AM_DEVWRITE_MODERN("crtc", mc6845_device, address_w)
	AM_RANGE( 0xe605, 0xe605 ) AM_DEVREADWRITE_MODERN("crtc", mc6845_device, register_r, register_w)
	AM_RANGE( 0xe628, 0xe628 ) AM_READ(keyboard_r)
	AM_RANGE( 0xe629, 0xe629 ) AM_READWRITE(video_mode_r,video_mode_w)
	AM_RANGE( 0xe62a, 0xe62a ) AM_READWRITE(keycheck_r,led_w)
	AM_RANGE( 0xe62b, 0xe62b ) AM_READWRITE(timer_r,speaker_w)
	AM_RANGE( 0xe62d, 0xe62d ) AM_READ(video_mode_r)
	AM_RANGE( 0xe62e, 0xe62e ) AM_READWRITE(keycheck_r,led_w)
	AM_RANGE( 0xe680, 0xe680 ) AM_WRITE(vdisk_page_w)
	AM_RANGE( 0xe681, 0xe681 ) AM_WRITE(vdisk_h_w)
	AM_RANGE( 0xe682, 0xe682 ) AM_WRITE(vdisk_l_w)
	AM_RANGE( 0xe683, 0xe683 ) AM_READWRITE(vdisk_data_r,vdisk_data_w)
	AM_RANGE( 0xe6c0, 0xe6c0 ) AM_READWRITE(floppy_r, floppy_w)
	AM_RANGE( 0xe6d0, 0xe6d0 ) AM_DEVREAD("upd765", upd765_status_r)
	AM_RANGE( 0xe6d1, 0xe6d1 ) AM_DEVREADWRITE("upd765", upd765_data_r, upd765_data_w)
	AM_RANGE( 0xe6f0, 0xe6f0 ) AM_READWRITE(rom_page_r, rom_page_w)
	AM_RANGE( 0xe700, 0xefff ) AM_RAMBANK("bank4")
	AM_RANGE( 0xf000, 0xffff ) AM_READ_BANK("bank5") AM_WRITE_BANK("bank6")
ADDRESS_MAP_END

/* Input ports */
/* A small note about natural keyboard mode: Ctrl is mapped to PGUP and the 'Lat/Cyr' key is mapped to PGDOWN */
static INPUT_PORTS_START( pyl601 )
	PORT_START("ROW1")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Lat/Cyr") PORT_CODE(KEYCODE_LALT) PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(PGDN))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Del") PORT_CODE(KEYCODE_BACKSPACE) PORT_CHAR(8)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2) PORT_CHAR('\\') PORT_CHAR('|')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS) PORT_CHAR('=') PORT_CHAR('+')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS) PORT_CHAR('-') PORT_CHAR('_')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0) PORT_CHAR('0') PORT_CHAR(')')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9) PORT_CHAR('9') PORT_CHAR('(')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8) PORT_CHAR('8') PORT_CHAR('*')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7) PORT_CHAR('7') PORT_CHAR('&')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6) PORT_CHAR('6') PORT_CHAR('^')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5) PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4) PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3) PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2) PORT_CHAR('2') PORT_CHAR('@')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1) PORT_CHAR('1') PORT_CHAR('!')

	PORT_START("ROW2")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT) PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_LEFT) PORT_CODE(KEYCODE_LEFT) PORT_CHAR(UCHAR_MAMEKEY(LEFT))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE) PORT_CHAR(']') PORT_CHAR('}')	// it should be the 4th key at right of 'L'
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH) PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE) PORT_CHAR('\'') PORT_CHAR('"')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON) PORT_CHAR(';') PORT_CHAR(':')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L) PORT_CHAR('L') PORT_CHAR('l')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K) PORT_CHAR('K') PORT_CHAR('k')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J) PORT_CHAR('J') PORT_CHAR('j')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H) PORT_CHAR('H') PORT_CHAR('h')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G) PORT_CHAR('G') PORT_CHAR('g')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F) PORT_CHAR('F') PORT_CHAR('f')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D) PORT_CHAR('D') PORT_CHAR('d')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S) PORT_CHAR('S') PORT_CHAR('s')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A) PORT_CHAR('A') PORT_CHAR('a')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW3")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP) PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CHAR(13)
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE) PORT_CHAR('`') PORT_CHAR('~')
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P) PORT_CHAR('P') PORT_CHAR('p')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O) PORT_CHAR('O') PORT_CHAR('o')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I) PORT_CHAR('I') PORT_CHAR('i')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U) PORT_CHAR('U') PORT_CHAR('u')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y) PORT_CHAR('Y') PORT_CHAR('y')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T) PORT_CHAR('T') PORT_CHAR('t')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R) PORT_CHAR('R') PORT_CHAR('r')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E) PORT_CHAR('E') PORT_CHAR('e')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W) PORT_CHAR('W') PORT_CHAR('w')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q) PORT_CHAR('Q') PORT_CHAR('q')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_TAB) PORT_CHAR('\t')

	PORT_START("ROW4")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME(UTF8_DOWN) PORT_CODE(KEYCODE_DOWN) PORT_CHAR(UCHAR_MAMEKEY(DOWN))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_NAME("Caps Lock") PORT_CODE(KEYCODE_CAPSLOCK)
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE) PORT_CHAR(' ')
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_UNUSED )
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH) PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP) PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA) PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M) PORT_CHAR('M') PORT_CHAR('m')
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N) PORT_CHAR('N') PORT_CHAR('n')
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B) PORT_CHAR('B') PORT_CHAR('b')
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V) PORT_CHAR('V') PORT_CHAR('v')
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C) PORT_CHAR('C') PORT_CHAR('c')
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X) PORT_CHAR('X') PORT_CHAR('x')
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z) PORT_CHAR('Z') PORT_CHAR('z')
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_UNUSED )

	PORT_START("ROW5")
	PORT_BIT( 0x0001, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGDN) PORT_CHAR(UCHAR_MAMEKEY(F15))
	PORT_BIT( 0x0002, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PGUP) PORT_CHAR(UCHAR_MAMEKEY(F14))
	PORT_BIT( 0x0004, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_HOME) PORT_CHAR(UCHAR_MAMEKEY(F13))
	PORT_BIT( 0x0008, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F12) PORT_CHAR(UCHAR_MAMEKEY(F12))
	PORT_BIT( 0x0010, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F11) PORT_CHAR(UCHAR_MAMEKEY(F11))
	PORT_BIT( 0x0020, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F10) PORT_CHAR(UCHAR_MAMEKEY(F10))
	PORT_BIT( 0x0040, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F9) PORT_CHAR(UCHAR_MAMEKEY(F9))
	PORT_BIT( 0x0080, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F8) PORT_CHAR(UCHAR_MAMEKEY(F8))
	PORT_BIT( 0x0100, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F7) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x0200, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F6) PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x0400, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5) PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x8000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4) PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x4000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3) PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x2000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2) PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x1000, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1) PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x0800, IP_ACTIVE_HIGH, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC) PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("SHIFT")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Shift") PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)

	PORT_START("CTRL")
	PORT_BIT( 0x0001, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Ctrl") PORT_CODE(KEYCODE_LCONTROL) PORT_CODE(KEYCODE_RCONTROL) PORT_CHAR(UCHAR_MAMEKEY(PGUP))
INPUT_PORTS_END

static MACHINE_RESET(pyl601)
{
	pyl601_state *state = machine.driver_data<pyl601_state>();
	UINT8 *ram = ram_get_ptr(machine.device(RAM_TAG));
	state->m_key_code = 0xff;
	memory_set_bankptr(machine, "bank1", ram + 0x0000);
	memory_set_bankptr(machine, "bank2", ram + 0xc000);
	memory_set_bankptr(machine, "bank3", ram + 0xe000);
	memory_set_bankptr(machine, "bank4", ram + 0xe700);
	memory_set_bankptr(machine, "bank5", machine.region("maincpu")->base() + 0xf000);
	memory_set_bankptr(machine, "bank6", ram + 0xf000);

	machine.device("maincpu")->reset();
}

static VIDEO_START( pyl601 )
{
}

static SCREEN_UPDATE( pyl601 )
{
	mc6845_device *mc6845 = screen->machine().device<mc6845_device>("crtc");
	mc6845->update( bitmap, cliprect);
	return 0;
}

static MC6845_UPDATE_ROW( pyl601_update_row )
{
	pyl601_state *state = device->machine().driver_data<pyl601_state>();
	UINT8 *charrom = device->machine().region("gfx1")->base();

	int column, bit, i;
	UINT8 data;
	if (BIT(state->m_video_mode, 5) == 0)
	{
		for (column = 0; column < x_count; column++)
		{
			UINT8 code = ram_get_ptr(device->machine().device(RAM_TAG))[(((ma + column) & 0x0fff) + 0xf000)];
			code = ((code << 1) | (code >> 7)) & 0xff;
			if (column == cursor_x-2)
			{
				data = 0xff;
			}
			else
			{
				data = charrom[((code << 3) | (ra & 0x07)) & 0x7ff];
			}
			for (bit = 0; bit < 8; bit++)
			{
				int x = (column * 8) + bit;
				int color = BIT(data, 7) ? 1 : 0;

				*BITMAP_ADDR16(bitmap, y, x) = color;

				data <<= 1;
			}
		}
	}
	else
	{
		for (i = 0; i < x_count; i++)
		{
			data = ram_get_ptr(device->machine().device(RAM_TAG))[(((ma + i) << 3) | (ra & 0x07)) & 0xffff];
			for (bit = 0; bit < 8; bit++)
			{
				*BITMAP_ADDR16(bitmap, y, (i * 8) + bit) = BIT(data, 7) ? 1 : 0;
				data <<= 1;
			}
		}
	}
}

static MC6845_UPDATE_ROW( pyl601a_update_row )
{
	pyl601_state *state = device->machine().driver_data<pyl601_state>();
	UINT8 *charrom = device->machine().region("gfx1")->base();

	int column, bit, i;
	UINT8 data;
	if (BIT(state->m_video_mode, 5) == 0)
	{
		for (column = 0; column < x_count; column++)
		{
			UINT8 code = ram_get_ptr(device->machine().device(RAM_TAG))[(((ma + column) & 0x0fff) + 0xf000)];
			data = charrom[((code << 4) | (ra & 0x07)) & 0xfff];
			if (column == cursor_x)
			{
				data = 0xff;
			}

			for (bit = 0; bit < 8; bit++)
			{
				int x = (column * 8) + bit;
				int color = BIT(data, 7) ? 1 : 0;

				*BITMAP_ADDR16(bitmap, y, x) = color;

				data <<= 1;
			}
		}
	}
	else
	{
		for (i = 0; i < x_count; i++)
		{
			data = ram_get_ptr(device->machine().device(RAM_TAG))[(((ma + i) << 3) | (ra & 0x07)) & 0xffff];
			for (bit = 0; bit < 8; bit++)
			{
				*BITMAP_ADDR16(bitmap, y, (i * 8) + bit) = BIT(data, 7) ? 1 : 0;
				data <<= 1;
			}
		}
	}
}


static const mc6845_interface pyl601_crtc6845_interface =
{
	"screen",
	8 /*?*/,
	NULL,
	pyl601_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

static const mc6845_interface pyl601a_crtc6845_interface =
{
	"screen",
	8 /*?*/,
	NULL,
	pyl601a_update_row,
	NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	NULL
};

static DRIVER_INIT(pyl601)
{
	memset(ram_get_ptr(machine.device(RAM_TAG)), 0, 64 * 1024);
}

static INTERRUPT_GEN( pyl601_interrupt )
{
	pyl601_state *state = device->machine().driver_data<pyl601_state>();
	state->m_tick50_mark = 0x80;
	device_set_input_line(device, 0, HOLD_LINE);
}

static LEGACY_FLOPPY_OPTIONS_START(pyldin)
	LEGACY_FLOPPY_OPTION(pyldin, "img", "Pyldin disk image", basicdsk_identify_default, basicdsk_construct_default, NULL,
		HEADS([2])
		TRACKS([80])
		SECTORS([9])
		SECTOR_LENGTH([512])
		FIRST_SECTOR_ID([1]))
LEGACY_FLOPPY_OPTIONS_END

static const floppy_interface pyldin_floppy_interface =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	FLOPPY_STANDARD_5_25_DSHD,
	LEGACY_FLOPPY_OPTIONS_NAME(pyldin),
	NULL,
	NULL
};

/* F4 Character Displayer */
static const gfx_layout pyl601_charlayout =
{
	8, 8,					/* 8 x 8 characters */
	256,					/* 256 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*8					/* every char takes 8 bytes */
};

static const gfx_layout pyl601a_charlayout =
{
	8, 8,					/* 8 x 8 characters */
	256,					/* 256 characters */
	1,					/* 1 bits per pixel */
	{ 0 },					/* no bitplanes */
	/* x offsets */
	{ 0, 1, 2, 3, 4, 5, 6, 7 },
	/* y offsets */
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8 },
	8*16					/* every char takes 16 bytes */
};

static GFXDECODE_START( pyl601 )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pyl601_charlayout, 0, 1 )
GFXDECODE_END

static GFXDECODE_START( pyl601a )
	GFXDECODE_ENTRY( "gfx1", 0x0000, pyl601a_charlayout, 0, 1 )
GFXDECODE_END

static MACHINE_CONFIG_START( pyl601, pyl601_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",M6800, XTAL_1MHz)
	MCFG_CPU_PROGRAM_MAP(pyl601_mem)
	MCFG_CPU_VBLANK_INT("screen", pyl601_interrupt)

	MCFG_MACHINE_RESET(pyl601)

	/* video hardware */
	MCFG_SCREEN_ADD("screen", RASTER)
	MCFG_SCREEN_REFRESH_RATE(50)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(640, 200)
	MCFG_SCREEN_VISIBLE_AREA(0, 640 - 1, 0, 200 - 1)
	MCFG_SCREEN_UPDATE( pyl601 )

	MCFG_GFXDECODE(pyl601)
	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(monochrome_green)

	MCFG_MC6845_ADD("crtc", MC6845, XTAL_2MHz, pyl601_crtc6845_interface)

	MCFG_VIDEO_START( pyl601 )

	MCFG_UPD765A_ADD("upd765", pyldin_upd765_interface)

	MCFG_LEGACY_FLOPPY_2_DRIVES_ADD(pyldin_floppy_interface)

	/* internal ram */
	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("576K") // 64 + 512
MACHINE_CONFIG_END

static MACHINE_CONFIG_DERIVED( pyl601a, pyl601 )
	MCFG_CPU_MODIFY("maincpu")
	MCFG_CPU_CLOCK( XTAL_2MHz)

	MCFG_GFXDECODE(pyl601a)
	MCFG_DEVICE_REMOVE("crtc")
	MCFG_MC6845_ADD("crtc", MC6845, XTAL_2MHz, pyl601a_crtc6845_interface)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( pyl601 )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bios.rom",   0xf000, 0x1000, CRC(41fe4c4b) SHA1(d8ca92aea0eb283e8d7779cb976bcdfa03e81aea))

	ROM_REGION(0x0800, "gfx1",0)
	ROM_LOAD( "video.rom",  0x0000, 0x0800, CRC(1c23ba43) SHA1(eb1cfc139858abd0aedbbf3d523f8ba55d27a11d))

	ROM_REGION(0x50000, "romdisk",ROMREGION_ERASEFF)
	ROM_LOAD( "rom0.rom", 0x00000, 0x10000, CRC(60103920) SHA1(ee5b4ee5b513c4a0204da751e53d63b8c6c0aab9))
	ROM_LOAD( "rom1.rom", 0x10000, 0x10000, CRC(cb4a9b22) SHA1(dd09e4ba35b8d1a6f60e6e262aaf2f156367e385))
	ROM_LOAD( "rom2.rom", 0x20000, 0x08000, CRC(0b7684bf) SHA1(c02ad1f2a6f484cd9d178d8b060c21c0d4e53442))
	ROM_COPY("romdisk", 0x20000, 0x28000, 0x08000)
	ROM_LOAD( "rom3.rom", 0x30000, 0x08000, CRC(e4a86dfa) SHA1(96e6bb9ffd66f81fca63bf7491fbba81c4ff1fd2))
	ROM_COPY("romdisk", 0x30000, 0x38000, 0x08000)
	ROM_LOAD( "rom4.rom", 0x40000, 0x08000, CRC(d88ac21d) SHA1(022db11fdcf8db81ce9efd9cd9fa50ebca88e79e))
	ROM_COPY("romdisk", 0x40000, 0x48000, 0x08000)

	ROM_REGION(0x0800, "keyboard",0)
	ROM_LOAD( "keyboard.rom", 0x0000, 0x0800, CRC(41fbe5ca) SHA1(875adaef53bc37e92ad0b6b6ee3d8fd28344d358))
ROM_END

ROM_START( pyl601a )
	ROM_REGION( 0x10000, "maincpu", ROMREGION_ERASEFF )
	ROM_LOAD( "bios_a.rom", 0xf000, 0x1000, CRC(e018b11e) SHA1(884d59abd5fa5af1295d1b5a53693facc7945b63))

	ROM_REGION(0x1000, "gfx1",0)
	ROM_LOAD( "video_a.rom", 0x0000,0x1000, CRC(00fa4077) SHA1(d39d15969a08bdb768d08bea4ec9a9cb498232fd))

	ROM_REGION(0x50000, "romdisk",ROMREGION_ERASEFF)
	ROM_LOAD( "rom0.rom", 0x00000, 0x10000, CRC(60103920) SHA1(ee5b4ee5b513c4a0204da751e53d63b8c6c0aab9))
	ROM_LOAD( "rom1.rom", 0x10000, 0x10000, CRC(cb4a9b22) SHA1(dd09e4ba35b8d1a6f60e6e262aaf2f156367e385))
	ROM_LOAD( "rom2.rom", 0x20000, 0x08000, CRC(0b7684bf) SHA1(c02ad1f2a6f484cd9d178d8b060c21c0d4e53442))
	ROM_COPY("romdisk", 0x20000, 0x28000, 0x08000)
	ROM_LOAD( "rom3.rom", 0x30000, 0x08000, CRC(e4a86dfa) SHA1(96e6bb9ffd66f81fca63bf7491fbba81c4ff1fd2))
	ROM_COPY("romdisk", 0x30000, 0x38000, 0x08000)
	ROM_LOAD( "rom4.rom", 0x40000, 0x08000, CRC(d88ac21d) SHA1(022db11fdcf8db81ce9efd9cd9fa50ebca88e79e))
	ROM_COPY("romdisk", 0x40000, 0x48000, 0x08000)

	ROM_REGION(0x0800, "keyboard",0)
	ROM_LOAD( "keyboard.rom", 0x0000, 0x0800, CRC(41fbe5ca) SHA1(875adaef53bc37e92ad0b6b6ee3d8fd28344d358))
ROM_END
/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT   COMPANY   FULLNAME       FLAGS */
COMP( 1989, pyl601,  0,       0,	pyl601, 	pyl601, pyl601, "Mikroelektronika",   "Pyldin-601",		GAME_NOT_WORKING | GAME_NO_SOUND)
COMP( 1989, pyl601a, pyl601,  0,	pyl601a,	pyl601, pyl601, "Mikroelektronika",   "Pyldin-601A",		GAME_NOT_WORKING | GAME_NO_SOUND)
