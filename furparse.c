#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <zlib.h>

#define dbp(...) printf(__VA_ARGS__)

typedef struct FurChipMeta
{
	const char *name;
	int channels;
} FurChipMeta;

static const FurChipMeta kchip_meta[] =
{
	[0x00] = {"None", 0},
	[0x01] = {"YMU759", 17},
	[0x02] = {"Genesis", 10},
	[0x03] = {"SMS (SN76489)", 4},
	[0x04] = {"Game Boy", 4},
	[0x05] = {"PC Engine", 6},
	[0x06] = {"NES", 5},
	[0x07] = {"C64 (8580)", 3},
	[0x08] = {"\"Arcade\" (YM2151+SegaPCM)", 13},
	[0x09] = {"Neo Geo CD (YM2610))", 13},
	[0x42] = {"Genesis extended", 13},
	[0x43] = {"SMS (SN76489) + OPLL (YM2413)", 13},
	[0x46] = {"NES + VRC7", 11},
	[0x47] = {"C64 (6581)", 3},
	[0x49] = {"Neo Geo CD extended", 16},
	[0x80] = {"AY-3-8910", 3},
	[0x81] = {"Amiga", 4},
	[0x82] = {"YM2151", 8},
	[0x83] = {"YM2612", 6},
	[0x84] = {"TIA", 2},
	[0x85] = {"VIC-20", 4},
	[0x86] = {"PET", 1},
	[0x87] = {"SNES", 8},
	[0x88] = {"VRC6", 3},
	[0x89] = {"OPLL (YM2413)", 9},
	[0x8A] = {"FDS", 1},
	[0x8B] = {"MMC5", 3},
	[0x8C] = {"Namco 163", 8},
	[0x8D] = {"OPN (YM2203)", 6},
	[0x8E] = {"PC-88 (YM2608)", 16},
	[0x8F] = {"OPL (YM3526)", 9},
	[0x90] = {"OPL2 (YM3812)", 9},
	[0x91] = {"OPL3 (YMF2622)", 18},
	[0x92] = {"MultiPCM", 28},
	[0x93] = {"Intel 8253 (beeper)", 1},
	[0x94] = {"POKEY", 4},
	[0x95] = {"RF5C68", 8},
	[0x96] = {"WonderSwan", 4},
	[0x97] = {"Phillips SAA1099", 6},
	[0x98] = {"OPZ (YM2414)", 8},
	[0x99] = {"Pokemon Mini", 1},
	[0x9A] = {"AY8930", 3},
	[0x9B] = {"SegaPCM", 16},
	[0x9C] = {"Virtual Boy", 6},
	[0x9D] = {"VRC7", 6},
	[0x9E] = {"YM2610B", 16},
	[0x9F] = {"ZX Spectrum (beeper)", 6},
	[0xA0] = {"YM2612 extended", 9},
	[0xA1] = {"Konami SCC", 5},
	[0xA2] = {"OPL drums (YM3526)", 11},
	[0xA3] = {"OPL2 drums (YM3812)", 11},
	[0xA4] = {"OPL3 drums (YMF262)", 20},
	[0xA5] = {"Neo Geo (YM2610)", 14},
	[0xA6] = {"Neo Geo extended (YM2610)", 17},
	[0xA7] = {"OPLL drums (YM2413)", 11},
	[0xA8] = {"Atari Lynx", 4},
	[0xA9] = {"SegaPCM (DefleMask compat.)", 5},
	[0xAA] = {"MSM6295", 4},
	[0xAB] = {"MSM6258", 1},
	[0xAC] = {"Commander X16 (VERA)", 17},
	[0xAD] = {"Bubble System WSG", 2},
	[0xAE] = {"OPL4 (YMF278B)", 42},
	[0xAF] = {"OPL4 drums (YMF278B)", 44},
	[0xB0] = {"Sega/Allumer X1-010", 16},
	[0xB1] = {"Ensoniq ES5506", 32},
	[0xB2] = {"Yamaha Y8950", 10},
	[0xB3] = {"Yamaha Y8950 drums", 12},
	[0xB4] = {"Konami SCC+", 5},
	[0xB5] = {"tilearrow Sound Unit", 8},
	[0xB6] = {"OPN extended", 9},
	[0xB7] = {"PC-98 extended", 19},
	[0xB8] = {"YMZ280B", 8},
	[0xB9] = {"Namco WSG", 3},
	[0xBA] = {"Namco 15xx", 8},
	[0xBB] = {"Namco CUS30", 8},
	[0xBC] = {"reserved", 8},
	[0xBD] = {"YM2612 extra features extended", 11},
	[0xBE] = {"YM2612 extra features", 7},
	[0xBF] = {"T6W28", 4},
	[0xC0] = {"PCM DAC", 1},
	[0xC1] = {"YM2612 CSM", 10},
	[0xC2] = {"Neo Geo CSM (YM2610)", 18},
	[0xC3] = {"OPN CSM", 10},
	[0xC4] = {"PC-98 CSM", 20},
	[0xC5] = {"YM2610B CSM", 20},
	[0xDE] = {"YM2610B extended", 10},
	[0xE0] = {"QSound", 19},
	[0xFD] = {"Dummy System", 8},
	[0xFE] = {"reserved for development", 0},
	[0xFF] = {"reserved for development", 0},
};

