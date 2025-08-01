/**
 * MIT License
 *
 * Copyright (c) 2018-2023 Mahyar Koshkouei
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to
 * deal in the Software without restriction, including without limitation the
 * rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
 * sell copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 * IN THE SOFTWARE.
 *
 * Please note that at least two parts of source code within this project was
 * taken from the SameBoy project at https://github.com/LIJI32/SameBoy/ which at
 * the time of this writing is released under the MIT License. Occurrences of
 * this code is marked as being taken from SameBoy with a comment.
 * SameBoy, and code marked as being taken from SameBoy,
 * is Copyright (c) 2015-2019 Lior Halphon.
 */

#ifndef PEANUT_GB_H
#define PEANUT_GB_H

#if defined(__has_include)
# if __has_include("version.all")
#  include "version.all"	/* Version information */
# endif
#else
/* Stub __has_include for later. */
# define __has_include(x) 0
#endif

#include <stdlib.h>	/* Required for abort */
#include <stdbool.h>	/* Required for bool types */
#include <stdint.h>	/* Required for int types */
#include <string.h>	/* Required for memset and memmove */
#include <time.h>	/* Required for tm struct */

/**
* If PEANUT_GB_IS_LITTLE_ENDIAN is positive, then Peanut-GB will be configured
* for a little endian platform. If 0, then big endian.
*/
#if !defined(PEANUT_GB_IS_LITTLE_ENDIAN)
/* If endian is not defined, then attempt to detect it. */
# if defined(__BYTE_ORDER__)
#  if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
/* Building for a big endian platform. */
#   define PEANUT_GB_IS_LITTLE_ENDIAN 0
#  else
#   define PEANUT_GB_IS_LITTLE_ENDIAN 1
#  endif /* __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__ */
# elif defined(_WIN32)
/* We assume that Windows is always little endian by default. */
#  define PEANUT_GB_IS_LITTLE_ENDIAN 1
# elif !defined(PEANUT_GB_IS_LITTLE_ENDIAN)
#  error "Could not detect target platform endian. Please define PEANUT_GB_IS_LITTLE_ENDIAN"
# endif
#endif /* !defined(PEANUT_GB_IS_LITTLE_ENDIAN) */

#if PEANUT_GB_IS_LITTLE_ENDIAN == 0
# error "Peanut-GB only supports little endian targets"
/* This is because the logic has been written with assumption of little
 * endian byte order. */
#endif

/** Definitions for compile-time setting of features. **/
/**
 * Sound support must be provided by an external library. When audio_read() and
 * audio_write() functions are provided, define ENABLE_SOUND to a non-zero value
 * before including peanut_gb.h in order for these functions to be used.
 */
#ifndef ENABLE_SOUND
# define ENABLE_SOUND 0
#endif

/* Enable LCD drawing. On by default. May be turned off for testing purposes. */
#ifndef ENABLE_LCD
# define ENABLE_LCD 1
#endif

/* Enable 16 bit colour palette. If disabled, only four colour shades are set in
 * pixel data. */
#ifndef PEANUT_GB_12_COLOUR
# define PEANUT_GB_12_COLOUR 1
#endif

/* Adds more code to improve LCD rendering accuracy. */
#ifndef PEANUT_GB_HIGH_LCD_ACCURACY
# define PEANUT_GB_HIGH_LCD_ACCURACY 1
#endif

/* Use intrinsic functions. This may produce smaller and faster code. */
#ifndef PEANUT_GB_USE_INTRINSICS
# define PEANUT_GB_USE_INTRINSICS 1
#endif

/* Only include function prototypes. At least one file must *not* have this
 * defined. */
// #define PEANUT_GB_HEADER_ONLY

/** Internal source code. **/
/* Interrupt masks */
#define VBLANK_INTR	0x01
#define LCDC_INTR	0x02
#define TIMER_INTR	0x04
#define SERIAL_INTR	0x08
#define CONTROL_INTR	0x10
#define ANY_INTR	0x1F

/* Memory section sizes for DMG */
#define WRAM_SIZE	0x2000
#define VRAM_SIZE	0x2000
#define HRAM_IO_SIZE	0x0100
#define OAM_SIZE	0x00A0

/* Memory addresses */
#define ROM_0_ADDR      0x0000
#define ROM_N_ADDR      0x4000
#define VRAM_ADDR       0x8000
#define CART_RAM_ADDR   0xA000
#define WRAM_0_ADDR     0xC000
#define WRAM_1_ADDR     0xD000
#define ECHO_ADDR       0xE000
#define OAM_ADDR        0xFE00
#define UNUSED_ADDR     0xFEA0
#define IO_ADDR         0xFF00
#define HRAM_ADDR       0xFF80
#define INTR_EN_ADDR    0xFFFF

/* Cart section sizes */
#define ROM_BANK_SIZE   0x4000
#define WRAM_BANK_SIZE  0x1000
#define CRAM_BANK_SIZE  0x2000
#define VRAM_BANK_SIZE  0x2000

/* DIV Register is incremented at rate of 16384Hz.
 * 4194304 / 16384 = 256 clock cycles for one increment. */
#define DIV_CYCLES          256

/* Serial clock locked to 8192Hz on DMG.
 * 4194304 / (8192 / 8) = 4096 clock cycles for sending 1 byte. */
#define SERIAL_CYCLES       4096

/* Calculating VSYNC. */
#define DMG_CLOCK_FREQ      4194304.0
#define SCREEN_REFRESH_CYCLES 70224.0
#define VERTICAL_SYNC       (DMG_CLOCK_FREQ/SCREEN_REFRESH_CYCLES)

/* Real Time Clock is locked to 1Hz. */
#define RTC_CYCLES	((uint_fast32_t)DMG_CLOCK_FREQ)

/* SERIAL SC register masks. */
#define SERIAL_SC_TX_START  0x80
#define SERIAL_SC_CLOCK_SRC 0x01

/* STAT register masks */
#define STAT_LYC_INTR       0x40
#define STAT_MODE_2_INTR    0x20
#define STAT_MODE_1_INTR    0x10
#define STAT_MODE_0_INTR    0x08
#define STAT_LYC_COINC      0x04
#define STAT_MODE           0x03
#define STAT_USER_BITS      0xF8

/* LCDC control masks */
#define LCDC_ENABLE         0x80
#define LCDC_WINDOW_MAP     0x40
#define LCDC_WINDOW_ENABLE  0x20
#define LCDC_TILE_SELECT    0x10
#define LCDC_BG_MAP         0x08
#define LCDC_OBJ_SIZE       0x04
#define LCDC_OBJ_ENABLE     0x02
#define LCDC_BG_ENABLE      0x01

/** LCD characteristics **/
/* There are 154 scanlines. LY < 154. */
#define LCD_VERT_LINES      154
#define LCD_WIDTH           160
#define LCD_HEIGHT          144
/* PPU cycles through modes every 456 cycles. */
#define LCD_LINE_CYCLES     456
#define LCD_MODE0_HBLANK_MAX_DRUATION	204
#define LCD_MODE0_HBLANK_MIN_DRUATION	87
#define LCD_MODE2_OAM_SCAN_DURATION	80
#define LCD_MODE3_LCD_DRAW_MIN_DURATION	172
#define LCD_MODE3_LCD_DRAW_MAX_DURATION	289
#define LCD_MODE1_VBLANK_DURATION	(LCD_LINE_CYCLES * (LCD_VERT_LINES - LCD_HEIGHT))
#define LCD_FRAME_CYCLES		(LCD_LINE_CYCLES * LCD_VERT_LINES)
/* The following assumes that Hblank starts on cycle 0. */
/* Mode 2 (OAM Scan) starts on cycle 204 (although this is dependent on the
 * duration of Mode 3 (LCD Draw). */
#define LCD_MODE_2_CYCLES   LCD_MODE0_HBLANK_MAX_DRUATION
/* Mode 3 starts on cycle 284. */
#define LCD_MODE_3_CYCLES   (LCD_MODE_2_CYCLES + LCD_MODE2_OAM_SCAN_DURATION)
/* Mode 0 starts on cycle 376. */
#define LCD_MODE_0_CYCLES   (LCD_MODE_3_CYCLES + LCD_MODE3_LCD_DRAW_MIN_DURATION)

#define LCD_MODE2_OAM_SCAN_START	0
#define LCD_MODE2_OAM_SCAN_END		(LCD_MODE2_OAM_SCAN_DURATION)
#define LCD_MODE3_LCD_DRAW_END		(LCD_MODE2_OAM_SCAN_END + LCD_MODE3_LCD_DRAW_MIN_DURATION)
#define LCD_MODE0_HBLANK_END		(LCD_MODE3_LCD_DRAW_END + LCD_MODE0_HBLANK_MAX_DRUATION)
#if LCD_MODE0_HBLANK_END != LCD_LINE_CYCLES
#error "LCD length not equal"
#endif

/* VRAM Locations */
#define VRAM_TILES_1        (0x8000 - VRAM_ADDR)
#define VRAM_TILES_2        (0x8800 - VRAM_ADDR)
#define VRAM_BMAP_1         (0x9800 - VRAM_ADDR)
#define VRAM_BMAP_2         (0x9C00 - VRAM_ADDR)
#define VRAM_TILES_3        (0x8000 - VRAM_ADDR + VRAM_BANK_SIZE)
#define VRAM_TILES_4        (0x8800 - VRAM_ADDR + VRAM_BANK_SIZE)

/* Interrupt jump addresses */
#define VBLANK_INTR_ADDR    0x0040
#define LCDC_INTR_ADDR      0x0048
#define TIMER_INTR_ADDR     0x0050
#define SERIAL_INTR_ADDR    0x0058
#define CONTROL_INTR_ADDR   0x0060

/* SPRITE controls */
#define NUM_SPRITES         0x28
#define MAX_SPRITES_LINE    0x0A
#define OBJ_PRIORITY        0x80
#define OBJ_FLIP_Y          0x40
#define OBJ_FLIP_X          0x20
#define OBJ_PALETTE         0x10

/* Joypad buttons */
#define JOYPAD_A            0x01
#define JOYPAD_B            0x02
#define JOYPAD_SELECT       0x04
#define JOYPAD_START        0x08
#define JOYPAD_RIGHT        0x10
#define JOYPAD_LEFT         0x20
#define JOYPAD_UP           0x40
#define JOYPAD_DOWN         0x80

#define ROM_HEADER_CHECKSUM_LOC	0x014D

/* Local macros. */
#ifndef MIN
# define MIN(a, b)          ((a) < (b) ? (a) : (b))
#endif

#define PEANUT_GB_ARRAYSIZE(array)    (sizeof(array)/sizeof(array[0]))

/** Allow setting deprecated functions and variables. */
#if (defined(__GNUC__) && __GNUC__ >= 6) || (defined(__clang__) && __clang_major__ >= 4)
# define PGB_DEPRECATED(msg) __attribute__((deprecated(msg)))
#else
# define PGB_DEPRECATED(msg)
#endif

#if !defined(__has_builtin)
/* Stub __has_builtin if it isn't available. */
# define __has_builtin(x) 0
#endif

/* The PGB_UNREACHABLE() macro tells the compiler that the code path will never
 * be reached, allowing for further optimisation. */
#if !defined(PGB_UNREACHABLE)
# if __has_builtin(__builtin_unreachable)
#  define PGB_UNREACHABLE() __builtin_unreachable()
# elif defined(_MSC_VER) && _MSC_VER >= 1200
#  /* __assume is not available before VC6. */
#  define PGB_UNREACHABLE() __assume(0)
# else
#  define PGB_UNREACHABLE() abort()
# endif
#endif /* !defined(PGB_UNREACHABLE) */

#if !defined(PGB_UNLIKELY)
# if __has_builtin(__builtin_expect)
#  define PGB_UNLIKELY(expr) __builtin_expect(!!(expr), 0)
# else
#  define PGB_UNLIKELY(expr) (expr)
# endif
#endif /* !defined(PGB_UNLIKELY) */
#if !defined(PGB_LIKELY)
# if __has_builtin(__builtin_expect)
#  define PGB_LIKELY(expr) __builtin_expect(!!(expr), 1)
# else
#  define PGB_LIKELY(expr) (expr)
# endif
#endif /* !defined(PGB_LIKELY) */

#if PEANUT_GB_USE_INTRINSICS
/* If using MSVC, only enable intrinsics for x86 platforms*/
# if defined(_MSC_VER) && __has_include("intrin.h") && \
	(defined(_M_IX86_FP) || defined(_M_AMD64) || defined(_M_X64))
/* Define intrinsic functions for MSVC. */
#  include <intrin.h>
#  define PGB_INTRIN_SBC(x,y,cin,res) _subborrow_u8(cin,x,y,&res)
#  define PGB_INTRIN_ADC(x,y,cin,res) _addcarry_u8(cin,x,y,&res)
# endif /* MSVC */

/* Check for intrinsic functions in GCC and Clang. */
# if __has_builtin(__builtin_sub_overflow)
#  define PGB_INTRIN_SBC(x,y,cin,res) __builtin_sub_overflow(x,y+cin,&res)
#  define PGB_INTRIN_ADC(x,y,cin,res) __builtin_add_overflow(x,y+cin,&res)
# endif
#endif /* PEANUT_GB_USE_INTRINSICS */

#if defined(PGB_INTRIN_SBC)
# define PGB_INSTR_SBC_R8(r,cin)						\
	{									\
		uint8_t temp;							\
		gb->cpu_reg.f.f_bits.c = PGB_INTRIN_SBC(gb->cpu_reg.a,r,cin,temp);\
		gb->cpu_reg.f.f_bits.h = ((gb->cpu_reg.a ^ r ^ temp) & 0x10) > 0;\
		gb->cpu_reg.f.f_bits.n = 1;					\
		gb->cpu_reg.f.f_bits.z = (temp == 0x00);			\
		gb->cpu_reg.a = temp;						\
	}

# define PGB_INSTR_CP_R8(r)							\
	{									\
		uint8_t temp;							\
		gb->cpu_reg.f.f_bits.c = PGB_INTRIN_SBC(gb->cpu_reg.a,r,0,temp);\
		gb->cpu_reg.f.f_bits.h = ((gb->cpu_reg.a ^ r ^ temp) & 0x10) > 0;\
		gb->cpu_reg.f.f_bits.n = 1;					\
		gb->cpu_reg.f.f_bits.z = (temp == 0x00);			\
	}
#else
# define PGB_INSTR_SBC_R8(r,cin)						\
	{									\
		uint16_t temp = gb->cpu_reg.a - (r + cin);			\
		gb->cpu_reg.f.f_bits.c = (temp & 0xFF00) ? 1 : 0;		\
		gb->cpu_reg.f.f_bits.h = ((gb->cpu_reg.a ^ r ^ temp) & 0x10) > 0; \
		gb->cpu_reg.f.f_bits.n = 1;					\
		gb->cpu_reg.f.f_bits.z = ((temp & 0xFF) == 0x00);		\
		gb->cpu_reg.a = (temp & 0xFF);					\
	}

# define PGB_INSTR_CP_R8(r)							\
	{									\
		uint16_t temp = gb->cpu_reg.a - r;				\
		gb->cpu_reg.f.f_bits.c = (temp & 0xFF00) ? 1 : 0;		\
		gb->cpu_reg.f.f_bits.h = ((gb->cpu_reg.a ^ r ^ temp) & 0x10) > 0; \
		gb->cpu_reg.f.f_bits.n = 1;					\
		gb->cpu_reg.f.f_bits.z = ((temp & 0xFF) == 0x00);		\
	}
#endif  /* PGB_INTRIN_SBC */

#if defined(PGB_INTRIN_ADC)
# define PGB_INSTR_ADC_R8(r,cin)						\
	{									\
		uint8_t temp;							\
		gb->cpu_reg.f.f_bits.c = PGB_INTRIN_ADC(gb->cpu_reg.a,r,cin,temp);\
		gb->cpu_reg.f.f_bits.h = ((gb->cpu_reg.a ^ r ^ temp) & 0x10) > 0; \
		gb->cpu_reg.f.f_bits.n = 0;					\
		gb->cpu_reg.f.f_bits.z = (temp == 0x00);			\
		gb->cpu_reg.a = temp;						\
	}
#else
# define PGB_INSTR_ADC_R8(r,cin)						\
	{									\
		uint16_t temp = gb->cpu_reg.a + r + cin;			\
		gb->cpu_reg.f.f_bits.c = (temp & 0xFF00) ? 1 : 0;		\
		gb->cpu_reg.f.f_bits.h = ((gb->cpu_reg.a ^ r ^ temp) & 0x10) > 0; \
		gb->cpu_reg.f.f_bits.n = 0;					\
		gb->cpu_reg.f.f_bits.z = ((temp & 0xFF) == 0x00);		\
		gb->cpu_reg.a = (temp & 0xFF);					\
	}
#endif /* PGB_INTRIN_ADC */

#define PGB_INSTR_INC_R8(r)							\
	r++;									\
	gb->cpu_reg.f.f_bits.h = ((r & 0x0F) == 0x00);				\
	gb->cpu_reg.f.f_bits.n = 0;						\
	gb->cpu_reg.f.f_bits.z = (r == 0x00)

#define PGB_INSTR_DEC_R8(r)							\
	r--;									\
	gb->cpu_reg.f.f_bits.h = ((r & 0x0F) == 0x0F);				\
	gb->cpu_reg.f.f_bits.n = 1;						\
	gb->cpu_reg.f.f_bits.z = (r == 0x00)

