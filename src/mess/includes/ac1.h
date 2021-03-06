/*****************************************************************************
 *
 * includes/ac1.h
 *
 ****************************************************************************/

#ifndef AC1_H_
#define AC1_H_

#include "machine/z80pio.h"
#include "imagedev/cassette.h"

class ac1_state : public driver_device
{
public:
	ac1_state(const machine_config &mconfig, device_type type, const char *tag)
		: driver_device(mconfig, type, tag) { }

	cassette_image_device *m_cassette;
};


/*----------- defined in machine/ac1.c -----------*/

DRIVER_INIT( ac1 );
MACHINE_RESET( ac1 );

extern const z80pio_interface ac1_z80pio_intf;

/*----------- defined in video/ac1.c -----------*/

extern const gfx_layout ac1_charlayout;

VIDEO_START( ac1 );
SCREEN_UPDATE( ac1 );
SCREEN_UPDATE( ac1_32 );


#endif /* AC1_h_ */
