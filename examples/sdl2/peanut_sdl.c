/**
 * MIT License
 * Copyright (c) 2018-2023 Mahyar Koshkouei
 *
 * An example of using the peanut_gb.h library. This example application uses
 * SDL2 to draw the screen and get input.
 */

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <js/glue.h>
#include <js/dom_pk_codes.h>

#if defined(ENABLE_SOUND_BLARGG)
#	include "blargg_apu/audio.h"
#elif defined(ENABLE_SOUND_MINIGB)
#	include "minigb_apu/minigb_apu.h"
#endif

uint8_t audio_read(uint16_t addr);
void audio_write(uint16_t addr, uint8_t val);

#include "../../peanut_gb.h"

struct priv_t
{
	/* Pointer to allocated memory holding GB file. */
	uint8_t *rom;
	/* Pointer to allocated memory holding save file. */
	uint8_t *cart_ram;
	/* Size of the cart_ram in bytes. */
	size_t save_size;
	/* Pointer to boot ROM binary if available. */
	uint8_t *bootrom;

	/* Colour palette for each BG, OBJ0, and OBJ1. */
	uint16_t selected_palette[3][4];
	uint16_t fb[LCD_HEIGHT][LCD_WIDTH];
};

static struct minigb_apu_ctx apu;

uint8_t *load_file(const char* file, size_t *datasize);
static bool onkey(void *userdata, bool pressed, int key, int code, int modifiers);
static bool quit = false;
uint32_t pixels[LCD_WIDTH * LCD_HEIGHT];
static unsigned int fast_mode = 1;
static unsigned int dump_bmp = 0;
static unsigned int selected_palette = 3;
struct priv_t priv =
{
	.rom = NULL,
	.cart_ram = NULL
};

/**
 * Returns a byte from the ROM file at the given address.
 */
uint8_t gb_rom_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv_t * const p = gb->direct.priv;
	return p->rom[addr];
}

/**
 * Returns a byte from the cartridge RAM at the given address.
 */
uint8_t gb_cart_ram_read(struct gb_s *gb, const uint_fast32_t addr)
{
	const struct priv_t * const p = gb->direct.priv;
	return p->cart_ram[addr];
}

/**
 * Writes a given byte to the cartridge RAM at the given address.
 */
void gb_cart_ram_write(struct gb_s *gb, const uint_fast32_t addr,
		       const uint8_t val)
{
	const struct priv_t * const p = gb->direct.priv;
	p->cart_ram[addr] = val;
}

uint8_t gb_bootrom_read(struct gb_s *gb, const uint_fast16_t addr)
{
	const struct priv_t * const p = gb->direct.priv;
	return p->bootrom[addr];
}

uint8_t audio_read(uint16_t addr)
{
	return minigb_apu_audio_read(&apu, addr);
}

void audio_write(uint16_t addr, uint8_t val)
{
	minigb_apu_audio_write(&apu, addr, val);
}

void audio_callback(void *ptr, uint8_t *data, int len)
{
	minigb_apu_audio_callback(&apu, (void *)data);
}

void read_cart_ram_file(const char *save_file_name, uint8_t **dest,
			const size_t len)
{
	FILE *f;

	/* If save file not required. */
	if(len == 0)
	{
		*dest = NULL;
		return;
	}

	/* Allocate enough memory to hold save file. */
	if((*dest = malloc(len)) == NULL)
	{
		fprintf(stderr, "line %d\n", __LINE__);
		exit(EXIT_FAILURE);
	}

	f = fopen(save_file_name, "rb");

	/* It doesn't matter if the save file doesn't exist. We initialise the
	 * save memory allocated above. The save file will be created on exit. */
	if(f == NULL)
	{
		memset(*dest, 0, len);
		return;
	}

	/* Read save file to allocated memory. */
	fread(*dest, sizeof(uint8_t), len, f);
	fclose(f);
}

