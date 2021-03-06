/*****************************************************************************
 *
 * video/abc80.c
 *
 ****************************************************************************/

#include "emu.h"
#include "includes/abc80.h"



//-------------------------------------------------
//  gfx_layout charlayout
//-------------------------------------------------

static const gfx_layout charlayout =
{
	6, 10,
	128,
	1,
	{ 0 },
	{ 0, 1, 2, 3, 4, 5 },
	{ 0*8, 1*8, 2*8, 3*8, 4*8, 5*8, 6*8, 7*8, 8*8, 9*8 },
	10*8
};


//-------------------------------------------------
//  GFXDECODE( abc80 )
//-------------------------------------------------

static GFXDECODE_START( abc80 )
	GFXDECODE_ENTRY( "chargen", 0,	   charlayout, 0, 2 ) // normal characters
	GFXDECODE_ENTRY( "chargen", 0x500, charlayout, 0, 2 ) // graphics characters
GFXDECODE_END


//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK( blink_tick )
//-------------------------------------------------

static TIMER_DEVICE_CALLBACK( blink_tick )
{
	abc80_state *state = timer.machine().driver_data<abc80_state>();

	state->m_blink = !state->m_blink;
}


//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK( vsync_on_tick )
//-------------------------------------------------

static TIMER_DEVICE_CALLBACK( vsync_on_tick )
{
	abc80_state *state = timer.machine().driver_data<abc80_state>();

	device_set_input_line(state->m_maincpu, INPUT_LINE_NMI, ASSERT_LINE);
}


//-------------------------------------------------
//  TIMER_DEVICE_CALLBACK( vsync_off_tick )
//-------------------------------------------------

static TIMER_DEVICE_CALLBACK( vsync_off_tick )
{
	abc80_state *state = timer.machine().driver_data<abc80_state>();

	device_set_input_line(state->m_maincpu, INPUT_LINE_NMI, CLEAR_LINE);
}


//-------------------------------------------------
//  update_screen -
//-------------------------------------------------

void abc80_state::update_screen(bitmap_t *bitmap, const rectangle *cliprect)
{
	int c = 0;
	int r = 0;
	int mode = 0;

	for (int y = 0; y < 312; y++)
	{
		UINT8 vsync_data = m_vsync_prom[y];
		UINT8 l = m_line_prom[y];
		int dv = (vsync_data & ABC80_K2_DV) ? 1 : 0;

		if (!(vsync_data & ABC80_K2_FRAME_RESET))
		{
			// reset F2
			r = 0;
		}

		for (int sx = 0; sx < 64; sx++)
		{
			UINT8 hsync_data = m_hsync_prom[sx];
			int dh = (hsync_data & ABC80_K5_DH) ? 1 : 0;
			UINT8 data = 0;

			if (hsync_data & ABC80_K5_LINE_END)
			{
				// reset F4
				c = 0;

				// reset J5
				mode = 0;
			}

			/*

                Video RAM Addressing Scheme

                A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
                R2 R1 R0 xx xx xx xx C2 C1 C0

                A6 A5 A4 A3 = 00 C5 C4 C3 + R4 R3 R4 R3

            */

			int a = (c >> 3) & 0x07;
			int b = ((r >> 1) & 0x0c) | ((r >> 3) & 0x03);
			int s = (a + b) & 0x0f;
			UINT16 videoram_addr = ((r & 0x07) << 7) | (s << 3) | (c & 0x07);
			UINT8 videoram_data = m_latch;
			UINT8 attr_addr = ((dh & dv) << 7) | (videoram_data & 0x7f);
			UINT8 attr_data = m_attr_prom[attr_addr];

			int blank = (attr_data & ABC80_J3_BLANK) ? 1 : 0;
			int j = (attr_data & ABC80_J3_TEXT) ? 1 : 0;
			int k = (attr_data & ABC80_J3_GRAPHICS) ? 1 : 0;
			int versal = (attr_data & ABC80_J3_VERSAL) ? 1 : 0;
			int cursor = (videoram_data & ABC80_CHAR_CURSOR) ? 1 : 0;

			if (!j && k) mode = 0;
			if (j && !k) mode = 1;
			if (j && k) mode = !mode;

			if (mode & versal)
			{
				// graphics mode
				int r0 = 1, r1 = 1, r2 = 1;

				if (l < 3) r0 = 0; else if (l < 7) r1 = 0; else r2 = 0;

				int c0 = BIT(videoram_data, 0) | r0;
				int c1 = BIT(videoram_data, 1) | r0;
				int c2 = BIT(videoram_data, 2) | r1;
				int c3 = BIT(videoram_data, 3) | r1;
				int c4 = BIT(videoram_data, 4) | r2;
				int c5 = BIT(videoram_data, 6) | r2;

				if (!(c0 & c2 & c4)) data |= 0xe0;
				if (!(c1 & c3 & c5)) data |= 0x1c;
			}
			else
			{
				// text mode
				UINT16 chargen_addr = ((videoram_data & 0x7f) * 10) + l;

				data = m_char_rom[chargen_addr];
			}

			// shift out pixels
			for (int bit = 0; bit < 6; bit++)
			{
				int color = BIT(data, 7);
				int x = (sx * 6) + bit;

				color ^= (cursor & m_blink);
				color &= blank;

				*BITMAP_ADDR16(bitmap, y, x) = color;

				data <<= 1;
			}

			m_latch = m_video_ram[videoram_addr];

			if (hsync_data & ABC80_K5_ROW_START)
			{
				// clock F4
				c++;
			}
		}

		if (vsync_data & ABC80_K2_FRAME_END)
		{
			// clock F2
			r++;
		}
	}
}


//-------------------------------------------------
//  VIDEO_START( abc80 )
//-------------------------------------------------

void abc80_state::video_start()
{
	// find memory regions
	m_char_rom = machine().region("chargen")->base();
	m_hsync_prom = machine().region("hsync")->base();
	m_vsync_prom = machine().region("vsync")->base();
	m_line_prom = machine().region("line")->base();
	m_attr_prom = machine().region("attr")->base();

	// register for state saving
	state_save_register_global(machine(), m_blink);
	state_save_register_global(machine(), m_latch);
}


//-------------------------------------------------
//  SCREEN_UPDATE( abc80 )
//-------------------------------------------------

bool abc80_state::screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect)
{
	update_screen(&bitmap, &cliprect);

	return 0;
}


//-------------------------------------------------
//  MACHINE_CONFIG_FRAGMENT( abc80_video )
//-------------------------------------------------

MACHINE_CONFIG_FRAGMENT( abc80_video )
	MCFG_TIMER_ADD_PERIODIC("blink", blink_tick, attotime::from_hz(ABC80_XTAL/2/6/64/312/16))
	MCFG_TIMER_ADD_SCANLINE("vsync_on", vsync_on_tick, SCREEN_TAG, 0, ABC80_VTOTAL)
	MCFG_TIMER_ADD_SCANLINE("vsync_off", vsync_off_tick, SCREEN_TAG, 16, ABC80_VTOTAL)

	MCFG_SCREEN_ADD(SCREEN_TAG, RASTER)
	MCFG_SCREEN_FORMAT(BITMAP_FORMAT_INDEXED16)

	MCFG_GFXDECODE(abc80)

	MCFG_PALETTE_LENGTH(2)
	MCFG_PALETTE_INIT(black_and_white)

	MCFG_SCREEN_RAW_PARAMS(ABC80_XTAL/2, ABC80_HTOTAL, ABC80_HBEND, ABC80_HBSTART, ABC80_VTOTAL, ABC80_VBEND, ABC80_VBSTART)
MACHINE_CONFIG_END
