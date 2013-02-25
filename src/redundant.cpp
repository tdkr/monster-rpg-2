#include "monster2.hpp"

// FIXME: oh boy
#define ASSERT ALLEGRO_ASSERT
#include <allegro5/internal/aintern_bitmap.h>
#include <allegro5/internal/aintern.h>
#include <allegro5/internal/aintern_convert.h>
#include <allegro5/internal/aintern_display.h>

#ifdef A5_OGL
#include "allegro5/internal/aintern_opengl.h"
#endif

static std::vector<LoadedBitmap *> loaded_bitmaps;

static float get_trap_peak(int topw, int bottomw, int length)
{
	int w = (bottomw - topw) / 2.0f;
	float a = atan((float)length/w);
	return tan(a) * (topw/2.0f);
}

static void null_lb(LoadedBitmap *lb)
{
	lb->load_type = (LoadType)0;
	lb->load.filename = "";
	lb->load.redraw = 0;
	lb->load.data = 0;
	lb->destroy.func = 0;
	lb->recreate.func = 0;
	lb->recreate.data = 0;
	lb->recreate.w = 0;
	lb->recreate.h = 0;
	lb->bitmap = 0;
	lb->format = 0;
	lb->delayed = 0;
}

MCOLOR m_map_rgba(int r, int g, int b, int a)
{
	return al_map_rgba(r, g, b, a);
}


MCOLOR m_map_rgb(int r, int g, int b)
{
	return al_map_rgb(r, g, b);
}

MCOLOR m_map_rgb_f(float r, float g, float b)
{
	return al_map_rgb_f(r, g, b);
}

void m_unmap_rgba(MCOLOR c,
	unsigned char *r,
	unsigned char *g,
	unsigned char *b,
	unsigned char *a)
{
	al_unmap_rgba(c, r, g, b, a);
}

MBITMAP *m_create_alpha_bitmap(int w, int h, void (*create)(MBITMAP *bitmap, RecreateData *), RecreateData *data, void (*destroy)(MBITMAP *b), bool delayed) // check
{
	int f = al_get_new_bitmap_format();
	al_set_new_bitmap_format(ALPHA_FMT);
	ALLEGRO_BITMAP *bitmap;
	int flags = al_get_new_bitmap_flags();

	int new_flags = flags;

	if (config.getUseOnlyMemoryBitmaps()) {
		al_set_new_bitmap_flags(new_flags|ALLEGRO_MEMORY_BITMAP);
		bitmap = al_create_bitmap(w, h);
	}
	else {
		bitmap = al_create_bitmap(w, h);
	}

	al_set_new_bitmap_flags(flags);
	al_set_new_bitmap_format(f);

	MBITMAP *m = new_mbitmap(bitmap);

	if (create) {
		create(m, data);
	}

#if defined ALLEGRO_ANDROID || defined A5_D3D
	if ((al_get_bitmap_flags(bitmap) & ALLEGRO_NO_PRESERVE_TEXTURE) &&
			!(al_get_bitmap_flags(bitmap) & ALLEGRO_MEMORY_BITMAP)) {
		LoadedBitmap *lb = new LoadedBitmap;
		null_lb(lb);
		lb->load_type = LOAD_CREATE;
		lb->flags = al_get_bitmap_flags(bitmap);
		lb->format = al_get_bitmap_format(bitmap);
		lb->destroy.func = destroy;
		lb->recreate.func = create;
		lb->recreate.data = data;
		lb->recreate.w = w;
		lb->recreate.h = h;
		lb->bitmap = m;
		lb->delayed = delayed;
		loaded_bitmaps.push_back(lb);
	}
	else {
#endif
	if (data) {
		delete data;
	}
#if defined ALLEGRO_ANDROID || defined A5_D3D
	}
#endif

	return m;
}

std::stack<SAVED_BLENDER> blender_stack;
ALLEGRO_COLOR _blend_color;

void m_draw_alpha_bitmap(MBITMAP *b, int x, int y)
{
	m_draw_bitmap(b, x, y, 0);
}

void m_draw_alpha_bitmap(MBITMAP *b, int x, int y, int flags)
{
	m_draw_bitmap(b, x, y, flags);
}

void m_textout(const MFONT *font, const char *text, int x, int y, MCOLOR color)
{
	al_draw_text(font, color, x, y-2, 0, text);
}

void m_textout_f(const MFONT *font, const char *text, float x, float y, MCOLOR color)
{
	al_draw_text(font, color, x, y-2, 0, text);
}

void m_textout_centre(const MFONT *font, const char *text, int x, int y, MCOLOR color)
{
	int length = al_get_text_width(font, text);
	m_textout(font, text, x-length/2, y, color);
}


