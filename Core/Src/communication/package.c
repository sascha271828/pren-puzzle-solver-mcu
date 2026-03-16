#include "package.h"

static int32_t com_parse_int32_t(const uint8_t *data) {
  return ((int32_t)(((data[3]) << 12) | ((data[2]) << 8) | ((data[1]) << 4) |
                    (data[0])));
}

static int16_t com_parse_int16_t(const uint8_t *data) {
  return ((int16_t)(((data[1]) << 4) | (data[0])));
}

static void com_parse_header(data_block_t *block, uint8_t header) {
  block->has_6_pieces = COM_HEADER_HAS_6_PIECES(header);
  block->header_parsed = true;
}

static void com_parse_body(const data_block_t *block,
                           uint8_t *data,
                           size_t piece_number) {
  block->pieces[piece_number]->start_x = com_parse_int32_t(data);
  block->pieces[piece_number]->start_y =
      com_parse_int32_t(data + 1 * (sizeof(int32_t)));
  block->pieces[piece_number]->end_x =
      com_parse_int32_t(data + 2 * (sizeof(int32_t)));
  block->pieces[piece_number]->end_y =
      com_parse_int32_t(data + 3 * (sizeof(int32_t)));

  block->pieces[piece_number]->rotation =
      com_parse_int16_t(data + 4 * (sizeof(int32_t)));
}

bool com_parse_data(data_block_t *block, uint8_t *data, size_t length_data) {
  if (!block | !data) return false;

  com_parse_header(block, data[0]);

  for (size_t i = 0; i < (block->has_6_pieces ? 6 : 4); i++) {
    com_parse_body(
        block, data + COM_LENGTH_HEADER + sizeof(data_piece_t) * i, i);
  }
  return true;
}

// static bool com_check_length();

// static bool com_checksum(); /* TODO */
