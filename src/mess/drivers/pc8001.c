/*

    http://www2.odn.ne.jp/~haf09260/Pc80/EnrPc.htm
    http://home1.catvmics.ne.jp/~kanemoto/n80/inside.html
    http://www.geocities.jp/retro_zzz/machines/nec/8001/index.html

*/

/*

    TODO:

    - uPD3301 attributes
    - PCG1000
    - Intel 8251
    - cassette
    - floppy
    - PC-8011
    - PC-8021
    - PC-8031

*/

#include "includes/pc8001.h"

/* Read/Write Handlers */

WRITE8_MEMBER( pc8001_state::port10_w )
{
	/*

        bit     description

        0       RTC C0
        1       RTC C1
        2       RTC C2
        3       RTC DATA IN
        4
        5
        6
        7

    */

	// RTC
	m_rtc->c0_w(BIT(data, 0));
	m_rtc->c1_w(BIT(data, 1));
	m_rtc->c2_w(BIT(data, 2));
	m_rtc->data_in_w(BIT(data, 3));

	// centronics
	centronics_data_w(m_centronics, 0, data);
}

WRITE8_MEMBER( pc8001_state::port30_w )
{
	/*

        bit     description

        0       characters per line (0=40, 1=80)
        1       color mode (0=color, 1=B&W)
        2       CMT CHIN
        3       CMT MOTOR (1=on)
        4       CMT BS1
        5       CMT BS2
        6       unused
        7       unused

    */

	/* characters per line */
	m_width80 = BIT(data, 0);

	/* color mode */
	m_color = BIT(data, 1);

	/* cassette motor */
	m_cassette->change_state(BIT(data,3) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);
}

WRITE8_MEMBER( pc8001mk2_state::port31_w )
{
	/*

        bit     description

        0       expansion ROM (0=expansion, 1=N80)
        1       memory mode (0=ROM, 1=RAM)
        2       color mode (0=attribute, 1=B&W)
        3       graphics enable
        4       resolution (0=640x200, 1=320x200)
        5       background color
        6       background color
        7       background color

    */
}

/* Memory Maps */

static ADDRESS_MAP_START( pc8001_mem, AS_PROGRAM, 8, pc8001_state )
	AM_RANGE(0x0000, 0x5fff) AM_RAMBANK("bank1")
	AM_RANGE(0x6000, 0x7fff) AM_RAMBANK("bank2")
	AM_RANGE(0x8000, 0xffff) AM_RAMBANK("bank3")
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc8001_io, AS_IO, 8, pc8001_state )
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00, 0x00) AM_READ_PORT("KEY0")
	AM_RANGE(0x01, 0x01) AM_READ_PORT("KEY1")
	AM_RANGE(0x02, 0x02) AM_READ_PORT("KEY2")
	AM_RANGE(0x03, 0x03) AM_READ_PORT("KEY3")
	AM_RANGE(0x04, 0x04) AM_READ_PORT("KEY4")
	AM_RANGE(0x05, 0x05) AM_READ_PORT("KEY5")
	AM_RANGE(0x06, 0x06) AM_READ_PORT("KEY6")
	AM_RANGE(0x07, 0x07) AM_READ_PORT("KEY7")
	AM_RANGE(0x08, 0x08) AM_READ_PORT("KEY8")
	AM_RANGE(0x09, 0x09) AM_READ_PORT("KEY9")
	AM_RANGE(0x10, 0x10) AM_MIRROR(0x0f) AM_WRITE(port10_w)
	AM_RANGE(0x20, 0x20) AM_MIRROR(0x0e) AM_DEVREADWRITE(I8251_TAG, i8251_device, data_r, data_w)
	AM_RANGE(0x21, 0x21) AM_MIRROR(0x0e) AM_DEVREADWRITE(I8251_TAG, i8251_device, status_r, control_w)
	AM_RANGE(0x30, 0x30) AM_MIRROR(0x0f) AM_WRITE(port30_w)
	AM_RANGE(0x40, 0x40) AM_MIRROR(0x0f) AM_READ_PORT("R40") AM_WRITE_PORT("W40")
	AM_RANGE(0x50, 0x51) AM_DEVREADWRITE(UPD3301_TAG, upd3301_device, read, write)
	AM_RANGE(0x60, 0x68) AM_DEVREADWRITE_LEGACY(I8257_TAG, i8257_r, i8257_w)