int m_text_height(const MFONT *font)
{
	return al_get_font_line_height(font);
}

static INLINE float get_factor(int operation, float alpha)
{
   switch (operation) {
       case ALLEGRO_ZERO: return 0;
       case ALLEGRO_ONE: return 1;
       case ALLEGRO_ALPHA: return alpha;
       case ALLEGRO_INVERSE_ALPHA: return 1 - alpha;
   }
   return 0; /* silence warning in release build */
}

static INLINE void _al_blend_inline(
   const ALLEGRO_COLOR *scol, const ALLEGRO_COLOR *dcol,
   ALLEGRO_COLOR *result)
{
	float a = scol->a;
	float inv_a = (1 - scol->a);
	result->r = scol->r*a + dcol->r*inv_a;
	result->g = scol->g*a + dcol->g*inv_a;
	result->b = scol->b*a + dcol->b*inv_a;
}

void m_draw_rectangle(float x1, float y1, float x2, float y2, MCOLOR color,
	int flags)
{
	if (flags & M_FILLED) {
		al_draw_filled_rectangle(x1, y1, x2, y2, color);
	}
	else {
		al_draw_rectangle(x1, y1, x2, y2, color, 1);
	}
}


void m_draw_line(int x1, int y1, int x2, int y2, MCOLOR color)
{
	al_draw_line(x1+0.5, y1+0.5, x2+0.5, y2+0.5, color, 1);
}


void m_draw_scaled_bitmap(MBITMAP *bmp, float sx, float sy, float sw, float sh,
	float dx, float dy, float dw, float dh, int flags)
{
	al_draw_tinted_scaled_bitmap(bmp->bitmap, _blend_color, sx, sy, sw, sh, dx, dy, dw, dh, flags);
}

void m_draw_scaled_bitmap(MBITMAP *bmp, float sx, float sy, float sw, float sh,
	float dx, float dy, float dw, float dh, int flags, float alpha)
{
	m_save_blender();
	float alpha_f = alpha/255.0;
	m_set_blender(M_ONE, M_INVERSE_ALPHA, al_map_rgba_f(alpha_f, alpha_f, alpha_f, alpha_f));
	al_draw_tinted_scaled_bitmap(bmp->bitmap, _blend_color, sx, sy, sw, sh, dx, dy, dw, dh, flags);
	m_restore_blender();
}


int m_get_bitmap_width(MBITMAP *bmp)
{
	return al_get_bitmap_width(bmp->bitmap);
}


int m_get_bitmap_height(MBITMAP *bmp)
{
	return al_get_bitmap_height(bmp->bitmap);
}

MBITMAP *new_mbitmap(ALLEGRO_BITMAP *bitmap)
{
	MBITMAP *m = new MBITMAP;
	m->bitmap = bitmap;
	return m;
}

MBITMAP *m_load_bitmap(const char *name, bool force_memory, bool ok_to_fail)
{
	ALLEGRO_BITMAP *bitmap = 0;
	int flags = al_get_new_bitmap_flags();

	if (force_memory || config.getUseOnlyMemoryBitmaps()) {
		al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
	}
	else {
		al_set_new_bitmap_flags((flags & ~ALLEGRO_MEMORY_BITMAP));
	}

	bitmap = my_load_bitmap(name, ok_to_fail);

	al_set_new_bitmap_flags(flags);

	if (!bitmap) {
		return NULL;
	}

	MBITMAP *m = new_mbitmap(bitmap);

#if defined ALLEGRO_ANDROID || defined A5_D3D
	if ((al_get_bitmap_flags(bitmap) & ALLEGRO_NO_PRESERVE_TEXTURE) &&
			!(al_get_bitmap_flags(bitmap) & ALLEGRO_MEMORY_BITMAP)) {
		LoadedBitmap *lb = new LoadedBitmap;
		null_lb(lb);
		lb->load_type = LOAD_LOAD;
		lb->flags = al_get_bitmap_flags(bitmap);
		lb->format = al_get_bitmap_format(bitmap);
		lb->load.filename = name;
		lb->load.redraw = NULL;
		lb->load.data = NULL;
		lb->bitmap = m;
		loaded_bitmaps.push_back(lb);
	}
#endif

	return m;
}


