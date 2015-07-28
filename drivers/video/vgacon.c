/*
 * Low-Level VGA-Based Console Driver.
 *
 * Including some common operations that can be used by VC layer. This is a
 * very simple implementation. And it is the upper layer, the VC, who
 * responsible for emulating VT102 and processing the escape sequences.
 *
 * Tue Jul 28 13:25:24 CST 2015
 */

#include <sandix/bootparam.h>
#include <sandix/console.h>
#include <sandix/irq.h>
#include <sandix/io.h>
#include <sandix/screen_info.h>
#include <sandix/types.h>
#include <video/vga.h>
#include <video/video.h>

static	u32	vga_visible_origin;	/* Upper left character */
static	u32	vga_vram_base;		/* Base of video memory */
static	u32	vga_vram_end;		/* End of video memory */
static	u32	vga_vram_size;		/* Size of video memory */
static  u8	vga_vram_attr;		/* Character attribute */
static	u32	vga_pos;		/* Cursor position address */
static	u32	vga_x;			/* Cursor x */
static	u32	vga_y;			/* Cursor y */
static	u32	vga_video_num_cols;	/* Number of text columns */
static	u32	vga_video_num_rows;	/* Number of text rows */
static	u16	vga_video_port_reg;	/* Video register select port */
static	u16	vga_video_port_val;	/* Video register value port */

#define VGA_ADDR_OFFSET(x, y)	((vga_video_num_cols*(y) + (x)) << 1)
#define VGA_CHAR_ATTR(ch)	((u16)(ch) & ((u16)(vga_vram_attr) << 8))
#define VGA_MEM_MAP(addr)	(unsigned long)phys_to_virt((addr))

static inline void write_vga(unsigned char reg, unsigned int val)
{
	outb(reg, vga_video_port_reg);
	outb(val, vga_video_port_val);
}

static inline void vga_set_mem_top(void)
{
	u32 offset;
	
	irq_disable();
	offset = (vga_visible_origin - vga_vram_base) >> 1;
	write_vga(VGA_CRTC_START_HI, offset >> 8);
	write_vga(VGA_CRTC_START_LO, offset);
	irq_enable();
}

static void vga_cursor(struct vc_struct *v)
{
	u32 offset;

	irq_disable();
	offset = (vga_pos - vga_vram_base) >> 1;
	write_vga(VGA_CRTC_CURSOR_HI, offset >> 8);
	write_vga(VGA_CRTC_CURSOR_LO, offset);
	irq_enable();
}

static void vgacon_putc(struct vc_strcut *vc, int ch, int y, int x)
{
}

static void vgacon_putcs(struct vc_struct *vc, const unsigned char *s,
			int count, int y, int x)
{
}

static void vgacon_clear(struct vc_struct *vc, int y, int x,
			int height, int width)
{

}

/*
 * Much of the dirty work seems should be done in boot process. For simplicity,
 * we assume that in mode 3, an EGA or a VGA+ is present. So the vram_size is
 * 0x8000. How to detect the type of video card? Please see linux/boot/video
 * for details.
 */
static void vgacon_startup(void)
{
	/* boot_params.screen_info initialized? */
	if ((screen_info.orig_video_mode  == 0) &&
	    (screen_info.orig_video_lines == 0) &&
	    (screen_info.orig_video_cols  == 0))
	    	return;

	switch(screen_info.orig_video_mode) {
	case 3:
		/* Color Display. EGA, VGA+ */
		vga_vram_base = 0xb8000;
		vga_vram_size = 0x8000;
		vga_video_port_reg = VGA_CRT_IC;
		vga_video_port_val = VGA_CRT_DC;
		break;
	case 7:
		/* Monochrome Display. MDA */
		vga_vram_base = 0xb0000;
		vga_vram_size = 0x2000;
		vga_video_port_reg = VGA_CRT_IM;
		vga_video_port_val = VGA_CRT_DM;
		break;
	default:
		return;
	}

	vga_vram_base = VGA_MEM_MAP(vga_vram_base);
	vga_vram_end = vga_vram_base + vga_vram_size;

	/* Back background. White foreground. No blink */
	vga_vram_attr = 0x7;
	
	vga_video_num_cols = screen_info.orig_video_cols;
	vga_video_num_rows = screen_info.orig_video_lines;
}

/* 
 * This functions is called everytime VC starts a new console. It is used to
 * initialize the new vc_struct. Note that, if it is the first time to call
 * vgacon_init, vgacon_startup must be called first.
 */
static void vgacon_init(struct vc_struct *vc)
{
	vc->vc_cols	= vga_video_num_cols;
	vc->vc_rows	= vga_video_num_rows;
	vc->vc_row_size	= vga_video_num_cols << 1;
	vc->vc_origin	= vga_vram_base;
	vc->vc_scr_end	= vga_vram_end;
	vc->vc_attr	= vga_vram_attr;
	vc->vc_visible_origin = vga_visible_origin;
}

static void vgacon_deinit(struct vc_struct *vc)
{
	return;
}

const struct con_driver vga_con = {
	.con_startup	=	vgacon_startup;
	.con_init	=	vgacon_init;
	.con_deinit	=	vgacon_deinit;
	.con_clear	=	vgacon_clear;
	.con_putc	=	vgacon_putc;
	.con_putcs	=	vgacon_putcs;
	.con_cursor	=	vgacon_cursor;
	.con_scroll	=	vgacon_scroll;
};