#define PGB_INSTR_XOR_R8(r)							\
	gb->cpu_reg.a ^= r;							\
	gb->cpu_reg.f.reg = 0;							\
	gb->cpu_reg.f.f_bits.z = (gb->cpu_reg.a == 0x00)

#define PGB_INSTR_OR_R8(r)							\
	gb->cpu_reg.a |= r;							\
        gb->cpu_reg.f.reg = 0;							\
	gb->cpu_reg.f.f_bits.z = (gb->cpu_reg.a == 0x00)

#define PGB_INSTR_AND_R8(r)							\
	gb->cpu_reg.a &= r;							\
	gb->cpu_reg.f.reg = 0;							\
	gb->cpu_reg.f.f_bits.z = (gb->cpu_reg.a == 0x00);			\
	gb->cpu_reg.f.f_bits.h = 1

#if PEANUT_GB_IS_LITTLE_ENDIAN
# define PEANUT_GB_GET_LSB16(x) (x & 0xFF)
# define PEANUT_GB_GET_MSB16(x) (x >> 8)
# define PEANUT_GB_GET_MSN16(x) (x >> 12)
# define PEANUT_GB_U8_TO_U16(h,l) ((l) | ((h) << 8))
#else
# define PEANUT_GB_GET_LSB16(x) (x >> 8)
# define PEANUT_GB_GET_MSB16(x) (x & 0xFF)
# define PEANUT_GB_GET_MSN16(x) ((x & 0xF0) >> 4)
# define PEANUT_GB_U8_TO_U16(h,l) ((h) | ((l) << 8))
#endif

struct cpu_registers_s
{
/* Change register order if big endian.
 * Macro receives registers in little endian order. */
#if PEANUT_GB_IS_LITTLE_ENDIAN
# define PEANUT_GB_LE_REG(x,y) x,y
#else
# define PEANUT_GB_LE_REG(x,y) y,x
#endif
	/* Define specific bits of Flag register. */
	union {
		struct {
			uint8_t  : 4; /* Unused. */
			uint8_t c: 1; /* Carry flag. */
			uint8_t h: 1; /* Half carry flag. */
			uint8_t n: 1; /* Add/sub flag. */
			uint8_t z: 1; /* Zero flag. */
		} f_bits;
		uint8_t reg;
	} f;
	uint8_t a;

	union
	{
		struct
		{
			uint8_t PEANUT_GB_LE_REG(c,b);
		} bytes;
		uint16_t reg;
	} bc;

	union
	{
		struct
		{
			uint8_t PEANUT_GB_LE_REG(e,d);
		} bytes;
		uint16_t reg;
	} de;

	union
	{
		struct
		{
			uint8_t PEANUT_GB_LE_REG(l,h);
		} bytes;
		uint16_t reg;
	} hl;

	/* Stack pointer */
	union
	{
		struct
		{
			uint8_t PEANUT_GB_LE_REG(p, s);
		} bytes;
		uint16_t reg;
	} sp;

	/* Program counter */
	union
	{
		struct
		{
			uint8_t PEANUT_GB_LE_REG(c, p);
		} bytes;
		uint16_t reg;
	} pc;
#undef PEANUT_GB_LE_REG
};

struct count_s
{
	uint_fast16_t lcd_count;	/* LCD Timing */
	uint_fast16_t div_count;	/* Divider Register Counter */
	uint_fast16_t tima_count;	/* Timer Counter */
	uint_fast16_t serial_count;	/* Serial Counter */
	uint_fast32_t rtc_count;	/* RTC Counter */
	uint_fast32_t lcd_off_count;	/* Cycles LCD has been disabled */
};

#if ENABLE_LCD
	/* Bit mask for the shade of pixel to display */
	#define LCD_COLOUR	0x03

# if PEANUT_GB_12_COLOUR
	/**
	* Bit mask for whether a pixel is OBJ0, OBJ1, or BG. Each may have a different
	* palette when playing a DMG game on CGB.
	*/
	#define LCD_PALETTE_OBJ	0x10
	#define LCD_PALETTE_BG	0x20
	/**
	* Bit mask for the two bits listed above.
	* LCD_PALETTE_ALL == 0b00 --> OBJ0
	* LCD_PALETTE_ALL == 0b01 --> OBJ1
	* LCD_PALETTE_ALL == 0b10 --> BG
	* LCD_PALETTE_ALL == 0b11 --> NOT POSSIBLE
	*/
	#define LCD_PALETTE_ALL 0x30
# endif
#endif

/**
 * Errors that may occur during emulation.
 */
enum gb_error_e
{
	GB_UNKNOWN_ERROR = 0,
	GB_INVALID_OPCODE = 1,
	GB_INVALID_READ = 2,
	GB_INVALID_WRITE = 3,

	/* GB_HALT_FOREVER is deprecated and will no longer be issued as an
	 * error by Peanut-GB. */
	GB_HALT_FOREVER PGB_DEPRECATED("Error no longer issued by Peanut-GB") = 4,

	GB_INVALID_MAX = 5
};

/**
 * Errors that may occur during library initialisation.
 */
enum gb_init_error_e
{
	GB_INIT_NO_ERROR = 0,
	GB_INIT_CARTRIDGE_UNSUPPORTED,
	GB_INIT_INVALID_CHECKSUM,

	GB_INIT_INVALID_MAX
};

/**
 * Return codes for serial receive function, mainly for clarity.
 */
enum gb_serial_rx_ret_e
{
	GB_SERIAL_RX_SUCCESS = 0,
	GB_SERIAL_RX_NO_CONNECTION = 1
};

union cart_rtc
{
	struct
	{
		uint8_t sec;
		uint8_t min;
		uint8_t hour;
		uint8_t yday;
		uint8_t high;
	} reg;
	uint8_t bytes[5];
};

/**
 * Emulator context.
 *
 * Only values within the `direct` struct may be modified directly by the
 * front-end implementation. Other variables must not be modified.
 */
struct gb_s
{
	/**
	 * Return byte from ROM at given address.
	 *
	 * \param gb_s	emulator context
	 * \param addr	address
	 * \return		byte at address in ROM
	 */
	uint8_t (*gb_rom_read)(struct gb_s*, const uint_fast32_t addr);

	/**
	 * Return byte from cart RAM at given address.
	 *
	 * \param gb_s	emulator context
	 * \param addr	address
	 * \return		byte at address in RAM
	 */
	uint8_t (*gb_cart_ram_read)(struct gb_s*, const uint_fast32_t addr);

	/**
	 * Write byte to cart RAM at given address.
	 *
	 * \param gb_s	emulator context
	 * \param addr	address
	 * \param val	value to write to address in RAM
	 */
	void (*gb_cart_ram_write)(struct gb_s*, const uint_fast32_t addr,
				  const uint8_t val);

	/**
	 * Notify front-end of error.
	 *
	 * \param gb_s		emulator context
	 * \param gb_error_e	error code
	 * \param addr		address of where error occurred
	 */
	void (*gb_error)(struct gb_s*, const enum gb_error_e, const uint16_t addr);

	/* Transmit one byte and return the received byte. */
	void (*gb_serial_tx)(struct gb_s*, const uint8_t tx);
	enum gb_serial_rx_ret_e (*gb_serial_rx)(struct gb_s*, uint8_t* rx);

	/* Read byte from boot ROM at given address. */
	uint8_t (*gb_bootrom_read)(struct gb_s*, const uint_fast16_t addr);

	struct
	{
		bool gb_halt	: 1;
		bool gb_ime	: 1;
		/* gb_frame is set when 0.016742706298828125 seconds have
		 * passed. It is likely that a new frame has been drawn since
		 * then, but it is possible that the LCD was switched off and
		 * nothing was drawn. */
		bool gb_frame	: 1;
		bool lcd_blank	: 1;
		/* Set if MBC3O cart is used. */
		bool cart_is_mbc3O : 1;
	};

	/* Cartridge information:
	 * Memory Bank Controller (MBC) type. */
	int8_t mbc;
	/* Whether the MBC has internal RAM. */
	uint8_t cart_ram;
	/* Number of ROM banks in cartridge. */
	uint16_t num_rom_banks_mask;
	/* Number of RAM banks in cartridge. Ignore for MBC2. */
	uint8_t num_ram_banks;

	uint16_t selected_rom_bank;
	/* WRAM and VRAM bank selection not available. */
	uint8_t cart_ram_bank;
	uint8_t enable_cart_ram;
	/* Cartridge ROM/RAM mode select. */
	uint8_t cart_mode_select;

	union cart_rtc rtc_latched, rtc_real;

	struct cpu_registers_s cpu_reg;
	//struct gb_registers_s gb_reg;
	struct count_s counter;

	/* TODO: Allow implementation to allocate WRAM, VRAM and Frame Buffer. */
	uint8_t wram[WRAM_SIZE];
	uint8_t vram[VRAM_SIZE];
	uint8_t oam[OAM_SIZE];
	uint8_t hram_io[HRAM_IO_SIZE];

	struct
	{
		/**
		 * Draw line on screen.
		 *
		 * \param gb_s		emulator context
		 * \param pixels	The 160 pixels to draw.
		 * 			Bits 1-0 are the colour to draw.
		 * 			Bits 5-4 are the palette, where:
		 * 				OBJ0 = 0b00,
		 * 				OBJ1 = 0b01,
		 * 				BG = 0b10
		 * 			Other bits are undefined.
		 * 			Bits 5-4 are only required by front-ends
		 * 			which want to use a different colour for
		 * 			different object palettes. This is what
		 * 			the Game Boy Color (CGB) does to DMG
		 * 			games.
		 * \param line		Line to draw pixels on. This is
		 * guaranteed to be between 0-144 inclusive.
		 */
		void (*lcd_draw_line)(struct gb_s *gb,
				const uint8_t *pixels,
				const uint_fast8_t line);

		/* Palettes */
		uint8_t bg_palette[4];
		uint8_t sp_palette[8];

		uint8_t window_clear;
		uint8_t WY;

		/* Only support 30fps frame skip. */
		bool frame_skip_count : 1;
		bool interlace_count : 1;
	} display;

	/**
	 * Variables that may be modified directly by the front-end.
	 * This method seems to be easier and possibly less overhead than
	 * calling a function to modify these variables each time.
	 *
	 * None of this is thread-safe.
	 */
	struct
	{
		/* Set to enable interlacing. Interlacing will start immediately
		 * (at the next line drawing).
		 */
		bool interlace : 1;
		bool frame_skip : 1;

		union
		{
			struct
			{
				/* Using this bitfield is deprecated due to
				 * portability concerns. It is recommended to
				 * use the JOYPAD_* defines instead.
				 */
				bool a		: 1;
				bool b		: 1;
				bool select	: 1;
				bool start	: 1;
				bool right	: 1;
				bool left	: 1;
				bool up		: 1;
				bool down	: 1;
			} joypad_bits;
			uint8_t joypad;
		};

		/* Implementation defined data. Set to NULL if not required. */
		void *priv;
	} direct;
};

#ifndef PEANUT_GB_HEADER_ONLY

#define IO_JOYP	0x00
#define IO_SB	0x01
#define IO_SC	0x02
#define IO_DIV	0x04
#define IO_TIMA	0x05
#define IO_TMA	0x06
#define IO_TAC	0x07
#define IO_IF	0x0F
#define IO_LCDC	0x40
#define IO_STAT	0x41
#define IO_SCY	0x42
#define IO_SCX	0x43
#define IO_LY	0x44
#define IO_LYC	0x45
#define	IO_DMA	0x46
#define	IO_BGP	0x47
#define	IO_OBP0	0x48
#define IO_OBP1	0x49
#define IO_WY	0x4A
#define IO_WX	0x4B
#define IO_BOOT	0x50
#define IO_IE	0xFF

#define IO_TAC_RATE_MASK	0x3
#define IO_TAC_ENABLE_MASK	0x4

/* LCD Mode defines. */
#define IO_STAT_MODE_HBLANK		0
#define IO_STAT_MODE_VBLANK		1
#define IO_STAT_MODE_OAM_SCAN		2
#define IO_STAT_MODE_LCD_DRAW		3
#define IO_STAT_MODE_VBLANK_OR_TRANSFER_MASK 0x1

/**
 * Internal function used to read bytes.
 * addr is host platform endian.
 */
uint8_t __gb_read(struct gb_s *gb, uint16_t addr)
{
	switch(PEANUT_GB_GET_MSN16(addr))
	{
	case 0x0:
		/* IO_BOOT is only set to 1 if gb->gb_bootrom_read was not NULL
		 * on reset. */
		if(gb->hram_io[IO_BOOT] == 0 && addr < 0x0100)
		{
			return gb->gb_bootrom_read(gb, addr);
		}

		/* Fallthrough */
	case 0x1:
	case 0x2:
	case 0x3:
		return gb->gb_rom_read(gb, addr);

	case 0x4:
	case 0x5:
	case 0x6:
	case 0x7:
		if(gb->mbc == 1 && gb->cart_mode_select)
			return gb->gb_rom_read(gb,
					       addr + ((gb->selected_rom_bank & 0x1F) - 1) * ROM_BANK_SIZE);
		else
			return gb->gb_rom_read(gb, addr + (gb->selected_rom_bank - 1) * ROM_BANK_SIZE);

	case 0x8:
	case 0x9:
		return gb->vram[addr - VRAM_ADDR];

	case 0xA:
	case 0xB:
		if(gb->mbc == 3 && gb->cart_ram_bank >= 0x08)
		{
			return gb->rtc_latched.bytes[gb->cart_ram_bank - 0x08];
		}
		else if(gb->cart_ram && gb->enable_cart_ram)
		{
			if(gb->mbc == 2)
			{
				/* Only 9 bits are available in address. */
				addr &= 0x1FF;
				return gb->gb_cart_ram_read(gb, addr);
			}
			else if((gb->cart_mode_select || gb->mbc != 1) &&
					gb->cart_ram_bank < gb->num_ram_banks)
			{
				return gb->gb_cart_ram_read(gb, addr - CART_RAM_ADDR +
							    (gb->cart_ram_bank * CRAM_BANK_SIZE));
			}
			else
				return gb->gb_cart_ram_read(gb, addr - CART_RAM_ADDR);
		}

		return 0xFF;

	case 0xC:
	case 0xD:
		return gb->wram[addr - WRAM_0_ADDR];

	case 0xE:
		return gb->wram[addr - ECHO_ADDR];

	case 0xF:
		if(addr < OAM_ADDR)
			return gb->wram[addr - ECHO_ADDR];

		if(addr < UNUSED_ADDR)
			return gb->oam[addr - OAM_ADDR];

		/* Unusable memory area. Reading from this area returns 0xFF.*/
		if(addr < IO_ADDR)
			return 0xFF;

		/* APU registers. */
		if((addr >= 0xFF10) && (addr <= 0xFF3F))
		{
#if ENABLE_SOUND
			return audio_read(addr);
#else
			static const uint8_t ortab[] = {
				0x80, 0x3f, 0x00, 0xff, 0xbf,
				0xff, 0x3f, 0x00, 0xff, 0xbf,
				0x7f, 0xff, 0x9f, 0xff, 0xbf,
				0xff, 0xff, 0x00, 0x00, 0xbf,
				0x00, 0x00, 0x70,
				0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff, 0xff,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
				0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
			};
			return gb->hram_io[addr - IO_ADDR] | ortab[addr - IO_ADDR];
#endif
		}

		/* HRAM */
		if(addr >= IO_ADDR)
			return gb->hram_io[addr - IO_ADDR];
	}


	/* Return address that caused read error. */
	(gb->gb_error)(gb, GB_INVALID_READ, addr);
	PGB_UNREACHABLE();
}

/**
 * Internal function used to write bytes.
 */
