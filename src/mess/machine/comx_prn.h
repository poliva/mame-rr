/**********************************************************************

    COMX-35 Serial/Parallel Printer Card emulation

    Copyright MESS Team.
    Visit http://mamedev.org for licensing and usage restrictions.

**********************************************************************/

#pragma once

#ifndef __COMX_PRN__
#define __COMX_PRN__

#define ADDRESS_MAP_MODERN

#include "emu.h"
#include "machine/comxexp.h"



//**************************************************************************
//  TYPE DEFINITIONS
//**************************************************************************

// ======================> comx_prn_device

class comx_prn_device : public device_t,
					    public device_comx_expansion_card_interface
{
public:
	// construction/destruction
	comx_prn_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);

	// optional information overrides
	virtual const rom_entry *device_rom_region() const;

protected:
	// device-level overrides
	virtual void device_start();
	virtual void device_reset();
    virtual void device_config_complete() { m_shortname = "comx_prn"; }

	// device_comx_expansion_card_interface overrides
	virtual UINT8 comx_mrd_r(offs_t offset, int *extrom);
	virtual UINT8 comx_io_r(offs_t offset);
	virtual void comx_io_w(offs_t offset, UINT8 data);

private:
	// internal state
	comx_expansion_slot_device *m_slot;

	UINT8 *m_rom;				// program ROM
};


// device type definition
extern const device_type COMX_PRN;


#endif
