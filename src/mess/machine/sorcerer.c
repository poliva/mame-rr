/******************************************************************************

  Exidy Sorcerer machine functions

*******************************************************************************/

#include "includes/sorcerer.h"

#if SORCERER_USING_RS232

/* The serial code (which was never connected to the outside) is disabled for now. */

/* timer for sorcerer serial chip transmit and receive */

static TIMER_CALLBACK(sorcerer_serial_tc)
{
	sorcerer_state *state = machine.driver_data<sorcerer_state>();
	/* if rs232 is enabled, uart is connected to clock defined by bit6 of port fe.
    Transmit and receive clocks are connected to the same clock */

	/* if rs232 is disabled, receive clock is linked to cassette hardware */
	if (state->m_fe & 0x80)
	{
		/* connect to rs232 */
	}
}
#endif


/* timer to read cassette waveforms */


static cassette_image_device *cassette_device_image(running_machine &machine)
{
	sorcerer_state *state = machine.driver_data<sorcerer_state>();
	if (state->m_fe & 0x20)
		return machine.device<cassette_image_device>(CASSETTE2_TAG);
	else
		return machine.device<cassette_image_device>(CASSETTE_TAG);
}



static TIMER_CALLBACK(sorcerer_cassette_tc)
{
	sorcerer_state *state = machine.driver_data<sorcerer_state>();
	UINT8 cass_ws = 0;
	switch (state->m_fe & 0xc0)		/*/ bit 7 low indicates cassette */
	{
		case 0x00:				/* Cassette 300 baud */

			/* loading a tape - this is basically the same as the super80.
                           We convert the 1200/2400 Hz signal to a 0 or 1, and send it to the uart. */

			state->m_cass_data.input.length++;

			cass_ws = ((cassette_device_image(machine))->input() > +0.02) ? 1 : 0;

			if (cass_ws != state->m_cass_data.input.level)
			{
				state->m_cass_data.input.level = cass_ws;
				state->m_cass_data.input.bit = ((state->m_cass_data.input.length < 0x6) || (state->m_cass_data.input.length > 0x20)) ? 1 : 0;
				state->m_cass_data.input.length = 0;
				ay31015_set_input_pin( state->m_uart, AY31015_SI, state->m_cass_data.input.bit );
			}

			/* saving a tape - convert the serial stream from the uart, into 1200 and 2400 Hz frequencies.
                           Synchronisation of the frequency pulses to the uart is extremely important. */

			state->m_cass_data.output.length++;
			if (!(state->m_cass_data.output.length & 0x1f))
			{
				cass_ws = ay31015_get_output_pin( state->m_uart, AY31015_SO );
				if (cass_ws != state->m_cass_data.output.bit)
				{
					state->m_cass_data.output.bit = cass_ws;
					state->m_cass_data.output.length = 0;
				}
			}

			if (!(state->m_cass_data.output.length & 3))
			{
				if (!((state->m_cass_data.output.bit == 0) && (state->m_cass_data.output.length & 4)))
				{
					state->m_cass_data.output.level ^= 1;			// toggle output state, except on 2nd half of low bit
					cassette_device_image(machine)->output(state->m_cass_data.output.level ? -1.0 : +1.0);
				}
			}
			return;

		case 0x40:			/* Cassette 1200 baud */
			/* loading a tape */
			state->m_cass_data.input.length++;

			cass_ws = ((cassette_device_image(machine))->input() > +0.02) ? 1 : 0;

			if (cass_ws != state->m_cass_data.input.level || state->m_cass_data.input.length == 10)
			{
				state->m_cass_data.input.bit = ((state->m_cass_data.input.length < 10) || (state->m_cass_data.input.length > 0x20)) ? 1 : 0;
				if ( cass_ws != state->m_cass_data.input.level )
				{
					state->m_cass_data.input.length = 0;
					state->m_cass_data.input.level = cass_ws;
				}
				ay31015_set_input_pin( state->m_uart, AY31015_SI, state->m_cass_data.input.bit );
			}

			/* saving a tape - convert the serial stream from the uart, into 600 and 1200 Hz frequencies. */

			state->m_cass_data.output.length++;
			if (!(state->m_cass_data.output.length & 7))
			{
				cass_ws = ay31015_get_output_pin( state->m_uart, AY31015_SO );
				if (cass_ws != state->m_cass_data.output.bit)
				{
					state->m_cass_data.output.bit = cass_ws;
					state->m_cass_data.output.length = 0;
				}
			}

			if (!(state->m_cass_data.output.length & 7))
			{
				if (!((state->m_cass_data.output.bit == 0) && (state->m_cass_data.output.length & 8)))
				{
					state->m_cass_data.output.level ^= 1;			// toggle output state, except on 2nd half of low bit
					cassette_device_image(machine)->output(state->m_cass_data.output.level ? -1.0 : +1.0);
				}
			}
			return;
	}
}


