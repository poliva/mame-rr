#ifndef __NEXTKBD_H__
#define __NEXTKBD_H__

#include "emu.h"

#define MCFG_NEXTKBD_ADD(_tag, _int_change_cb)	\
	MCFG_DEVICE_ADD(_tag, NEXTKBD, 0)			\
	downcast<nextkbd_device *>(device)->set_int_change_cb(_int_change_cb);

class nextkbd_device : public device_t {
public:
	typedef delegate<void ()> int_cb_t;

	nextkbd_device(const machine_config &mconfig, const char *tag, device_t *owner, UINT32 clock);
	void set_int_change_cb(int_cb_t int_change_cb);

	DECLARE_ADDRESS_MAP(amap, 32);
	ioport_constructor device_input_ports() const;

	DECLARE_READ32_MEMBER(ctrl_r);
	DECLARE_READ32_MEMBER(ctrl2_r);
	DECLARE_READ32_MEMBER(data_r);

	DECLARE_WRITE32_MEMBER(ctrl_w);
	DECLARE_WRITE32_MEMBER(ctrl2_w);
	DECLARE_WRITE32_MEMBER(data_w);

	bool int_level_get();

	DECLARE_INPUT_CHANGED_MEMBER(update);

protected:
	virtual void device_start();
	virtual void device_reset();
	virtual void device_timer(emu_timer &timer, device_timer_id id, int param, void *ptr);

private:
	enum { FLAG_INT = 0x800000, FLAG_DATA = 0x400000, FLAG_RESET = 0x000200 };
	enum { FIFO_SIZE = 32 };
	enum { KEYUP = 0x0080, KEYVALID = 0x8000 };

	int_cb_t int_change_cb;
	emu_timer *kbd_timer;

	UINT32 control, control2, data, fifo_ir, fifo_iw, fifo_size;
	UINT32 fifo[FIFO_SIZE];
	UINT16 modifiers_state;

	void fifo_push(UINT32 val);
	UINT32 fifo_pop();
	bool fifo_empty() const;

	void send();
};

extern const device_type NEXTKBD;

#endif
