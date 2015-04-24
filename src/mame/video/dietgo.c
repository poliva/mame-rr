#include "emu.h"
#include "video/deco16ic.h"
#include "includes/dietgo.h"
#include "video/decospr.h"

SCREEN_UPDATE( dietgo )
{
	dietgo_state *state = screen->machine().driver_data<dietgo_state>();
	UINT16 flip = deco16ic_pf_control_r(state->m_deco_tilegen1, 0, 0xffff);

	flip_screen_set(screen->machine(), BIT(flip, 7));
	deco16ic_pf_update(state->m_deco_tilegen1, state->m_pf1_rowscroll, state->m_pf2_rowscroll);

	bitmap_fill(bitmap, cliprect, 256); /* not verified */

	deco16ic_tilemap_2_draw(state->m_deco_tilegen1, bitmap, cliprect, TILEMAP_DRAW_OPAQUE, 0);
	deco16ic_tilemap_1_draw(state->m_deco_tilegen1, bitmap, cliprect, 0, 0);

	screen->machine().device<decospr_device>("spritegen")->draw_sprites(screen->machine(), bitmap, cliprect, state->m_spriteram, 0x400);
	return 0;
}