MBITMAP *m_load_bitmap_redraw(const char *name, void (*redraw)(MBITMAP *bmp, RecreateData *data), RecreateData *data, bool delayed)
{
	ALLEGRO_BITMAP *bitmap = 0;

	bitmap = my_load_bitmap(name);

	if (!bitmap) {
		debug_message("Error loading bitmap %s\n", name);
		return NULL;
	}

	MBITMAP *m = new_mbitmap(bitmap);

	if (redraw) {
		redraw(m, data);
	}

#if defined ALLEGRO_ANDROID || defined A5_D3D
	if ((al_get_bitmap_flags(bitmap) & ALLEGRO_NO_PRESERVE_TEXTURE) &&
			!(al_get_bitmap_flags(bitmap) & ALLEGRO_MEMORY_BITMAP)) {
		LoadedBitmap *lb = new LoadedBitmap;
		null_lb(lb);
		lb->load_type = LOAD_LOAD;
		lb->flags = al_get_bitmap_flags(bitmap);
		lb->format = al_get_bitmap_format(bitmap);
		lb->load.filename = name;
		lb->load.redraw = redraw;
		lb->load.data = data;
		lb->bitmap = m;
		lb->delayed = delayed;
		loaded_bitmaps.push_back(lb);
	}
#endif

	return m;
}


MBITMAP *m_load_alpha_bitmap(const char *name, bool force_memory)
{
	ALLEGRO_BITMAP *bitmap = 0;

	int old = al_get_new_bitmap_format();
	al_set_new_bitmap_format(ALPHA_FMT);

	if (!force_memory)
		bitmap = my_load_bitmap(name);

	if (!bitmap) {
		ALLEGRO_STATE s;
		al_store_state(&s, ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);
		al_set_new_bitmap_flags(ALLEGRO_MEMORY_BITMAP);
		bitmap = my_load_bitmap(name);
		al_restore_state(&s);
	}

	al_set_new_bitmap_format(old);

	if (!bitmap) {
		return NULL;
	}

	MBITMAP *m = new_mbitmap(bitmap);

#if defined ALLEGRO_ANDROID || defined A5_D3D
	if ((al_get_bitmap_flags(bitmap) & ALLEGRO_NO_PRESERVE_TEXTURE) &&
			!(al_get_bitmap_flags(bitmap) & ALLEGRO_MEMORY_BITMAP)) {
		LoadedBitmap *lb = new LoadedBitmap;
		null_lb(lb);
		lb->load_type = LOAD_LOAD;
		lb->flags = al_get_bitmap_flags(bitmap);
		lb->format = al_get_bitmap_format(bitmap);
		lb->load.filename = name;
		lb->load.redraw = NULL;
		lb->load.data = NULL;
		lb->bitmap = m;
		loaded_bitmaps.push_back(lb);
	}
#endif

	return m;
}


MFONT *m_load_font(const char *name)
{
	ALLEGRO_FONT *f;

	int flags = al_get_new_bitmap_flags();
	al_set_new_bitmap_flags(flags & ~ALLEGRO_NO_PRESERVE_TEXTURE);

	f = al_load_font(name, 0, 0);
	if (f) {
		al_set_new_bitmap_flags(flags);
		return f;
	}

	ALLEGRO_DEBUG("couldn't load font: using mem bitmaps");

	al_set_new_bitmap_flags(flags|ALLEGRO_MEMORY_BITMAP);
	f = al_load_font(name, 0, 0);
	al_set_new_bitmap_flags(flags);
	return f;
}


void my_clear_bitmap(MBITMAP *b)
{
	ALLEGRO_DEBUG("setting target in my_clear_bitmap!\n");
	ALLEGRO_BITMAP *target = al_get_target_bitmap();
	ALLEGRO_DEBUG("setting target to b\n");
	m_set_target_bitmap(b);
	ALLEGRO_DEBUG("calling m_clear!\n");
	m_clear(al_map_rgba(0, 0, 0, 0));
	ALLEGRO_DEBUG("resetting target\n");
	al_set_target_bitmap(target);
	ALLEGRO_DEBUG("my_clear_bitmap done\n");
}