/* after the first 4 bytes have been read from ROM, switch the ram back in */
static TIMER_CALLBACK( sorcerer_reset )
{
	memory_set_bank(machine, "boot", 0);
}

WRITE8_MEMBER(sorcerer_state::sorcerer_fc_w)
{
	ay31015_set_transmit_data( m_uart, data );
}


WRITE8_MEMBER(sorcerer_state::sorcerer_fd_w)
{
	/* Translate data to control signals */

	ay31015_set_input_pin( m_uart, AY31015_CS, 0 );
	ay31015_set_input_pin( m_uart, AY31015_NB1, data & 1);
	ay31015_set_input_pin( m_uart, AY31015_NB2, (BIT(data, 1)) ? 1 : 0 );
	ay31015_set_input_pin( m_uart, AY31015_TSB, (BIT(data, 2)) ? 1 : 0 );
	ay31015_set_input_pin( m_uart, AY31015_EPS, (BIT(data, 3)) ? 1 : 0 );
	ay31015_set_input_pin( m_uart, AY31015_NP,  (BIT(data, 4)) ? 1 : 0 );
	ay31015_set_input_pin( m_uart, AY31015_CS, 1 );
}

WRITE8_MEMBER(sorcerer_state::sorcerer_fe_w)
{
	UINT8 changed_bits = (m_fe ^ data) & 0xf0;
	m_fe = data;

	/* bits 0..3 */
	m_keyboard_line = data & 0x0f;

	if (!changed_bits) return;

	/* bits 4..5 */
	/* does user want to hear the sound? */

	if (!BIT(data, 7)) // cassette operations
	{
#if SORCERER_USING_RS232
		m_serial_timer->adjust(attotime::zero);
#endif

		UINT8 sound = input_port_read(machine(), "CONFIG") & 8;

		m_cass1->change_state(
			((BIT(data,4)) && (sound)) ? CASSETTE_SPEAKER_ENABLED : CASSETTE_SPEAKER_MUTED, CASSETTE_MASK_SPEAKER);

		m_cass2->change_state(
			((BIT(data,5)) && (sound)) ? CASSETTE_SPEAKER_ENABLED : CASSETTE_SPEAKER_MUTED, CASSETTE_MASK_SPEAKER);

		/* cassette 1 motor */
		m_cass1->change_state(
			(BIT(data,4)) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

		/* cassette 2 motor */
		m_cass2->change_state(
			(BIT(data,5)) ? CASSETTE_MOTOR_ENABLED : CASSETTE_MOTOR_DISABLED, CASSETTE_MASK_MOTOR);

		if (data & 0x30)
			m_cassette_timer->adjust(attotime::zero, 0, attotime::from_hz(19200));
		else
			m_cassette_timer->adjust(attotime::zero);
	}
#if SORCERER_USING_RS232
	else
	{
		m_serial_timer->adjust(attotime::zero, 0, attotime::from_hz(19200));
		m_cassette_timer->adjust(attotime::zero);
	}
#endif

	// bit 6 baud rate */
	if (BIT(changed_bits, 6))
	{
		ay31015_set_receiver_clock( m_uart, (BIT(data, 6)) ? 19200.0 : 4800.0);
		ay31015_set_transmitter_clock( m_uart, (BIT(data, 6)) ? 19200.0 : 4800.0);
	}
}

WRITE8_MEMBER(sorcerer_state::sorcerer_ff_w)
{
	/* reading the config switch */
	switch (input_port_read(machine(), "CONFIG") & 0x06)
	{
		case 0: /* speaker */
			dac_data_w(m_dac, data);
			break;

		case 2: /* Centronics 7-bit printer */
			/* bit 7 = strobe, bit 6..0 = data */
			centronics_strobe_w(m_printer, (~data>>7) & 0x01);
			centronics_data_w(m_printer, 0, data & 0x7f);
			break;

		case 4: /* 8-bit parallel output */
			/* hardware strobe driven from port select, bit 7..0 = data */
			centronics_strobe_w(m_printer, 1);
			centronics_data_w(m_printer, 0, data);
			centronics_strobe_w(m_printer, 0);
			break;
	}
}

READ8_MEMBER(sorcerer_state::sorcerer_fc_r)
{
	UINT8 data = ay31015_get_received_data( m_uart );
	ay31015_set_input_pin( m_uart, AY31015_RDAV, 0 );
	ay31015_set_input_pin( m_uart, AY31015_RDAV, 1 );
	return data;
}

READ8_MEMBER(sorcerer_state::sorcerer_fd_r)
{
	/* set unused bits high */
	UINT8 data = 0xe0;

	ay31015_set_input_pin( m_uart, AY31015_SWE, 0 );
	data |= ay31015_get_output_pin( m_uart, AY31015_TBMT ) ? 0x01 : 0;
	data |= ay31015_get_output_pin( m_uart, AY31015_DAV  ) ? 0x02 : 0;
	data |= ay31015_get_output_pin( m_uart, AY31015_OR   ) ? 0x04 : 0;
	data |= ay31015_get_output_pin( m_uart, AY31015_FE   ) ? 0x08 : 0;
	data |= ay31015_get_output_pin( m_uart, AY31015_PE   ) ? 0x10 : 0;
	ay31015_set_input_pin( m_uart, AY31015_SWE, 1 );

	return data;
}

READ8_MEMBER(sorcerer_state::sorcerer_fe_r)
{
	/* bits 6..7
     - hardware handshakes from user port
     - not emulated
     - tied high, allowing PARIN and PAROUT bios routines to run */

	UINT8 data = 0xc0;
	static const char *const keynames[] = {
		"LINE0", "LINE1", "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7",
		"LINE8", "LINE9", "LINE10", "LINE11", "LINE12", "LINE13", "LINE14", "LINE15"
	};

	/* bit 5 - vsync (inverted) */
	data |= (((~input_port_read(machine(), "VS")) & 0x01)<<5);

	/* bits 4..0 - keyboard data */
	data |= (input_port_read(machine(), keynames[m_keyboard_line]) & 0x1f);

	return data;
}

READ8_MEMBER(sorcerer_state::sorcerer_ff_r)
{
	/* The use of the parallel port as a general purpose port is not emulated.
    Currently the only use is to read the printer status in the Centronics CENDRV bios routine.
    This uses bit 7. The other bits have been set high (=nothing plugged in).
    This fixes those games that use a joystick. */

	UINT8 data=0x7f;

	/* bit 7 = printer busy   0 = printer is not busy */

	data |= centronics_busy_r(m_printer) << 7;

	return data;
}



/**********************************************************************************************************/

Z80BIN_EXECUTE( sorcerer )
{
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	if ((execute_address >= 0xc000) && (execute_address <= 0xdfff) && (space->read_byte(0xdffa) != 0xc3))
		return;					/* can't run a program if the cartridge isn't in */

	/* Since Exidy Basic is by Microsoft, it needs some preprocessing before it can be run.
    1. A start address of 01D5 indicates a basic program which needs its pointers fixed up.
    2. If autorunning, jump to C689 (command processor), else jump to C3DD (READY prompt).
    Important addresses:
        01D5 = start (load) address of a conventional basic program
        C858 = an autorun basic program will have this exec address on the tape
        C3DD = part of basic that displays READY and lets user enter input */

	if ((start_address == 0x1d5) || (execute_address == 0xc858))
	{
		UINT8 i;
		static const UINT8 data[]={
			0xcd, 0x26, 0xc4,	// CALL C426    ;set up other pointers
			0x21, 0xd4, 1,		// LD HL,01D4   ;start of program address (used by C689)
			0x36, 0,		// LD (HL),00   ;make sure dummy end-of-line is there
			0xc3, 0x89, 0xc6	// JP C689  ;run program
		};

		for (i = 0; i < ARRAY_LENGTH(data); i++)
			space->write_byte(0xf01f + i, data[i]);

		if (!autorun)
			space->write_word(0xf028,0xc3dd);

		/* tell BASIC where program ends */
		space->write_byte(0x1b7, end_address & 0xff);
		space->write_byte(0x1b8, (end_address >> 8) & 0xff);

		if ((execute_address != 0xc858) && autorun)
			space->write_word(0xf028, execute_address);

		cpu_set_reg(machine.device("maincpu"), STATE_GENPC, 0xf01f);
	}
	else
	{
		if (autorun)
			cpu_set_reg(machine.device("maincpu"), STATE_GENPC, execute_address);
	}
}

/******************************************************************************
 Snapshot Handling
******************************************************************************/

SNAPSHOT_LOAD(sorcerer)
{
	device_t *cpu = image.device().machine().device("maincpu");
	UINT8 *RAM = image.device().machine().region(cpu->tag())->base();
	address_space *space = cpu->memory().space(AS_PROGRAM);
	UINT8 header[28];
	unsigned char s_byte;

	/* check size */
	if (snapshot_size != 0x1001c)
	{
		image.seterror(IMAGE_ERROR_INVALIDIMAGE, "Snapshot must be 65564 bytes");
		image.message("Snapshot must be 65564 bytes");
		return IMAGE_INIT_FAIL;
	}

	/* get the header */
	image.fread( &header, sizeof(header));

	/* write it to ram */
	for (int i = 0; i < 0xc000; i++)
	{
		image.fread( &s_byte, 1);
		space->write_byte(i, s_byte);
	}
	image.fread( RAM+0xc000, 0x4000);

	/* patch CPU registers */
	cpu_set_reg(cpu, Z80_I, header[0]);
	cpu_set_reg(cpu, Z80_HL2, header[1] | (header[2] << 8));
	cpu_set_reg(cpu, Z80_DE2, header[3] | (header[4] << 8));
	cpu_set_reg(cpu, Z80_BC2, header[5] | (header[6] << 8));
	cpu_set_reg(cpu, Z80_AF2, header[7] | (header[8] << 8));
	cpu_set_reg(cpu, Z80_HL, header[9] | (header[10] << 8));
	cpu_set_reg(cpu, Z80_DE, header[11] | (header[12] << 8));
	cpu_set_reg(cpu, Z80_BC, header[13] | (header[14] << 8));
	cpu_set_reg(cpu, Z80_IY, header[15] | (header[16] << 8));
	cpu_set_reg(cpu, Z80_IX, header[17] | (header[18] << 8));
	cpu_set_reg(cpu, Z80_IFF1, header[19]&2 ? 1 : 0);
	cpu_set_reg(cpu, Z80_IFF2, header[19]&4 ? 1 : 0);
	cpu_set_reg(cpu, Z80_R, header[20]);
	cpu_set_reg(cpu, Z80_AF, header[21] | (header[22] << 8));
	cpu_set_reg(cpu, STATE_GENSP, header[23] | (header[24] << 8));
	cpu_set_reg(cpu, Z80_IM, header[25]);
	cpu_set_reg(cpu, STATE_GENPC, header[26] | (header[27] << 8));

	return IMAGE_INIT_PASS;
}

MACHINE_START( sorcerer )
{
	sorcerer_state *state = machine.driver_data<sorcerer_state>();
	state->m_cassette_timer = machine.scheduler().timer_alloc(FUNC(sorcerer_cassette_tc));
#if SORCERER_USING_RS232
	state->m_serial_timer = machine.scheduler().timer_alloc(FUNC(sorcerer_serial_tc));
#endif

	UINT16 endmem = 0xbfff;

	address_space *space = state->m_maincpu->memory().space(AS_PROGRAM);
	/* configure RAM */
	switch (ram_get_size(state->m_ram))
	{
	case 8*1024:
		space->unmap_readwrite(0x2000, endmem);
		break;

	case 16*1024:
		space->unmap_readwrite(0x4000, endmem);
		break;

	case 32*1024:
		space->unmap_readwrite(0x8000, endmem);
		break;
	}
}

MACHINE_START( sorcererd )
{
	sorcerer_state *state = machine.driver_data<sorcerer_state>();
	state->m_cassette_timer = machine.scheduler().timer_alloc(FUNC(sorcerer_cassette_tc));
#if SORCERER_USING_RS232
	state->m_serial_timer = machine.scheduler().timer_alloc(FUNC(sorcerer_serial_tc));
#endif

	UINT16 endmem = 0xbbff;

	address_space *space = state->m_maincpu->memory().space(AS_PROGRAM);
	/* configure RAM */
	switch (ram_get_size(state->m_ram))
	{
	case 8*1024:
		space->unmap_readwrite(0x2000, endmem);
		break;

	case 16*1024:
		space->unmap_readwrite(0x4000, endmem);
		break;

	case 32*1024:
		space->unmap_readwrite(0x8000, endmem);
		break;
	}
}

MACHINE_RESET( sorcerer )
{
	sorcerer_state *state = machine.driver_data<sorcerer_state>();
	address_space *space = machine.device("maincpu")->memory().space(AS_PROGRAM);

	/* Initialize cassette interface */
	state->m_cass_data.output.length = 0;
	state->m_cass_data.output.level = 1;
	state->m_cass_data.input.length = 0;
	state->m_cass_data.input.bit = 1;

	state->m_fe = 0xff;
	state->sorcerer_fe_w(*space, 0, 0, 0xff);

	memory_set_bank(machine, "boot", 1);
	machine.scheduler().timer_set(attotime::from_usec(10), FUNC(sorcerer_reset));
}