void __gb_write(struct gb_s *gb, uint_fast16_t addr, uint8_t val)
{
	switch(PEANUT_GB_GET_MSN16(addr))
	{
	case 0x0:
	case 0x1:
		/* Set RAM enable bit. MBC2 is handled in fall-through. */
		if(gb->mbc > 0 && gb->mbc != 2 && gb->cart_ram)
		{
			gb->enable_cart_ram = ((val & 0x0F) == 0x0A);
			return;
		}

	/* Intentional fall through. */
	case 0x2:
		if(gb->mbc == 5)
		{
			gb->selected_rom_bank = (gb->selected_rom_bank & 0x100) | val;
			gb->selected_rom_bank =
				gb->selected_rom_bank & gb->num_rom_banks_mask;
			return;
		}

	/* Intentional fall through. */
	case 0x3:
		if(gb->mbc == 1)
		{
			//selected_rom_bank = val & 0x7;
			gb->selected_rom_bank = (val & 0x1F) | (gb->selected_rom_bank & 0x60);

			if((gb->selected_rom_bank & 0x1F) == 0x00)
				gb->selected_rom_bank++;
		}
		else if(gb->mbc == 2)
		{
			/* If bit 8 is 1, then set ROM bank number. */
			if(addr & 0x100)
			{
				gb->selected_rom_bank = val & 0x0F;
				/* Setting ROM bank to 0, sets it to 1. */
				if(!gb->selected_rom_bank)
					gb->selected_rom_bank++;
			}
			/* Otherwise set whether RAM is enabled or not. */
			else
			{
				gb->enable_cart_ram = ((val & 0x0F) == 0x0A);
				return;
			}
		}
		else if(gb->mbc == 3)
		{
			gb->selected_rom_bank = val;
			if(!gb->cart_is_mbc3O)
				gb->selected_rom_bank = val & 0x7F;

			if(!gb->selected_rom_bank)
				gb->selected_rom_bank++;
		}
		else if(gb->mbc == 5)
			gb->selected_rom_bank = (val & 0x01) << 8 | (gb->selected_rom_bank & 0xFF);

		gb->selected_rom_bank = gb->selected_rom_bank & gb->num_rom_banks_mask;
		return;

	case 0x4:
	case 0x5:
		if(gb->mbc == 1)
		{
			gb->cart_ram_bank = (val & 3);
			gb->selected_rom_bank = ((val & 3) << 5) | (gb->selected_rom_bank & 0x1F);
			gb->selected_rom_bank = gb->selected_rom_bank & gb->num_rom_banks_mask;
		}
		else if(gb->mbc == 3)
		{
			gb->cart_ram_bank = val;
			/* If not using MBC3, only the first 4 cart RAM banks are useable.
			 * If cart RAM bank 0x8-0xC are selected, then the corresponding
			 * RTC register is selected instead of cart RAM. */
			if(!gb->cart_is_mbc3O && gb->cart_ram_bank < 0x8)
				gb->cart_ram_bank &= 0x3;
		}

		else if(gb->mbc == 5)
			gb->cart_ram_bank = (val & 0x0F);

		return;

	case 0x6:
	case 0x7:
		val &= 1;
		if(gb->mbc == 3 && val && gb->cart_mode_select == 0)
			memcpy(&gb->rtc_latched.bytes, &gb->rtc_real.bytes, sizeof(gb->rtc_latched.bytes));

		/* Set banking mode select. */
		gb->cart_mode_select = val;
		return;

	case 0x8:
	case 0x9:
		gb->vram[addr - VRAM_ADDR] = val;
		return;

	case 0xA:
	case 0xB:
		if(gb->mbc == 3 && gb->cart_ram_bank >= 0x08)
		{
			const uint8_t rtc_reg_mask[5] = {
				0x3F, 0x3F, 0x1F, 0xFF, 0xC1
			};
			uint8_t reg = gb->cart_ram_bank - 0x08;
			//if(reg == 0) gb->counter.rtc_count = 0;

			gb->rtc_real.bytes[reg] = val & rtc_reg_mask[reg];
		}
		/* Do not write to RAM if unavailable or disabled. */
		else if(gb->cart_ram && gb->enable_cart_ram)
		{
			if(gb->mbc == 2)
			{
				/* Only 9 bits are available in address. */
				addr &= 0x1FF;
				/* Data is only 4 bits wide in MBC2 RAM. */
				val &= 0x0F;
				/* Upper nibble is set to high. */
				val |= 0xF0;
				gb->gb_cart_ram_write(gb, addr, val);
			}
			/* If cart has RAM, use this. If MBC1, only the first
			 * RAM bank can be written to if the advanced banking
			 * mode is selected. */
			else if(((gb->mbc == 1 && gb->cart_mode_select) || gb->mbc != 1) &&
					gb->cart_ram_bank < gb->num_ram_banks)
			{
				gb->gb_cart_ram_write(gb,
					addr - CART_RAM_ADDR + (gb->cart_ram_bank * CRAM_BANK_SIZE), val);
			}
			else if(gb->num_ram_banks)
				gb->gb_cart_ram_write(gb, addr - CART_RAM_ADDR, val);
		}

		return;

	case 0xC:
		gb->wram[addr - WRAM_0_ADDR] = val;
		return;

	case 0xD:
		gb->wram[addr - WRAM_1_ADDR + WRAM_BANK_SIZE] = val;
		return;

	case 0xE:
		gb->wram[addr - ECHO_ADDR] = val;
		return;

	case 0xF:
		if(addr < OAM_ADDR)
		{
			gb->wram[addr - ECHO_ADDR] = val;
			return;
		}

		if(addr < UNUSED_ADDR)
		{
			gb->oam[addr - OAM_ADDR] = val;
			return;
		}

		/* Unusable memory area. */
		if(addr < IO_ADDR)
			return;

		if(HRAM_ADDR <= addr && addr < INTR_EN_ADDR)
		{
			gb->hram_io[addr - IO_ADDR] = val;
			return;
		}

		if((addr >= 0xFF10) && (addr <= 0xFF3F))
		{
#if ENABLE_SOUND
			audio_write(addr, val);
#else
			gb->hram_io[addr - IO_ADDR] = val;
#endif
			return;
		}

		/* IO and Interrupts. */
		switch(PEANUT_GB_GET_LSB16(addr))
		{
		/* Joypad */
		case 0x00:
			/* Only bits 5 and 4 are R/W.
			 * The lower bits are overwritten later, and the two most
			 * significant bits are unused. */
			gb->hram_io[IO_JOYP] = val;

			/* Direction keys selected */
			if((gb->hram_io[IO_JOYP] & 0x10) == 0)
				gb->hram_io[IO_JOYP] |= (gb->direct.joypad >> 4);
			/* Button keys selected */
			else
				gb->hram_io[IO_JOYP] |= (gb->direct.joypad & 0x0F);

			return;

		/* Serial */
		case 0x01:
			gb->hram_io[IO_SB] = val;
			return;

		case 0x02:
			gb->hram_io[IO_SC] = val;
			return;

		/* Timer Registers */
		case 0x04:
			gb->hram_io[IO_DIV] = 0x00;
			return;

		case 0x05:
			gb->hram_io[IO_TIMA] = val;
			return;

		case 0x06:
			gb->hram_io[IO_TMA] = val;
			return;

		case 0x07:
			gb->hram_io[IO_TAC] = val;
			return;

		/* Interrupt Flag Register */
		case 0x0F:
			gb->hram_io[IO_IF] = (val | 0xE0);
			return;

		/* LCD Registers */
		case 0x40:
		{
			uint8_t lcd_enabled;

			/* Check if LCD is already enabled. */
			lcd_enabled = (gb->hram_io[IO_LCDC] & LCDC_ENABLE);

			gb->hram_io[IO_LCDC] = val;

			/* Check if LCD is going to be switched on. */
			if (!lcd_enabled && (val & LCDC_ENABLE))
			{
				gb->lcd_blank = true;
			}
			/* Check if LCD is being switched off. */
			else if (lcd_enabled && !(val & LCDC_ENABLE))
			{
				/* Peanut-GB will happily turn off LCD outside
				 * of VBLANK even though this damages real
				 * hardware. */

				/* Set LCD to Mode 0. */
				gb->hram_io[IO_STAT] =
					(gb->hram_io[IO_STAT] & ~STAT_MODE) |
					IO_STAT_MODE_HBLANK;
				/* LY fixed to 0 when LCD turned off. */
				gb->hram_io[IO_LY] = 0;
				/* Keep track of lcd_count to correctly track
				 * passing time. */
				gb->counter.lcd_off_count += gb->counter.lcd_count;
				/* Reset LCD timer, since the LCD starts from
				 * the beginning on power on. */
				gb->counter.lcd_count = 0;
			}
			return;
		}

		case 0x41:
			gb->hram_io[IO_STAT] = (val & STAT_USER_BITS) | (gb->hram_io[IO_STAT] & STAT_MODE) | 0x80;
			return;

		case 0x42:
			gb->hram_io[IO_SCY] = val;
			return;

		case 0x43:
			gb->hram_io[IO_SCX] = val;
			return;

		/* LY (0xFF44) is read only. */
		case 0x45:
			gb->hram_io[IO_LYC] = val;
			return;

		/* DMA Register */
		case 0x46:
		{
			uint16_t dma_addr;
			uint16_t i;

			dma_addr = (uint_fast16_t)val << 8;
			gb->hram_io[IO_DMA] = val;

			for(i = 0; i < OAM_SIZE; i++)
			{
				gb->oam[i] = __gb_read(gb, dma_addr + i);
			}

			return;
		}

		/* DMG Palette Registers */
		case 0x47:
			gb->hram_io[IO_BGP] = val;
			gb->display.bg_palette[0] = (gb->hram_io[IO_BGP] & 0x03);
			gb->display.bg_palette[1] = (gb->hram_io[IO_BGP] >> 2) & 0x03;
			gb->display.bg_palette[2] = (gb->hram_io[IO_BGP] >> 4) & 0x03;
			gb->display.bg_palette[3] = (gb->hram_io[IO_BGP] >> 6) & 0x03;
			return;

		case 0x48:
			gb->hram_io[IO_OBP0] = val;
			gb->display.sp_palette[0] = (gb->hram_io[IO_OBP0] & 0x03);
			gb->display.sp_palette[1] = (gb->hram_io[IO_OBP0] >> 2) & 0x03;
			gb->display.sp_palette[2] = (gb->hram_io[IO_OBP0] >> 4) & 0x03;
			gb->display.sp_palette[3] = (gb->hram_io[IO_OBP0] >> 6) & 0x03;
			return;

		case 0x49:
			gb->hram_io[IO_OBP1] = val;
			gb->display.sp_palette[4] = (gb->hram_io[IO_OBP1] & 0x03);
			gb->display.sp_palette[5] = (gb->hram_io[IO_OBP1] >> 2) & 0x03;
			gb->display.sp_palette[6] = (gb->hram_io[IO_OBP1] >> 4) & 0x03;
			gb->display.sp_palette[7] = (gb->hram_io[IO_OBP1] >> 6) & 0x03;
			return;

		/* Window Position Registers */
		case 0x4A:
			gb->hram_io[IO_WY] = val;
			return;

		case 0x4B:
			gb->hram_io[IO_WX] = val;
			return;

		/* Turn off boot ROM */
		case 0x50:
			gb->hram_io[IO_BOOT] = 0x01;
			return;

		/* Interrupt Enable Register */
		case 0xFF:
			gb->hram_io[IO_IE] = val;
			return;
		}
	}

	/* Invalid writes are ignored. */
	return;
}

uint8_t __gb_execute_cb(struct gb_s *gb)
{
	uint8_t inst_cycles;
	uint8_t cbop = __gb_read(gb, gb->cpu_reg.pc.reg++);
	uint8_t r = (cbop & 0x7);
	uint8_t b = (cbop >> 3) & 0x7;
	uint8_t d = (cbop >> 3) & 0x1;
	uint8_t val;
	uint8_t writeback = 1;

	inst_cycles = 8;
	/* Add an additional 8 cycles to these sets of instructions. */
	switch(cbop & 0xC7)
	{
	case 0x06:
	case 0x86:
    	case 0xC6:
		inst_cycles += 8;
    	break;
    	case 0x46:
		inst_cycles += 4;
    	break;
	}

	switch(r)
	{
	case 0:
		val = gb->cpu_reg.bc.bytes.b;
		break;

	case 1:
		val = gb->cpu_reg.bc.bytes.c;
		break;

	case 2:
		val = gb->cpu_reg.de.bytes.d;
		break;

	case 3:
		val = gb->cpu_reg.de.bytes.e;
		break;

	case 4:
		val = gb->cpu_reg.hl.bytes.h;
		break;

	case 5:
		val = gb->cpu_reg.hl.bytes.l;
		break;

	case 6:
		val = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	/* Only values 0-7 are possible here, so we make the final case
	 * default to satisfy -Wmaybe-uninitialized warning. */
	default:
		val = gb->cpu_reg.a;
		break;
	}

	switch(cbop >> 6)
	{
	case 0x0:
		cbop = (cbop >> 4) & 0x3;

		switch(cbop)
		{
		case 0x0: /* RdC R */
		case 0x1: /* Rd R */
			if(d) /* RRC R / RR R */
			{
				uint8_t temp = val;
				val = (val >> 1);
				val |= cbop ? (gb->cpu_reg.f.f_bits.c << 7) : (temp << 7);
				gb->cpu_reg.f.reg = 0;
				gb->cpu_reg.f.f_bits.z = (val == 0x00);
				gb->cpu_reg.f.f_bits.c = (temp & 0x01);
			}
			else /* RLC R / RL R */
			{
				uint8_t temp = val;
				val = (val << 1);
				val |= cbop ? gb->cpu_reg.f.f_bits.c : (temp >> 7);
				gb->cpu_reg.f.reg = 0;
				gb->cpu_reg.f.f_bits.z = (val == 0x00);
				gb->cpu_reg.f.f_bits.c = (temp >> 7);
			}

			break;

		case 0x2:
			if(d) /* SRA R */
			{
				gb->cpu_reg.f.reg = 0;
				gb->cpu_reg.f.f_bits.c = val & 0x01;
				val = (val >> 1) | (val & 0x80);
				gb->cpu_reg.f.f_bits.z = (val == 0x00);
			}
			else /* SLA R */
			{
				gb->cpu_reg.f.reg = 0;
				gb->cpu_reg.f.f_bits.c = (val >> 7);
				val = val << 1;
				gb->cpu_reg.f.f_bits.z = (val == 0x00);
			}

			break;

		case 0x3:
			if(d) /* SRL R */
			{
				gb->cpu_reg.f.reg = 0;
				gb->cpu_reg.f.f_bits.c = val & 0x01;
				val = val >> 1;
				gb->cpu_reg.f.f_bits.z = (val == 0x00);
			}
			else /* SWAP R */
			{
				uint8_t temp = (val >> 4) & 0x0F;
				temp |= (val << 4) & 0xF0;
				val = temp;
				gb->cpu_reg.f.reg = 0;
				gb->cpu_reg.f.f_bits.z = (val == 0x00);
			}

			break;
		}

		break;

	case 0x1: /* BIT B, R */
		gb->cpu_reg.f.f_bits.z = !((val >> b) & 0x1);
		gb->cpu_reg.f.f_bits.n = 0;
		gb->cpu_reg.f.f_bits.h = 1;
		writeback = 0;
		break;

	case 0x2: /* RES B, R */
		val &= (0xFE << b) | (0xFF >> (8 - b));
		break;

	case 0x3: /* SET B, R */
		val |= (0x1 << b);
		break;
	}

	if(writeback)
	{
		switch(r)
		{
		case 0:
			gb->cpu_reg.bc.bytes.b = val;
			break;

		case 1:
			gb->cpu_reg.bc.bytes.c = val;
			break;

		case 2:
			gb->cpu_reg.de.bytes.d = val;
			break;

		case 3:
			gb->cpu_reg.de.bytes.e = val;
			break;

		case 4:
			gb->cpu_reg.hl.bytes.h = val;
			break;

		case 5:
			gb->cpu_reg.hl.bytes.l = val;
			break;

		case 6:
			__gb_write(gb, gb->cpu_reg.hl.reg, val);
			break;

		case 7:
			gb->cpu_reg.a = val;
			break;
		}
	}
	return inst_cycles;
}

#if ENABLE_LCD
struct sprite_data {
	uint8_t sprite_number;
	uint8_t x;
};

#if PEANUT_GB_HIGH_LCD_ACCURACY
static int compare_sprites(const struct sprite_data *const sd1, const struct sprite_data *const sd2)
{
	int x_res;

	x_res = (int)sd1->x - (int)sd2->x;
	if(x_res != 0)
		return x_res;

	return (int)sd1->sprite_number - (int)sd2->sprite_number;
}
#endif