typedef struct FurHeader
{
	uint8_t signature[16];  // Contains "-Furnace module-" unterminated.
	uint16_t version;
	uint32_t song_info_offs;  // Pointer (within file?) to song info block.
} FurHeader;

bool fur_fill_header(const uint8_t *data, size_t data_size, size_t offs, FurHeader *header)
{
	static const char kexpected_signature[] = "-Furnace module-";
	if (data_size < 32)
	{
		fprintf(stderr, "data unsifficient for even the header\n");
		return false;
	}
	memcpy(header->signature, data, sizeof(header->signature));
	memcpy(&header->version, &data[16], sizeof(header->version));
	memcpy(&header->song_info_offs, &data[20], sizeof(header->song_info_offs));

	char sig_str[17];
	memcpy(sig_str, header->signature, sizeof(header->signature));
	sig_str[16] = '\0';
	
	dbp("Signature: %s\n", sig_str);
	dbp("Version: %d\n", header->version);
	dbp("Song offs: $%08X\n", header->song_info_offs);

	if (strcmp(kexpected_signature, sig_str) != 0)
	{
		dbp("Signature check failed %s\n", sig_str);
		return false;
	}

	if (header->version > 103)
	{
		fprintf(stderr, "Don't know how to handle version %d\n", header->version);
		return false;
	}

	return true;
}

typedef struct FurInstBlock
{
	uint32_t block_id;
	uint32_t block_size;
	uint16_t format_version;
	uint8_t type;  // TODO: Enum
} FurInstblock;

typedef struct FurOrder
{
	uint8_t *pattern_ids;  // List of patterns for one channel.
	size_t num_pattern_ids;  // Inherited from order length.
} FurOrder;

typedef struct FurSong
{
	// Track / pattern metadata
	uint32_t block_id;
	uint32_t block_size;
	uint8_t time_base;
	uint8_t speed[2];
	uint8_t initial_arpeggio_time;
	float tick_rate_hz;

	uint16_t pattern_length;
	uint16_t orders_length;
	uint8_t highlight[2];

	// Instrument / chip related data
	uint16_t instrument_count;
	uint16_t wavetable_count;
	uint16_t sample_count;
	uint32_t pattern_count;

	uint8_t chips[32];  // Terminated with 0x00.
	int num_chips;
	int8_t chip_volumes[32];  // 64 = 1.0; 127 = 2.0 (why???)
	int8_t chip_panning[32];  // -128 = left, 127 = right...

	uint32_t chip_params[32];  // wtf is this
	// why the fuck is this data here
	char *song_name;
	char *song_author;

	float a4_tuning;

	uint8_t limit_slides;
	uint8_t linear_pitch;  // TODO: Enum for values 0, 1, and 2
	uint8_t loop_modality;
	uint8_t proper_noise_layout;
	uint8_t wave_duty_is_volume;
	bool reset_macro_on_porta;
	bool legacy_volume_slides;
	bool compatible_arpeggio;
	bool note_off_resets_slides;
	bool target_resets_slides;
	bool arpeggio_inhibits_portamento;
	uint8_t wack_algorithm_macro;
	bool broken_shortcut_slides;
	bool ignore_duplicate_slides;
	bool continuous_vibrato;
	bool broken_dac_mode;
	bool one_tick_cut;
	bool instrument_change_allowed_during_porta;
	bool reset_note_base_on_arpeggio_effect_stop;

	// Oh, here's the useful shit, so far down into the song info
	uint32_t instrument_offs;
	uint32_t wavetable_offs;
	uint32_t sample_offs;
	uint32_t pattern_offs;

	FurOrder *orders;
	size_t num_orders;

	uint8_t *effects_columns;  // Size is channel count.
	// I don't give a shit about channel hide or collapse status
	// For now, not concerned with channel names either
	// Nor the comment
	float master_volume;

	// Even more bullshit. Omitted ones that aren't related to MD/X68K
	bool broken_speed_selection;
	bool no_slides_on_first_tick;
	bool next_row_reset_arp_pos;
	bool ignore_jump_at_end;
	bool buggy_portamento_after_slide;
	bool extch_channel_state_is_shared;
	bool ignore_dac_mode_change_outside_intended_channel;
	bool e1_e2_slide_priority_over_slide00;
	bool sn_duty_macro_resets_phase;
	bool pitch_macro_linear;
	uint8_t pitch_slide_speed_in_linear_pitch_mode;
	bool old_active_boundary_behavior;
	bool disable_opn2_dac_volume_control;
	bool new_volume_scaling_strategy;
	bool volume_macro_still_applies_after_end;
	bool broken_outvol;
	bool e1_e2_stop_on_same_note;
	bool broken_initial_position_of_porta_after_arp;

	uint16_t virtual_tempo_numerator;
	uint16_t virtual_tempo_denominator;

	// TODO: Subsongs
} FurSong;

