/* Core includes */
#include "emu.h"
#include "includes/kc.h"

#define KC_DEBUG 0
#define LOG(x) do { if (KC_DEBUG) logerror x; } while (0)

struct kcc_header
{
	UINT8	name[10];
	UINT8	reserved[6];
	UINT8	number_addresses;
	UINT8	load_address_l;
	UINT8	load_address_h;
	UINT8	end_address_l;
	UINT8	end_address_h;
	UINT8	execution_address_l;
	UINT8	execution_address_h;
	UINT8	pad[128-2-2-2-1-16];
};

/* appears to work a bit.. */
/* load file, then type: MENU and it should now be displayed. */
/* now type name that has appeared! */

/* load snapshot */
QUICKLOAD_LOAD(kc)
{
	kc_state *state = image.device().machine().driver_data<kc_state>();
	UINT8 *data;
	struct kcc_header *header;
	int addr;
	int datasize;
	int execution_address;
	int i;

	/* get file size */
	datasize = image.length();

	if (datasize != 0)
	{
		/* malloc memory for this data */
		data = (UINT8 *)auto_alloc_array(image.device().machine(), UINT8, datasize);

		if (data != NULL)
			image.fread( data, datasize);
	}
	else
	{
		return IMAGE_INIT_FAIL;
	}

	header = (struct kcc_header *) data;
	addr = (header->load_address_l & 0x0ff) | ((header->load_address_h & 0x0ff)<<8);
	datasize = ((header->end_address_l & 0x0ff) | ((header->end_address_h & 0x0ff)<<8)) - addr;
	execution_address = (header->execution_address_l & 0x0ff) | ((header->execution_address_h & 0x0ff)<<8);

	if (datasize + 128 > image.length())
	{
		mame_printf_info("Invalid snapshot size: expected 0x%04x, found 0x%04x\n", datasize, (UINT32)image.length() - 128);
		datasize = image.length() - 128;
	}

	address_space *space = state->m_maincpu->memory().space( AS_PROGRAM );

	for (i=0; i<datasize; i++)
		space->write_byte((addr+i) & 0xffff, data[i+128]);

	if (execution_address != 0 && header->number_addresses >= 3 )
	{
		// if specified, jumps to the quickload start address
		cpu_set_reg(state->m_maincpu, STATE_GENPC, execution_address);
	}

	auto_free(image.device().machine(), data);

	logerror("Snapshot loaded at: 0x%04x-0x%04x, execution address: 0x%04x\n", addr, addr + datasize - 1, execution_address);

	return IMAGE_INIT_PASS;
}


//**************************************************************************
//  MODULE SYSTEM EMULATION
//**************************************************************************

// The KC85/4 and KC85/3 are "modular systems". These computers can be expanded with modules.

/*
    Module ID       Module Name         Module Description


                    D001                Basis Device
                    D002                Bus Driver device
    a7              D004                Floppy Disk Interface Device


    ef              M001                Digital IN/OUT
    ee              M003                V24
                    M005                User (empty)
                    M007                Adapter (empty)
    e7              M010                ADU1
    f6              M011                64k RAM
    fb              M012                Texor
    f4              M022                Expander RAM (16k)
    f7              M025                User PROM (8k)
    fb              M026                Forth
    fb              M027                Development
    e3              M029                DAU1
*/

READ8_MEMBER( kc_state::expansion_read )
{
	UINT8 result = 0xff;

	// assert MEI line of first slot
	m_expansions[0]->mei_w(ASSERT_LINE);

	for (int i=0; i<3; i++)
		m_expansions[i]->read(offset, result);

	return result;
}

WRITE8_MEMBER( kc_state::expansion_write )
{
	// assert MEI line of first slot
	m_expansions[0]->mei_w(ASSERT_LINE);

	for (int i=0; i<3; i++)
		m_expansions[i]->write(offset, data);
}

/*
    port xx80

    - xx is module id.

    Only addressess divisible by 4 are checked.
    If module does not exist, 0x0ff is returned.

    When xx80 is read, if a module exists a id will be returned.
    Id's for known modules are listed above.
*/

