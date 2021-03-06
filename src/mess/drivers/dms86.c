/***************************************************************************

        Digital Microsystems DMS-86

        11/01/2010 Skeleton driver.

Monitor commands:
A
B boot to HiNet
D dump memory to screen
I in port
L memory test
O out port (e.g. O 84 77)
P
S write a byte to memory
T
X display registers


Note that bit 3 of port 82 is tested at boot. If low, the computer bypasses
the monitor and goes straight to "Joining HiNet".

****************************************************************************/
#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/i86/i86.h"
#include "machine/terminal.h"


class dms86_state : public driver_device
{
public:
	dms86_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
	m_maincpu(*this, "maincpu"),
	m_terminal(*this, TERMINAL_TAG)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<device_t> m_terminal;
	DECLARE_READ16_MEMBER( dms86_82_r );
	DECLARE_READ16_MEMBER( dms86_84_r );
	DECLARE_READ16_MEMBER( dms86_86_r );
	DECLARE_READ16_MEMBER( dms86_9a_r );
	DECLARE_WRITE8_MEMBER( kbd_put );
	UINT8 *m_ram;
	UINT8 m_term_data;
};

READ16_MEMBER( dms86_state::dms86_82_r )
{
// HiNet / Monitor switch

	return 0xff;
}

READ16_MEMBER( dms86_state::dms86_84_r )
{
	UINT8 ret = m_term_data;
	m_term_data = 0;
	return ret;
}

READ16_MEMBER( dms86_state::dms86_86_r )
{
	return 4 | (m_term_data ? 1 : 0);
}

READ16_MEMBER( dms86_state::dms86_9a_r )
{
	return 0;
}


static ADDRESS_MAP_START(dms86_mem, AS_PROGRAM, 16, dms86_state)
	ADDRESS_MAP_UNMAP_HIGH
	AM_RANGE(0x00000, 0x1ffff) AM_RAM
	AM_RANGE(0xfe000, 0xfffff) AM_ROM AM_REGION("user1",0)
ADDRESS_MAP_END

static ADDRESS_MAP_START(dms86_io, AS_IO, 16, dms86_state)
	ADDRESS_MAP_UNMAP_HIGH
	ADDRESS_MAP_GLOBAL_MASK(0xff)
	AM_RANGE(0x82, 0x83) AM_READ(dms86_82_r)
	AM_RANGE(0x84, 0x85) AM_READ(dms86_84_r) AM_DEVWRITE8_LEGACY(TERMINAL_TAG, terminal_write, 0xff)
	AM_RANGE(0x86, 0x87) AM_READ(dms86_86_r)
	AM_RANGE(0x9A, 0x9B) AM_READ(dms86_9a_r)
	AM_RANGE(0x9c, 0x9d) AM_DEVWRITE8_LEGACY(TERMINAL_TAG, terminal_write, 0xff)
ADDRESS_MAP_END

/* Input ports */
static INPUT_PORTS_START( dms86 )
INPUT_PORTS_END


static MACHINE_RESET(dms86)
{
}

WRITE8_MEMBER( dms86_state::kbd_put )
{
	m_term_data = data;
}

static GENERIC_TERMINAL_INTERFACE( terminal_intf )
{
	DEVCB_DRIVER_MEMBER(dms86_state, kbd_put)
};


static MACHINE_CONFIG_START( dms86, dms86_state )
	/* basic machine hardware */
	MCFG_CPU_ADD("maincpu",I8086, XTAL_9_8304MHz)
	MCFG_CPU_PROGRAM_MAP(dms86_mem)
	MCFG_CPU_IO_MAP(dms86_io)

	MCFG_MACHINE_RESET(dms86)

	MCFG_FRAGMENT_ADD( generic_terminal )
	MCFG_GENERIC_TERMINAL_ADD(TERMINAL_TAG, terminal_intf)
MACHINE_CONFIG_END

/* ROM definition */
ROM_START( dms86 )
	ROM_REGION( 0x2000, "user1", ROMREGION_ERASEFF )
	ROM_LOAD16_BYTE( "hns-86_54-8678.bin", 0x0000, 0x1000, CRC(95f58e1c) SHA1(6fc8f087f0c887d8b429612cd035c6c1faab570c))
	ROM_LOAD16_BYTE( "hns-86_54-8677.bin", 0x0001, 0x1000, CRC(78fad756) SHA1(ddcbff1569ec6975b8489935cdcfa80eba413502))
ROM_END

/* Driver */

/*    YEAR  NAME    PARENT  COMPAT   MACHINE    INPUT    INIT     COMPANY              FULLNAME       FLAGS */
COMP( 1982, dms86,  0,       0,      dms86,     dms86,    0,   "Digital Microsystems", "DMS-86", GAME_NOT_WORKING | GAME_NO_SOUND)