void write_cart_ram_file(const char *save_file_name, uint8_t **dest,
			 const size_t len)
{
	// FILE *f;

	// if(len == 0 || *dest == NULL)
	// 	return;

	// if((f = fopen(save_file_name, "wb")) == NULL)
	// {
	// 	fprintf(stderr, "Unable to open save file\n");
	// 	return;
	// }

	// /* Record save file. */
	// fwrite(*dest, sizeof(uint8_t), len, f);
	JS_saveFile(save_file_name, *dest, len);
	// fclose(f);

	return;
}

/**
 * Handles an error reported by the emulator. The emulator context may be used
 * to better understand why the error given in gb_err was reported.
 */
void gb_error(struct gb_s *gb, const enum gb_error_e gb_err, const uint16_t addr)
{
	const char* gb_err_str[GB_INVALID_MAX] = {
		"UNKNOWN",
		"INVALID OPCODE",
		"INVALID READ",
		"INVALID WRITE",
		""
	};
	struct priv_t *priv = gb->direct.priv;
	char error_msg[256];
	char location[64] = "";
	uint8_t instr_byte;

	/* Record save file. */
	write_cart_ram_file("recovery.sav", &priv->cart_ram, priv->save_size);

	if(addr >= 0x4000 && addr < 0x8000)
	{
		uint32_t rom_addr;
		rom_addr = (uint32_t)addr * (uint32_t)gb->selected_rom_bank;
		snprintf(location, sizeof(location),
			" (bank %d mode %d, file offset %u)",
			gb->selected_rom_bank, gb->cart_mode_select, rom_addr);
	}

	instr_byte = __gb_read(gb, addr);

	snprintf(error_msg, sizeof(error_msg),
		"Error: %s at 0x%04X%s with instruction %02X.\n"
		"Cart RAM saved to recovery.sav\n"
		"Exiting.\n",
		gb_err_str[gb_err], addr, location, instr_byte);
	fprintf(stderr, "%s\n", error_msg);

	JS_alert(error_msg);

	/* Free memory and then exit. */
	free(priv->cart_ram);
	free(priv->rom);
	exit(EXIT_FAILURE);
}

/**
 * Automatically assigns a colour palette to the game using a given game
 * checksum.
 * TODO: Not all checksums are programmed in yet because I'm lazy.
 */
void auto_assign_palette(struct priv_t *priv, uint8_t game_checksum)
{
	size_t palette_bytes = 3 * 4 * sizeof(uint16_t);

	switch(game_checksum)
	{
	/* Balloon Kid and Tetris Blast */
	case 0x71:
	case 0xFF:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E60, 0x7C00, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x7E60, 0x7C00, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x7E60, 0x7C00, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Pokemon Yellow and Tetris */
	case 0x15:
	case 0xDB:
	case 0x95: /* Not officially */
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7FE0, 0x7C00, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x7FE0, 0x7C00, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x7FE0, 0x7C00, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Donkey Kong */
	case 0x19:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x7E60, 0x7C00, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Pokemon Blue */
	case 0x61:
	case 0x45:

	/* Pokemon Blue Star */
	case 0xD8:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Pokemon Red */
	case 0x14:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x3FE6, 0x0200, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Pokemon Red Star */
	case 0x8B:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x3FE6, 0x0200, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Kirby */
	case 0x27:
	case 0x49:
	case 0x5C:
	case 0xB3:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7D8A, 0x6800, 0x3000, 0x0000 }, /* OBJ0 */
			{ 0x001F, 0x7FFF, 0x7FEF, 0x021F }, /* OBJ1 */
			{ 0x527F, 0x7FE0, 0x0180, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Donkey Kong Land [1/2/III] */
	case 0x18:
	case 0x6A:
	case 0x4B:
	case 0x6B:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7F08, 0x7F40, 0x48E0, 0x2400 }, /* OBJ0 */
			{ 0x7FFF, 0x2EFF, 0x7C00, 0x001F }, /* OBJ1 */
			{ 0x7FFF, 0x463B, 0x2951, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Link's Awakening */
	case 0x70:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x03E0, 0x1A00, 0x0120 }, /* OBJ0 */
			{ 0x7FFF, 0x329F, 0x001F, 0x001F }, /* OBJ1 */
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* Mega Man [1/2/3] & others I don't care about. */
	case 0x01:
	case 0x10:
	case 0x29:
	case 0x52:
	case 0x5D:
	case 0x68:
	case 0x6D:
	case 0xF6:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 }, /* OBJ0 */
			{ 0x7FFF, 0x3FE6, 0x0200, 0x0000 }, /* OBJ1 */
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 }  /* BG */
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	default:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 }
		};
		printf("No palette found for 0x%02X.\n", game_checksum);
		memcpy(priv->selected_palette, palette, palette_bytes);
	}
	}
}

