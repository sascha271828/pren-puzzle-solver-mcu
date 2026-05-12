#ifndef __PARSER_H__
#define __PARSER_H__

/**
 * @file parser.h
 * @brief Legacy binary packet parser — superseded, no implementation.
 *
 * Defines the original binary framing types (DataPiece_t, DataHeader_t,
 * DataPackage_t) from the early protocol design. There is no corresponding
 * parser.c; communication is now handled via protobuf (puzzle.pb.h /
 * command_dispatcher.h). This header is kept for reference only.
 */

#include <stdbool.h>
#include <stdint.h>

/** @brief Coordinates and rotation for a single puzzle piece. */
typedef struct {
  uint32_t start_x;  /**< Pick position X [mm or steps, TBD]   */
  uint32_t start_y;  /**< Pick position Y [mm or steps, TBD]   */
  uint32_t end_x;    /**< Place position X [mm or steps, TBD]  */
  uint32_t end_y;    /**< Place position Y [mm or steps, TBD]  */
  int32_t rotation;  /**< Rotation to apply [0.1° units, TBD]  */
} DataPiece_t;

/** @brief Header of a binary data packet. */
typedef struct {
  uint8_t orginal_header; /**< Raw header byte from the sender          */
  bool SixPiece;          /**< True if the packet contains 6 pieces     */
} DataHeader_t;

/** @brief Complete binary data packet (header + up to 6 pieces + checksum). */
typedef struct {
  DataHeader_t header;
  DataPiece_t pieces[6];
  uint8_t checksum; /**< CRC-16 checksum (not implemented) */
} DataPackage_t;

/**
 * @brief Attempts to retrieve a decoded data package.
 *        Not implemented — always returns false.
 *
 * @param package  Output buffer for the decoded package.
 * @return false   No implementation exists.
 */
bool GetDataPackage(DataPackage_t* package);

#endif /* __PARSER_H__ */