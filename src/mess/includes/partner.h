/*****************************************************************************
 *
 * includes/partner.h
 *
 ****************************************************************************/

#ifndef partner_H_
#define partner_H_

#include "machine/i8255.h"
#include "machine/8257dma.h"
#include "machine/wd17xx.h"

class partner_state : public radio86_state
{
public:
	partner_state(const machine_config &mconfig, device_type type, const char *tag)
		: radio86_state(mconfig, type, tag) { }

	UINT8 m_mem_page;
	UINT8 m_win_mem_page;
};


/*----------- defined in machine/partner.c -----------*/

extern DRIVER_INIT( partner );
extern MACHINE_RESET( partner );
extern MACHINE_START( partner );

extern const i8257_interface partner_dma;
extern const wd17xx_interface partner_wd17xx_interface;

extern WRITE8_HANDLER (partner_mem_page_w );
extern WRITE8_HANDLER (partner_win_memory_page_w);
#endif /* partner_H_ */
