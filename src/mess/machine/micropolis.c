/***************************************************************************

    micropolis.c

    by Robbbert, August 2011.

This is a rough implementation of the Micropolis floppy-disk controller
as used for the Exidy Sorcerer. Since there is no documentation, coding
was done by looking at the Z80 code, and supplying the expected values.

Currently, only reading of disks is supported.

ToDo:
- Rewrite to be a standard device able to be used in a general way
- Support the ability to write to disk.


Ports:
BE00 and BE01 can be used as command registers (they are identical),
              and they are also used as status registers (different).

BE02 - read data
BE03 - unknown

Write data is unknown but could either be BE02 or BE03.

***************************************************************************/


#include "emu.h"
#include "imagedev/flopdrv.h"
#include "machine/micropolis.h"


/***************************************************************************
    CONSTANTS
***************************************************************************/


#define STAT_MOTOR_ON   0x20
#define STAT_TRACK0     0x08
#define STAT_READY      0x80

#define VERBOSE			0	/* General logging */
#define VERBOSE_DATA	0	/* Logging of each byte during read and write */

/* structure describing a single density track */
#define TRKSIZE_SD		16*270
#if 0
static const UINT8 track_SD[][2] = {
	{ 1, 0xff}, 	/*  1 * FF (marker)                      */
	{ 1, 0x00}, 	/*  1 byte, track number (00-4C)         */
	{ 1, 0x01}, 	/*  1 byte, sector number (00-0F)        */
	{10, 0x00},     /*  10 bytes of zeroes                   */
	{256, 0xe5},	/*  256 bytes of sector data             */
	{ 1, 0xb7}, 	/*  1 byte, CRC                          */
};
#endif


/***************************************************************************
    TYPE DEFINITIONS
***************************************************************************/

typedef struct _micropolis_state micropolis_state;
struct _micropolis_state
{
	/* register */
	UINT8 data;
	UINT8 drive_num;
	UINT8 track;
	UINT8 sector;
	UINT8 command;
	UINT8 status;

	UINT8	write_cmd;				/* last write command issued */

	UINT8	buffer[6144];			/* I/O buffer (holds up to a whole track) */
	UINT32	data_offset;			/* offset into I/O buffer */
	INT32	data_count; 			/* transfer count from/into I/O buffer */

	UINT32	sector_length;			/* sector length (byte) */

	/* this is the drive currently selected */
	device_t *drive;

	/* Pointer to interface */
	const micropolis_interface *intf;
};


/***************************************************************************
    DEFAULT INTERFACES
***************************************************************************/

const micropolis_interface default_micropolis_interface =
{
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, { FLOPPY_0, FLOPPY_1, FLOPPY_2, FLOPPY_3}
};

const micropolis_interface default_micropolis_interface_2_drives =
{
	DEVCB_NULL, DEVCB_NULL, DEVCB_NULL, { FLOPPY_0, FLOPPY_1, NULL, NULL}
};


/*****************************************************************************
    INLINE FUNCTIONS
*****************************************************************************/

INLINE micropolis_state *get_safe_token(device_t *device)
{
	assert(device != NULL);
	assert(device->type() == MICROPOLIS);

	return (micropolis_state *)downcast<legacy_device_base *>(device)->token();
}


/***************************************************************************
    IMPLEMENTATION
***************************************************************************/


/* read a sector */
static void micropolis_read_sector(device_t *device)
{
	micropolis_state *w = get_safe_token(device);
	w->data_offset = 0;
	w->data_count = w->sector_length;

	/* read data */
	floppy_drive_read_sector_data(w->drive, 0, w->sector, (char *)w->buffer, w->sector_length);
}


static void micropolis_write_sector(device_t *device)
{
#if 0
	micropolis_state *w = get_safe_token(device);
	/* at this point, the disc is write enabled, and data
     * has been transfered into our buffer - now write it to
     * the disc image or to the real disc
     */

	/* find sector */
	w->data_count = w->sector_length;

	/* write data */
	floppy_drive_write_sector_data(w->drive, 0, w->sector, (char *)w->buffer, w->sector_length, w->write_cmd & 0x01);
#endif
}




/***************************************************************************
    INTERFACE
***************************************************************************/

/* use this to determine which drive is controlled by WD */
void micropolis_set_drive(device_t *device, UINT8 drive)
{
	micropolis_state *w = get_safe_token(device);

	if (VERBOSE)
		logerror("micropolis_set_drive: $%02x\n", drive);

	if (w->intf->floppy_drive_tags[drive] != NULL)
	{
		if (device->owner() != NULL) {
			w->drive = device->owner()->subdevice(w->intf->floppy_drive_tags[drive]);
			if (w->drive == NULL) {
				w->drive = device->machine().device(w->intf->floppy_drive_tags[drive]);
			}
		}
		else
			w->drive = device->machine().device(w->intf->floppy_drive_tags[drive]);
	}
}


/***************************************************************************
    DEVICE HANDLERS
***************************************************************************/


