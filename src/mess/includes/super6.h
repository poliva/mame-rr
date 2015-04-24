#pragma once

#ifndef __SUPER6__
#define __SUPER6__

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "cpu/z80/z80.h"
#include "cpu/z80/z80daisy.h"
#include "formats/basicdsk.h"
#include "imagedev/flopdrv.h"
#include "machine/com8116.h"
#include "machine/z80ctc.h"
#include "machine/z80dart.h"
#include "machine/z80dma.h"
#include "machine/z80pio.h"
#include "machine/ram.h"
#include "machine/terminal.h"
#include "machine/wd17xx.h"

#define Z80_TAG			"u30"
#define Z80CTC_TAG		"u20"
#define Z80DART_TAG		"u38"
#define Z80DMA_TAG		"u21"
#define Z80PIO_TAG		"u7"
#define WD2793_TAG		"u27"
#define BR1945_TAG		"u31"
#define SCREEN_TAG		"screen"
#define TERMINAL_TAG	"terminal"

class super6_state : public driver_device
{
public:
	super6_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag),
		  m_maincpu(*this, Z80_TAG),
		  m_ctc(*this, Z80CTC_TAG),
		  m_dart(*this, Z80DART_TAG),
		  m_dma(*this, Z80DMA_TAG),
		  m_pio(*this, Z80PIO_TAG),
		  m_fdc(*this, WD2793_TAG),
		  m_brg(*this, BR1945_TAG),
		  m_ram(*this, RAM_TAG),
		  m_floppy0(*this, FLOPPY_0),
		  m_floppy1(*this, FLOPPY_1)
	{ }

	required_device<cpu_device> m_maincpu;
	required_device<z80ctc_device> m_ctc;
	required_device<z80dart_device> m_dart;
	required_device<z80dma_device> m_dma;
	required_device<z80pio_device> m_pio;
	required_device<device_t> m_fdc;
	required_device<com8116_device> m_brg;
	required_device<device_t> m_ram;
	required_device<device_t> m_floppy0;
	required_device<device_t> m_floppy1;

	virtual void machine_start();
	virtual void machine_reset();

	DECLARE_READ8_MEMBER( fdc_r );
	DECLARE_WRITE8_MEMBER( fdc_w );
	DECLARE_WRITE8_MEMBER( s100_w );
	DECLARE_WRITE8_MEMBER( bank0_w );
	DECLARE_WRITE8_MEMBER( bank1_w );
	DECLARE_WRITE8_MEMBER( baud_w );
	DECLARE_WRITE_LINE_MEMBER( fr_w );
	DECLARE_WRITE_LINE_MEMBER( intrq_w );
	DECLARE_WRITE_LINE_MEMBER( drq_w );

	void bankswitch();

	// memory state
	UINT8 m_s100;
	UINT8 m_bank0;
	UINT8 m_bank1;
};

#endif