/**
 * Assigns a palette. This is used to allow the user to manually select a
 * different colour palette if one was not found automatically, or if the user
 * prefers a different colour palette.
 * selection is the requestion colour palette. This should be a maximum of
 * NUMBER_OF_PALETTES - 1. The default greyscale palette is selected otherwise.
 */
void manual_assign_palette(struct priv_t *priv, uint8_t selection)
{
#define NUMBER_OF_PALETTES 12
	size_t palette_bytes = 3 * 4 * sizeof(uint16_t);

	switch(selection)
	{
	/* 0x05 (Right) */
	case 0:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x2BE0, 0x7D00, 0x0000 },
			{ 0x7FFF, 0x2BE0, 0x7D00, 0x0000 },
			{ 0x7FFF, 0x2BE0, 0x7D00, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x07 (A + Down) */
	case 1:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7FE0, 0x7C00, 0x0000 },
			{ 0x7FFF, 0x7FE0, 0x7C00, 0x0000 },
			{ 0x7FFF, 0x7FE0, 0x7C00, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x12 (Up) */
	case 2:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 },
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 },
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x13 (B + Right) */
	case 3:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x0000, 0x0210, 0x7F60, 0x7FFF },
			{ 0x0000, 0x0210, 0x7F60, 0x7FFF },
			{ 0x0000, 0x0210, 0x7F60, 0x7FFF }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x16 (B + Left, DMG Palette) */
	default:
	case 4:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 },
			{ 0x7FFF, 0x5294, 0x294A, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x17 (Down) */
	case 5:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FF4, 0x7E52, 0x4A5F, 0x0000 },
			{ 0x7FF4, 0x7E52, 0x4A5F, 0x0000 },
			{ 0x7FF4, 0x7E52, 0x4A5F, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x19 (B + Up) */
	case 6:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 },
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 },
			{ 0x7F98, 0x6670, 0x41A5, 0x2CC1 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x1C (A + Right) */
	case 7:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 },
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 },
			{ 0x7FFF, 0x3FE6, 0x0198, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x0D (A + Left) */
	case 8:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 },
			{ 0x7FFF, 0x7EAC, 0x40C0, 0x0000 },
			{ 0x7FFF, 0x463B, 0x2951, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x10 (A + Up) */
	case 9:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x3FE6, 0x0200, 0x0000 },
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 },
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x18 (Left) */
	case 10:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x7E10, 0x48E7, 0x0000 },
			{ 0x7FFF, 0x3FE6, 0x0200, 0x0000 },
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}

	/* 0x1A (B + Down) */
	case 11:
	{
		const uint16_t palette[3][4] =
		{
			{ 0x7FFF, 0x329F, 0x001F, 0x0000 },
			{ 0x7FFF, 0x3FE6, 0x0200, 0x0000 },
			{ 0x7FFF, 0x7FE0, 0x3D20, 0x0000 }
		};
		memcpy(priv->selected_palette, palette, palette_bytes);
		break;
	}
	}

	return;
}

#if ENABLE_LCD
/**
 * Draws scanline into framebuffer.
 */
void lcd_draw_line(struct gb_s *gb, const uint8_t pixels[160],
		   const uint_fast8_t line)
{
	struct priv_t *priv = gb->direct.priv;

	for(unsigned int x = 0; x < LCD_WIDTH; x++)
	{
		priv->fb[line][x] = priv->selected_palette
				    [(pixels[x] & LCD_PALETTE_ALL) >> 4]
				    [pixels[x] & 3];
	}
}
#endif