READ8_MEMBER( kc_state::expansion_io_read )
{
	UINT8 result = 0xff;

	// assert MEI line of first slot
	m_expansions[0]->mei_w(ASSERT_LINE);

	if ((offset & 0xff) == 0x80)
	{
		UINT8 slot_id = (offset>>8) & 0xff;

		if (slot_id == 0x08 || slot_id == 0x0c)
			result = m_expansions[(slot_id - 8) >> 2]->module_id_r();
		else
			m_expansions[2]->io_read(offset, result);
	}
	else
	{
		for (int i=0; i<3; i++)
			m_expansions[i]->io_read(offset, result);
	}

	return result;
}

WRITE8_MEMBER( kc_state::expansion_io_write )
{
	// assert MEI line of first slot
	m_expansions[0]->mei_w(ASSERT_LINE);

	if ((offset & 0xff) == 0x80)
	{
		UINT8 slot_id = (offset>>8) & 0xff;

		if (slot_id == 0x08 || slot_id == 0x0c)
			m_expansions[(slot_id - 8) >> 2]->control_w(data);
		else
			m_expansions[2]->io_write(offset, data);
	}
	else
	{
		for (int i=0; i<3; i++)
			m_expansions[i]->io_write(offset, data);
	}
}

// module read/write handlers
READ8_MEMBER ( kc_state::expansion_4000_r ){ return expansion_read(space, offset + 0x4000); }
WRITE8_MEMBER( kc_state::expansion_4000_w ){ expansion_write(space, offset + 0x4000, data); }
READ8_MEMBER ( kc_state::expansion_8000_r ){ return expansion_read(space, offset + 0x8000); }
WRITE8_MEMBER( kc_state::expansion_8000_w ){ expansion_write(space, offset + 0x8000, data); }
READ8_MEMBER ( kc_state::expansion_c000_r ){ return expansion_read(space, offset + 0xc000); }
WRITE8_MEMBER( kc_state::expansion_c000_w ){ expansion_write(space, offset + 0xc000, data); }
READ8_MEMBER ( kc_state::expansion_e000_r ){ return expansion_read(space, offset + 0xe000); }
WRITE8_MEMBER( kc_state::expansion_e000_w ){ expansion_write(space, offset + 0xe000, data); }


//**************************************************************************
//  CASSETTE EMULATION
//**************************************************************************

/*
    The cassette motor is controlled by bit 6 of PIO port A.
    The cassette read data is connected to /ASTB input of the PIO.
    A edge from the cassette therefore will trigger a interrupt
    from the PIO.
    The duration between two edges can be timed and the data-bit
    identified.

    I have used a timer to feed data into /ASTB. The timer is only
    active when the cassette motor is on.
*/


#define KC_CASSETTE_TIMER_FREQUENCY attotime::from_hz(4800)

/* this timer is used to update the cassette */
/* this is the current state of the cassette motor */
/* ardy output from pio */

static TIMER_CALLBACK(kc_cassette_timer_callback)
{
	kc_state *state = machine.driver_data<kc_state>();

	/* the cassette data is linked to /astb input of the pio. */
	int bit = (state->m_cassette->input() > 0.0038) ? 1 : 0;

	/* update astb with bit */
	z80pio_astb_w(state->m_z80pio, bit & state->m_ardy);
}