void __gb_draw_line(struct gb_s *gb)
{
	uint8_t pixels[160] = {0};

	/* If LCD not initialised by front-end, don't render anything. */
	if(gb->display.lcd_draw_line == NULL)
		return;

	if(gb->direct.frame_skip && !gb->display.frame_skip_count)
		return;

	/* If interlaced mode is activated, check if we need to draw the current
	 * line. */
	if(gb->direct.interlace)
	{
		if((!gb->display.interlace_count
				&& (gb->hram_io[IO_LY] & 1) == 0)
				|| (gb->display.interlace_count
				    && (gb->hram_io[IO_LY] & 1) == 1))
		{
			/* Compensate for missing window draw if required. */
			if(gb->hram_io[IO_LCDC] & LCDC_WINDOW_ENABLE
					&& gb->hram_io[IO_LY] >= gb->display.WY
					&& gb->hram_io[IO_WX] <= 166)
				gb->display.window_clear++;

			return;
		}
	}

	/* If background is enabled, draw it. */
	if(gb->hram_io[IO_LCDC] & LCDC_BG_ENABLE)
	{
		uint8_t bg_y, disp_x, bg_x, idx, py, px, t1, t2;
		uint16_t bg_map, tile;

		/* Calculate current background line to draw. Constant because
		 * this function draws only this one line each time it is
		 * called. */
		bg_y = gb->hram_io[IO_LY] + gb->hram_io[IO_SCY];

		/* Get selected background map address for first tile
		 * corresponding to current line.
		 * 0x20 (32) is the width of a background tile, and the bit
		 * shift is to calculate the address. */
		bg_map =
			((gb->hram_io[IO_LCDC] & LCDC_BG_MAP) ?
			 VRAM_BMAP_2 : VRAM_BMAP_1)
			+ (bg_y >> 3) * 0x20;

		/* The displays (what the player sees) X coordinate, drawn right
		 * to left. */
		disp_x = LCD_WIDTH - 1;

		/* The X coordinate to begin drawing the background at. */
		bg_x = disp_x + gb->hram_io[IO_SCX];

		/* Get tile index for current background tile. */
		idx = gb->vram[bg_map + (bg_x >> 3)];
		/* Y coordinate of tile pixel to draw. */
		py = (bg_y & 0x07);
		/* X coordinate of tile pixel to draw. */
		px = 7 - (bg_x & 0x07);

		/* Select addressing mode. */
		if(gb->hram_io[IO_LCDC] & LCDC_TILE_SELECT)
			tile = VRAM_TILES_1 + idx * 0x10;
		else
			tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;

		tile += 2 * py;

		/* fetch first tile */
		t1 = gb->vram[tile] >> px;
		t2 = gb->vram[tile + 1] >> px;

		for(; disp_x != 0xFF; disp_x--)
		{
			uint8_t c;

			if(px == 8)
			{
				/* fetch next tile */
				px = 0;
				bg_x = disp_x + gb->hram_io[IO_SCX];
				idx = gb->vram[bg_map + (bg_x >> 3)];

				if(gb->hram_io[IO_LCDC] & LCDC_TILE_SELECT)
					tile = VRAM_TILES_1 + idx * 0x10;
				else
					tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;

				tile += 2 * py;
				t1 = gb->vram[tile];
				t2 = gb->vram[tile + 1];
			}

			/* copy background */
			c = (t1 & 0x1) | ((t2 & 0x1) << 1);
			pixels[disp_x] = gb->display.bg_palette[c];
#if PEANUT_GB_12_COLOUR
			pixels[disp_x] |= LCD_PALETTE_BG;
#endif
			t1 = t1 >> 1;
			t2 = t2 >> 1;
			px++;
		}
	}

	/* draw window */
	if(gb->hram_io[IO_LCDC] & LCDC_WINDOW_ENABLE
			&& gb->hram_io[IO_LY] >= gb->display.WY
			&& gb->hram_io[IO_WX] <= 166)
	{
		uint16_t win_line, tile;
		uint8_t disp_x, win_x, py, px, idx, t1, t2, end;

		/* Calculate Window Map Address. */
		win_line = (gb->hram_io[IO_LCDC] & LCDC_WINDOW_MAP) ?
				    VRAM_BMAP_2 : VRAM_BMAP_1;
		win_line += (gb->display.window_clear >> 3) * 0x20;

		disp_x = LCD_WIDTH - 1;
		win_x = disp_x - gb->hram_io[IO_WX] + 7;

		// look up tile
		py = gb->display.window_clear & 0x07;
		px = 7 - (win_x & 0x07);
		idx = gb->vram[win_line + (win_x >> 3)];

		if(gb->hram_io[IO_LCDC] & LCDC_TILE_SELECT)
			tile = VRAM_TILES_1 + idx * 0x10;
		else
			tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;

		tile += 2 * py;

		// fetch first tile
		t1 = gb->vram[tile] >> px;
		t2 = gb->vram[tile + 1] >> px;

		// loop & copy window
		end = (gb->hram_io[IO_WX] < 7 ? 0 : gb->hram_io[IO_WX] - 7) - 1;

		for(; disp_x != end; disp_x--)
		{
			uint8_t c;

			if(px == 8)
			{
				// fetch next tile
				px = 0;
				win_x = disp_x - gb->hram_io[IO_WX] + 7;
				idx = gb->vram[win_line + (win_x >> 3)];

				if(gb->hram_io[IO_LCDC] & LCDC_TILE_SELECT)
					tile = VRAM_TILES_1 + idx * 0x10;
				else
					tile = VRAM_TILES_2 + ((idx + 0x80) % 0x100) * 0x10;

				tile += 2 * py;
				t1 = gb->vram[tile];
				t2 = gb->vram[tile + 1];
			}

			// copy window
			c = (t1 & 0x1) | ((t2 & 0x1) << 1);
			pixels[disp_x] = gb->display.bg_palette[c];
#if PEANUT_GB_12_COLOUR
			pixels[disp_x] |= LCD_PALETTE_BG;
#endif
			t1 = t1 >> 1;
			t2 = t2 >> 1;
			px++;
		}

		gb->display.window_clear++; // advance window line
	}

	// draw sprites
	if(gb->hram_io[IO_LCDC] & LCDC_OBJ_ENABLE)
	{
		uint8_t sprite_number;
#if PEANUT_GB_HIGH_LCD_ACCURACY
		uint8_t number_of_sprites = 0;

		struct sprite_data sprites_to_render[MAX_SPRITES_LINE];

		/* Record number of sprites on the line being rendered, limited
		 * to the maximum number sprites that the Game Boy is able to
		 * render on each line (10 sprites). */
		for(sprite_number = 0;
				sprite_number < NUM_SPRITES;
				sprite_number++)
		{
			/* Sprite Y position. */
			uint8_t OY = gb->oam[4 * sprite_number + 0];
			/* Sprite X position. */
			uint8_t OX = gb->oam[4 * sprite_number + 1];

			/* If sprite isn't on this line, continue. */
			if (gb->hram_io[IO_LY] +
				(gb->hram_io[IO_LCDC] & LCDC_OBJ_SIZE ? 0 : 8) >= OY
					|| gb->hram_io[IO_LY] + 16 < OY)
				continue;

			struct sprite_data current;

			current.sprite_number = sprite_number;
			current.x = OX;

			uint8_t place;
			for (place = number_of_sprites; place != 0; place--)
			{
				if(compare_sprites(&sprites_to_render[place - 1], &current) < 0)
					break;
			}
			if(place >= MAX_SPRITES_LINE)
				continue;
			memmove(
				&sprites_to_render[place + 1],
				&sprites_to_render[place],
				(MAX_SPRITES_LINE - place - 1) * sizeof(current)
			);
			if(number_of_sprites < MAX_SPRITES_LINE)
				number_of_sprites++;
			sprites_to_render[place] = current;
		}
#endif

		/* Render each sprite, from low priority to high priority. */
#if PEANUT_GB_HIGH_LCD_ACCURACY
		/* Render the top ten prioritised sprites on this scanline. */
		for(sprite_number = number_of_sprites - 1;
				sprite_number != 0xFF;
				sprite_number--)
		{
			uint8_t s = sprites_to_render[sprite_number].sprite_number;
#else
		for (sprite_number = NUM_SPRITES - 1;
			sprite_number != 0xFF;
			sprite_number--)
		{
			uint8_t s = sprite_number;
#endif
			uint8_t py, t1, t2, dir, start, end, shift, disp_x;
			/* Sprite Y position. */
			uint8_t OY = gb->oam[4 * s + 0];
			/* Sprite X position. */
			uint8_t OX = gb->oam[4 * s + 1];
			/* Sprite Tile/Pattern Number. */
			uint8_t OT = gb->oam[4 * s + 2]
				     & (gb->hram_io[IO_LCDC] & LCDC_OBJ_SIZE ? 0xFE : 0xFF);
			/* Additional attributes. */
			uint8_t OF = gb->oam[4 * s + 3];

#if !PEANUT_GB_HIGH_LCD_ACCURACY
			/* If sprite isn't on this line, continue. */
			if(gb->hram_io[IO_LY] +
					(gb->hram_io[IO_LCDC] & LCDC_OBJ_SIZE ? 0 : 8) >= OY ||
					gb->hram_io[IO_LY] + 16 < OY)
				continue;
#endif

			/* Continue if sprite not visible. */
			if(OX == 0 || OX >= 168)
				continue;

			// y flip
			py = gb->hram_io[IO_LY] - OY + 16;

			if(OF & OBJ_FLIP_Y)
				py = (gb->hram_io[IO_LCDC] & LCDC_OBJ_SIZE ? 15 : 7) - py;

			// fetch the tile
			t1 = gb->vram[VRAM_TILES_1 + OT * 0x10 + 2 * py];
			t2 = gb->vram[VRAM_TILES_1 + OT * 0x10 + 2 * py + 1];

			// handle x flip
			if(OF & OBJ_FLIP_X)
			{
				dir = 1;
				start = (OX < 8 ? 0 : OX - 8);
				end = MIN(OX, LCD_WIDTH);
				shift = 8 - OX + start;
			}
			else
			{
				dir = (uint8_t)-1;
				start = MIN(OX, LCD_WIDTH) - 1;
				end = (OX < 8 ? 0 : OX - 8) - 1;
				shift = OX - (start + 1);
			}

			// copy tile
			t1 >>= shift;
			t2 >>= shift;

			/* TODO: Put for loop within the to if statements
			 * because the BG priority bit will be the same for
			 * all the pixels in the tile. */
			for(disp_x = start; disp_x != end; disp_x += dir)
			{
				uint8_t c = (t1 & 0x1) | ((t2 & 0x1) << 1);
				// check transparency / sprite overlap / background overlap

				if(c && !(OF & OBJ_PRIORITY && !((pixels[disp_x] & 0x3) == gb->display.bg_palette[0])))
				{
					/* Set pixel colour. */
					pixels[disp_x] = (OF & OBJ_PALETTE)
						? gb->display.sp_palette[c + 4]
						: gb->display.sp_palette[c];
#if PEANUT_GB_12_COLOUR
					/* Set pixel palette (OBJ0 or OBJ1). */
					pixels[disp_x] |= (OF & OBJ_PALETTE);
#endif
				}

				t1 = t1 >> 1;
				t2 = t2 >> 1;
			}
		}
	}

	gb->display.lcd_draw_line(gb, pixels, gb->hram_io[IO_LY]);
}
#endif

/**
 * Internal function used to step the CPU.
 */
void __gb_step_cpu(struct gb_s *gb)
{
	uint8_t opcode;
	uint_fast16_t inst_cycles;
	static const uint8_t op_cycles[0x100] =
	{
		/* *INDENT-OFF* */
		/*0 1 2  3  4  5  6  7  8  9  A  B  C  D  E  F	*/
		4,12, 8, 8, 4, 4, 8, 4,20, 8, 8, 8, 4, 4, 8, 4,	/* 0x00 */
		4,12, 8, 8, 4, 4, 8, 4,12, 8, 8, 8, 4, 4, 8, 4,	/* 0x10 */
		8,12, 8, 8, 4, 4, 8, 4, 8, 8, 8, 8, 4, 4, 8, 4,	/* 0x20 */
		8,12, 8, 8,12,12,12, 4, 8, 8, 8, 8, 4, 4, 8, 4,	/* 0x30 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	/* 0x40 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	/* 0x50 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	/* 0x60 */
		8, 8, 8, 8, 8, 8, 4, 8, 4, 4, 4, 4, 4, 4, 8, 4, /* 0x70 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	/* 0x80 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	/* 0x90 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	/* 0xA0 */
		4, 4, 4, 4, 4, 4, 8, 4, 4, 4, 4, 4, 4, 4, 8, 4,	/* 0xB0 */
		8,12,12,16,12,16, 8,16, 8,16,12, 8,12,24, 8,16,	/* 0xC0 */
		8,12,12, 0,12,16, 8,16, 8,16,12, 0,12, 0, 8,16,	/* 0xD0 */
		12,12,8, 0, 0,16, 8,16,16, 4,16, 0, 0, 0, 8,16,	/* 0xE0 */
		12,12,8, 4, 0,16, 8,16,12, 8,16, 4, 0, 0, 8,16	/* 0xF0 */
		/* *INDENT-ON* */
	};
	static const uint_fast16_t TAC_CYCLES[4] = {1024, 16, 64, 256};

	/* Handle interrupts */
	/* If gb_halt is positive, then an interrupt must have occurred by the
	 * time we reach here, because on HALT, we jump to the next interrupt
	 * immediately. */
	while(gb->gb_halt || (gb->gb_ime &&
			gb->hram_io[IO_IF] & gb->hram_io[IO_IE] & ANY_INTR))
	{
		gb->gb_halt = false;

		if(!gb->gb_ime)
			break;

		/* Disable interrupts */
		gb->gb_ime = false;

		/* Push Program Counter */
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);

		/* Call interrupt handler if required. */
		if(gb->hram_io[IO_IF] & gb->hram_io[IO_IE] & VBLANK_INTR)
		{
			gb->cpu_reg.pc.reg = VBLANK_INTR_ADDR;
			gb->hram_io[IO_IF] ^= VBLANK_INTR;
		}
		else if(gb->hram_io[IO_IF] & gb->hram_io[IO_IE] & LCDC_INTR)
		{
			gb->cpu_reg.pc.reg = LCDC_INTR_ADDR;
			gb->hram_io[IO_IF] ^= LCDC_INTR;
		}
		else if(gb->hram_io[IO_IF] & gb->hram_io[IO_IE] & TIMER_INTR)
		{
			gb->cpu_reg.pc.reg = TIMER_INTR_ADDR;
			gb->hram_io[IO_IF] ^= TIMER_INTR;
		}
		else if(gb->hram_io[IO_IF] & gb->hram_io[IO_IE] & SERIAL_INTR)
		{
			gb->cpu_reg.pc.reg = SERIAL_INTR_ADDR;
			gb->hram_io[IO_IF] ^= SERIAL_INTR;
		}
		else if(gb->hram_io[IO_IF] & gb->hram_io[IO_IE] & CONTROL_INTR)
		{
			gb->cpu_reg.pc.reg = CONTROL_INTR_ADDR;
			gb->hram_io[IO_IF] ^= CONTROL_INTR;
		}

		break;
	}

	/* Obtain opcode */
	opcode = __gb_read(gb, gb->cpu_reg.pc.reg++);
	inst_cycles = op_cycles[opcode];

	/* Execute opcode */
	switch(opcode)
	{
	case 0x00: /* NOP */
		break;

	case 0x01: /* LD BC, imm */
		gb->cpu_reg.bc.bytes.c = __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.bc.bytes.b = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x02: /* LD (BC), A */
		__gb_write(gb, gb->cpu_reg.bc.reg, gb->cpu_reg.a);
		break;

	case 0x03: /* INC BC */
		gb->cpu_reg.bc.reg++;
		break;

	case 0x04: /* INC B */
		PGB_INSTR_INC_R8(gb->cpu_reg.bc.bytes.b);
		break;

	case 0x05: /* DEC B */
		PGB_INSTR_DEC_R8(gb->cpu_reg.bc.bytes.b);
		break;

	case 0x06: /* LD B, imm */
		gb->cpu_reg.bc.bytes.b = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x07: /* RLCA */
		gb->cpu_reg.a = (gb->cpu_reg.a << 1) | (gb->cpu_reg.a >> 7);
		gb->cpu_reg.f.reg = 0;
		gb->cpu_reg.f.f_bits.c = (gb->cpu_reg.a & 0x01);
		break;

	case 0x08: /* LD (imm), SP */
	{
		uint8_t h, l;
		uint16_t temp;
		l = __gb_read(gb, gb->cpu_reg.pc.reg++);
		h = __gb_read(gb, gb->cpu_reg.pc.reg++);
		temp = PEANUT_GB_U8_TO_U16(h,l);
		__gb_write(gb, temp++, gb->cpu_reg.sp.bytes.p);
		__gb_write(gb, temp, gb->cpu_reg.sp.bytes.s);
		break;
	}

	case 0x09: /* ADD HL, BC */
	{
		uint_fast32_t temp = gb->cpu_reg.hl.reg + gb->cpu_reg.bc.reg;
		gb->cpu_reg.f.f_bits.n = 0;
		gb->cpu_reg.f.f_bits.h =
			(temp ^ gb->cpu_reg.hl.reg ^ gb->cpu_reg.bc.reg) & 0x1000 ? 1 : 0;
		gb->cpu_reg.f.f_bits.c = (temp & 0xFFFF0000) ? 1 : 0;
		gb->cpu_reg.hl.reg = (temp & 0x0000FFFF);
		break;
	}

	case 0x0A: /* LD A, (BC) */
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.bc.reg);
		break;

	case 0x0B: /* DEC BC */
		gb->cpu_reg.bc.reg--;
		break;

	case 0x0C: /* INC C */
		PGB_INSTR_INC_R8(gb->cpu_reg.bc.bytes.c);
		break;

	case 0x0D: /* DEC C */
		PGB_INSTR_DEC_R8(gb->cpu_reg.bc.bytes.c);
		break;

	case 0x0E: /* LD C, imm */
		gb->cpu_reg.bc.bytes.c = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x0F: /* RRCA */
		gb->cpu_reg.f.reg = 0;
		gb->cpu_reg.f.f_bits.c = gb->cpu_reg.a & 0x01;
		gb->cpu_reg.a = (gb->cpu_reg.a >> 1) | (gb->cpu_reg.a << 7);
		break;

	case 0x10: /* STOP */
		//gb->gb_halt = true;
		break;

	case 0x11: /* LD DE, imm */
		gb->cpu_reg.de.bytes.e = __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.de.bytes.d = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x12: /* LD (DE), A */
		__gb_write(gb, gb->cpu_reg.de.reg, gb->cpu_reg.a);
		break;

	case 0x13: /* INC DE */
		gb->cpu_reg.de.reg++;
		break;

	case 0x14: /* INC D */
		PGB_INSTR_INC_R8(gb->cpu_reg.de.bytes.d);
		break;

	case 0x15: /* DEC D */
		PGB_INSTR_DEC_R8(gb->cpu_reg.de.bytes.d);
		break;

	case 0x16: /* LD D, imm */
		gb->cpu_reg.de.bytes.d = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x17: /* RLA */
	{
		uint8_t temp = gb->cpu_reg.a;
		gb->cpu_reg.a = (gb->cpu_reg.a << 1) | gb->cpu_reg.f.f_bits.c;
		gb->cpu_reg.f.reg = 0;
		gb->cpu_reg.f.f_bits.c = (temp >> 7) & 0x01;
		break;
	}

	case 0x18: /* JR imm */
	{
		int8_t temp = (int8_t) __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.pc.reg += temp;
		break;
	}

	case 0x19: /* ADD HL, DE */
	{
		uint_fast32_t temp = gb->cpu_reg.hl.reg + gb->cpu_reg.de.reg;
		gb->cpu_reg.f.f_bits.n = 0;
		gb->cpu_reg.f.f_bits.h =
			(temp ^ gb->cpu_reg.hl.reg ^ gb->cpu_reg.de.reg) & 0x1000 ? 1 : 0;
		gb->cpu_reg.f.f_bits.c = (temp & 0xFFFF0000) ? 1 : 0;
		gb->cpu_reg.hl.reg = (temp & 0x0000FFFF);
		break;
	}

	case 0x1A: /* LD A, (DE) */
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.de.reg);
		break;

	case 0x1B: /* DEC DE */
		gb->cpu_reg.de.reg--;
		break;

	case 0x1C: /* INC E */
		PGB_INSTR_INC_R8(gb->cpu_reg.de.bytes.e);
		break;

	case 0x1D: /* DEC E */
		PGB_INSTR_DEC_R8(gb->cpu_reg.de.bytes.e);
		break;

	case 0x1E: /* LD E, imm */
		gb->cpu_reg.de.bytes.e = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x1F: /* RRA */
	{
		uint8_t temp = gb->cpu_reg.a;
		gb->cpu_reg.a = gb->cpu_reg.a >> 1 | (gb->cpu_reg.f.f_bits.c << 7);
		gb->cpu_reg.f.reg = 0;
		gb->cpu_reg.f.f_bits.c = temp & 0x1;
		break;
	}

	case 0x20: /* JR NZ, imm */
		if(!gb->cpu_reg.f.f_bits.z)
		{
			int8_t temp = (int8_t) __gb_read(gb, gb->cpu_reg.pc.reg++);
			gb->cpu_reg.pc.reg += temp;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg++;

		break;

	case 0x21: /* LD HL, imm */
		gb->cpu_reg.hl.bytes.l = __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.hl.bytes.h = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x22: /* LDI (HL), A */
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.a);
		gb->cpu_reg.hl.reg++;
		break;

	case 0x23: /* INC HL */
		gb->cpu_reg.hl.reg++;
		break;

	case 0x24: /* INC H */
		PGB_INSTR_INC_R8(gb->cpu_reg.hl.bytes.h);
		break;

	case 0x25: /* DEC H */
		PGB_INSTR_DEC_R8(gb->cpu_reg.hl.bytes.h);
		break;

	case 0x26: /* LD H, imm */
		gb->cpu_reg.hl.bytes.h = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x27: /* DAA */
	{
		/* The following is from SameBoy. MIT License. */
		int16_t a = gb->cpu_reg.a;

		if(gb->cpu_reg.f.f_bits.n)
		{
			if(gb->cpu_reg.f.f_bits.h)
				a = (a - 0x06) & 0xFF;

			if(gb->cpu_reg.f.f_bits.c)
				a -= 0x60;
		}
		else
		{
			if(gb->cpu_reg.f.f_bits.h || (a & 0x0F) > 9)
				a += 0x06;

			if(gb->cpu_reg.f.f_bits.c || a > 0x9F)
				a += 0x60;
		}

		if((a & 0x100) == 0x100)
			gb->cpu_reg.f.f_bits.c = 1;

		gb->cpu_reg.a = a;
		gb->cpu_reg.f.f_bits.z = (gb->cpu_reg.a == 0);
		gb->cpu_reg.f.f_bits.h = 0;

		break;
	}

	case 0x28: /* JR Z, imm */
		if(gb->cpu_reg.f.f_bits.z)
		{
			int8_t temp = (int8_t) __gb_read(gb, gb->cpu_reg.pc.reg++);
			gb->cpu_reg.pc.reg += temp;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg++;

		break;

	case 0x29: /* ADD HL, HL */
	{
		gb->cpu_reg.f.f_bits.c = (gb->cpu_reg.hl.reg & 0x8000) > 0;
		gb->cpu_reg.hl.reg <<= 1;
		gb->cpu_reg.f.f_bits.n = 0;
		gb->cpu_reg.f.f_bits.h = (gb->cpu_reg.hl.reg & 0x1000) > 0;
		break;
	}

	case 0x2A: /* LD A, (HL+) */
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.hl.reg++);
		break;

	case 0x2B: /* DEC HL */
		gb->cpu_reg.hl.reg--;
		break;

	case 0x2C: /* INC L */
		PGB_INSTR_INC_R8(gb->cpu_reg.hl.bytes.l);
		break;

	case 0x2D: /* DEC L */
		PGB_INSTR_DEC_R8(gb->cpu_reg.hl.bytes.l);
		break;

	case 0x2E: /* LD L, imm */
		gb->cpu_reg.hl.bytes.l = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x2F: /* CPL */
		gb->cpu_reg.a = ~gb->cpu_reg.a;
		gb->cpu_reg.f.f_bits.n = 1;
		gb->cpu_reg.f.f_bits.h = 1;
		break;

	case 0x30: /* JR NC, imm */
		if(!gb->cpu_reg.f.f_bits.c)
		{
			int8_t temp = (int8_t) __gb_read(gb, gb->cpu_reg.pc.reg++);
			gb->cpu_reg.pc.reg += temp;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg++;

		break;

	case 0x31: /* LD SP, imm */
		gb->cpu_reg.sp.bytes.p = __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.sp.bytes.s = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x32: /* LD (HL), A */
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.a);
		gb->cpu_reg.hl.reg--;
		break;

	case 0x33: /* INC SP */
		gb->cpu_reg.sp.reg++;
		break;

	case 0x34: /* INC (HL) */
	{
		uint8_t temp = __gb_read(gb, gb->cpu_reg.hl.reg);
		PGB_INSTR_INC_R8(temp);
		__gb_write(gb, gb->cpu_reg.hl.reg, temp);
		break;
	}

	case 0x35: /* DEC (HL) */
	{
		uint8_t temp = __gb_read(gb, gb->cpu_reg.hl.reg);
		PGB_INSTR_DEC_R8(temp);
		__gb_write(gb, gb->cpu_reg.hl.reg, temp);
		break;
	}

	case 0x36: /* LD (HL), imm */
		__gb_write(gb, gb->cpu_reg.hl.reg, __gb_read(gb, gb->cpu_reg.pc.reg++));
		break;

	case 0x37: /* SCF */
		gb->cpu_reg.f.f_bits.n = 0;
		gb->cpu_reg.f.f_bits.h = 0;
		gb->cpu_reg.f.f_bits.c = 1;
		break;

	case 0x38: /* JR C, imm */
		if(gb->cpu_reg.f.f_bits.c)
		{
			int8_t temp = (int8_t) __gb_read(gb, gb->cpu_reg.pc.reg++);
			gb->cpu_reg.pc.reg += temp;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg++;

		break;

	case 0x39: /* ADD HL, SP */
	{
		uint_fast32_t temp = gb->cpu_reg.hl.reg + gb->cpu_reg.sp.reg;
		gb->cpu_reg.f.f_bits.n = 0;
		gb->cpu_reg.f.f_bits.h =
			((gb->cpu_reg.hl.reg & 0xFFF) + (gb->cpu_reg.sp.reg & 0xFFF)) & 0x1000 ? 1 : 0;
		gb->cpu_reg.f.f_bits.c = temp & 0x10000 ? 1 : 0;
		gb->cpu_reg.hl.reg = (uint16_t)temp;
		break;
	}

	case 0x3A: /* LD A, (HL) */
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.hl.reg--);
		break;

	case 0x3B: /* DEC SP */
		gb->cpu_reg.sp.reg--;
		break;

	case 0x3C: /* INC A */
		PGB_INSTR_INC_R8(gb->cpu_reg.a);
		break;

	case 0x3D: /* DEC A */
		PGB_INSTR_DEC_R8(gb->cpu_reg.a);
		break;

	case 0x3E: /* LD A, imm */
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.pc.reg++);
		break;

	case 0x3F: /* CCF */
		gb->cpu_reg.f.f_bits.n = 0;
		gb->cpu_reg.f.f_bits.h = 0;
		gb->cpu_reg.f.f_bits.c = ~gb->cpu_reg.f.f_bits.c;
		break;

	case 0x40: /* LD B, B */
		break;

	case 0x41: /* LD B, C */
		gb->cpu_reg.bc.bytes.b = gb->cpu_reg.bc.bytes.c;
		break;

	case 0x42: /* LD B, D */
		gb->cpu_reg.bc.bytes.b = gb->cpu_reg.de.bytes.d;
		break;

	case 0x43: /* LD B, E */
		gb->cpu_reg.bc.bytes.b = gb->cpu_reg.de.bytes.e;
		break;

	case 0x44: /* LD B, H */
		gb->cpu_reg.bc.bytes.b = gb->cpu_reg.hl.bytes.h;
		break;

	case 0x45: /* LD B, L */
		gb->cpu_reg.bc.bytes.b = gb->cpu_reg.hl.bytes.l;
		break;

	case 0x46: /* LD B, (HL) */
		gb->cpu_reg.bc.bytes.b = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x47: /* LD B, A */
		gb->cpu_reg.bc.bytes.b = gb->cpu_reg.a;
		break;

	case 0x48: /* LD C, B */
		gb->cpu_reg.bc.bytes.c = gb->cpu_reg.bc.bytes.b;
		break;

	case 0x49: /* LD C, C */
		break;

	case 0x4A: /* LD C, D */
		gb->cpu_reg.bc.bytes.c = gb->cpu_reg.de.bytes.d;
		break;

	case 0x4B: /* LD C, E */
		gb->cpu_reg.bc.bytes.c = gb->cpu_reg.de.bytes.e;
		break;

	case 0x4C: /* LD C, H */
		gb->cpu_reg.bc.bytes.c = gb->cpu_reg.hl.bytes.h;
		break;

	case 0x4D: /* LD C, L */
		gb->cpu_reg.bc.bytes.c = gb->cpu_reg.hl.bytes.l;
		break;

	case 0x4E: /* LD C, (HL) */
		gb->cpu_reg.bc.bytes.c = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x4F: /* LD C, A */
		gb->cpu_reg.bc.bytes.c = gb->cpu_reg.a;
		break;

	case 0x50: /* LD D, B */
		gb->cpu_reg.de.bytes.d = gb->cpu_reg.bc.bytes.b;
		break;

	case 0x51: /* LD D, C */
		gb->cpu_reg.de.bytes.d = gb->cpu_reg.bc.bytes.c;
		break;

	case 0x52: /* LD D, D */
		break;

	case 0x53: /* LD D, E */
		gb->cpu_reg.de.bytes.d = gb->cpu_reg.de.bytes.e;
		break;

	case 0x54: /* LD D, H */
		gb->cpu_reg.de.bytes.d = gb->cpu_reg.hl.bytes.h;
		break;

	case 0x55: /* LD D, L */
		gb->cpu_reg.de.bytes.d = gb->cpu_reg.hl.bytes.l;
		break;

	case 0x56: /* LD D, (HL) */
		gb->cpu_reg.de.bytes.d = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x57: /* LD D, A */
		gb->cpu_reg.de.bytes.d = gb->cpu_reg.a;
		break;

	case 0x58: /* LD E, B */
		gb->cpu_reg.de.bytes.e = gb->cpu_reg.bc.bytes.b;
		break;

	case 0x59: /* LD E, C */
		gb->cpu_reg.de.bytes.e = gb->cpu_reg.bc.bytes.c;
		break;

	case 0x5A: /* LD E, D */
		gb->cpu_reg.de.bytes.e = gb->cpu_reg.de.bytes.d;
		break;

	case 0x5B: /* LD E, E */
		break;

	case 0x5C: /* LD E, H */
		gb->cpu_reg.de.bytes.e = gb->cpu_reg.hl.bytes.h;
		break;

	case 0x5D: /* LD E, L */
		gb->cpu_reg.de.bytes.e = gb->cpu_reg.hl.bytes.l;
		break;

	case 0x5E: /* LD E, (HL) */
		gb->cpu_reg.de.bytes.e = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x5F: /* LD E, A */
		gb->cpu_reg.de.bytes.e = gb->cpu_reg.a;
		break;

	case 0x60: /* LD H, B */
		gb->cpu_reg.hl.bytes.h = gb->cpu_reg.bc.bytes.b;
		break;

	case 0x61: /* LD H, C */
		gb->cpu_reg.hl.bytes.h = gb->cpu_reg.bc.bytes.c;
		break;

	case 0x62: /* LD H, D */
		gb->cpu_reg.hl.bytes.h = gb->cpu_reg.de.bytes.d;
		break;

	case 0x63: /* LD H, E */
		gb->cpu_reg.hl.bytes.h = gb->cpu_reg.de.bytes.e;
		break;

	case 0x64: /* LD H, H */
		break;

	case 0x65: /* LD H, L */
		gb->cpu_reg.hl.bytes.h = gb->cpu_reg.hl.bytes.l;
		break;

	case 0x66: /* LD H, (HL) */
		gb->cpu_reg.hl.bytes.h = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x67: /* LD H, A */
		gb->cpu_reg.hl.bytes.h = gb->cpu_reg.a;
		break;

	case 0x68: /* LD L, B */
		gb->cpu_reg.hl.bytes.l = gb->cpu_reg.bc.bytes.b;
		break;

	case 0x69: /* LD L, C */
		gb->cpu_reg.hl.bytes.l = gb->cpu_reg.bc.bytes.c;
		break;

	case 0x6A: /* LD L, D */
		gb->cpu_reg.hl.bytes.l = gb->cpu_reg.de.bytes.d;
		break;

	case 0x6B: /* LD L, E */
		gb->cpu_reg.hl.bytes.l = gb->cpu_reg.de.bytes.e;
		break;

	case 0x6C: /* LD L, H */
		gb->cpu_reg.hl.bytes.l = gb->cpu_reg.hl.bytes.h;
		break;

	case 0x6D: /* LD L, L */
		break;

	case 0x6E: /* LD L, (HL) */
		gb->cpu_reg.hl.bytes.l = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x6F: /* LD L, A */
		gb->cpu_reg.hl.bytes.l = gb->cpu_reg.a;
		break;

	case 0x70: /* LD (HL), B */
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.bc.bytes.b);
		break;

	case 0x71: /* LD (HL), C */
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.bc.bytes.c);
		break;

	case 0x72: /* LD (HL), D */
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.de.bytes.d);
		break;

	case 0x73: /* LD (HL), E */
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.de.bytes.e);
		break;

	case 0x74: /* LD (HL), H */
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.hl.bytes.h);
		break;

	case 0x75: /* LD (HL), L */
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.hl.bytes.l);
		break;

	case 0x76: /* HALT */
	{
		int_fast16_t halt_cycles = INT_FAST16_MAX;

		/* TODO: Emulate HALT bug? */
		gb->gb_halt = true;

		if(gb->hram_io[IO_SC] & SERIAL_SC_TX_START)
		{
			int serial_cycles = SERIAL_CYCLES -
				gb->counter.serial_count;

			if(serial_cycles < halt_cycles)
				halt_cycles = serial_cycles;
		}

		if(gb->hram_io[IO_TAC] & IO_TAC_ENABLE_MASK)
		{
			int tac_cycles = TAC_CYCLES[gb->hram_io[IO_TAC] & IO_TAC_RATE_MASK] -
				gb->counter.tima_count;

			if(tac_cycles < halt_cycles)
				halt_cycles = tac_cycles;
		}

		if((gb->hram_io[IO_LCDC] & LCDC_ENABLE))
		{
			int lcd_cycles;

			/* If LCD is in HBlank, calculate the number of cycles
			 * until the end of HBlank and the start of mode 2 or
			 * mode 1. */
			if((gb->hram_io[IO_STAT] & STAT_MODE) == IO_STAT_MODE_HBLANK)
			{
				lcd_cycles = LCD_MODE0_HBLANK_MAX_DRUATION - gb->counter.lcd_count;
			}
			else if((gb->hram_io[IO_STAT] & STAT_MODE) == IO_STAT_MODE_OAM_SCAN)
			{
				lcd_cycles = LCD_MODE3_LCD_DRAW_MIN_DURATION - gb->counter.lcd_count;
			}
			else if((gb->hram_io[IO_STAT] & STAT_MODE) == IO_STAT_MODE_LCD_DRAW)
			{
				lcd_cycles = LCD_MODE0_HBLANK_MAX_DRUATION - gb->counter.lcd_count;
			}
			else
			{
				/* VBlank */
				lcd_cycles = LCD_LINE_CYCLES - gb->counter.lcd_count;
			}

			if(lcd_cycles < halt_cycles)
				halt_cycles = lcd_cycles;
		}

		/* Some halt cycles may already be very high, so make sure we
		 * don't underflow here. */
		if(halt_cycles <= 0)
			halt_cycles = 4;

		inst_cycles = (uint_fast16_t)halt_cycles;
		break;
	}

	case 0x77: /* LD (HL), A */
		__gb_write(gb, gb->cpu_reg.hl.reg, gb->cpu_reg.a);
		break;

	case 0x78: /* LD A, B */
		gb->cpu_reg.a = gb->cpu_reg.bc.bytes.b;
		break;

	case 0x79: /* LD A, C */
		gb->cpu_reg.a = gb->cpu_reg.bc.bytes.c;
		break;

	case 0x7A: /* LD A, D */
		gb->cpu_reg.a = gb->cpu_reg.de.bytes.d;
		break;

	case 0x7B: /* LD A, E */
		gb->cpu_reg.a = gb->cpu_reg.de.bytes.e;
		break;

	case 0x7C: /* LD A, H */
		gb->cpu_reg.a = gb->cpu_reg.hl.bytes.h;
		break;

	case 0x7D: /* LD A, L */
		gb->cpu_reg.a = gb->cpu_reg.hl.bytes.l;
		break;

	case 0x7E: /* LD A, (HL) */
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.hl.reg);
		break;

	case 0x7F: /* LD A, A */
		break;

	case 0x80: /* ADD A, B */
		PGB_INSTR_ADC_R8(gb->cpu_reg.bc.bytes.b, 0);
		break;

	case 0x81: /* ADD A, C */
		PGB_INSTR_ADC_R8(gb->cpu_reg.bc.bytes.c, 0);
		break;

	case 0x82: /* ADD A, D */
		PGB_INSTR_ADC_R8(gb->cpu_reg.de.bytes.d, 0);
		break;

	case 0x83: /* ADD A, E */
		PGB_INSTR_ADC_R8(gb->cpu_reg.de.bytes.e, 0);
		break;

	case 0x84: /* ADD A, H */
		PGB_INSTR_ADC_R8(gb->cpu_reg.hl.bytes.h, 0);
		break;

	case 0x85: /* ADD A, L */
		PGB_INSTR_ADC_R8(gb->cpu_reg.hl.bytes.l, 0);
		break;

	case 0x86: /* ADD A, (HL) */
		PGB_INSTR_ADC_R8(__gb_read(gb, gb->cpu_reg.hl.reg), 0);
		break;

	case 0x87: /* ADD A, A */
		PGB_INSTR_ADC_R8(gb->cpu_reg.a, 0);
		break;

	case 0x88: /* ADC A, B */
		PGB_INSTR_ADC_R8(gb->cpu_reg.bc.bytes.b, gb->cpu_reg.f.f_bits.c);
		break;

	case 0x89: /* ADC A, C */
		PGB_INSTR_ADC_R8(gb->cpu_reg.bc.bytes.c, gb->cpu_reg.f.f_bits.c);
		break;

	case 0x8A: /* ADC A, D */
		PGB_INSTR_ADC_R8(gb->cpu_reg.de.bytes.d, gb->cpu_reg.f.f_bits.c);
		break;

	case 0x8B: /* ADC A, E */
		PGB_INSTR_ADC_R8(gb->cpu_reg.de.bytes.e, gb->cpu_reg.f.f_bits.c);
		break;

	case 0x8C: /* ADC A, H */
		PGB_INSTR_ADC_R8(gb->cpu_reg.hl.bytes.h, gb->cpu_reg.f.f_bits.c);
		break;

	case 0x8D: /* ADC A, L */
		PGB_INSTR_ADC_R8(gb->cpu_reg.hl.bytes.l, gb->cpu_reg.f.f_bits.c);
		break;

	case 0x8E: /* ADC A, (HL) */
		PGB_INSTR_ADC_R8(__gb_read(gb, gb->cpu_reg.hl.reg), gb->cpu_reg.f.f_bits.c);
		break;

	case 0x8F: /* ADC A, A */
		PGB_INSTR_ADC_R8(gb->cpu_reg.a, gb->cpu_reg.f.f_bits.c);
		break;

	case 0x90: /* SUB B */
		PGB_INSTR_SBC_R8(gb->cpu_reg.bc.bytes.b, 0);
		break;

	case 0x91: /* SUB C */
		PGB_INSTR_SBC_R8(gb->cpu_reg.bc.bytes.c, 0);
		break;

	case 0x92: /* SUB D */
		PGB_INSTR_SBC_R8(gb->cpu_reg.de.bytes.d, 0);
		break;

	case 0x93: /* SUB E */
		PGB_INSTR_SBC_R8(gb->cpu_reg.de.bytes.e, 0);
		break;

	case 0x94: /* SUB H */
		PGB_INSTR_SBC_R8(gb->cpu_reg.hl.bytes.h, 0);
		break;

	case 0x95: /* SUB L */
		PGB_INSTR_SBC_R8(gb->cpu_reg.hl.bytes.l, 0);
		break;

	case 0x96: /* SUB (HL) */
		PGB_INSTR_SBC_R8(__gb_read(gb, gb->cpu_reg.hl.reg), 0);
		break;

	case 0x97: /* SUB A */
		gb->cpu_reg.a = 0;
		gb->cpu_reg.f.reg = 0;
		gb->cpu_reg.f.f_bits.z = 1;
		gb->cpu_reg.f.f_bits.n = 1;
		break;

	case 0x98: /* SBC A, B */
		PGB_INSTR_SBC_R8(gb->cpu_reg.bc.bytes.b, gb->cpu_reg.f.f_bits.c);
		break;

	case 0x99: /* SBC A, C */
		PGB_INSTR_SBC_R8(gb->cpu_reg.bc.bytes.c, gb->cpu_reg.f.f_bits.c);
		break;

	case 0x9A: /* SBC A, D */
		PGB_INSTR_SBC_R8(gb->cpu_reg.de.bytes.d, gb->cpu_reg.f.f_bits.c);
		break;

	case 0x9B: /* SBC A, E */
		PGB_INSTR_SBC_R8(gb->cpu_reg.de.bytes.e, gb->cpu_reg.f.f_bits.c);
		break;

	case 0x9C: /* SBC A, H */
		PGB_INSTR_SBC_R8(gb->cpu_reg.hl.bytes.h, gb->cpu_reg.f.f_bits.c);
		break;

	case 0x9D: /* SBC A, L */
		PGB_INSTR_SBC_R8(gb->cpu_reg.hl.bytes.l, gb->cpu_reg.f.f_bits.c);
		break;

	case 0x9E: /* SBC A, (HL) */
		PGB_INSTR_SBC_R8(__gb_read(gb, gb->cpu_reg.hl.reg), gb->cpu_reg.f.f_bits.c);
		break;

	case 0x9F: /* SBC A, A */
		gb->cpu_reg.a = gb->cpu_reg.f.f_bits.c ? 0xFF : 0x00;
		gb->cpu_reg.f.f_bits.z = !gb->cpu_reg.f.f_bits.c;
		gb->cpu_reg.f.f_bits.n = 1;
		gb->cpu_reg.f.f_bits.h = gb->cpu_reg.f.f_bits.c;
		break;

	case 0xA0: /* AND B */
		PGB_INSTR_AND_R8(gb->cpu_reg.bc.bytes.b);
		break;

	case 0xA1: /* AND C */
		PGB_INSTR_AND_R8(gb->cpu_reg.bc.bytes.c);
		break;

	case 0xA2: /* AND D */
		PGB_INSTR_AND_R8(gb->cpu_reg.de.bytes.d);
		break;

	case 0xA3: /* AND E */
		PGB_INSTR_AND_R8(gb->cpu_reg.de.bytes.e);
		break;

	case 0xA4: /* AND H */
		PGB_INSTR_AND_R8(gb->cpu_reg.hl.bytes.h);
		break;

	case 0xA5: /* AND L */
		PGB_INSTR_AND_R8(gb->cpu_reg.hl.bytes.l);
		break;

	case 0xA6: /* AND (HL) */
		PGB_INSTR_AND_R8(__gb_read(gb, gb->cpu_reg.hl.reg));
		break;

	case 0xA7: /* AND A */
		PGB_INSTR_AND_R8(gb->cpu_reg.a);
		break;

	case 0xA8: /* XOR B */
		PGB_INSTR_XOR_R8(gb->cpu_reg.bc.bytes.b);
		break;

	case 0xA9: /* XOR C */
		PGB_INSTR_XOR_R8(gb->cpu_reg.bc.bytes.c);
		break;

	case 0xAA: /* XOR D */
		PGB_INSTR_XOR_R8(gb->cpu_reg.de.bytes.d);
		break;

	case 0xAB: /* XOR E */
		PGB_INSTR_XOR_R8(gb->cpu_reg.de.bytes.e);
		break;

	case 0xAC: /* XOR H */
		PGB_INSTR_XOR_R8(gb->cpu_reg.hl.bytes.h);
		break;

	case 0xAD: /* XOR L */
		PGB_INSTR_XOR_R8(gb->cpu_reg.hl.bytes.l);
		break;

	case 0xAE: /* XOR (HL) */
		PGB_INSTR_XOR_R8(__gb_read(gb, gb->cpu_reg.hl.reg));
		break;

	case 0xAF: /* XOR A */
		PGB_INSTR_XOR_R8(gb->cpu_reg.a);
		break;

	case 0xB0: /* OR B */
		PGB_INSTR_OR_R8(gb->cpu_reg.bc.bytes.b);
		break;

	case 0xB1: /* OR C */
		PGB_INSTR_OR_R8(gb->cpu_reg.bc.bytes.c);
		break;

	case 0xB2: /* OR D */
		PGB_INSTR_OR_R8(gb->cpu_reg.de.bytes.d);
		break;

	case 0xB3: /* OR E */
		PGB_INSTR_OR_R8(gb->cpu_reg.de.bytes.e);
		break;

	case 0xB4: /* OR H */
		PGB_INSTR_OR_R8(gb->cpu_reg.hl.bytes.h);
		break;

	case 0xB5: /* OR L */
		PGB_INSTR_OR_R8(gb->cpu_reg.hl.bytes.l);
		break;

	case 0xB6: /* OR (HL) */
		PGB_INSTR_OR_R8(__gb_read(gb, gb->cpu_reg.hl.reg));
		break;

	case 0xB7: /* OR A */
		PGB_INSTR_OR_R8(gb->cpu_reg.a);
		break;

	case 0xB8: /* CP B */
		PGB_INSTR_CP_R8(gb->cpu_reg.bc.bytes.b);
		break;

	case 0xB9: /* CP C */
		PGB_INSTR_CP_R8(gb->cpu_reg.bc.bytes.c);
		break;

	case 0xBA: /* CP D */
		PGB_INSTR_CP_R8(gb->cpu_reg.de.bytes.d);
		break;

	case 0xBB: /* CP E */
		PGB_INSTR_CP_R8(gb->cpu_reg.de.bytes.e);
		break;

	case 0xBC: /* CP H */
		PGB_INSTR_CP_R8(gb->cpu_reg.hl.bytes.h);
		break;

	case 0xBD: /* CP L */
		PGB_INSTR_CP_R8(gb->cpu_reg.hl.bytes.l);
		break;

	case 0xBE: /* CP (HL) */
		PGB_INSTR_CP_R8(__gb_read(gb, gb->cpu_reg.hl.reg));
		break;

	case 0xBF: /* CP A */
		gb->cpu_reg.f.reg = 0;
		gb->cpu_reg.f.f_bits.z = 1;
		gb->cpu_reg.f.f_bits.n = 1;
		break;

	case 0xC0: /* RET NZ */
		if(!gb->cpu_reg.f.f_bits.z)
		{
			gb->cpu_reg.pc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
			gb->cpu_reg.pc.bytes.p = __gb_read(gb, gb->cpu_reg.sp.reg++);
			inst_cycles += 12;
		}

		break;

	case 0xC1: /* POP BC */
		gb->cpu_reg.bc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->cpu_reg.bc.bytes.b = __gb_read(gb, gb->cpu_reg.sp.reg++);
		break;

	case 0xC2: /* JP NZ, imm */
		if(!gb->cpu_reg.f.f_bits.z)
		{
			uint8_t p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xC3: /* JP imm */
	{
		uint8_t p, c;
		c = __gb_read(gb, gb->cpu_reg.pc.reg++);
		p = __gb_read(gb, gb->cpu_reg.pc.reg);
		gb->cpu_reg.pc.bytes.c = c;
		gb->cpu_reg.pc.bytes.p = p;
		break;
	}

	case 0xC4: /* CALL NZ imm */
		if(!gb->cpu_reg.f.f_bits.z)
		{
			uint8_t p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg++);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 12;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xC5: /* PUSH BC */
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.bc.bytes.b);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.bc.bytes.c);
		break;

	case 0xC6: /* ADD A, imm */
	{
		uint8_t val = __gb_read(gb, gb->cpu_reg.pc.reg++);
		PGB_INSTR_ADC_R8(val, 0);
		break;
	}

	case 0xC7: /* RST 0x0000 */
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0000;
		break;

	case 0xC8: /* RET Z */
		if(gb->cpu_reg.f.f_bits.z)
		{
			gb->cpu_reg.pc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
			gb->cpu_reg.pc.bytes.p = __gb_read(gb, gb->cpu_reg.sp.reg++);
			inst_cycles += 12;
		}
		break;

	case 0xC9: /* RET */
	{
		gb->cpu_reg.pc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->cpu_reg.pc.bytes.p = __gb_read(gb, gb->cpu_reg.sp.reg++);
		break;
	}

	case 0xCA: /* JP Z, imm */
		if(gb->cpu_reg.f.f_bits.z)
		{
			uint8_t p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xCB: /* CB INST */
		inst_cycles = __gb_execute_cb(gb);
		break;

	case 0xCC: /* CALL Z, imm */
		if(gb->cpu_reg.f.f_bits.z)
		{
			uint8_t p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg++);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 12;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xCD: /* CALL imm */
	{
		uint8_t p, c;
		c = __gb_read(gb, gb->cpu_reg.pc.reg++);
		p = __gb_read(gb, gb->cpu_reg.pc.reg++);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.bytes.c = c;
		gb->cpu_reg.pc.bytes.p = p;
	}
	break;

	case 0xCE: /* ADC A, imm */
	{
		uint8_t val = __gb_read(gb, gb->cpu_reg.pc.reg++);
		PGB_INSTR_ADC_R8(val, gb->cpu_reg.f.f_bits.c);
		break;
	}

	case 0xCF: /* RST 0x0008 */
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0008;
		break;

	case 0xD0: /* RET NC */
		if(!gb->cpu_reg.f.f_bits.c)
		{
			gb->cpu_reg.pc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
			gb->cpu_reg.pc.bytes.p = __gb_read(gb, gb->cpu_reg.sp.reg++);
			inst_cycles += 12;
		}

		break;

	case 0xD1: /* POP DE */
		gb->cpu_reg.de.bytes.e = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->cpu_reg.de.bytes.d = __gb_read(gb, gb->cpu_reg.sp.reg++);
		break;

	case 0xD2: /* JP NC, imm */
		if(!gb->cpu_reg.f.f_bits.c)
		{
			uint8_t p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xD4: /* CALL NC, imm */
		if(!gb->cpu_reg.f.f_bits.c)
		{
			uint8_t p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg++);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 12;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xD5: /* PUSH DE */
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.de.bytes.d);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.de.bytes.e);
		break;

	case 0xD6: /* SUB imm */
	{
		uint8_t val = __gb_read(gb, gb->cpu_reg.pc.reg++);
		uint16_t temp = gb->cpu_reg.a - val;
		gb->cpu_reg.f.f_bits.z = ((temp & 0xFF) == 0x00);
		gb->cpu_reg.f.f_bits.n = 1;
		gb->cpu_reg.f.f_bits.h =
			(gb->cpu_reg.a ^ val ^ temp) & 0x10 ? 1 : 0;
		gb->cpu_reg.f.f_bits.c = (temp & 0xFF00) ? 1 : 0;
		gb->cpu_reg.a = (temp & 0xFF);
		break;
	}

	case 0xD7: /* RST 0x0010 */
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0010;
		break;

	case 0xD8: /* RET C */
		if(gb->cpu_reg.f.f_bits.c)
		{
			gb->cpu_reg.pc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
			gb->cpu_reg.pc.bytes.p = __gb_read(gb, gb->cpu_reg.sp.reg++);
			inst_cycles += 12;
		}

		break;

	case 0xD9: /* RETI */
	{
		gb->cpu_reg.pc.bytes.c = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->cpu_reg.pc.bytes.p = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->gb_ime = true;
	}
	break;

	case 0xDA: /* JP C, imm */
		if(gb->cpu_reg.f.f_bits.c)
		{
			uint8_t p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 4;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xDC: /* CALL C, imm */
		if(gb->cpu_reg.f.f_bits.c)
		{
			uint8_t p, c;
			c = __gb_read(gb, gb->cpu_reg.pc.reg++);
			p = __gb_read(gb, gb->cpu_reg.pc.reg++);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
			__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
			gb->cpu_reg.pc.bytes.c = c;
			gb->cpu_reg.pc.bytes.p = p;
			inst_cycles += 12;
		}
		else
			gb->cpu_reg.pc.reg += 2;

		break;

	case 0xDE: /* SBC A, imm */
	{
		uint8_t val = __gb_read(gb, gb->cpu_reg.pc.reg++);
		PGB_INSTR_SBC_R8(val, gb->cpu_reg.f.f_bits.c);
		break;
	}

	case 0xDF: /* RST 0x0018 */
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0018;
		break;

	case 0xE0: /* LD (0xFF00+imm), A */
		__gb_write(gb, 0xFF00 | __gb_read(gb, gb->cpu_reg.pc.reg++),
			   gb->cpu_reg.a);
		break;

	case 0xE1: /* POP HL */
		gb->cpu_reg.hl.bytes.l = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->cpu_reg.hl.bytes.h = __gb_read(gb, gb->cpu_reg.sp.reg++);
		break;

	case 0xE2: /* LD (C), A */
		__gb_write(gb, 0xFF00 | gb->cpu_reg.bc.bytes.c, gb->cpu_reg.a);
		break;

	case 0xE5: /* PUSH HL */
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.hl.bytes.h);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.hl.bytes.l);
		break;

	case 0xE6: /* AND imm */
	{
		uint8_t temp = __gb_read(gb, gb->cpu_reg.pc.reg++);
		PGB_INSTR_AND_R8(temp);
		break;
	}

	case 0xE7: /* RST 0x0020 */
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0020;
		break;

	case 0xE8: /* ADD SP, imm */
	{
		int8_t offset = (int8_t) __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.f.reg = 0;
		gb->cpu_reg.f.f_bits.h = ((gb->cpu_reg.sp.reg & 0xF) + (offset & 0xF) > 0xF) ? 1 : 0;
		gb->cpu_reg.f.f_bits.c = ((gb->cpu_reg.sp.reg & 0xFF) + (offset & 0xFF) > 0xFF);
		gb->cpu_reg.sp.reg += offset;
		break;
	}

	case 0xE9: /* JP (HL) */
		gb->cpu_reg.pc.reg = gb->cpu_reg.hl.reg;
		break;

	case 0xEA: /* LD (imm), A */
	{
		uint8_t h, l;
		uint16_t addr;
		l = __gb_read(gb, gb->cpu_reg.pc.reg++);
		h = __gb_read(gb, gb->cpu_reg.pc.reg++);
		addr = PEANUT_GB_U8_TO_U16(h, l);
		__gb_write(gb, addr, gb->cpu_reg.a);
		break;
	}

	case 0xEE: /* XOR imm */
		PGB_INSTR_XOR_R8(__gb_read(gb, gb->cpu_reg.pc.reg++));
		break;

	case 0xEF: /* RST 0x0028 */
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0028;
		break;

	case 0xF0: /* LD A, (0xFF00+imm) */
		gb->cpu_reg.a =
			__gb_read(gb, 0xFF00 | __gb_read(gb, gb->cpu_reg.pc.reg++));
		break;

	case 0xF1: /* POP AF */
	{
		uint8_t temp_8 = __gb_read(gb, gb->cpu_reg.sp.reg++);
		gb->cpu_reg.f.f_bits.z = (temp_8 >> 7) & 1;
		gb->cpu_reg.f.f_bits.n = (temp_8 >> 6) & 1;
		gb->cpu_reg.f.f_bits.h = (temp_8 >> 5) & 1;
		gb->cpu_reg.f.f_bits.c = (temp_8 >> 4) & 1;
		gb->cpu_reg.a = __gb_read(gb, gb->cpu_reg.sp.reg++);
		break;
	}

	case 0xF2: /* LD A, (C) */
		gb->cpu_reg.a = __gb_read(gb, 0xFF00 | gb->cpu_reg.bc.bytes.c);
		break;

	case 0xF3: /* DI */
		gb->gb_ime = false;
		break;

	case 0xF5: /* PUSH AF */
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.a);
		__gb_write(gb, --gb->cpu_reg.sp.reg,
			   gb->cpu_reg.f.f_bits.z << 7 | gb->cpu_reg.f.f_bits.n << 6 |
			   gb->cpu_reg.f.f_bits.h << 5 | gb->cpu_reg.f.f_bits.c << 4);
		break;

	case 0xF6: /* OR imm */
		PGB_INSTR_OR_R8(__gb_read(gb, gb->cpu_reg.pc.reg++));
		break;

	case 0xF7: /* PUSH AF */
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0030;
		break;

	case 0xF8: /* LD HL, SP+/-imm */
	{
		/* Taken from SameBoy, which is released under MIT Licence. */
		int8_t offset = (int8_t) __gb_read(gb, gb->cpu_reg.pc.reg++);
		gb->cpu_reg.hl.reg = gb->cpu_reg.sp.reg + offset;
		gb->cpu_reg.f.reg = 0;
		gb->cpu_reg.f.f_bits.h = ((gb->cpu_reg.sp.reg & 0xF) + (offset & 0xF) > 0xF) ? 1 : 0;
		gb->cpu_reg.f.f_bits.c = ((gb->cpu_reg.sp.reg & 0xFF) + (offset & 0xFF) > 0xFF) ? 1 : 0;
		break;
	}

	case 0xF9: /* LD SP, HL */
		gb->cpu_reg.sp.reg = gb->cpu_reg.hl.reg;
		break;

	case 0xFA: /* LD A, (imm) */
	{
		uint8_t h, l;
		uint16_t addr;
		l = __gb_read(gb, gb->cpu_reg.pc.reg++);
		h = __gb_read(gb, gb->cpu_reg.pc.reg++);
		addr = PEANUT_GB_U8_TO_U16(h, l);
		gb->cpu_reg.a = __gb_read(gb, addr);
		break;
	}

	case 0xFB: /* EI */
		gb->gb_ime = true;
		break;

	case 0xFE: /* CP imm */
	{
		uint8_t val = __gb_read(gb, gb->cpu_reg.pc.reg++);
		PGB_INSTR_CP_R8(val);
		break;
	}

	case 0xFF: /* RST 0x0038 */
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.p);
		__gb_write(gb, --gb->cpu_reg.sp.reg, gb->cpu_reg.pc.bytes.c);
		gb->cpu_reg.pc.reg = 0x0038;
		break;

	default:
		/* Return address where invalid opcode that was read. */
		(gb->gb_error)(gb, GB_INVALID_OPCODE, gb->cpu_reg.pc.reg - 1);
		PGB_UNREACHABLE();
	}

	do
	{
		/* DIV register timing */
		gb->counter.div_count += inst_cycles;
		while(gb->counter.div_count >= DIV_CYCLES)
		{
			gb->hram_io[IO_DIV]++;
			gb->counter.div_count -= DIV_CYCLES;
		}

		/* Check for RTC tick. */
		if(gb->mbc == 3 && (gb->rtc_real.reg.high & 0x40) == 0)
		{
			gb->counter.rtc_count += inst_cycles;
			while(PGB_UNLIKELY(gb->counter.rtc_count >= RTC_CYCLES))
			{
				gb->counter.rtc_count -= RTC_CYCLES;

				/* Detect invalid rollover. */
				if(PGB_UNLIKELY(gb->rtc_real.reg.sec == 63))
				{
					gb->rtc_real.reg.sec = 0;
					continue;
				}

				if(++gb->rtc_real.reg.sec != 60)
					continue;

				gb->rtc_real.reg.sec = 0;
				if(gb->rtc_real.reg.min == 63)
				{
					gb->rtc_real.reg.min = 0;
					continue;
				}
				if(++gb->rtc_real.reg.min != 60)
					continue;

				gb->rtc_real.reg.min = 0;
				if(gb->rtc_real.reg.hour == 31)
				{
					gb->rtc_real.reg.hour = 0;
					continue;
				}
				if(++gb->rtc_real.reg.hour != 24)
					continue;

				gb->rtc_real.reg.hour = 0;
				if(++gb->rtc_real.reg.yday != 0)
					continue;

				if(gb->rtc_real.reg.high & 1)  /* Bit 8 of days*/
					gb->rtc_real.reg.high |= 0x80; /* Overflow bit */

				gb->rtc_real.reg.high ^= 1;
			}
		}

		/* Check serial transmission. */
		if(gb->hram_io[IO_SC] & SERIAL_SC_TX_START)
		{
			/* If new transfer, call TX function. */
			if(gb->counter.serial_count == 0 &&
				gb->gb_serial_tx != NULL)
				(gb->gb_serial_tx)(gb, gb->hram_io[IO_SB]);

			gb->counter.serial_count += inst_cycles;

			/* If it's time to receive byte, call RX function. */
			if(gb->counter.serial_count >= SERIAL_CYCLES)
			{
				/* If RX can be done, do it. */
				/* If RX failed, do not change SB if using external
				 * clock, or set to 0xFF if using internal clock. */
				uint8_t rx;

				if(gb->gb_serial_rx != NULL &&
					(gb->gb_serial_rx(gb, &rx) ==
						GB_SERIAL_RX_SUCCESS))
				{
					gb->hram_io[IO_SB] = rx;

					/* Inform game of serial TX/RX completion. */
					gb->hram_io[IO_SC] &= 0x01;
					gb->hram_io[IO_IF] |= SERIAL_INTR;
				}
				else if(gb->hram_io[IO_SC] & SERIAL_SC_CLOCK_SRC)
				{
					/* If using internal clock, and console is not
					 * attached to any external peripheral, shifted
					 * bits are replaced with logic 1. */
					gb->hram_io[IO_SB] = 0xFF;

					/* Inform game of serial TX/RX completion. */
					gb->hram_io[IO_SC] &= 0x01;
					gb->hram_io[IO_IF] |= SERIAL_INTR;
				}
				else
				{
					/* If using external clock, and console is not
					 * attached to any external peripheral, bits are
					 * not shifted, so SB is not modified. */
				}

				gb->counter.serial_count = 0;
			}
		}

		/* TIMA register timing */
		/* TODO: Change tac_enable to struct of TAC timer control bits. */
		if(gb->hram_io[IO_TAC] & IO_TAC_ENABLE_MASK)
		{
			gb->counter.tima_count += inst_cycles;

			while(gb->counter.tima_count >=
				TAC_CYCLES[gb->hram_io[IO_TAC] & IO_TAC_RATE_MASK])
			{
				gb->counter.tima_count -=
					TAC_CYCLES[gb->hram_io[IO_TAC] & IO_TAC_RATE_MASK];

				if(++gb->hram_io[IO_TIMA] == 0)
				{
					gb->hram_io[IO_IF] |= TIMER_INTR;
					/* On overflow, set TMA to TIMA. */
					gb->hram_io[IO_TIMA] = gb->hram_io[IO_TMA];
				}
			}
		}

		/* If LCD is off, don't update LCD state or increase the LCD
		 * ticks. Instead, keep track of the amount of time that is
		 * being passed. */
		if(!(gb->hram_io[IO_LCDC] & LCDC_ENABLE))
		{
			gb->counter.lcd_off_count += inst_cycles;
			if(gb->counter.lcd_off_count >= LCD_FRAME_CYCLES)
			{
				gb->counter.lcd_off_count -= LCD_FRAME_CYCLES;
				gb->gb_frame = true;
			}
			continue;
		}

		/* LCD Timing */
		gb->counter.lcd_count += inst_cycles;

		/* New Scanline. HBlank -> VBlank or OAM Scan */
		if(gb->counter.lcd_count >= LCD_LINE_CYCLES)
		{
			gb->counter.lcd_count -= LCD_LINE_CYCLES;

			/* Next line */
			gb->hram_io[IO_LY] = gb->hram_io[IO_LY] + 1;
			if (gb->hram_io[IO_LY] == LCD_VERT_LINES)
				gb->hram_io[IO_LY] = 0;

			/* LYC Update */
			if(gb->hram_io[IO_LY] == gb->hram_io[IO_LYC])
			{
				gb->hram_io[IO_STAT] |= STAT_LYC_COINC;

				if(gb->hram_io[IO_STAT] & STAT_LYC_INTR)
					gb->hram_io[IO_IF] |= LCDC_INTR;
			}
			else
				gb->hram_io[IO_STAT] &= 0xFB;

			/* Check if LCD should be in Mode 1 (VBLANK) state */
			if(gb->hram_io[IO_LY] == LCD_HEIGHT)
			{
				gb->hram_io[IO_STAT] =
					(gb->hram_io[IO_STAT] & ~STAT_MODE) | IO_STAT_MODE_VBLANK;
				gb->gb_frame = true;
				gb->hram_io[IO_IF] |= VBLANK_INTR;
				gb->lcd_blank = false;

				if(gb->hram_io[IO_STAT] & STAT_MODE_1_INTR)
					gb->hram_io[IO_IF] |= LCDC_INTR;

#if ENABLE_LCD
				/* If frame skip is activated, check if we need to draw
				 * the frame or skip it. */
				if(gb->direct.frame_skip)
				{
					gb->display.frame_skip_count =
						!gb->display.frame_skip_count;
				}

				/* If interlaced is activated, change which lines get
				 * updated. Also, only update lines on frames that are
				 * actually drawn when frame skip is enabled. */
				if(gb->direct.interlace &&
						(!gb->direct.frame_skip ||
						 gb->display.frame_skip_count))
				{
					gb->display.interlace_count =
						!gb->display.interlace_count;
				}
#endif
                                /* If halted forever, then return on VBLANK. */
                                if(gb->gb_halt && !gb->hram_io[IO_IE])
					break;
			}
			/* Start of normal Line (not in VBLANK) */
			else if(gb->hram_io[IO_LY] < LCD_HEIGHT)
			{
				if(gb->hram_io[IO_LY] == 0)
				{
					/* Clear Screen */
					gb->display.WY = gb->hram_io[IO_WY];
					gb->display.window_clear = 0;
				}

				/* OAM Search occurs at the start of the line. */
				gb->hram_io[IO_STAT] = (gb->hram_io[IO_STAT] & ~STAT_MODE) | IO_STAT_MODE_OAM_SCAN;
				gb->counter.lcd_count = 0;

				if(gb->hram_io[IO_STAT] & STAT_MODE_2_INTR)
					gb->hram_io[IO_IF] |= LCDC_INTR;

				/* If halted immediately jump to next LCD mode.
				 * From OAM Search to LCD Draw. */
				//if(gb->counter.lcd_count < LCD_MODE2_OAM_SCAN_END)
				//	inst_cycles = LCD_MODE2_OAM_SCAN_END - gb->counter.lcd_count;
				inst_cycles = LCD_MODE2_OAM_SCAN_DURATION;
			}
		}
		/* Go from Mode 3 (LCD Draw) to Mode 0 (HBLANK). */
		else if((gb->hram_io[IO_STAT] & STAT_MODE) == IO_STAT_MODE_LCD_DRAW &&
				gb->counter.lcd_count >= LCD_MODE3_LCD_DRAW_END)
		{
			gb->hram_io[IO_STAT] = (gb->hram_io[IO_STAT] & ~STAT_MODE) | IO_STAT_MODE_HBLANK;

			if(gb->hram_io[IO_STAT] & STAT_MODE_0_INTR)
				gb->hram_io[IO_IF] |= LCDC_INTR;

			/* If halted immediately, jump from OAM Scan to LCD Draw. */
			if (gb->counter.lcd_count < LCD_MODE0_HBLANK_MAX_DRUATION)
				inst_cycles = LCD_MODE0_HBLANK_MAX_DRUATION - gb->counter.lcd_count;
		}
		/* Go from Mode 2 (OAM Scan) to Mode 3 (LCD Draw). */
		else if((gb->hram_io[IO_STAT] & STAT_MODE) == IO_STAT_MODE_OAM_SCAN &&
				gb->counter.lcd_count >= LCD_MODE2_OAM_SCAN_END)
		{
			gb->hram_io[IO_STAT] = (gb->hram_io[IO_STAT] & ~STAT_MODE) | IO_STAT_MODE_LCD_DRAW;
#if ENABLE_LCD
			if(!gb->lcd_blank)
				__gb_draw_line(gb);
#endif
			/* If halted immediately jump to next LCD mode. */
			if (gb->counter.lcd_count < LCD_MODE3_LCD_DRAW_MIN_DURATION)
				inst_cycles = LCD_MODE3_LCD_DRAW_MIN_DURATION - gb->counter.lcd_count;
		}
	} while(gb->gb_halt && (gb->hram_io[IO_IF] & gb->hram_io[IO_IE]) == 0);
	/* If halted, loop until an interrupt occurs. */
}