/**
 * Saves the LCD screen as a 15-bit BMP file.
 */
int save_lcd_bmp(struct gb_s* gb, uint16_t fb[LCD_HEIGHT][LCD_WIDTH])
{
	/* Should be enough to record up to 828 days worth of frames. */
	static uint_fast32_t file_num = 0;
	char file_name[32];
	char title_str[16];
	FILE *f;
	int ret = -1;

	snprintf(file_name, 32, "%.16s_%010u.bmp",
		 gb_get_rom_name(gb, title_str), file_num);

	f = fopen(file_name, "wb");
	if(f == NULL)
#ifdef __XCC
		return ret;
#else
		goto ret;
#endif

	const uint8_t bmp_hdr_rgb555[] = {
		0x42, 0x4d, 0x36, 0xb4, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x36, 0x00, 0x00, 0x00, 0x28, 0x00, 0x00, 0x00, 0xa0, 0x00,
		0x00, 0x00, 0x70, 0xff, 0xff, 0xff, 0x01, 0x00, 0x10, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0xb4, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00
	};

	fwrite(bmp_hdr_rgb555, sizeof(uint8_t), sizeof(bmp_hdr_rgb555), f);
	fwrite(fb, sizeof(uint16_t), LCD_HEIGHT * LCD_WIDTH, f);
	ret = fclose(f);

	file_num++;

ret:
	return ret;
}