void kc_state::cassette_set_motor(int motor_state)
{
	/* set new motor state in cassette device */
	m_cassette->change_state(motor_state ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

	if (motor_state)
	{
		/* start timer */
		m_cassette_timer->adjust(attotime::zero, 0, KC_CASSETTE_TIMER_FREQUENCY);
	}
	else
	{
		/* stop timer */
		m_cassette_timer->reset();
	}
}

/*
  pin 2 = gnd
  pin 3 = read
  pin 1 = k1        ?? modulating signal
  pin 4 = k0        ?? signal??
  pin 5 = motor on


    Tape signals:
        K0, K1      ??
        MOTON       motor control
        ASTB        read?

        T1-T4 give 4 bit a/d tone sound?
        K1, K0 are mixed with tone.

    Cassette read goes into ASTB of PIO.
    From this, KC must be able to detect the length
    of the pulses and can read the data.


    Tape write: clock comes from CTC?
    truck signal resets, 5v signal for set.
    output gives k0 and k1.

*/



//**************************************************************************
//  KC85 bankswitch
//**************************************************************************

/* update status of memory area 0x0000-0x03fff */
void kc_state::update_0x00000()
{
	address_space *space = m_maincpu->memory().space( AS_PROGRAM );

	/* access ram? */
	if (m_pio_data[0] & (1<<1))
	{
		LOG(("ram0 enabled\n"));

		/* yes; set address of bank */
		space->install_read_bank(0x0000, 0x3fff, "bank1");
		memory_set_bankptr(machine(), "bank1", m_ram_base);

		/* write protect ram? */
		if ((m_pio_data[0] & (1<<3)) == 0)
		{
			/* yes */
			LOG(("ram0 write protected\n"));

			/* ram is enabled and write protected */
			space->unmap_write(0x0000, 0x3fff);
		}
		else
		{
			LOG(("ram0 write enabled\n"));

			/* ram is enabled and write enabled */
			space->install_write_bank(0x0000, 0x3fff, "bank1");
		}
	}
	else
	{
		LOG(("Module at 0x0000\n"));

		space->install_read_handler (0x0000, 0x3fff, 0, 0, read8_delegate(FUNC(kc_state::expansion_read), this), 0);
		space->install_write_handler(0x0000, 0x3fff, 0, 0, write8_delegate(FUNC(kc_state::expansion_write), this), 0);
	}
}

/* update status of memory area 0x4000-0x07fff */
void kc_state::update_0x04000()
{
	address_space *space = m_maincpu->memory().space( AS_PROGRAM );

	LOG(("Module at 0x4000\n"));

	space->install_read_handler (0x4000, 0x7fff, 0, 0, read8_delegate(FUNC(kc_state::expansion_4000_r), this), 0);
	space->install_write_handler(0x4000, 0x7fff, 0, 0, write8_delegate(FUNC(kc_state::expansion_4000_w), this), 0);

}


/* update memory address 0x0c000-0x0e000 */
void kc_state::update_0x0c000()
{
	address_space *space = m_maincpu->memory().space( AS_PROGRAM );

	if ((m_pio_data[0] & (1<<7)) && machine().region("basic")->base() != NULL)
	{
		/* BASIC takes next priority */
        	LOG(("BASIC rom 0x0c000\n"));

        memory_set_bankptr(machine(), "bank4", machine().region("basic")->base());
		space->install_read_bank(0xc000, 0xdfff, "bank4");
		space->unmap_write(0xc000, 0xdfff);
	}
	else
	{
		LOG(("Module at 0x0c000\n"));

		space->install_read_handler (0xc000, 0xdfff, 0, 0, read8_delegate(FUNC(kc_state::expansion_c000_r), this), 0);
		space->install_write_handler(0xc000, 0xdfff, 0, 0, write8_delegate(FUNC(kc_state::expansion_c000_w), this), 0);
	}
}

/* update memory address 0x0e000-0x0ffff */
void kc_state::update_0x0e000()
{
	address_space *space = m_maincpu->memory().space( AS_PROGRAM );

	if (m_pio_data[0] & (1<<0))
	{
		/* enable CAOS rom in memory range 0x0e000-0x0ffff */
		LOG(("CAOS rom 0x0e000\n"));
		/* read will access the rom */
		memory_set_bankptr(machine(), "bank5", machine().region("caos")->base() + 0x2000);
		space->install_read_bank(0xe000, 0xffff, "bank5");
		space->unmap_write(0xe000, 0xffff);
	}
	else
	{
		LOG(("Module at 0x0e000\n"));

		space->install_read_handler (0xe000, 0xffff, 0, 0, read8_delegate(FUNC(kc_state::expansion_e000_r), this), 0);
		space->install_write_handler(0xe000, 0xffff, 0, 0, write8_delegate(FUNC(kc_state::expansion_e000_w), this), 0);
	}
}


/* update status of memory area 0x08000-0x0ffff */
void kc_state::update_0x08000()
{
	address_space *space = m_maincpu->memory().space( AS_PROGRAM );

    if (m_pio_data[0] & (1<<2))
    {
        /* IRM enabled */
        LOG(("IRM enabled\n"));

		memory_set_bankptr(machine(), "bank3", m_ram_base + 0x4000);
		space->install_readwrite_bank(0x8000, 0xbfff, "bank3");
    }
    else
    {
		LOG(("Module at 0x8000!\n"));

		space->install_read_handler(0x8000, 0xbfff, 0, 0, read8_delegate(FUNC(kc_state::expansion_8000_r), this), 0);
		space->install_write_handler(0x8000, 0xbfff, 0, 0, write8_delegate(FUNC(kc_state::expansion_8000_w), this), 0);
    }
}


/* update status of memory area 0x4000-0x07fff */
void kc85_4_state::update_0x04000()
{
	address_space *space = m_maincpu->memory().space( AS_PROGRAM );

	/* access ram? */
	if (m_port_86_data & (1<<0))
	{
		LOG(("RAM4 enabled\n"));

		/* yes */
		space->install_read_bank(0x4000, 0x7fff, "bank2");
		/* set address of bank */
		memory_set_bankptr(machine(), "bank2", m_ram_base + 0x4000);

		/* write protect ram? */
		if ((m_port_86_data & (1<<1)) == 0)
		{
			/* yes */
			LOG(("ram4 write protected\n"));

			/* ram is enabled and write protected */
			space->nop_write(0x4000, 0x7fff);
		}
		else
		{
			LOG(("ram4 write enabled\n"));

			/* ram is enabled and write enabled */
			space->install_write_bank(0x4000, 0x7fff, "bank2");
		}
	}
	else
	{
		LOG(("Module at 0x4000\n"));

		space->install_read_handler (0x4000, 0x7fff, 0, 0, read8_delegate(FUNC(kc_state::expansion_4000_r), this), 0);
		space->install_write_handler(0x4000, 0x7fff, 0, 0, write8_delegate(FUNC(kc_state::expansion_4000_w), this), 0);
	}

}

/* update memory address 0x0c000-0x0e000 */
void kc85_4_state::update_0x0c000()
{
	address_space *space = m_maincpu->memory().space( AS_PROGRAM );

	if (m_port_86_data & (1<<7))
	{
		/* CAOS rom takes priority */
		LOG(("CAOS rom 0x0c000\n"));

		memory_set_bankptr(machine(), "bank4", machine().region("caos")->base());
		space->install_read_bank(0xc000, 0xdfff, "bank4");
		space->unmap_write(0xc000, 0xdfff);
	}
	else
	{
		kc_state::update_0x0c000();
	}
}

void kc85_4_state::update_0x08000()
{
	address_space *space = m_maincpu->memory().space( AS_PROGRAM );

	if (m_pio_data[0] & (1<<2))
	{
		/* IRM enabled - has priority over RAM8 enabled */
		LOG(("IRM enabled\n"));

		UINT8* ram_page = (UINT8*)kc85_4_get_video_ram_base(machine(), (m_port_84_data & 0x04), (m_port_84_data & 0x02));

		memory_set_bankptr(machine(), "bank3", ram_page);
		space->install_readwrite_bank(0x8000, 0xa7ff, "bank3");

		ram_page = (UINT8*)kc85_4_get_video_ram_base(machine(), 0, 0);

		memory_set_bankptr(machine(), "bank6", ram_page + 0x2800);
		space->install_readwrite_bank(0xa800, 0xbfff, "bank6");
	}
    else if (m_pio_data[1] & (1<<5))
    {
		LOG(("RAM8 enabled\n"));

		int ram8_block;
		UINT8 *mem_ptr;

		/* ram8 block chosen */

		if (ram_get_size(m_ram) == 64 * 1024)
		{
			// kc85_4 64K RAM
			ram8_block = ((m_port_84_data)>>4) & 0x01;
			mem_ptr = m_ram_base + 0x8000 + (ram8_block<<14);
		}
		else
		{
			// kc85_5 224K RAM
			if ((m_port_84_data & 0x0e) == 0)
			{
				ram8_block = ((m_port_84_data)>>4) & 0x01;
				mem_ptr = m_ram_base + (ram8_block<<14);
			}
			else
			{
				ram8_block = (((m_port_84_data)>>4) & 0x0f) - 2;
				mem_ptr = m_ram_base + (ram8_block<<14);
			}
		}

		memory_set_bankptr(machine(), "bank3", mem_ptr);
		memory_set_bankptr(machine(), "bank6", mem_ptr + 0x2800);
		space->install_read_bank(0x8000, 0xa7ff, "bank3");
		space->install_read_bank(0xa800, 0xbfff, "bank6");

		/* write protect RAM8 ? */
		if ((m_pio_data[1] & (1<<6)) == 0)
		{
			/* ram8 is enabled and write protected */
			LOG(("RAM8 write protected\n"));

			space->nop_write(0x8000, 0xa7ff);
			space->nop_write(0xa800, 0xbfff);
		}
		else
		{
			LOG(("RAM8 write enabled\n"));

			/* ram8 is enabled and write enabled */
			space->install_write_bank(0x8000, 0xa7ff, "bank3");
			space->install_write_bank(0xa800, 0xbfff, "bank6");
		}
    }
    else
    {
		LOG(("Module at 0x8000\n"));

		space->install_read_handler(0x8000, 0xbfff, 0, 0, read8_delegate(FUNC(kc_state::expansion_8000_r), this), 0);
		space->install_write_handler(0x8000, 0xbfff, 0, 0, write8_delegate(FUNC(kc_state::expansion_8000_w), this), 0);
    }
}

//**************************************************************************
//  KC85 Z80PIO Interface
//**************************************************************************


/* PIO PORT A: port 0x088:

bit 7: ROM C (BASIC)
bit 6: Tape Motor on
bit 5: LED
bit 4: K OUT
bit 3: WRITE PROTECT RAM 0
bit 2: IRM
bit 1: ACCESS RAM 0
bit 0: CAOS ROM E
*/

READ8_MEMBER( kc_state::pio_porta_r )
{
	return m_pio_data[0];
}

WRITE8_MEMBER( kc_state::pio_porta_w )
{
	if (m_pio_data[0] != data) // to avoid a severe slowdown during cassette loading
	{
		m_pio_data[0] = data;

		update_0x00000();
		update_0x08000();
		update_0x0c000();
		update_0x0e000();

		cassette_set_motor(BIT(data, 6));
	}
}


/* PIO PORT B: port 0x089:
bit 7: BLINK ENABLE
bit 6: WRITE PROTECT RAM 8
bit 5: ACCESS RAM 8
bit 4: TONE 4
bit 3: TONE 3
bit 2: TONE 2
bit 1: TONE 1
bit 0: TRUCK */

READ8_MEMBER( kc_state::pio_portb_r )
{
	return m_pio_data[1];
}

WRITE8_MEMBER( kc_state::pio_portb_w )
{
	m_pio_data[1] = data;

	update_0x08000();

	/* 16 speaker levels */
	m_speaker_level = (data>>1) & 0x0f;

	speaker_update();
}

/* port 0x84/0x85:

bit 7: RAF3
bit 6: RAF2
bit 5: RAF1
bit 4: RAF0
bit 3: FPIX. high resolution
bit 2: BLA1 .access screen
bit 1: BLA0 .pixel/color
bit 0: BILD .display screen 0 or 1
*/

WRITE8_MEMBER( kc85_4_state::kc85_4_84_w )
{
	LOG(("0x84 W: %02x\n", data));

	m_port_84_data = data;

	kc85_4_video_ram_select_bank(machine(), data & 0x01);

	m_high_resolution = (data & 0x08) ? 0 : 1;

	update_0x08000();
}

READ8_MEMBER( kc85_4_state::kc85_4_84_r )
{
	return m_port_84_data;
}


/* port 0x86/0x87:

bit 7: ROCC
bit 6: ROF1
bit 5: ROF0
bit 4-2 are not connected
bit 1: WRITE PROTECT RAM 4
bit 0: ACCESS RAM 4
*/

WRITE8_MEMBER( kc85_4_state::kc85_4_86_w )
{
	LOG(("0x86 W: %02x\n", data));

	m_port_86_data = data;

	update_0x0c000();
	update_0x04000();
}

READ8_MEMBER( kc85_4_state::kc85_4_86_r )
{
	return m_port_86_data;
}

/*****************************************************************/


/* callback for ardy output from PIO */
/* used in KC85/4 & KC85/3 cassette interface */
WRITE_LINE_MEMBER( kc_state::pio_ardy_cb)
{
	m_ardy = state & 0x01;
}

/* callback for brdy output from PIO */
/* used in KC85/4 & KC85/3 keyboard interface */
WRITE_LINE_MEMBER( kc_state::pio_brdy_cb)
{
	m_brdy = state & 0x01;
}

/* used in cassette write -> K0 */
WRITE_LINE_MEMBER( kc_state::ctc_zc0_callback )
{
	if (state)
	{
		m_k0_line^=1;
		speaker_update();
	}
}

/* used in cassette write -> K1 */
WRITE_LINE_MEMBER( kc_state::ctc_zc1_callback)
{
	if (state)
	{
		m_k1_line^=1;
		speaker_update();

		// K1 line is also cassette output
		m_cassette->output((m_k1_line & 1) ? +1 : -1);
	}

}

static TIMER_CALLBACK(kc85_15khz_timer_callback )
{
	kc_state *state = machine.driver_data<kc_state>();

	/* toggle state of square wave */
	state->m_kc85_15khz_state^=1;

	/* set clock input for channel 2 and 3 to ctc */
	z80ctc_trg0_w(state->m_z80ctc, state->m_kc85_15khz_state);
	z80ctc_trg1_w(state->m_z80ctc, state->m_kc85_15khz_state);
}

static TIMER_CALLBACK(kc85_50hz_timer_callback)
{
	kc_state *state = machine.driver_data<kc_state>();

	/* toggle state of square wave */
	state->m_kc85_50hz_state^=1;

	/* set clock input for channel 2 and 3 to ctc */
	z80ctc_trg2_w(state->m_z80ctc, state->m_kc85_50hz_state);
	z80ctc_trg3_w(state->m_z80ctc, state->m_kc85_50hz_state);
}

/* video blink */
WRITE_LINE_MEMBER( kc_state::ctc_zc2_callback )
{
	/* is blink enabled? */
	if (m_pio_data[1] & (1<<7))
	{
		/* yes */
		/* toggle state of blink to video hardware */
		kc85_video_set_blink_state(machine(), state);
	}
}

void kc_state::speaker_update()
{
	/* this might not be correct, the range might be logarithmic and not linear! */
	speaker_level_w(m_speaker, m_k0_line ? (m_speaker_level | (m_k1_line ? 0x01 : 0)) : 0);
}

/* keyboard callback */
WRITE_LINE_MEMBER( kc_state::keyboard_cb )
{
	z80pio_bstb_w(m_z80pio, state & m_brdy);

	// FIXME: understand why the PIO fail to acknowledge the irq on kc85_2/3
	z80pio_d_w(m_z80pio, 1, m_pio_data[1]);
}


void kc_state::machine_start()
{
	m_cassette_timer = machine().scheduler().timer_alloc(FUNC(kc_cassette_timer_callback));

	// kc85 has a 50 Hz input to the ctc channel 2 and 3
	// channel 2 this controls the video colour flash
	// kc85 has a 15 kHz (?) input to the ctc channel 0 and 1
	// channel 0 and channel 1 are used for cassette write
	machine().scheduler().timer_pulse(attotime::from_hz(50), FUNC(kc85_50hz_timer_callback), 0, NULL);
	machine().scheduler().timer_pulse(attotime::from_hz(15625), FUNC(kc85_15khz_timer_callback), 0, NULL);

	m_ram_base = ram_get_ptr(m_ram);

	m_expansions[0] = machine().device<kcexp_slot_device>("m1");
	m_expansions[1] = machine().device<kcexp_slot_device>("m2");
	m_expansions[2] = machine().device<kcexp_slot_device>("exp");
}

void kc_state::machine_reset()
{
	m_pio_data[0] = 0x0f;
	m_pio_data[1] = 0xf1;

	update_0x00000();
	update_0x04000();
	update_0x08000();
	update_0x0c000();
	update_0x0e000();

	m_kc85_50hz_state = 0;
	m_kc85_15khz_state = 0;

	// set low resolution at reset
	m_high_resolution = 0;

	cassette_set_motor(0);

	/* this is temporary. Normally when a Z80 is reset, it will
    execute address 0. It appears the KC85 series pages the rom
    at address 0x0000-0x01000 which has a single jump in it,
    can't see yet where it disables it later!!!! so for now
    here will be a override */
	cpu_set_reg(m_maincpu, STATE_GENPC, 0x0f000);
}

void kc85_4_state::machine_reset()
{
	kc_state::machine_reset();

	m_port_84_data = 0x00;
	m_port_86_data = 0x00;
}