void gb_run_frame(struct gb_s *gb)
{
	gb->gb_frame = false;

	while(!gb->gb_frame)
		__gb_step_cpu(gb);
}

int gb_get_save_size_s(struct gb_s *gb, size_t *ram_size)
{
	const uint_fast16_t ram_size_location = 0x0149;
	const uint_fast32_t ram_sizes[] =
	{
		/* 0,  2KiB,   8KiB,  32KiB,  128KiB,   64KiB */
		0x00, 0x800, 0x2000, 0x8000, 0x20000, 0x10000
	};
	uint8_t ram_size_code = gb->gb_rom_read(gb, ram_size_location);

	/* MBC2 always has 512 half-bytes of cart RAM.
	 * This assumes that only the lower nibble of each byte is used; the
	 * nibbles are not packed. */
	if(gb->mbc == 2)
	{
		*ram_size = 0x200;
		return 0;
	}

	/* Return -1 on invalid or unsupported RAM size. */
	if(ram_size_code >= PEANUT_GB_ARRAYSIZE(ram_sizes))
		return -1;

	*ram_size = ram_sizes[ram_size_code];
	return 0;
}

PGB_DEPRECATED("Does not return error code. Use gb_get_save_size_s instead.")
uint_fast32_t gb_get_save_size(struct gb_s *gb)
{
	const uint_fast16_t ram_size_location = 0x0149;
	const uint_fast32_t ram_sizes[] =
	{
		/* 0,  2KiB,   8KiB,  32KiB,  128KiB,   64KiB */
		0x00, 0x800, 0x2000, 0x8000, 0x20000, 0x10000
	};
	uint8_t ram_size_code = gb->gb_rom_read(gb, ram_size_location);

	/* MBC2 always has 512 half-bytes of cart RAM.
	 * This assumes that only the lower nibble of each byte is used; the
	 * nibbles are not packed. */
	if(gb->mbc == 2)
		return 0x200;

	/* Return 0 on invalid or unsupported RAM size. */
	if(ram_size_code >= PEANUT_GB_ARRAYSIZE(ram_sizes))
		return 0;

	return ram_sizes[ram_size_code];
}

