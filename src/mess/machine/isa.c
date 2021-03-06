/***************************************************************************

        ISA bus device

***************************************************************************/

#include "emu.h"
#include "emuopts.h"
#include "machine/isa.h"


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8_SLOT = &device_creator<isa8_slot_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_slot_device - constructor
//-------------------------------------------------
isa8_slot_device::isa8_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, ISA8_SLOT, "ISA8_SLOT", tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}

isa8_slot_device::isa8_slot_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, type, name, tag, owner, clock),
		device_slot_interface(mconfig, *this)
{
}

void isa8_slot_device::static_set_isa8_slot(device_t &device, const char *tag)
{
	isa8_slot_device &isa_card = dynamic_cast<isa8_slot_device &>(device);
	isa_card.m_isa_tag = tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_slot_device::device_start()
{
	device_isa8_card_interface *dev = dynamic_cast<device_isa8_card_interface *>(get_card_device());
	const device_isa16_card_interface *intf;
	if (get_card_device()->interface(intf))
		fatalerror("Error ISA16 device in ISA8 slot\n");

	if (dev) device_isa8_card_interface::static_set_isabus_tag(*dev,m_isa_tag);
}



//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA16_SLOT = &device_creator<isa16_slot_device>;

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa16_slot_device - constructor
//-------------------------------------------------
isa16_slot_device::isa16_slot_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        isa8_slot_device(mconfig, ISA16_SLOT, "ISA16_SLOT", tag, owner, clock)
{
}

void isa16_slot_device::static_set_isa16_slot(device_t &device, const char *tag)
{
	isa16_slot_device &isa_card = dynamic_cast<isa16_slot_device &>(device);
	isa_card.m_isa_tag = tag;
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa16_slot_device::device_start()
{
	device_isa8_card_interface *dev = dynamic_cast<device_isa8_card_interface *>(get_card_device());
	if (dev) device_isa8_card_interface::static_set_isabus_tag(*dev,m_isa_tag);
}


//**************************************************************************
//  GLOBAL VARIABLES
//**************************************************************************

const device_type ISA8 = &device_creator<isa8_device>;

void isa8_device::static_set_cputag(device_t &device, const char *tag)
{
	isa8_device &isa = downcast<isa8_device &>(device);
	isa.m_cputag = tag;
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void isa8_device::device_config_complete()
{
	// inherit a copy of the static data
	const isa8bus_interface *intf = reinterpret_cast<const isa8bus_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<isa8bus_interface *>(this) = *intf;
	}

	// or initialize to defaults if none provided
	else
	{
    	memset(&m_out_irq2_cb, 0, sizeof(m_out_irq2_cb));
    	memset(&m_out_irq3_cb, 0, sizeof(m_out_irq3_cb));
    	memset(&m_out_irq4_cb, 0, sizeof(m_out_irq4_cb));
    	memset(&m_out_irq5_cb, 0, sizeof(m_out_irq5_cb));
    	memset(&m_out_irq6_cb, 0, sizeof(m_out_irq6_cb));
    	memset(&m_out_irq7_cb, 0, sizeof(m_out_irq7_cb));
    	memset(&m_out_drq1_cb, 0, sizeof(m_out_drq1_cb));
    	memset(&m_out_drq2_cb, 0, sizeof(m_out_drq2_cb));
    	memset(&m_out_drq3_cb, 0, sizeof(m_out_drq3_cb));
	}
	m_maincpu = mconfig().devicelist().find(m_cputag);
}

//**************************************************************************
//  LIVE DEVICE
//**************************************************************************

//-------------------------------------------------
//  isa8_device - constructor
//-------------------------------------------------

isa8_device::isa8_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, ISA8, "ISA8", tag, owner, clock)
{
}

isa8_device::isa8_device(const machine_config &mconfig, device_type type, const char *name, const char *tag, device_t *owner, UINT32 clock) :
        device_t(mconfig, type, name, tag, owner, clock)
{
}
//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa8_device::device_start()
{
	// resolve callbacks
	m_out_irq2_func.resolve(m_out_irq2_cb, *this);
	m_out_irq3_func.resolve(m_out_irq3_cb, *this);
	m_out_irq4_func.resolve(m_out_irq4_cb, *this);
	m_out_irq5_func.resolve(m_out_irq5_cb, *this);
	m_out_irq6_func.resolve(m_out_irq6_cb, *this);
	m_out_irq7_func.resolve(m_out_irq7_cb, *this);
	m_out_drq1_func.resolve(m_out_drq1_cb, *this);
	m_out_drq2_func.resolve(m_out_drq2_cb, *this);
	m_out_drq3_func.resolve(m_out_drq3_cb, *this);
}

//-------------------------------------------------
//  device_reset - device-specific reset
//-------------------------------------------------

void isa8_device::device_reset()
{
}

void isa8_device::add_isa_card(device_isa8_card_interface *card)
{
	m_device_list.append(*card);
}

void isa8_device::install_device(device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_device_func rhandler, const char* rhandler_name, write8_device_func whandler, const char *whandler_name)
{
	int buswidth = m_maincpu->memory().space_config(AS_PROGRAM)->m_databus_width;
	switch(buswidth)
	{
		case 8:
			m_maincpu->memory().space(AS_IO)->install_legacy_readwrite_handler(*dev, start, end, mask, mirror, rhandler, rhandler_name, whandler, whandler_name,0);
			break;
		case 16:
			m_maincpu->memory().space(AS_IO)->install_legacy_readwrite_handler(*dev, start, end, mask, mirror, rhandler, rhandler_name, whandler, whandler_name,0xffff);
			break;
		case 32:
			m_maincpu->memory().space(AS_IO)->install_legacy_readwrite_handler(*dev, start, end, mask, mirror, rhandler, rhandler_name, whandler, whandler_name,0xffffffff);
			break;
		default:
			fatalerror("ISA8: Bus width %d not supported", buswidth);
			break;
	}
}

void isa8_device::install_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read8_delegate rhandler, write8_delegate whandler)
{
	int buswidth = m_maincpu->memory().space_config(AS_PROGRAM)->m_databus_width;
	switch(buswidth)
	{
		case 8:
			m_maincpu->memory().space(AS_IO)->install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0);
			break;
		case 16:
			m_maincpu->memory().space(AS_IO)->install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0xffff);
			break;
		case 32:
			m_maincpu->memory().space(AS_IO)->install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0xffffffff);
			break;
		default:
			fatalerror("ISA8: Bus width %d not supported", buswidth);
			break;
	}
}