#define READWALK(field, walkptr) \
	memcpy(&field, walkptr, sizeof(field)); \
	walkptr += sizeof(field);

// song should be cleaned up with fur_shutdown_song() if it returns true.
bool fur_init_song(const uint8_t *data, size_t data_size, size_t offs, FurSong *song)
{
	memset(song, 0, sizeof(*song));
	const uint8_t *walk = &data[offs];

	READWALK(song->block_id, walk);
	READWALK(song->block_size, walk);
	song->time_base = *walk++;
	song->speed[0] = *walk++;
	song->speed[1] = *walk++;
	song->initial_arpeggio_time = *walk++;
	READWALK(song->tick_rate_hz, walk);
	READWALK(song->pattern_length, walk);
	READWALK(song->orders_length, walk);
	song->highlight[0] = *walk++;
	song->highlight[1] = *walk++;
	READWALK(song->instrument_count, walk);
	READWALK(song->wavetable_count, walk);
	READWALK(song->sample_count, walk);
	READWALK(song->pattern_count, walk);
	READWALK(song->chips, walk);
	song->num_chips = 0;
	for (int i = 0; i < sizeof(song->chips); i++)
	{
		if (song->chips[i] != 0x00) song->num_chips++;
	}
	READWALK(song->chip_volumes, walk);
	READWALK(song->chip_panning, walk);
	READWALK(song->chip_params, walk);

	// TODO: Store this if we're ever doing to do anything with it.
	dbp("\n");
	dbp("Song name: \"");
	char namechar = ' ';
	do
	{
		namechar = *walk++;
		dbp("%c", namechar);
	} while (namechar);
	dbp("\"\n");

	dbp("Author: \"");
	do
	{
		namechar = *walk++;
		dbp("%c", namechar);
	} while (namechar);
	dbp("\"\n");

	READWALK(song->a4_tuning, walk);

	// Print some information about the track.

	dbp("\n");
	char block_str[5];
	memcpy(block_str, &song->block_id, sizeof(song->block_id));
	block_str[4] = '\0';
	dbp("Block ID \"%s\"\n", block_str);
	dbp("TB %d speed %d / %d; init arp %d\n", song->time_base, song->speed[0], song->speed[1], song->initial_arpeggio_time);
	dbp("Tick rate %4.1f Hz\n", song->tick_rate_hz);
	dbp("Chips: %d\n", song->num_chips);
	for (int i = 0; i < song->num_chips; i++)
	{
		const int chip = song->chips[i];
		const FurChipMeta *meta = &kchip_meta[chip];
		dbp("  %02d: $%02X \"%s\" : %d channels vol $%02X pan %d param $%08X\n", i, chip, meta->name, meta->channels, song->chip_volumes[i], song->chip_panning[i], song->chip_params[i]);
	}
	dbp("A4 Tuning %f\n", song->a4_tuning);
	return true;
}

#undef READWALK

// Free resources acquired by the song
void fur_shutdown_song(FurSong *song)
{

}

typedef struct FurFile
{
	FurHeader header;
	FurSong *songs;
	size_t num_songs;
} FurFile;