void gb_init_serial(struct gb_s *gb,
		    void (*gb_serial_tx)(struct gb_s*, const uint8_t),
		    enum gb_serial_rx_ret_e (*gb_serial_rx)(struct gb_s*,
			    uint8_t*))
{
	gb->gb_serial_tx = gb_serial_tx;
	gb->gb_serial_rx = gb_serial_rx;
}

uint8_t gb_colour_hash(struct gb_s *gb)
{
#define ROM_TITLE_START_ADDR	0x0134
#define ROM_TITLE_END_ADDR	0x0143

	uint8_t x = 0;
	uint16_t i;

	for(i = ROM_TITLE_START_ADDR; i <= ROM_TITLE_END_ADDR; i++)
		x += gb->gb_rom_read(gb, i);

	return x;
}

/**
 * Resets the context, and initialises startup values for a DMG console.
 */
void gb_reset(struct gb_s *gb)
{
	gb->gb_halt = false;
	gb->gb_ime = true;

	/* Initialise MBC values. */
	gb->selected_rom_bank = 1;
	gb->cart_ram_bank = 0;
	gb->enable_cart_ram = 0;
	gb->cart_mode_select = 0;

	/* Use values as though the boot ROM was already executed. */
	if(gb->gb_bootrom_read == NULL)
	{
		uint8_t hdr_chk;
		hdr_chk = gb->gb_rom_read(gb, ROM_HEADER_CHECKSUM_LOC) != 0;

		gb->cpu_reg.a = 0x01;
		gb->cpu_reg.f.f_bits.z = 1;
		gb->cpu_reg.f.f_bits.n = 0;
		gb->cpu_reg.f.f_bits.h = hdr_chk;
		gb->cpu_reg.f.f_bits.c = hdr_chk;
		gb->cpu_reg.bc.reg = 0x0013;
		gb->cpu_reg.de.reg = 0x00D8;
		gb->cpu_reg.hl.reg = 0x014D;
		gb->cpu_reg.sp.reg = 0xFFFE;
		gb->cpu_reg.pc.reg = 0x0100;

		gb->hram_io[IO_DIV ] = 0xAB;
		gb->hram_io[IO_LCDC] = 0x91;
		gb->hram_io[IO_STAT] = 0x85;
		gb->hram_io[IO_BOOT] = 0x01;

		__gb_write(gb, 0xFF26, 0xF1);

		memset(gb->vram, 0x00, VRAM_SIZE);
	}
	else
	{
		/* Set value as though the console was just switched on.
		 * CPU registers are uninitialised. */
		gb->cpu_reg.pc.reg = 0x0000;
		gb->hram_io[IO_DIV ] = 0x00;
		gb->hram_io[IO_LCDC] = 0x00;
		gb->hram_io[IO_STAT] = 0x84;
		gb->hram_io[IO_BOOT] = 0x00;
	}

	gb->counter.lcd_count = 0;
	gb->counter.div_count = 0;
	gb->counter.tima_count = 0;
	gb->counter.serial_count = 0;
	gb->counter.rtc_count = 0;
	gb->counter.lcd_off_count = 0;

	gb->direct.joypad = 0xFF;
	gb->hram_io[IO_JOYP] = 0xCF;
	gb->hram_io[IO_SB  ] = 0x00;
	gb->hram_io[IO_SC  ] = 0x7E;
	/* DIV */
	gb->hram_io[IO_TIMA] = 0x00;
	gb->hram_io[IO_TMA ] = 0x00;
	gb->hram_io[IO_TAC ] = 0xF8;
	gb->hram_io[IO_IF  ] = 0xE1;

	/* LCDC */
	/* STAT */
	gb->hram_io[IO_SCY ] = 0x00;
	gb->hram_io[IO_SCX ] = 0x00;
	gb->hram_io[IO_LY  ] = 0x00;
	gb->hram_io[IO_LYC ] = 0x00;
	__gb_write(gb, 0xFF47, 0xFC); // BGP
	__gb_write(gb, 0xFF48, 0xFF); // OBJP0
	__gb_write(gb, 0xFF49, 0xFF); // OBJP1
	gb->hram_io[IO_WY] = 0x00;
	gb->hram_io[IO_WX] = 0x00;
	gb->hram_io[IO_IE] = 0x00;
	gb->hram_io[IO_IF] = 0xE1;
}