void isa8_device::install_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror, const char *tag, UINT8 *data)
{
	address_space *space = m_maincpu->memory().space(AS_PROGRAM);
	space->install_readwrite_bank(start, end, mask, mirror, tag );
	memory_set_bankptr(machine(), tag, data);
}

void isa8_device::unmap_bank(offs_t start, offs_t end, offs_t mask, offs_t mirror)
{
	address_space *space = m_maincpu->memory().space(AS_PROGRAM);
	space->unmap_readwrite(start, end, mask, mirror);
}

void isa8_device::install_rom(device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, const char *tag, const char *region)
{
	astring tempstring;
	address_space *space = m_maincpu->memory().space(AS_PROGRAM);
	space->install_read_bank(start, end, mask, mirror, tag);
	space->unmap_write(start, end, mask, mirror);
	memory_set_bankptr(machine(), tag, machine().region(dev->subtag(tempstring, region))->base());
}

void isa8_device::unmap_rom(offs_t start, offs_t end, offs_t mask, offs_t mirror)
{
	address_space *space = m_maincpu->memory().space(AS_PROGRAM);
	space->unmap_read(start, end, mask, mirror);
}

// interrupt request from isa card
WRITE_LINE_MEMBER( isa8_device::irq2_w ) { m_out_irq2_func(state); }
WRITE_LINE_MEMBER( isa8_device::irq3_w ) { m_out_irq3_func(state); }
WRITE_LINE_MEMBER( isa8_device::irq4_w ) { m_out_irq4_func(state); }
WRITE_LINE_MEMBER( isa8_device::irq5_w ) { m_out_irq5_func(state); }
WRITE_LINE_MEMBER( isa8_device::irq6_w ) { m_out_irq6_func(state); }
WRITE_LINE_MEMBER( isa8_device::irq7_w ) { m_out_irq7_func(state); }