static bool read_file_as_zlib_data(FILE *f, uint8_t **furdata,
                                   size_t *furdata_size)
{
	fseek(f, 0, SEEK_SET);

	FILE *dest = fopen("__furtemp.bin", "wb");
	if (!dest)
	{
		fprintf(stderr, "Couldn't open temp file for writing\n");
		return false;
	}

	static const size_t kchunk = 0x4000;

	unsigned int have;
	z_stream strm;
	uint8_t in[kchunk];
	uint8_t out[kchunk];
	strm.zalloc = Z_NULL;
	strm.zfree = Z_NULL;
	strm.opaque = Z_NULL;
	strm.avail_in = 0;
	strm.next_in = Z_NULL;
	int ret = inflateInit(&strm);
	if (ret != Z_OK)
	{
		fprintf(stderr, "Couldn't init stream for inflation\n");
		return false;
	}

	do
	{
		strm.avail_in = fread(in, 1, kchunk, f);
		if (ferror(f))
		{
			(void)inflateEnd(&strm);
			return false;
		}
		if (strm.avail_in == 0) break;
		strm.next_in = in;

		do
		{
			strm.avail_out = kchunk;
			strm.next_out = out;
			ret = inflate(&strm, Z_NO_FLUSH);
			if (ret == Z_STREAM_ERROR)
			{
				fprintf(stderr, "Inflate state clobbered\n");
				return false;
			}
			switch (ret)
			{
				case Z_NEED_DICT:
					ret = Z_DATA_ERROR;
					// Fall-through intended
				case Z_DATA_ERROR:
				case Z_MEM_ERROR:
					(void)inflateEnd(&strm);
					return false;
			}
			have = kchunk - strm.avail_out;
			if (fwrite(out, 1, have, dest) != have || ferror(dest))
			{
				fprintf(stderr, "Couldn't write to temp\n");
				(void)inflateEnd(&strm);
				return false;
			}
		} while (strm.avail_out == 0);
	} while (ret != Z_STREAM_END);

	fclose(dest);

	(void)inflateEnd(&strm);

	return true;
}

static bool is_compressed_file(FILE *f)
{
	const int first_byte = fgetc(f);
	if (first_byte != 0x78) return false;
	const int second_byte = fgetc(f);
	if (second_byte == EOF)
	{
		fprintf(stderr, "Data is less than two bytes?!");
		return false;
	}
	switch (second_byte)
	{
		default:
			return false;
		case 0x01:
		case 0x9C:
		case 0xDA:
			return true;
	}
	return false;
}

static bool read_file(const char *fname, uint8_t **furdata, size_t *furdata_size)
{
	FILE *f = fopen(fname, "rb");

	if (!f)
	{
		fprintf(stderr, "Couldn't open \"%s\"\n", fname);
		return false;
	}

	// First check the header and see if it's just zlib data
	bool is_compressed = is_compressed_file(f);
	if (is_compressed)
	{
		if (!(read_file_as_zlib_data(f, furdata, furdata_size))) return false;
		fclose(f);
		f = fopen("__furtemp.bin", "rb");
		if (!f)
		{
			fprintf(stderr, "Can't open the temp file we just created\n");
			return false;
		}
	}

	fseek(f, 0, SEEK_END);
	*furdata_size = ftell(f);
	if (*furdata_size == 0)
	{
		fprintf(stderr, "Furdata size is zero\n");
		fclose(f);
		return false;
	}
	fseek(f, 0, SEEK_SET);

	*furdata = malloc(*furdata_size);
	if (*furdata == NULL)
	{
		fprintf(stderr, "Couldn't alloc for furdata\n");
		fclose(f);
		return false;
	}

	fread(*furdata, 1, *furdata_size, f);
	fclose(f);

	if (is_compressed) remove("__furtemp.bin");
	return true;
}

int main(int argc, char **argv)
{
	if (argc < 2)
	{
		printf("Usage: %s in.fur\n", argv[0]);
		return 0;
	}

	const char *in_fname = argv[1];
	uint8_t *furdata;
	size_t furdata_size;

	if (!read_file(in_fname, &furdata, &furdata_size)) return -1;

	FurHeader header;
	if (!fur_fill_header(furdata, furdata_size, 0, &header)) return -1;
	FurSong song;
	if (!fur_init_song(furdata, furdata_size, header.song_info_offs, &song)) return -1;

	free(furdata);

	return 0;
}
