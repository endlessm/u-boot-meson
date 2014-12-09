#ifndef PANEL_TYPICAL_TIMING_H
#define PANEL_TYPICAL_TIMING_H

//*****************************************
// Define LCD Typical Timing Parameters
//*****************************************
#define MODEL_NAME			"HLY080ML154"	/** lcd model name */
#define ACITVE_AREA_WIDTH	197	/** lcd active_area or display_area horizontal size(unit in mm, you can find it on the home page of lcd spec) */
#define ACITVE_AREA_HEIGHT	114	/** lcd active_area or display_area vertical size(unit in mm, you can find it on the home page of lcd spec) */
#define LCD_TYPE			LCD_DIGITAL_LVDS   /** lcd interface(LCD_DIGITAL_MIPI, LCD_DIGITAL_LVDS, LCD_DIGITAL_EDP, LCD_DIGITAL_TTL) */
#define LCD_BITS			8	/** lcd bits(6, 8) */
#define BITS_OPTION			0	/** bits_option(0=only support one mode as LCD_BITS define, 1=both support 6/8bit) */

#define H_ACTIVE			1024		/** horizontal resolution */
#define V_ACTIVE			600		/** vertical resolution */
#define H_PERIOD			1344		/** horizontal period(htotal) */
#define V_PERIOD			635		/** vertical period(vtotal)*/

#define	LCD_CLK				42900000	/** clock(unit in Hz, both support clk and frame_rate, >200 regard as clk, <200 regard as frame_rate) */
#define CLK_POL				0
#define HS_WIDTH			20
#define HS_BACK_PORCH		160	//include HS_WIDTH
#define HS_POL				0	//0: negative, 1: positive
#define VS_WIDTH			3
#define VS_BACK_PORCH		23	//include VS_WIDTH
#define VS_POL				0	//0: negative, 1: positive

//*****************************************
// recommend settings, don't modify them unless there is display problem
//*****************************************
#define TTL_H_OFFSET		0	//adjust ttl display h_offset
#define H_OFFSET_SIGN		1	//0: negative value, 1: positive value
#define TTL_V_OFFSET		0	//adjust ttl display v_offset
#define V_OFFSET_SIGN		1	//0: negative value, 1: positive value
#define VIDEO_ON_PIXEL		80
#define VIDEO_ON_LINE		32
#define VSYNC_H_ADJUST_SIGN 0
#define VSYNC_H_ADJUST 0

//************************************************

#endif