// dma request from isa card
WRITE_LINE_MEMBER( isa8_device::drq1_w ) { m_out_drq1_func(state); }
WRITE_LINE_MEMBER( isa8_device::drq2_w ) { m_out_drq2_func(state); }
WRITE_LINE_MEMBER( isa8_device::drq3_w ) { m_out_drq3_func(state); }

UINT8 isa8_device::dack_r(int line)
{
	UINT8 retVal = 0xff;
	device_isa8_card_interface *entry = m_device_list.first();
	while(entry) {
		if (entry->have_dack(line)) {
			retVal = entry->dack_r(line);
			break;
		}
		entry = entry->next();
	}
	return retVal;
}

void isa8_device::dack_w(int line,UINT8 data)
{
	device_isa8_card_interface *entry = m_device_list.first();
	while(entry) {
		if (entry->have_dack(line)) {
			entry->dack_w(line,data);
		}
		entry = entry->next();
	}
}

void isa8_device::eop_w(int state)
{
	device_isa8_card_interface *entry = m_device_list.first();
	while(entry) {
		entry->eop_w(state);
		entry = entry->next();
	}
}


//**************************************************************************
//  DEVICE CONFIG ISA8 CARD INTERFACE
//**************************************************************************


//**************************************************************************
//  DEVICE ISA8 CARD INTERFACE
//**************************************************************************

//-------------------------------------------------
//  device_isa8_card_interface - constructor
//-------------------------------------------------

device_isa8_card_interface::device_isa8_card_interface(const machine_config &mconfig, device_t &device)
	: device_slot_card_interface(mconfig, device),
	  m_isa(NULL),
	  m_isa_tag(NULL)
{
}


//-------------------------------------------------
//  ~device_isa8_card_interface - destructor
//-------------------------------------------------

device_isa8_card_interface::~device_isa8_card_interface()
{
}

UINT8 device_isa8_card_interface::dack_r(int line)
{
	return 0;
}
void device_isa8_card_interface::dack_w(int line,UINT8 data)
{
}
void device_isa8_card_interface::eop_w(int state)
{
}

bool device_isa8_card_interface::have_dack(int line)
{
	return FALSE;
}

void device_isa8_card_interface::static_set_isabus_tag(device_t &device, const char *tag)
{
	device_isa8_card_interface &isa_card = dynamic_cast<device_isa8_card_interface &>(device);
	isa_card.m_isa_tag = tag;
}

void device_isa8_card_interface::set_isa_device()
{
	m_isa = dynamic_cast<isa8_device *>(device().machine().device(m_isa_tag));
	m_isa->add_isa_card(this);
}


const device_type ISA16 = &device_creator<isa16_device>;

//-------------------------------------------------
//  isa16_device - constructor
//-------------------------------------------------

isa16_device::isa16_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock) :
        isa8_device(mconfig, ISA16, "ISA16", tag, owner, clock)
{
}

//-------------------------------------------------
//  device_config_complete - perform any
//  operations now that the configuration is
//  complete
//-------------------------------------------------