/* read the FDC status register. */
READ8_DEVICE_HANDLER( micropolis_status_r )
{
	micropolis_state *w = get_safe_token(device);
	static bool inv = 0;

	if (offset)
		return w->status | w->drive_num;
	else
	{
		w->sector = (w->sector + 3 + inv) & 15;
		micropolis_read_sector(device);
		inv ^= 1;
		return (w->status & STAT_READY) | w->sector;
	}
}


/* read the FDC data register */
READ8_DEVICE_HANDLER( micropolis_data_r )
{
	micropolis_state *w = get_safe_token(device);

	if (w->data_offset >= w->sector_length)
		w->data_offset = 0;

	return w->buffer[w->data_offset++];
}

/* write the FDC command register */
WRITE8_DEVICE_HANDLER( micropolis_command_w )
{
	micropolis_state *w = get_safe_token(device);
	int direction = 0;

	if ((data & 0x60)==0x20)
	{
		w->drive_num = data & 3;
		floppy_mon_w(w->drive, ASSERT_LINE); // turn off the old drive
		micropolis_set_drive(device, w->drive_num); // select new drive
		floppy_mon_w(w->drive, CLEAR_LINE); // turn it on
	}

	w->status = 0;

	if (BIT(data, 5))
		w->status = STAT_MOTOR_ON | STAT_READY;

	floppy_drive_set_ready_state(w->drive, 1,0);

	if ((data & 0x41) == 0x41)
	{
		if (w->track < 77)
		{
			w->track++;
			direction = 1;
		}
	}
	else
	if ((data & 0x41) == 0x40)
	{
		if (w->track)
		{
			w->track--;
			direction = -1;
		}
	}

	if (!w->track)
		w->status |= STAT_TRACK0;

	floppy_drive_seek(w->drive, direction);
}


/* write the FDC data register */
WRITE8_DEVICE_HANDLER( micropolis_data_w )
{
	micropolis_state *w = get_safe_token(device);

	if (w->data_count > 0)
	{
		/* put byte into buffer */
		if (VERBOSE_DATA)
			logerror("micropolis_info buffered data: $%02X at offset %d.\n", data, w->data_offset);

		w->buffer[w->data_offset++] = data;

		if (--w->data_count < 1)
		{
			micropolis_write_sector(device);

			w->data_offset = 0;
		}
	}
	else
	{
		if (VERBOSE)
			logerror("%s: micropolis_data_w $%02X\n", device->machine().describe_context(), data);
	}
	w->data = data;
}

READ8_DEVICE_HANDLER( micropolis_r )
{
	UINT8 data = 0;

	switch (offset & 0x03)
	{
	case 0: data = micropolis_status_r(device, 0); break;
	case 1:	data = micropolis_status_r(device, 1); break;
	case 2:	data = micropolis_data_r(device, 0); break;
	case 3:	data = 0; break;
	}

	return data;
}

WRITE8_DEVICE_HANDLER( micropolis_w )
{
	switch (offset & 0x03)
	{
	case 0: micropolis_command_w(device, 0, data); break;
	case 1:	micropolis_command_w(device, 1, data); break;
	case 2:     break;
	case 3: micropolis_data_w(device, 0, data);    break;
	}
}


/***************************************************************************
    MAME DEVICE INTERFACE
***************************************************************************/

static DEVICE_START( micropolis )
{
	micropolis_state *w = get_safe_token(device);

	assert(device->static_config() != NULL);

	w->intf = (const micropolis_interface*)device->static_config();
}

static DEVICE_RESET( micropolis )
{
	micropolis_state *w = get_safe_token(device);
	int i;

	/* set the default state of some input lines */
	for (i = 0; i < 4; i++)
	{
		if(w->intf->floppy_drive_tags[i]!=NULL) {
			device_t *img = NULL;

			if (device->owner() != NULL)
				img = device->owner()->subdevice(w->intf->floppy_drive_tags[i]);
				if (img == NULL) {
					img = device->machine().device(w->intf->floppy_drive_tags[i]);
				}

			else
				img = device->machine().device(w->intf->floppy_drive_tags[i]);

			if (img!=NULL) {
				floppy_drive_set_controller(img,device);
				floppy_drive_set_rpm( img, 300.);
			}
		}
	}

	micropolis_set_drive(device, 0);

	w->drive_num = 0;
	w->sector = 0;
	w->track = 0;
	w->sector_length = 270;
	w->status = 0;
}

void micropolis_reset(device_t *device)
{
	DEVICE_RESET_CALL( micropolis );
}


/***************************************************************************
    DEVICE GETINFO
***************************************************************************/

static const char DEVTEMPLATE_SOURCE[] = __FILE__;

#define DEVTEMPLATE_ID(p,s)				p##micropolis##s
#define DEVTEMPLATE_FEATURES			DT_HAS_START | DT_HAS_RESET
#define DEVTEMPLATE_NAME				"MICROPOLIS"
#define DEVTEMPLATE_FAMILY				"MICROPOLIS"
#define DEVTEMPLATE_VERSION				"0.1"
#define DEVTEMPLATE_CREDITS				"Copyright MESS Team"
#include "devtempl.h"

DEFINE_LEGACY_DEVICE(MICROPOLIS, micropolis);