//  AM_RANGE(0x70, 0x7f) unused
//  AM_RANGE(0x80, 0x80) AM_MIRROR(0x0f) AM_WRITE(pc8011_ext0_w)
//  AM_RANGE(0x90, 0x90) AM_MIRROR(0x0f) AM_WRITE(pc8011_ext1_w)
//  AM_RANGE(0xa0, 0xa0) AM_MIRROR(0x0f) AM_WRITE(pc8011_ext2_w)
//  AM_RANGE(0xb0, 0xb0) AM_READ(pc8011_gpio8_r)
//  AM_RANGE(0xb1, 0xb1) AM_WRITE(pc8011_gpio8_w)
//  AM_RANGE(0xb2, 0xb2) AM_READ(pc8011_gpio4_r)
//  AM_RANGE(0xb3, 0xb3) AM_WRITE(pc8011_gpio4_w)
//  AM_RANGE(0xc0, 0xc0) AM_DEVREADWRITE_LEGACY(PC8011_CH1_I8251_TAG, i8251_device, data_r, data_w)
//  AM_RANGE(0xc1, 0xc1) AM_DEVREADWRITE_LEGACY(PC8011_CH1_I8251_TAG, i8251_device, status_r, control_w)
//  AM_RANGE(0xc2, 0xc2) AM_DEVREADWRITE_LEGACY(PC8011_CH2_I8251_TAG, i8251_device, data_r, data_w)
//  AM_RANGE(0xc3, 0xc3) AM_DEVREADWRITE_LEGACY(PC8011_CH2_I8251_TAG, i8251_device, status_r, control_w)
//  AM_RANGE(0xc8, 0xc8) RS-232 output enable?
//  AM_RANGE(0xca, 0xca) RS-232 output disable?
//  AM_RANGE(0xd0, 0xd3) AM_DEVREADWRITE_LEGACY(PC8011_IEEE488_I8255A_TAG, i8255a_r, i8255a_w)
//  AM_RANGE(0xd8, 0xd8) AM_READ(pc8011_ieee488_control_signal_input_r)
//  AM_RANGE(0xda, 0xda) AM_READ(pc8011_ieee488_bus_address_mode_r)
//  AM_RANGE(0xdc, 0xdc) AM_WRITE(pc8011_ieee488_nrfd_w)
//  AM_RANGE(0xde, 0xde) AM_WRITE(pc8011_ieee488_bus_mode_control_w)
//  AM_RANGE(0xe0, 0xe3) AM_WRITE(expansion_storage_mode_w)
//  AM_RANGE(0xe4, 0xe4) AM_MIRROR(0x01) AM_WRITE(irq_level_w)
//  AM_RANGE(0xe6, 0xe6) AM_WRITE(irq_mask_w)
//  AM_RANGE(0xe7, 0xe7) AM_WRITE(pc8012_memory_mode_w)
//  AM_RANGE(0xe8, 0xfb) unused
	AM_RANGE(0xfc, 0xff) AM_DEVREADWRITE(I8255A_TAG, i8255_device, read, write)
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc8001mk2_mem, AS_PROGRAM, 8, pc8001mk2_state )
	AM_RANGE(0x0000, 0x5fff) AM_RAMBANK("bank1")
	AM_RANGE(0x6000, 0x7fff) AM_RAMBANK("bank2")
	AM_RANGE(0x8000, 0xbfff) AM_RAMBANK("bank3")
	AM_RANGE(0xc000, 0xffff) AM_RAMBANK("bank4")
ADDRESS_MAP_END

static ADDRESS_MAP_START( pc8001mk2_io, AS_IO, 8, pc8001mk2_state )
	AM_IMPORT_FROM(pc8001_io)
	AM_RANGE(0x30, 0x30) AM_WRITE_BASE(pc8001_state, port30_w)
	AM_RANGE(0x31, 0x31) AM_WRITE(port31_w)