int main(int argc, char **argv)
{
	struct gb_s gb;
	const double target_speed_ms = 1000.0 / VERTICAL_SYNC;
	double speed_compensation = 0.0;
	uint_fast32_t new_ticks, old_ticks;
	enum gb_init_error_e gb_ret;
	unsigned int fast_mode_timer = 1;
	/* Must be freed */
	char *rom_file_name = NULL;
	char *save_file_name = NULL;
	int ret = EXIT_SUCCESS;

	/* Initialise frontend implementation, in this case, JS. */
	JS_createCanvas(LCD_WIDTH, LCD_HEIGHT, "2d");
	JS_setTitle("Peanut-GB: Opening File");
	JS_addKeyEventListener(&gb, onkey);

	switch(argc)
	{
	case 2:
		/* Apply file name to rom_file_name
		 * Set save_file_name to NULL. */
		rom_file_name = argv[1];
		break;

	case 3:
		/* Apply file name to rom_file_name
		 * Apply save name to save_file_name */
		rom_file_name = argv[1];
		save_file_name = argv[2];
		break;

	default:
		fprintf(stderr, "Usage: %s [ROM] [SAVE]\n", argv[0]);
		fprintf(stderr, "A file picker is presented if ROM is not given.\n");
		fprintf(stderr, "SAVE is set by default if not provided.\n");

	    JS_setFont("bold 16px Roboto");
	    JS_fillStyle("white");

	    char buf[UINT8_MAX];
	    const char ext[] = ".gb";
	    snprintf(buf, sizeof(buf), "Click to browse... (%s)", ext); // TODO maybe gbc if peanut merges it to main
	    JS_fillText(buf, (LCD_WIDTH - JS_measureTextWidth(buf)) / 2, LCD_HEIGHT / 2);

	    uint8_t *file = JS_openFilePicker(&rom_file_name, NULL, ext);
	    if (!file) {
			fprintf(stderr, "line %d\n", __LINE__);
			ret = EXIT_FAILURE;
			goto out;
	    }
	    priv.rom = file;
	}

	/* Copy input ROM file to allocated memory. */
	if (argc == 2 || argc == 3) {
		priv.rom = load_file(rom_file_name, NULL);
		if (!priv.rom) {
			fprintf(stderr, "line %d\n", __LINE__);
			ret = EXIT_FAILURE;
			goto out;
		}
	}

	/* If no save file is specified, copy save file (with specific name) to
	 * allocated memory. */
	if(save_file_name == NULL)
	{
		char *str_replace;
		const char extension[] = ".sav";

		/* Allocate enough space for the ROM file name, for the "sav"
		 * extension and for the null terminator. */
		save_file_name = malloc(
				strlen(rom_file_name) + strlen(extension) + 1);

		if(save_file_name == NULL)
		{
			fprintf(stderr, "line %d\n", __LINE__);
			ret = EXIT_FAILURE;
			goto out;
		}

		/* Copy the ROM file name to allocated space. */
		strcpy(save_file_name, rom_file_name);

		/* If the file name does not have a dot, or the only dot is at
		 * the start of the file name, set the pointer to begin
		 * replacing the string to the end of the file name, otherwise
		 * set it to the dot. */
		if((str_replace = strrchr(save_file_name, '.')) == NULL ||
				str_replace == save_file_name)
			str_replace = save_file_name + strlen(save_file_name);

		/* Copy extension to string including terminating null byte. */
		for(unsigned int i = 0; i <= strlen(extension); i++)
			*(str_replace++) = extension[i];
	}

	/* TODO: Sanity check input GB file. */

	/* Initialise emulator context. */
	gb_ret = gb_init(&gb, &gb_rom_read, &gb_cart_ram_read, &gb_cart_ram_write,
			 &gb_error, &priv);

	switch(gb_ret)
	{
	case GB_INIT_NO_ERROR:
		break;

	case GB_INIT_CARTRIDGE_UNSUPPORTED:
		fprintf(stderr, "Unsupported cartridge.\n");
		ret = EXIT_FAILURE;
#ifdef __XCC
		return ret;
#else
		goto out;
#endif

	case GB_INIT_INVALID_CHECKSUM:
		fprintf(stderr, "Invalid ROM: Checksum failure.\n");
		ret = EXIT_FAILURE;
#ifdef __XCC
		return ret;
#else
		goto out;
#endif

	default:
		fprintf(stderr, "Unknown error: %d\n", gb_ret);
		ret = EXIT_FAILURE;
#ifdef __XCC
		return ret;
#else
		goto out;
#endif
	}

	/* Copy dmg_boot.bin boot ROM file to allocated memory. */
	priv.bootrom = load_file("dmg_boot.bin", NULL);
	if (!priv.bootrom) {
		printf("No dmg_boot.bin file found; disabling boot ROM\n");
	} else {
		printf("boot ROM enabled\n");
		gb_set_bootrom(&gb, gb_bootrom_read);
		gb_reset(&gb);
	}

	/* Load Save File. */
	if(gb_get_save_size_s(&gb, &priv.save_size) < 0)
	{
		fprintf(stderr, "Unable to get save size\n");
		ret = EXIT_FAILURE;
		goto out;
	}

	/* Only attempt to load a save file if the ROM actually supports saves.*/
	if(priv.save_size > 0)
		read_cart_ram_file(save_file_name, &priv.cart_ram, priv.save_size);

	/* Set the RTC of the game cartridge. Only used by games that support it. */
	{
		time_t rawtime;
		time(&rawtime);
#ifdef _POSIX_C_SOURCE
		struct tm timeinfo;
		localtime_r(&rawtime, &timeinfo);
#else
		struct tm *timeinfo;
		timeinfo = localtime(&rawtime);
#endif

		/* You could potentially force the game to allow the player to
		 * reset the time by setting the RTC to invalid values.
		 *
		 * Using memset(&gb->cart_rtc, 0xFF, sizeof(gb->cart_rtc)) for
		 * example causes Pokemon Gold/Silver to say "TIME NOT SET",
		 * allowing the player to set the time without having some dumb
		 * password.
		 *
		 * The memset has to be done directly to gb->cart_rtc because
		 * gb_set_rtc() processes the input values, which may cause
		 * games to not detect invalid values.
		 */

		/* Set RTC. Only games that specify support for RTC will use
		 * these values. */
#ifdef _POSIX_C_SOURCE
		gb_set_rtc(&gb, &timeinfo);
#else
		gb_set_rtc(&gb, timeinfo);
#endif
	}

#if ENABLE_SOUND == 0
	// Sound is disabled, so do nothing.
#elif defined(ENABLE_SOUND_BLARGG)
	audio_init(&dev);
#elif defined(ENABLE_SOUND_MINIGB)
	{
		// SDL_AudioSpec want, have;

		// want.freq = AUDIO_SAMPLE_RATE;
		// want.format   = AUDIO_S16,
		// want.channels = 2;
		// want.samples = AUDIO_SAMPLES;
		// want.callback = audio_callback;
		// want.userdata = NULL;

		// printf("Audio driver: %s\n", SDL_GetAudioDeviceName(0, 0));

		// if((dev = SDL_OpenAudioDevice(NULL, 0, &want, &have, 0)) == 0)
		// {
		// 	fprintf(stderr, "SDL could not open audio device\n");
		// 	exit(EXIT_FAILURE);
		// }

		minigb_apu_audio_init(&apu);
	}
#endif

#if ENABLE_LCD
	gb_init_lcd(&gb, &lcd_draw_line);
#endif

	/* Open the first available controller. */
	// for(int i = 0; i < SDL_NumJoysticks(); i++)
	// {
	// 	if(!SDL_IsGameController(i))
	// 		continue;

	// 	controller = SDL_GameControllerOpen(i);

	// 	if(controller)
	// 	{
	// 		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
	// 				SDL_LOG_PRIORITY_INFO,
	// 				"Game Controller %s connected.",
	// 				SDL_GameControllerName(controller));
	// 		break;
	// 	}
	// 	else
	// 	{
	// 		SDL_LogMessage(LOG_CATERGORY_PEANUTSDL,
	// 				SDL_LOG_PRIORITY_INFO,
	// 				"Could not open game controller %i: %s\n",
	// 				i, SDL_GetError());
	// 	}
	// }

	{
		/* 11 for "Peanut-GB: " and a maximum of 16 for the title. */
		char title_str[27] = "Peanut-GB: ";
		gb_get_rom_name(&gb, title_str + strlen(title_str));
		printf("%s\n", title_str);
		JS_setTitle(title_str);
	}

	JS_requestAnimationFrame();

	auto_assign_palette(&priv, gb_colour_hash(&gb));

	while(!quit)
	{
		int delay;
		static double rtc_timer = 0;

		/* Calculate the time taken to draw frame, then later add a
		 * delay to cap at 60 fps. */
		old_ticks = JS_performanceNow();

		/* Execute CPU cycles until the screen has to be redrawn. */
		gb_run_frame(&gb);

		/* Tick the internal RTC when 1 second has passed. */
		rtc_timer += target_speed_ms / (double) fast_mode;

		if(rtc_timer >= 1000.0)
		{
			rtc_timer -= 1000.0;
		}

		/* Skip frames during fast mode. */
		if(fast_mode_timer > 1)
		{
			fast_mode_timer--;
			/* We continue here since the rest of the logic in the
			 * loop is for drawing the screen and delaying. */
			continue;
		}

		fast_mode_timer = fast_mode;

#if ENABLE_SOUND_BLARGG
		/* Process audio. */
		audio_frame();
#endif

#if ENABLE_LCD
		/* Copy frame buffer to SDL screen. */
		for (int i = 0; i < LCD_WIDTH * LCD_HEIGHT; i++) {
		    uint16_t p = priv.fb[0][i];

			uint8_t r = (p >> 10) & 0x1F;
			uint8_t g = (p >> 5)  & 0x1F;
			uint8_t b = p         & 0x1F;

		    r = (r << 3) | (r >> 2);
		    g = (g << 3) | (g >> 2);
		    b = (b << 3) | (b >> 2);

		    pixels[i] = (0xFF << 24) | (b << 16) | (g << 8) | r;
		}
		JS_setPixelsAlpha(pixels);
		JS_requestAnimationFrame();

		if(dump_bmp)
		{
			if(save_lcd_bmp(&gb, priv.fb) != 0)
			{
				fprintf(stderr, "Failure dumping frame\n");
				dump_bmp = 0;
				printf("Stopped dumping frames\n");
			}
		}

#endif

		/* Use a delay that will draw the screen at a rate of 59.7275 Hz. */
		new_ticks = JS_performanceNow();

		/* Since we can only delay for a maximum resolution of 1ms, we
		 * can accumulate the error and compensate for the delay
		 * accuracy when the delay compensation surpasses 1ms. */
		speed_compensation += target_speed_ms - (new_ticks - old_ticks);

		/* We cast the delay compensation value to an integer, since it
		 * is the type used by SDL_Delay. This is where delay accuracy
		 * is lost. */
		delay = (int)(speed_compensation);

		/* We then subtract the actual delay value by the requested
		 * delay value. */
		speed_compensation -= delay;

		/* Only run delay logic if required. */
		if(delay > 0)
		{
			uint_fast32_t delay_ticks = JS_performanceNow();
			uint_fast32_t after_delay_ticks;

			/* Tick the internal RTC when 1 second has passed. */
			rtc_timer += delay;

			if(rtc_timer >= 1000)
			{
				rtc_timer -= 1000;
			}

			/* This will delay for at least the number of
			 * milliseconds requested, so we have to compensate for
			 * error here too. */
			JS_setTimeout(delay);

			after_delay_ticks = JS_performanceNow();
			speed_compensation += (double)delay -
					      (int)(after_delay_ticks - delay_ticks);
		}
	}

#ifdef ENABLE_SOUND_BLARGG
	audio_cleanup();
#endif

	/* Record save file. */
	write_cart_ram_file(save_file_name, &priv.cart_ram, priv.save_size);

out:
	free(priv.rom);
	free(priv.cart_ram);

	/* If the save file name was automatically generated (which required memory
	 * allocated on the help), then free it here. */
	if(argc == 2)
		free(save_file_name);

	if(argc == 1)
		free(rom_file_name);

	return ret;
}