void isa16_device::device_config_complete()
{
	// inherit a copy of the static data
	const isa16bus_interface *intf = reinterpret_cast<const isa16bus_interface *>(static_config());
	if (intf != NULL)
	{
		*static_cast<isa16bus_interface *>(this) = *intf;
		memcpy(&(isa8bus_interface::m_out_irq2_cb),&(isa16bus_interface::m_out_irq2_cb), sizeof(isa16bus_interface::m_out_irq2_cb));
    	memcpy(&(isa8bus_interface::m_out_irq3_cb),&(isa16bus_interface::m_out_irq3_cb), sizeof(isa16bus_interface::m_out_irq3_cb));
    	memcpy(&(isa8bus_interface::m_out_irq4_cb),&(isa16bus_interface::m_out_irq4_cb), sizeof(isa16bus_interface::m_out_irq4_cb));
    	memcpy(&(isa8bus_interface::m_out_irq5_cb),&(isa16bus_interface::m_out_irq5_cb), sizeof(isa16bus_interface::m_out_irq5_cb));
    	memcpy(&(isa8bus_interface::m_out_irq6_cb),&(isa16bus_interface::m_out_irq6_cb), sizeof(isa16bus_interface::m_out_irq6_cb));
    	memcpy(&(isa8bus_interface::m_out_irq7_cb),&(isa16bus_interface::m_out_irq7_cb), sizeof(isa16bus_interface::m_out_irq7_cb));
    	memcpy(&(isa8bus_interface::m_out_drq1_cb),&(isa16bus_interface::m_out_drq1_cb), sizeof(isa16bus_interface::m_out_drq1_cb));
    	memcpy(&(isa8bus_interface::m_out_drq2_cb),&(isa16bus_interface::m_out_drq2_cb), sizeof(isa16bus_interface::m_out_drq2_cb));
    	memcpy(&(isa8bus_interface::m_out_drq3_cb),&(isa16bus_interface::m_out_drq3_cb), sizeof(isa16bus_interface::m_out_drq3_cb));
	}

	// or initialize to defaults if none provided
	else
	{
    	memset(&(isa8bus_interface::m_out_irq2_cb), 0, sizeof(isa8bus_interface::m_out_irq2_cb));
    	memset(&(isa8bus_interface::m_out_irq3_cb), 0, sizeof(isa8bus_interface::m_out_irq3_cb));
    	memset(&(isa8bus_interface::m_out_irq4_cb), 0, sizeof(isa8bus_interface::m_out_irq4_cb));
    	memset(&(isa8bus_interface::m_out_irq5_cb), 0, sizeof(isa8bus_interface::m_out_irq5_cb));
    	memset(&(isa8bus_interface::m_out_irq6_cb), 0, sizeof(isa8bus_interface::m_out_irq6_cb));
    	memset(&(isa8bus_interface::m_out_irq7_cb), 0, sizeof(isa8bus_interface::m_out_irq7_cb));

		memset(&m_out_irq10_cb, 0, sizeof(m_out_irq10_cb));
    	memset(&m_out_irq11_cb, 0, sizeof(m_out_irq11_cb));
    	memset(&m_out_irq12_cb, 0, sizeof(m_out_irq12_cb));
    	memset(&m_out_irq14_cb, 0, sizeof(m_out_irq14_cb));
    	memset(&m_out_irq15_cb, 0, sizeof(m_out_irq15_cb));

		memset(&m_out_drq0_cb, 0, sizeof(m_out_drq0_cb));
    	memset(&(isa8bus_interface::m_out_drq1_cb), 0, sizeof(isa8bus_interface::m_out_drq1_cb));
    	memset(&(isa8bus_interface::m_out_drq2_cb), 0, sizeof(isa8bus_interface::m_out_drq2_cb));
    	memset(&(isa8bus_interface::m_out_drq3_cb), 0, sizeof(isa8bus_interface::m_out_drq3_cb));

		memset(&m_out_drq5_cb, 0, sizeof(m_out_drq5_cb));
    	memset(&m_out_drq6_cb, 0, sizeof(m_out_drq6_cb));
    	memset(&m_out_drq7_cb, 0, sizeof(m_out_drq7_cb));
	}
	m_maincpu = mconfig().devicelist().find(m_cputag);
}

//-------------------------------------------------
//  device_start - device-specific startup
//-------------------------------------------------