//  AM_RANGE(0x5c, 0x5c) AM_WRITE(gram_on_w)
//  AM_RANGE(0x5f, 0x5f) AM_WRITE(gram_off_w)
//  AM_RANGE(0xe8, 0xe8) kanji_address_lo_w, kanji_data_lo_r
//  AM_RANGE(0xe9, 0xe9) kanji_address_hi_w, kanji_data_hi_r
//  AM_RANGE(0xea, 0xea) kanji_readout_start_w
//  AM_RANGE(0xeb, 0xeb) kanji_readout_end_w
//  AM_RANGE(0xf3, 0xf3) DMA type disk unit interface selection port
//  AM_RANGE(0xf4, 0xf4) DMA type 8 inch control
//  AM_RANGE(0xf5, 0xf5) DMA type 8 inch margin control
//  AM_RANGE(0xf6, 0xf6) DMA type 8 inch FDC status
//  AM_RANGE(0xf7, 0xf7) DMA type 8 inch FDC data register
//  AM_RANGE(0xf8, 0xf8) DMA type 5 inch control
//  AM_RANGE(0xf9, 0xf9) DMA type 5 inch margin control
//  AM_RANGE(0xfa, 0xfa) DMA type 5 inch FDC status
//  AM_RANGE(0xfb, 0xfb) DMA type 5 inch FDC data register
ADDRESS_MAP_END

/* Input Ports */

