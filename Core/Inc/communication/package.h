#ifndef __PACKAGE_H__
#define __PACKAGE_H__

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>


/* do not change unless whole code refactored! */

/* header */
/* in Bytes*/
#define COM_LENGTH_HEADER (1)
#define COM_HEADER_HAS_6_PIECES(a) ((a) & (1 << (0)))

#define COM_LENGTH_BODY_PIECE (16) /* length of one peice in the */

#define COM_BODY_OFFSET_POSITION
#define COM_BODY_OFFSET_ROTATION

typedef struct {
  int32_t start_x;
  int32_t start_y;
  int32_t end_x;
  int32_t end_y;
  int16_t rotation;
} data_piece_t;

typedef struct {
  data_piece_t *pieces[6];
  /* header max 1 byte */
  bool header_parsed;
  bool has_6_pieces;
} data_block_t;

bool com_parse_data(data_block_t *block, uint8_t *data, size_t length_data);

#endif /* __PACKAGE_H__ */