void isa16_device::device_start()
{
	isa8_device::device_start();

	// resolve callbacks
	m_out_irq10_func.resolve(m_out_irq10_cb, *this);
	m_out_irq11_func.resolve(m_out_irq11_cb, *this);
	m_out_irq12_func.resolve(m_out_irq12_cb, *this);
	m_out_irq14_func.resolve(m_out_irq14_cb, *this);
	m_out_irq15_func.resolve(m_out_irq15_cb, *this);

	m_out_drq0_func.resolve(m_out_drq0_cb, *this);
	m_out_drq5_func.resolve(m_out_drq5_cb, *this);
	m_out_drq6_func.resolve(m_out_drq6_cb, *this);
	m_out_drq7_func.resolve(m_out_drq7_cb, *this);
}

void isa16_device::install16_device(device_t *dev, offs_t start, offs_t end, offs_t mask, offs_t mirror, read16_device_func rhandler, const char* rhandler_name, write16_device_func whandler, const char *whandler_name)
{
	int buswidth = m_maincpu->memory().space_config(AS_PROGRAM)->m_databus_width;
	switch(buswidth)
	{
		case 16:
			m_maincpu->memory().space(AS_IO)->install_legacy_readwrite_handler(*dev, start, end, mask, mirror, rhandler, rhandler_name, whandler, whandler_name,0);
			break;
		case 32:
			m_maincpu->memory().space(AS_IO)->install_legacy_readwrite_handler(*dev, start, end, mask, mirror, rhandler, rhandler_name, whandler, whandler_name,0xffffffff);
			break;
		default:
			fatalerror("ISA16: Bus width %d not supported", buswidth);
			break;
	}
}

void isa16_device::install16_device(offs_t start, offs_t end, offs_t mask, offs_t mirror, read16_delegate rhandler, write16_delegate whandler)
{
	int buswidth = m_maincpu->memory().space_config(AS_PROGRAM)->m_databus_width;
	switch(buswidth)
	{
		case 16:
			m_maincpu->memory().space(AS_IO)->install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0);
			break;
		case 32:
			m_maincpu->memory().space(AS_IO)->install_readwrite_handler(start, end, mask, mirror, rhandler, whandler, 0xffffffff);
			break;
		default:
			fatalerror("ISA16: Bus width %d not supported", buswidth);
			break;
	}
}

// interrupt request from isa card
WRITE_LINE_MEMBER( isa16_device::irq10_w ) { m_out_irq10_func(state); }
WRITE_LINE_MEMBER( isa16_device::irq11_w ) { m_out_irq11_func(state); }
WRITE_LINE_MEMBER( isa16_device::irq12_w ) { m_out_irq12_func(state); }
WRITE_LINE_MEMBER( isa16_device::irq14_w ) { m_out_irq14_func(state); }
WRITE_LINE_MEMBER( isa16_device::irq15_w ) { m_out_irq15_func(state); }

// dma request from isa card
WRITE_LINE_MEMBER( isa16_device::drq0_w ) { m_out_drq0_func(state); }
WRITE_LINE_MEMBER( isa16_device::drq5_w ) { m_out_drq5_func(state); }
WRITE_LINE_MEMBER( isa16_device::drq6_w ) { m_out_drq6_func(state); }
WRITE_LINE_MEMBER( isa16_device::drq7_w ) { m_out_drq7_func(state); }

//-------------------------------------------------
//  device_isa16_card_interface - constructor
//-------------------------------------------------

device_isa16_card_interface::device_isa16_card_interface(const machine_config &mconfig, device_t &device)
	: device_isa8_card_interface(mconfig,device)
{
}


//-------------------------------------------------
//  ~device_isa16_card_interface - destructor
//-------------------------------------------------

device_isa16_card_interface::~device_isa16_card_interface()
{
}

void device_isa16_card_interface::set_isa_device()
{
	m_isa = dynamic_cast<isa16_device *>(device().machine().device(m_isa_tag));
	m_isa->add_isa_card(this);
}