static INPUT_PORTS_START( pc8001 )
	PORT_START("KEY0")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0_PAD)		PORT_CHAR(UCHAR_MAMEKEY(0_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1_PAD)		PORT_CHAR(UCHAR_MAMEKEY(1_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2_PAD)		PORT_CHAR(UCHAR_MAMEKEY(2_PAD))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3_PAD)		PORT_CHAR(UCHAR_MAMEKEY(3_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4_PAD)		PORT_CHAR(UCHAR_MAMEKEY(4_PAD))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5_PAD)		PORT_CHAR(UCHAR_MAMEKEY(5_PAD))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6_PAD)		PORT_CHAR(UCHAR_MAMEKEY(6_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7_PAD)		PORT_CHAR(UCHAR_MAMEKEY(7_PAD))

	PORT_START("KEY1")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8_PAD)		PORT_CHAR(UCHAR_MAMEKEY(8_PAD))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9_PAD)		PORT_CHAR(UCHAR_MAMEKEY(9_PAD))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ASTERISK)	PORT_CHAR(UCHAR_MAMEKEY(ASTERISK))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_PLUS_PAD)	PORT_CHAR(UCHAR_MAMEKEY(PLUS_PAD))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad =") PORT_CODE(KEYCODE_PGUP)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Keypad ,") PORT_CODE(KEYCODE_PGDN)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_DEL_PAD)	PORT_CHAR(UCHAR_MAMEKEY(DEL_PAD))
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Return") PORT_CODE(KEYCODE_ENTER) PORT_CODE(KEYCODE_ENTER_PAD) PORT_CHAR(13)

	PORT_START("KEY2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_OPENBRACE)	PORT_CHAR('@')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_A)			PORT_CHAR('a') PORT_CHAR('A')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_B)			PORT_CHAR('b') PORT_CHAR('B')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_C)			PORT_CHAR('c') PORT_CHAR('C')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_D)			PORT_CHAR('d') PORT_CHAR('D')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_E)			PORT_CHAR('e') PORT_CHAR('E')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F)			PORT_CHAR('f') PORT_CHAR('F')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_G)			PORT_CHAR('g') PORT_CHAR('G')

	PORT_START("KEY3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_H)			PORT_CHAR('h') PORT_CHAR('H')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_I)			PORT_CHAR('i') PORT_CHAR('I')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_J)			PORT_CHAR('j') PORT_CHAR('J')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_K)			PORT_CHAR('k') PORT_CHAR('K')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_L)			PORT_CHAR('l') PORT_CHAR('L')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_M)			PORT_CHAR('m') PORT_CHAR('M')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_N)			PORT_CHAR('n') PORT_CHAR('N')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_O)			PORT_CHAR('o') PORT_CHAR('O')

	PORT_START("KEY4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_P)			PORT_CHAR('p') PORT_CHAR('P')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Q)			PORT_CHAR('q') PORT_CHAR('Q')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_R)			PORT_CHAR('r') PORT_CHAR('R')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_S)			PORT_CHAR('s') PORT_CHAR('S')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_T)			PORT_CHAR('t') PORT_CHAR('T')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_U)			PORT_CHAR('u') PORT_CHAR('U')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_V)			PORT_CHAR('v') PORT_CHAR('V')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_W)			PORT_CHAR('w') PORT_CHAR('W')

	PORT_START("KEY5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_X)			PORT_CHAR('x') PORT_CHAR('X')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Y)			PORT_CHAR('y') PORT_CHAR('Y')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_Z)			PORT_CHAR('z') PORT_CHAR('Z')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_CLOSEBRACE)	PORT_CHAR('[') PORT_CHAR('{')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH2)	PORT_CHAR('\xA5') PORT_CHAR('|')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_BACKSLASH)	PORT_CHAR(']') PORT_CHAR('}')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_EQUALS)		PORT_CHAR('^')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_MINUS)		PORT_CHAR('-') PORT_CHAR('=')

	PORT_START("KEY6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_0)			PORT_CHAR('0')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_1)			PORT_CHAR('1') PORT_CHAR('!')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_2)			PORT_CHAR('2') PORT_CHAR('"')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_3)			PORT_CHAR('3') PORT_CHAR('#')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_4)			PORT_CHAR('4') PORT_CHAR('$')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_5)			PORT_CHAR('5') PORT_CHAR('%')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_6)			PORT_CHAR('6') PORT_CHAR('&')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_7)			PORT_CHAR('7') PORT_CHAR('\'')

	PORT_START("KEY7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_8)			PORT_CHAR('8') PORT_CHAR('(')
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_9)			PORT_CHAR('9') PORT_CHAR(')')
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_QUOTE)		PORT_CHAR(':') PORT_CHAR('*')
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COLON)		PORT_CHAR(';') PORT_CHAR('+')
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_COMMA)		PORT_CHAR(',') PORT_CHAR('<')
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_STOP)		PORT_CHAR('.') PORT_CHAR('>')
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SLASH)		PORT_CHAR('/') PORT_CHAR('?')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("  _") PORT_CODE(KEYCODE_DEL)			PORT_CHAR(0) PORT_CHAR('_')

	PORT_START("KEY8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Clr Home") PORT_CODE(KEYCODE_HOME)		PORT_CHAR(UCHAR_MAMEKEY(HOME))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_UP) PORT_CODE(KEYCODE_UP)	PORT_CHAR(UCHAR_MAMEKEY(UP))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME(UTF8_RIGHT) PORT_CODE(KEYCODE_RIGHT)	PORT_CHAR(UCHAR_MAMEKEY(RIGHT))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Del Ins") PORT_CODE(KEYCODE_BACKSPACE)	PORT_CHAR(UCHAR_MAMEKEY(DEL)) PORT_CHAR(UCHAR_MAMEKEY(INSERT))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Grph") PORT_CODE(KEYCODE_LALT)	PORT_CODE(KEYCODE_RALT) PORT_CHAR(UCHAR_MAMEKEY(F7))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Kana") PORT_CODE(KEYCODE_LCONTROL) PORT_TOGGLE PORT_CHAR(UCHAR_MAMEKEY(F6))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_LSHIFT) PORT_CODE(KEYCODE_RSHIFT) PORT_CHAR(UCHAR_SHIFT_1)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_RCONTROL)						PORT_CHAR(UCHAR_SHIFT_2)

	PORT_START("KEY9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_NAME("Stop") PORT_CODE(KEYCODE_ESC)			PORT_CHAR(UCHAR_MAMEKEY(ESC))
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F1)								PORT_CHAR(UCHAR_MAMEKEY(F1))
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F2)								PORT_CHAR(UCHAR_MAMEKEY(F2))
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F3)								PORT_CHAR(UCHAR_MAMEKEY(F3))
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F4)								PORT_CHAR(UCHAR_MAMEKEY(F4))
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_F5)								PORT_CHAR(UCHAR_MAMEKEY(F5))
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_SPACE)							PORT_CHAR(' ')
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD ) PORT_CODE(KEYCODE_ESC)							PORT_CHAR(UCHAR_MAMEKEY(ESC))

	PORT_START("DSW1")

	PORT_START("R40")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE(CENTRONICS_TAG, centronics_busy_r)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE(CENTRONICS_TAG, centronics_ack_r)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_SPECIAL ) // CMT CDIN
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_SPECIAL ) // EXP /EXTON
	PORT_BIT( 0x10, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER(UPD1990A_TAG, upd1990a_device, data_out_r)
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_SPECIAL ) PORT_READ_LINE_DEVICE_MEMBER(UPD3301_TAG, upd3301_device, vrtc_r)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )

	PORT_START("W40")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE(CENTRONICS_TAG, centronics_strobe_w)
	PORT_BIT( 0x02, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER(UPD1990A_TAG, upd1990a_device, stb_w)
	PORT_BIT( 0x04, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE_MEMBER(UPD1990A_TAG, upd1990a_device, clk_w)
	PORT_BIT( 0x08, IP_ACTIVE_HIGH, IPT_OUTPUT ) // CRT /CLDS CLK
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x20, IP_ACTIVE_HIGH, IPT_OUTPUT ) PORT_WRITE_LINE_DEVICE(SPEAKER_TAG, speaker_level_w)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_UNUSED )
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_UNUSED )
INPUT_PORTS_END

