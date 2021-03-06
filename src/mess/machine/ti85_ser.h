
#ifndef __TI85SERIAL_H_
#define __TI85SERIAL_H_


DECLARE_LEGACY_IMAGE_DEVICE(TI85SERIAL, ti85serial);
DECLARE_LEGACY_IMAGE_DEVICE(TI73SERIAL, ti73serial);
DECLARE_LEGACY_IMAGE_DEVICE(TI82SERIAL, ti82serial);
DECLARE_LEGACY_IMAGE_DEVICE(TI83PSERIAL, ti83pserial);
DECLARE_LEGACY_IMAGE_DEVICE(TI86SERIAL, ti86serial);

extern void ti85_update_serial(device_t *device);

#define MCFG_TI85SERIAL_ADD( _tag ) \
		MCFG_DEVICE_ADD( _tag, TI85SERIAL, 0 )

#define MCFG_TI73SERIAL_ADD( _tag ) \
		MCFG_DEVICE_ADD( _tag, TI73SERIAL, 0 )

#define MCFG_TI82SERIAL_ADD( _tag ) \
		MCFG_DEVICE_ADD( _tag, TI82SERIAL, 0 )

#define MCFG_TI83PSERIAL_ADD( _tag ) \
		MCFG_DEVICE_ADD( _tag, TI83PSERIAL, 0 )

#define MCFG_TI86SERIAL_ADD( _tag ) \
		MCFG_DEVICE_ADD( _tag, TI86SERIAL, 0 )

WRITE8_DEVICE_HANDLER( ti85serial_red_out );
WRITE8_DEVICE_HANDLER( ti85serial_white_out );
READ8_DEVICE_HANDLER( ti85serial_red_in );
READ8_DEVICE_HANDLER( ti85serial_white_in );

#endif