enum gb_init_error_e gb_init(struct gb_s *gb,
			     uint8_t (*gb_rom_read)(struct gb_s*, const uint_fast32_t),
			     uint8_t (*gb_cart_ram_read)(struct gb_s*, const uint_fast32_t),
			     void (*gb_cart_ram_write)(struct gb_s*, const uint_fast32_t, const uint8_t),
			     void (*gb_error)(struct gb_s*, const enum gb_error_e, const uint16_t),
			     void *priv)
{
	const uint16_t mbc_location = 0x0147;
	const uint16_t bank_count_location = 0x0148;
	const uint16_t ram_size_location = 0x0149;
	/**
	 * Table for cartridge type (MBC). -1 if invalid.
	 * TODO: MMM01 is untested.
	 * TODO: MBC6 is untested.
	 * TODO: MBC7 is unsupported.
	 * TODO: POCKET CAMERA is unsupported.
	 * TODO: BANDAI TAMA5 is unsupported.
	 * TODO: HuC3 is unsupported.
	 * TODO: HuC1 is unsupported.
	 **/
	const int8_t cart_mbc[] =
	{
		0, 1, 1, 1, -1, 2, 2, -1, 0, 0, -1, 0, 0, 0, -1, 3,
		3, 3, 3, 3, -1, -1, -1, -1, -1, 5, 5, 5, 5, 5, 5, -1
	};
	/* Whether cart has RAM. */
	const uint8_t cart_ram[] =
	{
		0, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0,
		1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0
	};
	/* How large the ROM is in banks of 16 KiB. */
	const uint16_t num_rom_banks_mask[] =
	{
		2, 4, 8, 16, 32, 64, 128, 256, 512
	};
	/* How large the cart RAM is in banks of 8 KiB. Code $01 is unused, but
	 * some early homebrew ROMs supposedly may use this value. */
	const uint8_t num_ram_banks[] = { 0, 1, 1, 4, 16, 8 };

	gb->gb_rom_read = gb_rom_read;
	gb->gb_cart_ram_read = gb_cart_ram_read;
	gb->gb_cart_ram_write = gb_cart_ram_write;
	gb->gb_error = gb_error;
	gb->direct.priv = priv;

	/* Initialise serial transfer function to NULL. If the front-end does
	 * not provide serial support, Peanut-GB will emulate no cable connected
	 * automatically. */
	gb->gb_serial_tx = NULL;
	gb->gb_serial_rx = NULL;

	gb->gb_bootrom_read = NULL;

	/* Check valid ROM using checksum value. */
	{
		uint8_t x = 0;
		uint16_t i;

		for(i = 0x0134; i <= 0x014C; i++)
			x = x - gb->gb_rom_read(gb, i) - 1;

		if(x != gb->gb_rom_read(gb, ROM_HEADER_CHECKSUM_LOC))
			return GB_INIT_INVALID_CHECKSUM;
	}

	/* Check if cartridge type is supported, and set MBC type. */
	{
		const uint8_t mbc_value = gb->gb_rom_read(gb, mbc_location);

		if(mbc_value > sizeof(cart_mbc) - 1 ||
				(gb->mbc = cart_mbc[mbc_value]) == -1)
			return GB_INIT_CARTRIDGE_UNSUPPORTED;
	}

	gb->num_rom_banks_mask = num_rom_banks_mask[gb->gb_rom_read(gb, bank_count_location)] - 1;
	gb->cart_ram = cart_ram[gb->gb_rom_read(gb, mbc_location)];
	gb->num_ram_banks = num_ram_banks[gb->gb_rom_read(gb, ram_size_location)];

	/* If the ROM says that it support RAM, but has 0 RAM banks, then
	 * disable RAM reads from the cartridge. */
	if(gb->cart_ram == 0 || gb->num_ram_banks == 0)
	{
		gb->cart_ram = 0;
		gb->num_ram_banks = 0;
	}

	/* If MBC3 and number of ROM or RAM banks are larger than 128 or 8,
	 * respectively, then select MBC3O mode. */
	if(gb->mbc == 3)
		gb->cart_is_mbc3O = gb->num_rom_banks_mask > 128 || gb->num_ram_banks > 4;

	/* Note that MBC2 will appear to have no RAM banks, but it actually
	 * always has 512 half-bytes of RAM. Hence, gb->num_ram_banks must be
	 * ignored for MBC2. */

	gb->lcd_blank = false;
	gb->display.lcd_draw_line = NULL;

	gb_reset(gb);

	return GB_INIT_NO_ERROR;
}

