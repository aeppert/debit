/*
 * Copyright (C) 2006, 2007 Jean-Baptiste Note <jean-baptiste.note@m4x.org>
 *
 * This file is part of debit.
 *
 * Debit is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Debit is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with debit.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef _LOCALPIPS_H
#define _LOCALPIPS_H

#include "bitstream_parser.h"
#include "wiring.h"
#include "sites.h"

/** \file */

typedef enum
  {
    /* log flags */
    PIP_LOG_DATA             = 1 << G_LOG_LEVEL_USER_SHIFT,
  } PipLevelFlags;

/** pip database opaque type
 *
 * This is an abstract view of the pip database for a chip.
 */

#ifdef __COMPILED_PIPSDB

#include "data/pips_compiled_common.h"

typedef struct pip_db {
  const pipdb_control_t *memorydb;
  wire_db_t *wiredb;
} pip_db_t;

#else /* __COMPILED_PIPSDB */

typedef struct pip_db {
  /* Different databases, sorted on increasing order of complexity */
  /* Database of implicit pips -- only 2-uples of wires */
  GNode *implicitdb[NR_SWITCH_TYPE];
  /* Database of pips */
  GNode *memorydb[NR_SWITCH_TYPE];
  /* Connectivity database for logic elements */
  GNode *connexdb[NR_SWITCH_TYPE];
  wire_db_t *wiredb;
} pip_db_t;

#endif /* __COMPILED_PIPSDB */

typedef struct pip_read {
  gboolean connected;
  wire_atom_t origin;
  /* NB: could also be separated */
  GNode *connect;
} pip_read_t;

/* This stucture contains the normal form of the bitstream, fully
   unrolled into memory. It is an array of pip_read structure, which
   have their location implicitly referring to their (site, destination
   wire), with the associated input wire. Please note that the
   "connected" boolean could be reduced to a mere special wire_atom_t --
   for instance the value zero.

   Sorting in the array: sites, then wires. Seems the most natural at
   first sight; however wires, sites would probably allow for more data
   locality when iterating over sites in the innermost loops.
 */
typedef struct _pip_parsed {
  pip_read_t *bitpips;
} pip_parsed_t;

/* rather that having a full structure, we can record the pip
   completely and sort along the destination wire. This could be much
   more cache-friendly, as the data can be dense, and the cost is
   be a dichotomy during the lookup. To be benchmarked.
   However, this structure is only good for read-only operations;
   modifying it is a real pita.
*/

typedef struct _pip_parsed_dense {
  pip_t *bitpips;
  unsigned *site_index;
} pip_parsed_dense_t;

pip_db_t *get_pipdb(const gchar *datadir);
void free_pipdb(pip_db_t *pipdb);


/* utility functions */

/* This should be benchmarked and run as fast as humanly possible */
pip_parsed_dense_t *
pips_of_bitstream(const pip_db_t *pipdb, const chip_descr_t *chipdb,
		  const bitstream_parsed_t *bitstream);
void free_pipdat(pip_parsed_dense_t *pipdat);

pip_t *pips_of_site(const pip_db_t *pipdb,
		    const bitstream_parsed_t *bitstream,
		    const csite_descr_t *site,
		    gsize *size);

pip_t *pips_of_site_dense(const pip_parsed_dense_t *dat,
			  const site_ref_t site,
			  gsize *size);

gboolean
get_interconnect_startpoint(const pip_parsed_dense_t *pipdat,
			    wire_atom_t *wire,
			    const wire_atom_t orig,
			    const site_ref_t site);

gboolean
get_implicit_startpoint(wire_atom_t *wire,
			const pip_db_t *pipdb,
			const chip_descr_t *chip,
			const wire_atom_t orig,
			const site_ref_t site);

typedef void (*bitpip_iterator_t)(gpointer, const pip_t, const site_ref_t);

typedef int (*sitepip_iterator_t)(unsigned site_x, unsigned site_y,
				  csite_descr_t *site, gpointer dat);

void
iterate_over_bitpips(const pip_parsed_dense_t *pipdat,
		     const chip_descr_t *chip,
		     bitpip_iterator_t fun, gpointer data);

void
iterate_over_bitpips_complex(const pip_parsed_dense_t *pipdat,
			     const chip_descr_t *chip,
			     sitepip_iterator_t fun1, bitpip_iterator_t fun2,
			     gpointer data);

/*
 * Detailed lookup function
 */

int
bitpip_lookup(const sited_pip_t pip,
	      const chip_descr_t *chip,
	      const pip_db_t *pipdb,
	      const unsigned **cfgbits, size_t *nbits,
	      uint32_t *vals);

/*
 * Connexity database API
 */

uint32_t
get_input_wires(const pip_db_t *pipdb,
                const logic_t pip, const switch_type_t swit,
                gsize *size, wire_atom_t **array);

/*
 * Iteration over database
 */

typedef void (* logic_callback_t)(const logic_atom_t start, void *data);

void
iter_logic_input(const pip_db_t *pipdb,
                 const logic_t pip, const switch_type_t swit,
                 logic_callback_t logcall, void *data);

#endif /* _LOCALPIPS_H */