/* Video */

static PALETTE_INIT( pc8001 )
{
}

void pc8001_state::video_start()
{
	// find memory regions
	m_char_rom = machine().region("chargen")->base();
}

bool pc8001_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	m_crtc->update_screen(&bitmap, &cliprect);

	return 0;
}

/* uPD3301 Interface */

static UPD3301_DISPLAY_PIXELS( pc8001_display_pixels )
{
	pc8001_state *state = device->machine().driver_data<pc8001_state>();

	UINT8 data = state->m_char_rom[(cc << 3) | lc];
	int i;

	if (lc >= 8) return;
	if (csr) data = 0xff;

	if (state->m_width80)
	{
		for (i = 0; i < 8; i++)
		{
			int color = BIT(data, 7) ^ rvv;

			*BITMAP_ADDR16(bitmap, y, (sx * 8) + i) = color ? 7 : 0;

			data <<= 1;
		}
	}
	else
	{
		if (sx % 2) return;

		for (i = 0; i < 8; i++)
		{
			int color = BIT(data, 7) ^ rvv;

			*BITMAP_ADDR16(bitmap, y, (sx/2 * 16) + (i * 2)) = color ? 7 : 0;
			*BITMAP_ADDR16(bitmap, y, (sx/2 * 16) + (i * 2) + 1) = color ? 7 : 0;

			data <<= 1;
		}
	}
}

static UPD3301_INTERFACE( pc8001_upd3301_intf )
{
	SCREEN_TAG,
	8,
	pc8001_display_pixels,
	DEVCB_NULL,
	DEVCB_DEVICE_LINE(I8257_TAG, i8257_drq2_w),
	DEVCB_NULL,
	DEVCB_NULL
};

/* 8251 Interface */

static const i8251_interface uart_intf =
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

/* 8255 Interface */

static I8255A_INTERFACE( ppi_intf )
{
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL
};

/* 8257 Interface */

WRITE_LINE_MEMBER( pc8001_state::hrq_w )
{
	/* HACK - this should be connected to the BUSREQ line of Z80 */
	m_maincpu->set_input_line(INPUT_LINE_HALT, state);

	/* HACK - this should be connected to the BUSACK line of Z80 */
	i8257_hlda_w(m_dma, state);
}

WRITE8_MEMBER( pc8001_state::dma_mem_w )
{
	//if (channel == 2)
	{
		m_crtc->dack_w(space, offset, data);
	}
}

READ8_MEMBER( pc8001_state::dma_io_r )
{
	address_space *program = m_maincpu->memory().space(AS_PROGRAM);

	return program->read_byte(offset);
}

WRITE8_MEMBER( pc8001_state::dma_io_w )
{
	address_space *program = m_maincpu->memory().space(AS_PROGRAM);

	program->write_byte(offset, data);
}

static I8257_INTERFACE( dmac_intf )
{
	DEVCB_DRIVER_LINE_MEMBER(pc8001_state, hrq_w),
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_NULL,
	DEVCB_DRIVER_MEMBER(pc8001_state, dma_mem_w),
	{ DEVCB_DRIVER_MEMBER(pc8001_state, dma_io_r), DEVCB_DRIVER_MEMBER(pc8001_state, dma_io_r), DEVCB_DRIVER_MEMBER(pc8001_state, dma_io_r), DEVCB_DRIVER_MEMBER(pc8001_state, dma_io_r) },
	{ DEVCB_DRIVER_MEMBER(pc8001_state, dma_io_w), DEVCB_DRIVER_MEMBER(pc8001_state, dma_io_w), DEVCB_DRIVER_MEMBER(pc8001_state, dma_io_w), DEVCB_DRIVER_MEMBER(pc8001_state, dma_io_w) },
};