MBITMAP *m_create_bitmap(int w, int h, void (*create)(MBITMAP *bitmap, RecreateData *), RecreateData *data, void (*destroy)(MBITMAP *b), bool delayed) // check
{
	ALLEGRO_BITMAP *bitmap = NULL;
	int flags = al_get_new_bitmap_flags();
	
	int new_flags = flags;

	if (!config.getUseOnlyMemoryBitmaps()) {
		ALLEGRO_DEBUG("calling al_create_bitmap (!use memory bitmaps)\n");
		al_set_new_bitmap_flags(new_flags);
		bitmap = al_create_bitmap(w, h);
	}
	if (!bitmap) {
		ALLEGRO_DEBUG("!bitmap, trying memory\n");
		al_set_new_bitmap_flags(new_flags|ALLEGRO_MEMORY_BITMAP);
		bitmap = al_create_bitmap(w, h);
	}

	al_set_new_bitmap_flags(flags);
		
	if (!bitmap) {
		printf("error creating bitmap\n");
		return NULL;
	}

	ALLEGRO_DEBUG("creating mbitmap\n");

	MBITMAP *m = new_mbitmap(bitmap);

	ALLEGRO_DEBUG("clearing it\n");
	my_clear_bitmap(m);

	ALLEGRO_DEBUG("cleared!\n");
	
	if (create) {
		create(m, data);
	}

#if defined ALLEGRO_ANDROID || defined A5_D3D
	if ((al_get_bitmap_flags(bitmap) & ALLEGRO_NO_PRESERVE_TEXTURE) &&
			!(al_get_bitmap_flags(bitmap) & ALLEGRO_MEMORY_BITMAP)) {
		LoadedBitmap *lb = new LoadedBitmap;
		null_lb(lb);
		lb->load_type = LOAD_CREATE;
		lb->flags = al_get_bitmap_flags(bitmap);
		lb->format = al_get_bitmap_format(bitmap);
		lb->destroy.func = destroy;
		lb->recreate.func = create;
		lb->recreate.data = data;
		lb->recreate.w = w;
		lb->recreate.h = h;
		lb->bitmap = m;
		lb->delayed = delayed;
		loaded_bitmaps.push_back(lb);
	}
	else {
#endif
	if (data) {
		delete data;
	}
#if defined ALLEGRO_ANDROID || defined A5_D3D
	}
#endif

	ALLEGRO_DEBUG("returning from m_create_bitmap\n");

	return m;
}


void m_destroy_bitmap(MBITMAP *bmp, bool internals_only)
{
	for (size_t i = 0; i < loaded_bitmaps.size(); i++) {
		if (loaded_bitmaps[i]->bitmap == bmp) {
			if (loaded_bitmaps[i]->load_type == LOAD_CREATE && loaded_bitmaps[i]->recreate.data) {
				delete loaded_bitmaps[i]->recreate.data;
			}
			delete loaded_bitmaps[i];
			loaded_bitmaps.erase(loaded_bitmaps.begin()+i);
			break;
		}
	}

	if (bmp->bitmap) {
		al_destroy_bitmap(bmp->bitmap);
		bmp->bitmap = NULL;
	}
	
	if (!internals_only) {
		delete bmp;
	}
}


void m_flip_display(void)
{
	bool skip_flip = false;

	if (prompt_for_close_on_next_flip) {
		bool hide = is_cursor_hidden();
		show_mouse_cursor();
		prompt_for_close_on_next_flip = false;
		int r = triple_prompt("", "Really quit game or return to menu?", "", "Menu", "Quit", "Cancel", 2, true);
		if (hide) {
			hide_mouse_cursor();
		}
		if (r == 0) {
			break_main_loop = true;
		}
		else if (r == 1) {
			do_close_exit_game();
		}
		skip_flip = true;
	}

	if (show_item_info_on_flip >= 0) {
		int tmp = show_item_info_on_flip;
		show_item_info_on_flip = -1;
		showItemInfo(tmp, true);
		skip_flip = true;
	}

	if (!skip_flip) {
		al_flip_display();
	}

	int xxx, yyy, www, hhh;
	al_get_clipping_rectangle(&xxx, &yyy, &www, &hhh);
	al_set_clipping_rectangle(
		0, 0,
		al_get_display_width(display),
		al_get_display_height(display)
	);
	m_clear(black);
	al_set_clipping_rectangle(xxx, yyy, www, hhh);

	if (controller_display)
	{
		ALLEGRO_BITMAP *target = al_get_target_bitmap();
		al_set_target_backbuffer(controller_display);
		al_flip_display();
		al_set_target_bitmap(target);
	}

	fps_frames++;
	double elapsed = al_get_time() - fps_counter;
	if (fps_on && elapsed > 2) {
		fps = (int)((float)fps_frames/elapsed);
		fps_counter = al_get_time();
		fps_frames = 0;
	}
}

void m_draw_circle(int x, int y, int radius, MCOLOR color,
	int flags)
{
	if (flags & M_FILLED) {
		al_draw_filled_circle(x, y, radius, color);
	}
	else {
		al_draw_circle(x, y, radius, color, 1);
	}
}


void m_rest(double seconds)
{
	int i = (int)seconds;
	double remainder = seconds-i;

	for (int j = 0; j < i; j++) {
		al_rest(1);
		if (is_close_pressed()) {
			do_close();
			close_pressed = false;
		}
		if (break_main_loop) {
			break;
		}
	}

	al_rest(remainder);
}


int m_get_display_width(void)
{
	return al_get_display_width(display);
}


int m_get_display_height(void)
{
	return al_get_display_height(display);
}


