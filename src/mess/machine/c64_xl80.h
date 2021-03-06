/**********************************************************************

    XL 80 cartridge emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __XL80__
#define __XL80__

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "machine/c64exp.h"
#include "video/mc6845.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> c64_xl80_device

class c64_xl80_device : public device_t,
					    public device_c64_expansion_card_interface
{
public:
	// construction/destruction
	c64_xl80_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;
	virtual machine_config_constructor device_mconfig_additions() const;

	// not really public
	void crtc_update_row(mc6845_device *device, bitmap_t *bitmap, const rectangle *cliprect, UINT16 ma, UINT8 ra, UINT16 y, UINT8 x_count, INT8 cursor_x, void *param);

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
    virtual void device_config_complete() { m_shortname = "c64_xl80"; }

	// device_c64_expansion_card_interface overrides
	virtual UINT8 c64_cd_r(offs_t offset, int roml, int romh, int io1, int io2);
	virtual void c64_cd_w(offs_t offset, UINT8 data, int roml, int romh, int io1, int io2);
	virtual int c64_game_r();
	virtual int c64_exrom_r();
	virtual bool c64_screen_update(screen_device &screen, bitmap_t &bitmap, const rectangle &cliprect);

private:
	c64_expansion_slot_device *m_slot;

	// internal state
	required_device<h46505_device> m_crtc;

	UINT8 *m_rom;
	UINT8 *m_char_rom;
	UINT8 *m_video_ram;
};


// device type definition
extern const device_type C64_XL80;


#endif