/* uPD1990A Interface */

static UPD1990A_INTERFACE( pc8001_upd1990a_intf )
{
	DEVCB_NULL,
	DEVCB_NULL
};

/* Machine Initialization */

void pc8001_state::machine_start()
{
	address_space *program = m_maincpu->memory().space(AS_PROGRAM);

	/* initialize RTC */
	m_rtc->cs_w(1);
	m_rtc->oe_w(1);

	/* initialize DMA */
	i8257_ready_w(m_dma, 1);

	/* setup memory banking */
	UINT8 *ram = ram_get_ptr(m_ram);

	memory_configure_bank(machine(), "bank1", 1, 1, machine().region("n80")->base(), 0);
	program->install_read_bank(0x0000, 0x5fff, "bank1");
	program->unmap_write(0x0000, 0x5fff);

	switch (ram_get_size(m_ram))
	{
	case 16*1024:
		memory_configure_bank(machine(), "bank3", 0, 1, ram, 0);
		program->unmap_readwrite(0x6000, 0xbfff);
		program->unmap_readwrite(0x8000, 0xbfff);
		program->install_readwrite_bank(0xc000, 0xffff, "bank3");
		break;

	case 32*1024:
		memory_configure_bank(machine(), "bank3", 0, 1, ram, 0);
		program->unmap_readwrite(0x6000, 0xbfff);
		program->install_readwrite_bank(0x8000, 0xffff, "bank3");
		break;

	case 64*1024:
		memory_configure_bank(machine(), "bank1", 0, 1, ram, 0);
		memory_configure_bank(machine(), "bank2", 0, 1, ram + 0x6000, 0);
		memory_configure_bank(machine(), "bank3", 0, 1, ram + 0x8000, 0);
		program->install_readwrite_bank(0x0000, 0x5fff, "bank1");
		program->install_readwrite_bank(0x6000, 0xbfff, "bank2");
		program->install_readwrite_bank(0x8000, 0xffff, "bank3");
		memory_set_bank(machine(), "bank2", 0);
		break;
	}

	memory_set_bank(machine(), "bank1", 1);
	memory_set_bank(machine(), "bank3", 0);

	/* register for state saving */
	save_item(NAME(m_width80));
	save_item(NAME(m_color));
}

/* Cassette Configuration */

static const cassette_interface pc8001_cassette_interface =
{
	cassette_default_formats,
	NULL,
	(cassette_state)(CASSETTE_STOPPED | CASSETTE_MOTOR_ENABLED | CASSETTE_SPEAKER_MUTED),
	NULL,
	NULL
};

/* Machine Drivers */

static MACHINE_CONFIG_START( pc8001, pc8001_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(pc8001_mem)
	MCFG_CPU_IO_MAP(pc8001_io)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(640, 220)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)

	MCFG_PALETTE_LENGTH(8)
	MCFG_PALETTE_INIT(pc8001)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_I8251_ADD(I8251_TAG, uart_intf)
	MCFG_I8255A_ADD(I8255A_TAG, ppi_intf)
	MCFG_I8257_ADD(I8257_TAG, 4000000, dmac_intf)
	MCFG_UPD1990A_ADD(UPD1990A_TAG, XTAL_32_768kHz, pc8001_upd1990a_intf)
	MCFG_UPD3301_ADD(UPD3301_TAG, 14318180, pc8001_upd3301_intf)

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, standard_centronics)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, pc8001_cassette_interface)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("16K")
	MCFG_RAM_EXTRA_OPTIONS("32K,64K")
MACHINE_CONFIG_END