const char* gb_get_rom_name(struct gb_s* gb, char *title_str)
{
	uint_fast16_t title_loc = 0x134;
	/* End of title may be 0x13E for newer games. */
	const uint_fast16_t title_end = 0x143;
	const char* title_start = title_str;

	for(; title_loc <= title_end; title_loc++)
	{
		const char title_char = gb->gb_rom_read(gb, title_loc);

		if(title_char >= ' ' && title_char <= '_')
		{
			*title_str = title_char;
			title_str++;
		}
		else
			break;
	}

	*title_str = '\0';
	return title_start;
}

#if ENABLE_LCD
void gb_init_lcd(struct gb_s *gb,
		void (*lcd_draw_line)(struct gb_s *gb,
			const uint8_t *pixels,
			const uint_fast8_t line))
{
	gb->display.lcd_draw_line = lcd_draw_line;

	gb->direct.interlace = false;
	gb->display.interlace_count = false;
	gb->direct.frame_skip = false;
	gb->display.frame_skip_count = false;

	gb->display.window_clear = 0;
	gb->display.WY = 0;

	return;
}
#endif

void gb_set_bootrom(struct gb_s *gb,
		 uint8_t (*gb_bootrom_read)(struct gb_s*, const uint_fast16_t))
{
	gb->gb_bootrom_read = gb_bootrom_read;
}

/**
 * Deprecated. Will be removed in the next major version.
 */
PGB_DEPRECATED("RTC is now ticked internally; this function has no effect")
void gb_tick_rtc(struct gb_s *gb)
{
	(void) gb;
	return;
}

void gb_set_rtc(struct gb_s *gb, const struct tm * const time)
{
	gb->rtc_real.bytes[0] = time->tm_sec;
	gb->rtc_real.bytes[1] = time->tm_min;
	gb->rtc_real.bytes[2] = time->tm_hour;
	gb->rtc_real.bytes[3] = time->tm_yday & 0xFF; /* Low 8 bits of day counter. */
	gb->rtc_real.bytes[4] = time->tm_yday >> 8; /* High 1 bit of day counter. */
}
#endif // PEANUT_GB_HEADER_ONLY

/** Function prototypes: Required functions **/
/**
 * Initialises the emulator context to a known state. Call this before calling
 * any other peanut-gb function.
 * To reset the emulator, you can call gb_reset() instead.
 *
 * \param gb	Allocated emulator context. Must not be NULL.
 * \param gb_rom_read Pointer to function that reads ROM data. ROM banking is
 * 		already handled by Peanut-GB. Must not be NULL.
 * \param gb_cart_ram_read Pointer to function that reads Cart RAM. Must not be
 * 		NULL.
 * \param gb_cart_ram_write Pointer to function to writes to Cart RAM. Must not
 * 		be NULL.
 * \param gb_error Pointer to function that is called when an unrecoverable
 *		error occurs. Must not be NULL. Returning from this
 *		function is undefined and will result in SIGABRT.
 * \param priv	Private data that is stored within the emulator context. Set to
 * 		NULL if unused.
 * \returns	0 on success or an enum that describes the error.
 */
enum gb_init_error_e gb_init(struct gb_s *gb,
			     uint8_t (*gb_rom_read)(struct gb_s*, const uint_fast32_t),
			     uint8_t (*gb_cart_ram_read)(struct gb_s*, const uint_fast32_t),
			     void (*gb_cart_ram_write)(struct gb_s*, const uint_fast32_t, const uint8_t),
			     void (*gb_error)(struct gb_s*, const enum gb_error_e, const uint16_t),
			     void *priv);

/**
 * Executes the emulator and runs for the duration of time equal to one frame.
 *
 * \param	An initialised emulator context. Must not be NULL.
 */
void gb_run_frame(struct gb_s *gb);

/**
 * Internal function used to step the CPU. Used mainly for testing.
 * Use gb_run_frame() instead.
 *
 * \param	An initialised emulator context. Must not be NULL.
 */
void __gb_step_cpu(struct gb_s *gb);

/** Function prototypes: Optional Functions **/
/**
 * Reset the emulator, like turning the Game Boy off and on again.
 * This function can be called at any time.
 *
 * \param	An initialised emulator context. Must not be NULL.
 */
void gb_reset(struct gb_s *gb);

/**
 * Initialises the display context of the emulator. Only available when
 * ENABLE_LCD is defined to a non-zero value.
 * The pixel data sent to lcd_draw_line comes with both shade and layer data.
 * The first two least significant bits are the shade data (black, dark, light,
 * white). Bits 4 and 5 are layer data (OBJ0, OBJ1, BG), which can be used to
 * add more colours to the game in the same way that the Game Boy Color does to
 * older Game Boy games.
 * This function can be called at any time.
 *
 * \param gb	An initialised emulator context. Must not be NULL.
 * \param lcd_draw_line Pointer to function that draws the 2-bit pixel data on the line
 *		"line". Must not be NULL.
 */
#if ENABLE_LCD
void gb_init_lcd(struct gb_s *gb,
		void (*lcd_draw_line)(struct gb_s *gb,
			const uint8_t *pixels,
			const uint_fast8_t line));
#endif

/**
 * Initialises the serial connection of the emulator. This function is optional,
 * and if not called, the emulator will assume that no link cable is connected
 * to the game.
 *
 * \param gb	An initialised emulator context. Must not be NULL.
 * \param gb_serial_tx Pointer to function that transmits a byte of data over
 *		the serial connection. Must not be NULL.
 * \param gb_serial_rx Pointer to function that receives a byte of data over the
 *		serial connection. If no byte is received,
 *		return GB_SERIAL_RX_NO_CONNECTION. Must not be NULL.
 */
void gb_init_serial(struct gb_s *gb,
		    void (*gb_serial_tx)(struct gb_s*, const uint8_t),
		    enum gb_serial_rx_ret_e (*gb_serial_rx)(struct gb_s*,
			    uint8_t*));

/**
 * Obtains the save size of the game (size of the Cart RAM). Required by the
 * frontend to allocate enough memory for the Cart RAM.
 *
 * \param gb	An initialised emulator context. Must not be NULL.
 * \param ram_size Pointer to size_t variable that will be set to the size of
 *		the Cart RAM in bytes. Must not be NULL.
 *		If the Cart RAM is not battery backed, this will be set to 0.
 *		If the Cart RAM size is invalid or unknown, this will not be
 *		set.
 * \returns	0 on success, or -1 if the RAM size is invalid or unknown.
 */
int gb_get_save_size_s(struct gb_s *gb, size_t *ram_size);

/**
 * Deprecated. Use gb_get_save_size_s() instead.
 * Obtains the save size of the game (size of the Cart RAM). Required by the
 * frontend to allocate enough memory for the Cart RAM.
 *
 * \param gb	An initialised emulator context. Must not be NULL.
 * \returns	Size of the Cart RAM in bytes. 0 if Cartridge has not battery
 *		backed RAM.
 *		0 is also returned on invalid or unknown RAM size.
 */
uint_fast32_t gb_get_save_size(struct gb_s *gb);

/**
 * Calculates and returns a hash of the game header in the same way the Game
 * Boy Color does for colourising old Game Boy games. The frontend can use this
 * hash to automatically set a colour palette.
 *
 * \param gb	An initialised emulator context. Must not be NULL.
 * \returns	Hash of the game header.
 */
uint8_t gb_colour_hash(struct gb_s *gb);

/**
 * Returns the title of ROM.
 *
 * \param gb	An initialised emulator context. Must not be NULL.
 * \param title_str Allocated string at least 16 characters.
 * \returns	Pointer to start of string, null terminated.
 */
const char* gb_get_rom_name(struct gb_s* gb, char *title_str);

/**
 * Deprecated. Will be removed in the next major version.
 * RTC is ticked internally and this function has no effect.
 */
void gb_tick_rtc(struct gb_s *gb);

/**
 * Set initial values in RTC.
 * Should be called after gb_init().
 *
 * \param gb	An initialised emulator context. Must not be NULL.
 * \param time	Time structure with date and time.
 */
void gb_set_rtc(struct gb_s *gb, const struct tm * const time);

/**
 * Use boot ROM on reset. gb_reset() must be called for this to take affect.
 * \param gb 	An initialised emulator context. Must not be NULL.
 * \param gb_bootrom_read Function pointer to read boot ROM binary.
 */
void gb_set_bootrom(struct gb_s *gb,
	uint8_t (*gb_bootrom_read)(struct gb_s*, const uint_fast16_t));

/* Undefine CPU Flag helper functions. */
#undef PEANUT_GB_CPUFLAG_MASK_CARRY
#undef PEANUT_GB_CPUFLAG_MASK_HALFC
#undef PEANUT_GB_CPUFLAG_MASK_ARITH
#undef PEANUT_GB_CPUFLAG_MASK_ZERO
#undef PEANUT_GB_CPUFLAG_BIT_CARRY
#undef PEANUT_GB_CPUFLAG_BIT_HALFC
#undef PEANUT_GB_CPUFLAG_BIT_ARITH
#undef PEANUT_GB_CPUFLAG_BIT_ZERO
#undef PGB_SET_CARRY
#undef PGB_SET_HALFC
#undef PGB_SET_ARITH
#undef PGB_SET_ZERO
#undef PGB_GET_CARRY
#undef PGB_GET_HALFC
#undef PGB_GET_ARITH
#undef PGB_GET_ZERO
#endif //PEANUT_GB_H
