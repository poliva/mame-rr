/**********************************************************************

 Mephisto Chess Computers

**********************************************************************/
#include "emu.h"
#include "machine/mboard.h"

static void set_artwork(running_machine &machine );
static void check_board_buttons(running_machine &machine );

UINT8 mboard_lcd_invert;
UINT8 mboard_key_select;
UINT8 mboard_key_selector;

static const int start_board[64] =
{
	BR, BN, BB, BQ, BK, BB, BN, BR,
	BP, BP, BP, BP, BP, BP, BP, BP,
	EM, EM, EM, EM, EM, EM, EM, EM,
	EM, EM, EM, EM, EM, EM, EM, EM,
	EM, EM, EM, EM, EM, EM, EM, EM,
	EM, EM, EM, EM, EM, EM, EM, EM,
	WP, WP, WP, WP, WP, WP, WP, WP,
	WR, WN, WB, WQ, WK, WB, WN, WR
};

static UINT8 border_pieces[12] = {WK,WQ,WR,WB,WN,WP,BK,BQ,BR,BB,BN,BP,};

static int m_board[64];
static int save_board[64];
static UINT16 Line18_LED;
static UINT16 Line18_REED;

static MOUSE_HOLD mouse_hold;

static int read_board_flag = TRUE;

static int get_first_bit(UINT8 data)
{
	int i;

	for (i = 0; i < 8; i++)
		if (BIT(data, i))
			return i;

	return NOT_VALID;
}

static int get_first_cleared_bit(UINT8 data)
{
	int i;

	for (i=0;i<8;i++)
		if (!BIT(data, i))
			return i;

	return NOT_VALID;
}

static UINT8 read_board(void)
{
	  UINT8 i_18, i_AH;
	  UINT8 data;

	  data = 0xff;

/*

Example board scan:
Starting postion and pawn on E2 is lifted


mask: 7f 0111 1111  Line 8
data:  0 0000 0000  all fields occupied
mask: bf 1011 1111  Line 7
data:  0 0000 0000  all fields occupied
mask: df 1101 1111  Line 6
data: ff 1111 1111  all fields empty
mask: ef 1110 1111  Line 5
data: ff 1111 1111  all fields empty
mask: f7 1111 0111  Line 4
data: ff 1111 1111  all fields empty
mask: fb 1111 1011  Line 3
data: ff 1111 1111  all fields empty
mask: fd 1111 1101  Line 2
data: 10 0001 0000  E2 is empty rest is occupied
mask: fe 1111 1110  Line 1
data:  0 0000 0000  all fields occupied


*/

/* looking for cleared bit in mask Line18_REED => current line */

 if (data && Line18_REED)
 {
		i_18=get_first_cleared_bit(Line18_REED);

/* looking for a piece in this line and clear bit in data if found */

	   for ( i_AH = 0; i_AH < 8; i_AH = i_AH + 1)
			if (IsPiece(64-(i_18*8 + 8-i_AH)))
				data &= ~(1 << i_AH);			// clear bit

	   read_board_flag = TRUE;
 }

	return data;
}


static void write_board( running_machine &machine, UINT8 data)
{

	Line18_REED=data;

	if (read_board_flag && !strcmp(machine.system().name,"glasgow") ) //HACK
		Line18_LED = 0;
	else
		Line18_LED = data;

	 read_board_flag = FALSE;

	if (data == 0xff)
		mboard_key_selector = 0;
}



static void write_LED(UINT8 data)
{
	int i;
	UINT8 i_AH, i_18;
	UINT8 LED;

	mboard_lcd_invert = 1;
/*

Example: turn led E2 on

mask:  fd 1111 1101 Line 2
data:  10 0001 0000 Line E

*/

	for (i=0; i < 64; i++)							/* all  LED's off */
		output_set_led_value(i, 0);

    if (Line18_LED)
    {
		for (i_AH = 0; i_AH < 8; i_AH++)				/* turn  LED on depending on bit masks */
		{
			if (BIT(data,i_AH))
			{
				for (i_18 = 0; i_18 < 8; i_18++)
				{
					LED = (i_18*8 + 8-i_AH-1);
					if (!(Line18_LED & (1 << i_18)))	/* cleared bit */
						output_set_led_value(LED, 1);
					//else
					//  output_set_led_value(LED, 0);
				}
			}
		}
	}

}