void m_clear(MCOLOR color)
{
	al_clear_to_color(color);
}


static int m_text_length_real(const MFONT *font, const char *text)
{
	return al_get_text_width(font, text);
}

void m_set_target_bitmap(MBITMAP *bmp)
{
	al_set_target_bitmap(bmp->bitmap);
}


void m_set_clip(int x1, int y1, int x2, int y2)
{
	if (al_get_target_bitmap() == al_get_backbuffer(display)) {
		int dx, dy, dw, dh;
		get_screen_offset_size(&dx, &dy, &dw, &dh);
		al_set_clipping_rectangle(dx+x1*screenScaleX, dy+y1*screenScaleY, (x2-x1)*screenScaleX, (y2-y1)*screenScaleY);
	}
	else {
		al_set_clipping_rectangle(
			x1, y1, x2-x1, y2-y1
		);
	}
}



void m_put_pixel(int x, int y, MCOLOR color)
{
	al_put_pixel(x, y, color);
}

void m_draw_trans_pixel(int x, int y, MCOLOR color)
{
#if defined A5_D3D || defined ALLEGRO_IPHONE
	al_draw_pixel(x, y, color);
#else
	// Workaround for some OpenGL drivers
	m_draw_rectangle(x, y, x+1, y+1, color, M_FILLED);
#endif
}

// -angle?!?! really?!
void m_draw_rotated_bitmap(MBITMAP *bitmap, int cx, int cy,
	int dx, int dy, float angle, int flags)
{
	al_draw_tinted_rotated_bitmap(bitmap->bitmap, _blend_color, cx, cy, dx, dy, -angle, flags);
}

void m_fill_ellipse(int x, int y, int rx, int ry, MCOLOR color)
{
	if (rx <= 0 || ry <= 0) return;
	al_draw_filled_ellipse(x, y, rx, ry, color);
}

void m_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, MCOLOR c)
{
	al_draw_filled_triangle(x1, y1, x2, y2, x3, y3, c);
}

MBITMAP *create_trapezoid(int dir, int topw, int bottomw, int length, MCOLOR color)
{
	MBITMAP *b;

	color.a = 1.0f;
	
	m_push_target_bitmap();

	int flgs = al_get_new_bitmap_flags();
	al_set_new_bitmap_flags(flgs & ~ALLEGRO_NO_PRESERVE_TEXTURE);
	
	if (dir == DIRECTION_NORTH) {
		b = m_create_bitmap(bottomw, length); // check
	}
	else if (dir == DIRECTION_EAST) {
		b = m_create_bitmap(length, bottomw); // check
	}
	else if (dir == DIRECTION_SOUTH) {
		b = m_create_bitmap(bottomw, length); // check
	}
	else {
		b = m_create_bitmap(length, bottomw); // check
	}

	al_set_new_bitmap_flags(flgs);

	m_set_target_bitmap(b);
	m_clear(m_map_rgba(0, 0, 0, 0));

	float y = get_trap_peak(topw, bottomw, length);

	if (dir == DIRECTION_NORTH) {
		m_draw_triangle(bottomw/2, length+y-1, 0, 0, bottomw-1, 0, color);
	}
	else if (dir == DIRECTION_EAST) {
		m_draw_triangle(-y, bottomw/2, length-1, 0, length-1, bottomw-1, color);
	}
	else if (dir == DIRECTION_SOUTH) {
		m_draw_triangle(bottomw/2, -y, bottomw-1, length-1, 0, length-1, color);
	}
	else if (dir == DIRECTION_WEST) {
		m_draw_triangle(length+y-1, bottomw/2, 0, 0, 0, bottomw-1, color);
	}

	m_pop_target_bitmap();

	return b;
}



void m_draw_trans_bitmap(MBITMAP *b, int x, int y, int alpha)
{
	m_save_blender();

	m_set_blender(M_ONE, M_INVERSE_ALPHA, m_map_rgba(alpha, alpha, alpha, alpha));

	m_draw_bitmap(b, x, y, 0);

	m_restore_blender();
}


void m_destroy_font(MFONT *f)
{
	al_destroy_font(f);
}

MBITMAP *m_clone_bitmap(MBITMAP *b)
{
	ALLEGRO_BITMAP *bmp = al_clone_bitmap(b->bitmap);
	MBITMAP *m = new_mbitmap(bmp);
	return m;
}

MBITMAP *m_make_display_bitmap(MBITMAP *b)
{
	if (!b) return NULL;
	ALLEGRO_BITMAP *tmp = al_clone_bitmap(b->bitmap);
	al_destroy_bitmap(b->bitmap);
	b->bitmap = tmp;
	return b;
}