// case SDL_CONTROLLERBUTTONDOWN:
// 	switch(event.cbutton.button)
// 	{
// 	case SDL_CONTROLLER_BUTTON_A:
// 		gb.direct.joypad &= ~JOYPAD_A;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_B:
// 		gb.direct.joypad &= ~JOYPAD_B;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_BACK:
// 		gb.direct.joypad &= ~JOYPAD_SELECT;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_START:
// 		gb.direct.joypad &= ~JOYPAD_START;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_DPAD_UP:
// 		gb.direct.joypad &= ~JOYPAD_UP;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
// 		gb.direct.joypad &= ~JOYPAD_RIGHT;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
// 		gb.direct.joypad &= ~JOYPAD_DOWN;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
// 		gb.direct.joypad &= ~JOYPAD_LEFT;
// 		break;
// 	}

// 	break;

// case SDL_CONTROLLERBUTTONUP:
// 	switch(event.cbutton.button)
// 	{
// 	case SDL_CONTROLLER_BUTTON_A:
// 		gb.direct.joypad |= JOYPAD_A;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_B:
// 		gb.direct.joypad |= JOYPAD_B;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_BACK:
// 		gb.direct.joypad |= JOYPAD_SELECT;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_START:
// 		gb.direct.joypad |= JOYPAD_START;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_DPAD_UP:
// 		gb.direct.joypad |= JOYPAD_UP;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_DPAD_RIGHT:
// 		gb.direct.joypad |= JOYPAD_RIGHT;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_DPAD_DOWN:
// 		gb.direct.joypad |= JOYPAD_DOWN;
// 		break;