static MACHINE_CONFIG_START( pc8001mk2, pc8001mk2_state )
	/* basic machine hardware */
	MCFG_CPU_ADD(Z80_TAG, Z80, 4000000)
	MCFG_CPU_PROGRAM_MAP(pc8001mk2_mem)
	MCFG_CPU_IO_MAP(pc8001mk2_io)

	/* video hardware */
	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_REFRESH_RATE(60)
	MCFG_SCREEN_VBLANK_TIME(ATTOSECONDS_IN_USEC(2500)) /* not accurate */
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)
	MCFG_SCREEN_SIZE(640, 220)
	MCFG_SCREEN_VISIBLE_AREA(0, 640-1, 0, 200-1)

	MCFG_PALETTE_LENGTH(8)
	MCFG_PALETTE_INIT(pc8001)

	/* sound hardware */
	MCFG_SPEAKER_STANDARD_MONO("mono")
	MCFG_SOUND_ADD(SPEAKER_TAG, SPEAKER_SOUND, 0)
	MCFG_SOUND_ROUTE(ALL_OUTPUTS, "mono", 0.25)

	/* devices */
	MCFG_I8251_ADD(I8251_TAG, uart_intf)
	MCFG_I8255A_ADD(I8255A_TAG, ppi_intf)
	MCFG_I8257_ADD(I8257_TAG, 4000000, dmac_intf)
	MCFG_UPD1990A_ADD(UPD1990A_TAG, XTAL_32_768kHz, pc8001_upd1990a_intf)
	MCFG_UPD3301_ADD(UPD3301_TAG, 14318180, pc8001_upd3301_intf)

	MCFG_CENTRONICS_ADD(CENTRONICS_TAG, standard_centronics)
	MCFG_CASSETTE_ADD(CASSETTE_TAG, pc8001_cassette_interface)

	MCFG_RAM_ADD(RAM_TAG)
	MCFG_RAM_DEFAULT_SIZE("64K")
MACHINE_CONFIG_END

/* ROMs */

ROM_START( pc8001 )
	ROM_REGION( 0x6000, "n80", 0 )
	ROM_SYSTEM_BIOS( 0, "v101", "N-BASIC v1.01" )
	ROMX_LOAD( "n80v101.rom", 0x00000, 0x6000, CRC(a2cc9f22) SHA1(6d2d838de7fea20ddf6601660d0525d5b17bf8a3), ROM_BIOS(1) )
	ROM_SYSTEM_BIOS( 1, "v102", "N-BASIC v1.02" )
	ROMX_LOAD( "n80v102.rom", 0x00000, 0x6000, CRC(ed01ca3f) SHA1(b34a98941499d5baf79e7c0e5578b81dbede4a58), ROM_BIOS(2) )
	ROM_SYSTEM_BIOS( 2, "v110", "N-BASIC v1.10" )
	ROMX_LOAD( "n80v110.rom", 0x00000, 0x6000, CRC(1e02d93f) SHA1(4603cdb7a3833e7feb257b29d8052c872369e713), ROM_BIOS(3) )

	ROM_REGION( 0x800, "chargen", 0)
	ROM_LOAD( "font.rom", 0x000, 0x800, CRC(56653188) SHA1(84b90f69671d4b72e8f219e1fe7cd667e976cf7f) )
ROM_END

ROM_START( pc8001mk2 )
	ROM_REGION( 0x8000, "n80", 0 )
	ROM_LOAD( "n80_2.rom", 0x00000, 0x8000, CRC(03cce7b6) SHA1(c12d34e42021110930fed45a8af98db52136f1fb) )

	ROM_REGION( 0x800, "chargen", 0)
	ROM_LOAD( "font.rom", 0x0000, 0x0800, CRC(56653188) SHA1(84b90f69671d4b72e8f219e1fe7cd667e976cf7f) )

	ROM_REGION( 0x20000, "kanji", 0)
	ROM_LOAD( "kanji1.rom", 0x00000, 0x20000, CRC(6178bd43) SHA1(82e11a177af6a5091dd67f50a2f4bafda84d6556) )
ROM_END

/* System Drivers */

/*    YEAR  NAME            PARENT      COMPAT      MACHINE     INPUT       INIT        COMPANY FULLNAME        FLAGS */
COMP( 1979, pc8001,         0,			0,			pc8001,		pc8001,		0,			"Nippon Electronic Company",	"PC-8001",		GAME_NOT_WORKING )
COMP( 1983, pc8001mk2,      pc8001,		0,			pc8001mk2,	pc8001,		0,			"Nippon Electronic Company",	"PC-8001mkII",	GAME_NOT_WORKING )