MBITMAP *m_make_alpha_display_bitmap(MBITMAP *in)
{
	MBITMAP *bitmap = 0;

	int old = al_get_new_bitmap_format();
	al_set_new_bitmap_format(ALPHA_FMT);

	bitmap = m_clone_bitmap(in);

	al_set_new_bitmap_format(old);

	m_destroy_bitmap(in);

	return bitmap;
}



static int find_char(const char *text)
{
	int i = 0;

	while (text[i]) {
		if (text[i] == '{') {
			return i;
		}
		i++;
	}

	return -1;
}


// find length taking into account {xxx} special icon codes
int m_text_length(const MFONT *font, const char *text)
{
	char part[100];
	int n;
	const char *p = text;
	int length = 0;

	while ((n = find_char(p)) >= 0) {
		int i;
		for (i = 0; i < n; i++) {
			part[i] = p[i];
		}
		part[i] = 0;
		length += m_text_length_real(font, part) + m_text_height(font) + 2;
		p += n+5;
	}

	length += m_text_length_real(font, p);

	return length;
}


static ALLEGRO_BITMAP *pushed_target;

void m_push_target_bitmap(void)
{
	pushed_target = al_get_target_bitmap();
}

void m_pop_target_bitmap(void)
{
	al_set_target_bitmap(pushed_target);
}

void m_get_mouse_state(ALLEGRO_MOUSE_STATE *s)
{
	al_get_mouse_state(s);
	if (!config.getMaintainAspectRatio())
		tguiConvertMousePosition(&s->x, &s->y, 0, 0, screen_ratio_x, screen_ratio_y);
	else
		tguiConvertMousePosition(&s->x, &s->y, screen_offset_x, screen_offset_y, 1, 1);
}

float my_get_opengl_version(void)
{
   const char *s = (char *)glGetString(GL_VERSION);
   if (strstr(s, "2.0")) {
      return 2.0;
   }
   else if (strstr(s, "1.1"))
      return 1.5;
   else
      return 1.3;
}

void m_set_blender(int s, int d, MCOLOR c)
{
	_blend_color = c;
	al_set_blender(ALLEGRO_ADD, s, d);
}

void m_save_blender(void)
{
	SAVED_BLENDER sb;

	al_get_blender(
		&sb.oldColorOp,
		&sb.oldSrcColorFactor,
		&sb.oldDestColorFactor
	);

	sb.oldBlendColor = _blend_color;

	blender_stack.push(sb);
}


void m_restore_blender(void)
{
	SAVED_BLENDER sb = blender_stack.top();
	blender_stack.pop();

	al_set_blender(
		sb.oldColorOp,
		sb.oldSrcColorFactor,
		sb.oldDestColorFactor
	);

	_blend_color = sb.oldBlendColor;
}

void m_draw_prim (const void* vtxs, const ALLEGRO_VERTEX_DECL* decl, MBITMAP* texture, int start, int end, int type)
{
/*
#if !defined ALLEGRO_ANDROID && !defined ALLEGRO_RASPBERRYPI
#if defined __linux__
	// work around for nvidia+gallium
	if (!is_intel_gpu_on_desktop_linux && type == ALLEGRO_PRIM_POINT_LIST) {
		ALLEGRO_VERTEX *verts = (ALLEGRO_VERTEX *)vtxs;
		for (int i = start; i < end; i++) {
			m_draw_trans_pixel(verts[i].x, verts[i].y, verts[i].color);
		}
		return;
	}
#endif
#endif
*/
#if 1//defined ALLEGRO_RASPBERRYPI || defined ALLEGRO_ANDROID
	if (type == ALLEGRO_PRIM_POINT_LIST) {
		int n = end-start;
		ALLEGRO_VERTEX *v = new ALLEGRO_VERTEX[n*6];
		ALLEGRO_VERTEX *verts = (ALLEGRO_VERTEX *)vtxs;
		for (int i = 0; i < n; i++) {
			v[i*6+0].x = verts[i+start].x;
			v[i*6+0].y = verts[i+start].y;
			v[i*6+0].z = 0;
			v[i*6+0].color = verts[i+start].color;
			v[i*6+1].x = verts[i+start].x+1;
			v[i*6+1].y = verts[i+start].y;
			v[i*6+1].z = 0;
			v[i*6+1].color = verts[i+start].color;
			v[i*6+2].x = verts[i+start].x+1;
			v[i*6+2].y = verts[i+start].y+1;
			v[i*6+2].z = 0;
			v[i*6+2].color = verts[i+start].color;
			v[i*6+3].x = verts[i+start].x;
			v[i*6+3].y = verts[i+start].y+1;
			v[i*6+3].z = 0;
			v[i*6+3].color = verts[i+start].color;
			v[i*6+4].x = verts[i+start].x;
			v[i*6+4].y = verts[i+start].y;
			v[i*6+4].z = 0;
			v[i*6+4].color = verts[i+start].color;
			v[i*6+5].x = verts[i+start].x+1;
			v[i*6+5].y = verts[i+start].y+1;
			v[i*6+5].z = 0;
			v[i*6+5].color = verts[i+start].color;
		}
		al_draw_prim(v, 0, 0, 0, n*6, ALLEGRO_PRIM_TRIANGLE_LIST);
		delete[] v;
	}
	else
#endif
	al_draw_prim(vtxs, decl, (texture ? texture->bitmap : NULL), start, end, type);
}