// 	case SDL_CONTROLLER_BUTTON_DPAD_LEFT:
// 		gb.direct.joypad |= JOYPAD_LEFT;
// 		break;
// 	}

// 	break;

/* Get joypad input. */
static bool onkey(void *userdata, bool pressed, int key, int code, int modifiers) {
    (void)key, (void)modifiers;
	struct gb_s *gb = (struct gb_s*)userdata;
	if (pressed) {
		switch(code)
		{
		case DOM_PK_ESCAPE:
			quit = true;
			break;

		case DOM_PK_ENTER:
			gb->direct.joypad &= ~JOYPAD_START;
			break;

		case DOM_PK_BACKSPACE:
			gb->direct.joypad &= ~JOYPAD_SELECT;
			break;

		case DOM_PK_Z:
			gb->direct.joypad &= ~JOYPAD_A;
			break;

		case DOM_PK_X:
			gb->direct.joypad &= ~JOYPAD_B;
			break;

		case DOM_PK_A:
			gb->direct.joypad ^= JOYPAD_A;
			break;

		case DOM_PK_S:
			gb->direct.joypad ^= JOYPAD_B;
			break;

		case DOM_PK_ARROW_UP:
			gb->direct.joypad &= ~JOYPAD_UP;
			break;

		case DOM_PK_ARROW_RIGHT:
			gb->direct.joypad &= ~JOYPAD_RIGHT;
			break;

		case DOM_PK_ARROW_DOWN:
			gb->direct.joypad &= ~JOYPAD_DOWN;
			break;

		case DOM_PK_ARROW_LEFT:
			gb->direct.joypad &= ~JOYPAD_LEFT;
			break;

		case DOM_PK_SPACE:
			fast_mode = 2;
			break;

		case DOM_PK_1:
			fast_mode = 1;
			break;

		case DOM_PK_2:
			fast_mode = 2;
			break;

		case DOM_PK_3:
			fast_mode = 3;
			break;

		case DOM_PK_4:
			fast_mode = 4;
			break;

		case DOM_PK_R:
			gb_reset(gb);
			break;
#if ENABLE_LCD

		case DOM_PK_I:
			gb->direct.interlace = !gb->direct.interlace;
			break;

		case DOM_PK_O:
			gb->direct.frame_skip = !gb->direct.frame_skip;
			break;

		case DOM_PK_B:
			dump_bmp = ~dump_bmp;

			if(dump_bmp)
				printf("Dumping frames\n");
			else
				printf("Stopped dumping frames\n");

			break;
#endif

		case DOM_PK_P:
			if(modifiers & KMOD_SHIFT)
			{
				auto_assign_palette(&priv, gb_colour_hash(gb));
				break;
			}

			if(++selected_palette == NUMBER_OF_PALETTES)
				selected_palette = 0;

			manual_assign_palette(&priv, selected_palette);
			break;
		}
	} else {
		switch(code)
		{
		case DOM_PK_ENTER:
			gb->direct.joypad |= JOYPAD_START;
			break;

		case DOM_PK_BACKSPACE:
			gb->direct.joypad |= JOYPAD_SELECT;
			break;

		case DOM_PK_Z:
			gb->direct.joypad |= JOYPAD_A;
			break;

		case DOM_PK_X:
			gb->direct.joypad |= JOYPAD_B;
			break;

		case DOM_PK_A:
			gb->direct.joypad |= JOYPAD_A;
			break;

		case DOM_PK_S:
			gb->direct.joypad |= JOYPAD_B;
			break;

		case DOM_PK_ARROW_UP:
			gb->direct.joypad |= JOYPAD_UP;
			break;

		case DOM_PK_ARROW_RIGHT:
			gb->direct.joypad |= JOYPAD_RIGHT;
			break;

		case DOM_PK_ARROW_DOWN:
			gb->direct.joypad |= JOYPAD_DOWN;
			break;

		case DOM_PK_ARROW_LEFT:
			gb->direct.joypad |= JOYPAD_LEFT;
			break;

		case DOM_PK_SPACE:
			fast_mode = 1;
			break;
		}
    }

    if (code == DOM_PK_F12) {
        return 0;
    }
    return 1;
}
uint8_t *load_file(const char* file, size_t *datasize) {
	FILE *f = fopen(file, "rb");
	if (!f) {
		return NULL;
	}

	fseek(f, 0, SEEK_END);
	long size = ftell(f);
	rewind(f);

	if (datasize) {
		*datasize = size;
	}

	void *buffer = malloc(size);
	fread(buffer, 1, size, f);
	fclose(f);
	return buffer;
}