READ8_HANDLER( mboard_read_board_8 )
{
	UINT8 data;

	data=read_board();
	logerror("Read Board Port  Data = %d\n  ",data);
	return data;
}

READ16_HANDLER( mboard_read_board_16 )
{
	UINT8 data;

	data=read_board();
	return data << 8;
}

READ32_HANDLER( mboard_read_board_32 )
{
	UINT8 data;

	data=read_board();
	return data<<24;
}

WRITE8_HANDLER( mboard_write_board_8 )
{
	write_board(space->machine(),data);
	logerror("Write Board Port  Data = %02x\n",data);
}

WRITE16_HANDLER( mboard_write_board_16 )
{
	if (data & 0xff) write_board(space->machine(),data);
	logerror("write board 16 %08x\n",data);
	write_board(space->machine(),data>>8);
}

WRITE32_HANDLER( mboard_write_board_32 )
{
//  data |= data << 24;
//printf("write board %08x %08x\n",offset,data);
	logerror("write board 32 o: %08x d: %08x\n",offset,data);
	if (offset) write_board(space->machine(),data);
	else write_board(space->machine(),data>>24);
}

WRITE8_HANDLER( mboard_write_LED_8 )
{
	write_LED(data);
	device_spin_until_time(&space->device(), attotime::from_usec(7));
}

WRITE16_HANDLER( mboard_write_LED_16 )
{
	 write_LED(data >> 8);
	 device_spin_until_time(&space->device(), attotime::from_usec(9));
}

WRITE32_HANDLER( mboard_write_LED_32 )
{
//  data = data | data << 24;
//printf("write LED %08x %08x\n",offset,data);
	if (offset) write_LED(data);
	else write_LED(data >> 24);
	logerror("write LED   32 o: %08x d: %08x\n",offset,data);
//  device_spin_until_time(&space->device(), ATTOTIME_IN_USEC(20));
}


/* save states callback */

static void board_presave(running_machine *machine)
{
	int i;
	for (i=0;i<64;i++)
		save_board[i]=m_board[i];
}

static void board_postload(running_machine *machine)
{
	int i;
	for (i=0;i<64;i++)
		m_board[i]=save_board[i];

}

void mboard_savestate_register(running_machine &machine)
{
	state_save_register_global_array(machine,save_board);
	machine.save().register_postload(save_prepost_delegate(FUNC(board_postload),&machine));
	machine.save().register_presave(save_prepost_delegate(FUNC(board_presave),&machine));
}

void mboard_set_board( void )
{
	int i;
	for (i=0;i<64;i++)
		m_board[i]=start_board[i];
}

static void clear_board( void )
{
	int i;
	for (i=0;i<64;i++)
		m_board[i]=EM;
}

static void set_artwork ( running_machine &machine )
{
	int i;
	for (i=0;i<64;i++)
		output_set_indexed_value("P", i, m_board[i]);
}

void mboard_set_border_pieces (void)
{
	int i;
	for (i=0;i<12;i++)
		output_set_indexed_value("Q", i, border_pieces[i]);
}

TIMER_DEVICE_CALLBACK( mboard_update_artwork )
{
	check_board_buttons(timer.machine());
	set_artwork(timer.machine());
	mboard_set_border_pieces();
}