MBITMAP *m_create_sub_bitmap(MBITMAP *parent, int x, int y, int w, int h) // check
{
	ALLEGRO_BITMAP *sub = al_create_sub_bitmap(parent->bitmap, x, y, w, h);
	MBITMAP *b = new_mbitmap(sub);
	return b;
}

ALLEGRO_LOCKED_REGION *m_lock_bitmap(MBITMAP *b, int format, int flags)
{
	return al_lock_bitmap(b->bitmap, format, flags);
}

ALLEGRO_LOCKED_REGION *m_lock_bitmap_region(MBITMAP *b, int x, int y, int w, int h, int format, int flags)
{
	return al_lock_bitmap_region(b->bitmap, x, y, w, h, format, flags);
}

void m_unlock_bitmap(MBITMAP *b)
{
	al_unlock_bitmap(b->bitmap);
}

void _destroy_loaded_bitmaps(void)
{
	if (cached_bitmap) {
		al_destroy_bitmap(cached_bitmap);
		cached_bitmap = NULL;
		cached_bitmap_filename = "";
	}

	for (size_t i = 0; i < loaded_bitmaps.size(); i++) {
		if (!(loaded_bitmaps[i]->flags & ALLEGRO_NO_PRESERVE_TEXTURE)) {
			continue;
		}
		if (loaded_bitmaps[i]->load_type == LOAD_CREATE && loaded_bitmaps[i]->destroy.func) {
			(*loaded_bitmaps[i]->destroy.func)(loaded_bitmaps[i]->bitmap);
		}
		else {
			MBITMAP *m = loaded_bitmaps[i]->bitmap;
			al_destroy_bitmap(m->bitmap);
			m->bitmap = NULL;
		}
	}
}

void _reload_loaded_bitmaps(void)
{
	int flags = al_get_new_bitmap_flags();
	int format = al_get_new_bitmap_format();

	for (size_t i = 0; i < loaded_bitmaps.size(); i++) {
		MBITMAP *m = loaded_bitmaps[i]->bitmap;
		if ((loaded_bitmaps[i]->flags & ALLEGRO_NO_PRESERVE_TEXTURE) && !loaded_bitmaps[i]->delayed) {
			al_set_new_bitmap_flags(loaded_bitmaps[i]->flags);
			al_set_new_bitmap_format(loaded_bitmaps[i]->format);
			if (loaded_bitmaps[i]->load_type == LOAD_LOAD) {
				m->bitmap = my_load_bitmap(loaded_bitmaps[i]->load.filename.c_str());
				if (loaded_bitmaps[i]->load.redraw) {
					loaded_bitmaps[i]->load.redraw(m, loaded_bitmaps[i]->load.data);
				}
			}
			else { // create
				Recreate *d = &loaded_bitmaps[i]->recreate;
				m->bitmap = al_create_bitmap(d->w, d->h);
				if (d->func) { // recreate with func
					d->func(m, d->data);
				}
			}
		}
	}

	al_set_new_bitmap_flags(flags);
	al_set_new_bitmap_format(format);
}

void _reload_loaded_bitmaps_delayed(void)
{
	int flags = al_get_new_bitmap_flags();
	int format = al_get_new_bitmap_format();

	for (size_t i = 0; i < loaded_bitmaps.size(); i++) {
		MBITMAP *m = loaded_bitmaps[i]->bitmap;
		if ((loaded_bitmaps[i]->flags & ALLEGRO_NO_PRESERVE_TEXTURE) && loaded_bitmaps[i]->delayed) {
			al_set_new_bitmap_flags(loaded_bitmaps[i]->flags);
			al_set_new_bitmap_format(loaded_bitmaps[i]->format);
			if (loaded_bitmaps[i]->load_type == LOAD_LOAD) {
				m->bitmap = my_load_bitmap(loaded_bitmaps[i]->load.filename.c_str());
				if (loaded_bitmaps[i]->load.redraw) {
					loaded_bitmaps[i]->load.redraw(m, loaded_bitmaps[i]->load.data);
				}
			}
			else { // create
				Recreate *d = &loaded_bitmaps[i]->recreate;
				m->bitmap = al_create_bitmap(d->w, d->h);
				if (d->func) { // recreate with func
					d->func(m, d->data);
				}
			}
		}
	}

	al_set_new_bitmap_flags(flags);
	al_set_new_bitmap_format(format);
}

