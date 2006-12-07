/*
 * (C) Copyright 2006 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 * All rights reserved.
 *
 */

#ifndef _BITSTREAM_PARSER_H
#define _BITSTREAM_PARSER_H

#include <stdint.h>
#include <glib.h>

typedef enum _id {
  XC2V40 = 0, XC2V80,
  XC2V250, XC2V500,
  XC2V1000, XC2V1500,
  XC2V2000, XC2V3000,
  XC2V4000, XC2V6000,
  XC2V8000,
  XC2__NUM,
} v2_id_t;

/* This structure holds the finale results of the parsing phase */
typedef struct _bitstream_parsed {
  /* First header informations */
  gchar *filename;
  gchar *device;
  gchar *build_date;
  gchar *build_time;

  /* (virtex-II) chip type info -- from bitstream data */
  v2_id_t chip;
  const void *chip_struct;

  /* frames */
  const gchar ***frames;
  GArray *frame_array;

  /* mmapped file information */
  GMappedFile *file;

} bitstream_parsed_t;

bitstream_parsed_t *parse_bitstream(const gchar*filename);
void free_bitstream(bitstream_parsed_t *bitstream);

typedef void (*frame_iterator_t)(const char *frame,
				 guint type,
				 guint index,
				 guint frameidx,
				 void *data);

/****
 * Bitstream frame indexing
 ****/

void iterate_over_frames(const bitstream_parsed_t *parsed,
			 frame_iterator_t iter, void *data);

typedef struct _frame_record {
  guint32 far;
  guint32 offset;
  unsigned framelen;
  const char *frame;
} frame_record_t;

typedef void (*frame_unk_iterator_t)(const frame_record_t *frame,
				     void *data);

void iterate_over_unk_frames(const bitstream_parsed_t *parsed,
			     frame_unk_iterator_t iter, void *itdat);

/* for v2 */
void
typed_frame_name(char *buf, unsigned buf_len,
		 const unsigned type,
		 const unsigned index,
		 const unsigned frameid);

/* for v4 */
int
snprintf_far(char *buf, const size_t buf_len,
	     const uint32_t hwfar);

#endif /* _BITSTREAM_PARSER_H */