static void check_board_buttons ( running_machine &machine )
{
	int field;
	int i;
	UINT8 port_input=0;
	UINT8 data = 0xff;
	static const char *const keynames[] = { "LINE2", "LINE3", "LINE4", "LINE5", "LINE6", "LINE7", "LINE8", "LINE9" };
	static UINT8 board_row = 0;
	static UINT16 mouse_down = 0;
	UINT8 pos2num_res = 0;
	board_row++;
	board_row &= 7;
	int click_on_border_piece=FALSE;


/* check click on border pieces */
	i=0;
	port_input=input_port_read(machine, "B_BLACK");
	if (port_input)
	{
		i=get_first_bit(port_input)+6;
		click_on_border_piece=TRUE;
	}

	port_input=input_port_read(machine, "B_WHITE");
	if (port_input)
	{
		i=get_first_bit(port_input);
		click_on_border_piece=TRUE;
	}

	if (click_on_border_piece)
	{
		if (!mouse_down)
		{
			if (border_pieces[i] > 12 )		/* second click on selected border piece */
			{
				mouse_hold.border_piece=FALSE;
				border_pieces[i]=border_pieces[i]-12;
				mouse_hold.from=0;
				mouse_hold.piece=0;
			}
			else if (!mouse_hold.piece)		/*select border piece */
			{
				if  (mouse_hold.border_piece)
					border_pieces[mouse_hold.from]=border_pieces[mouse_hold.from]-12;

				mouse_hold.from=i;
				mouse_hold.piece=border_pieces[i];
				border_pieces[i]=border_pieces[i]+12;
				mouse_hold.border_piece=TRUE;
			}

			mouse_down = board_row + 1;

		}
		return;
	}


/* check click on board */
	data = input_port_read_safe(machine, keynames[board_row], 0xff);

	if ((data != 0xff) && (!mouse_down) )
	{

		pos2num_res = pos_to_num(data);
		field=64-(board_row*8+8-pos2num_res);


		if (!(pos2num_res < 8))
			logerror("Position out of bound!");

		else if ((mouse_hold.piece) && (!IsPiece(field)))
		{
			/* Moving a piece onto a blank */
			m_board[field] = mouse_hold.piece;

			if (mouse_hold.border_piece)
			{
				border_pieces[mouse_hold.from]=border_pieces[mouse_hold.from]-12;
			}else if ( field != mouse_hold.from  )	/* Put a selected piece back to the source field */
				m_board[mouse_hold.from] = 0;


			mouse_hold.from  = 0;
			mouse_hold.piece = 0;
			mouse_hold.border_piece=FALSE;
		}
		else if ((!mouse_hold.piece) )
		{
			/* Picking up a piece */

			if (IsPiece(field))
			{
				mouse_hold.from  = field;
				mouse_hold.piece = m_board[field];
				m_board[field] = m_board[field]+12;
			}

		}

		mouse_down = board_row + 1;
	}
	else if ((data == 0xff) && (mouse_down == (board_row + 1)))	/* Wait for mouse to be released */
		mouse_down = 0;

/* check click on border - remove selected piece*/
	if (input_port_read_safe(machine, "LINE10", 0x01))
	{
		if (mouse_hold.piece)
		{
			if (mouse_hold.border_piece)
				border_pieces[mouse_hold.from]=border_pieces[mouse_hold.from]-12;
			else
				m_board[mouse_hold.from] = 0;

			mouse_hold.from  = 0;
			mouse_hold.piece = 0;
			mouse_hold.border_piece = FALSE;
		}

		return;
	}

/* check additional buttons */
	if (data == 0xff)
	{

		port_input=input_port_read(machine, "B_BUTTONS");
		if (port_input==0x01)
		{
			clear_board();
			return;
		}else if (port_input==0x02)
		{
			mboard_set_board();
			return;
		}


	}

}

extern INPUT_PORTS_START( chessboard )

	PORT_START("LINE2")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE3")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE4")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE5")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE6")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE7")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE8")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_START("LINE9")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x02, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x04, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x08, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x10, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x20, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x40, IP_ACTIVE_LOW, IPT_KEYBOARD)
	PORT_BIT( 0x80, IP_ACTIVE_LOW, IPT_KEYBOARD)

	PORT_START("LINE10")
	PORT_BIT( 0x01, IP_ACTIVE_HIGH, IPT_KEYBOARD)

	PORT_START("B_WHITE")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD)

	PORT_START("B_BLACK")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x04,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x08,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x010, IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x020, IP_ACTIVE_HIGH, IPT_KEYBOARD)

	PORT_START("B_BUTTONS")
	PORT_BIT(0x01,  IP_ACTIVE_HIGH, IPT_KEYBOARD)
	PORT_BIT(0x02,  IP_ACTIVE_HIGH, IPT_KEYBOARD)

	PORT_START("MOUSE_X")
	PORT_BIT( 0xffff, 0x00, IPT_MOUSE_X) PORT_SENSITIVITY(100)

	PORT_START("MOUSE_Y")
	PORT_BIT( 0xffff, 0x00, IPT_MOUSE_Y ) PORT_SENSITIVITY(100)

	PORT_START("BUTTON_L")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON2) PORT_CODE(MOUSECODE_BUTTON1) PORT_NAME("left button")

	PORT_START("BUTTON_R")
	PORT_BIT( 0x01, IP_ACTIVE_LOW, IPT_BUTTON1) PORT_CODE(MOUSECODE_BUTTON2) PORT_NAME("right button")


INPUT_PORTS_END