static MBITMAP *clone_sub_bitmap(MBITMAP *b)
{
	ALLEGRO_STATE st;
	al_store_state(&st, ALLEGRO_STATE_TARGET_BITMAP | ALLEGRO_STATE_BLENDER | ALLEGRO_STATE_NEW_BITMAP_PARAMETERS);

	MBITMAP *clone = m_create_alpha_bitmap(
		m_get_bitmap_width(b),
		m_get_bitmap_height(b)
	);

	m_set_target_bitmap(clone);
	m_clear(al_map_rgba_f(0, 0, 0, 0));

	al_set_blender(ALLEGRO_ADD, ALLEGRO_ONE, ALLEGRO_ZERO);

	m_draw_bitmap(b, 0, 0, 0);

	al_restore_state(&st);

	return clone;
}

void m_draw_bitmap_to_self(MBITMAP *b, int x, int y, int flags)
{
	MBITMAP *tmp = clone_sub_bitmap(b);
	m_draw_bitmap(tmp, x, y, flags);
	m_destroy_bitmap(tmp);
}

void m_draw_bitmap_region_to_self(MBITMAP *b, int sx, int sy, int sw, int sh, int dx, int dy, int flags)
{
	MBITMAP *tmp = clone_sub_bitmap(b);
	m_draw_bitmap_region(tmp, sx, sy, sw, sh, dx, dy, flags);
	m_destroy_bitmap(tmp);
}

void m_draw_scaled_backbuffer(int sx, int sy, int sw, int sh, int dx, int dy, int dw, int dh, MBITMAP *dest)
{
	ALLEGRO_BITMAP *old_target = al_get_target_bitmap();
	int old_format = al_get_new_bitmap_format();
	al_set_new_bitmap_format(al_get_bitmap_format(al_get_backbuffer(display)));
	MBITMAP *tmp = m_create_bitmap(sw, sh);
	int scr_w = al_get_display_width(display);
	int scr_h = al_get_display_height(display);
	if (sx+sw > scr_w) {
		sw = scr_w-sx;
	}
	else if (sx < 0) {
		sw -= sx;
		sx = 0;
	}
	if (sy+sh > scr_h) {
		sh = scr_h-sy;
	}
	else if (sy < 0) {
		sh -= sy;
		sy = 0;
	}
#ifdef ALLEGRO_RASPBERRYPI
	ALLEGRO_LOCKED_REGION *lr1 = al_lock_bitmap(tmp->bitmap, ALLEGRO_PIXEL_FORMAT_ANY, ALLEGRO_LOCK_WRITEONLY);
	ALLEGRO_LOCKED_REGION *lr2 = al_lock_bitmap_region(
		al_get_backbuffer(display),
		sx, sy, sw, sh,
		ALLEGRO_PIXEL_FORMAT_ANY,
		ALLEGRO_LOCK_READONLY
	);
	int pixel_size = al_get_pixel_size(al_get_bitmap_format(al_get_backbuffer(display)));
	for (int y = 0; y < sh; y++) {
		uint8_t *d1 = (uint8_t *)lr1->data + lr1->pitch * y;
		uint8_t *d2 = (uint8_t *)lr2->data + lr2->pitch * y;
		memcpy(d1, d2, pixel_size*sw);
	}
	al_unlock_bitmap(tmp->bitmap);
	al_unlock_bitmap(al_get_backbuffer(display));
#else
	al_set_target_bitmap(tmp->bitmap);
	al_draw_bitmap_region(al_get_backbuffer(display), sx, sy, sw, sh, 0, 0, 0);
#endif
	al_set_target_bitmap(dest->bitmap);
	al_draw_scaled_bitmap(
		tmp->bitmap,
		0, 0, sw, sh,
		dx, dy, dw, dh,
		0
	);
	m_destroy_bitmap(tmp);
	al_set_target_bitmap(old_target);
	al_set_new_bitmap_format(old_format);
}

void m_draw_bitmap_identity_view(MBITMAP *bmp, int x, int y, int flags)
{
	ALLEGRO_TRANSFORM backup, t;
	al_copy_transform(&backup, al_get_current_transform());
	al_identity_transform(&t);
	al_use_transform(&t);
	m_draw_bitmap(bmp, x, y, flags);
	al_use_transform(&backup);
}

