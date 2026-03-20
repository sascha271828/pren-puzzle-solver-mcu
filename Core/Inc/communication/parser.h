#ifndef __PARSER_H__
#define __PARSER_H__

#include <stdbool.h>
#include <stdint.h>

/* parse the received data to the package */
/* package will be stored in the c file of parser */

typedef struct{
    uint32_t start_x;
    uint32_t start_y;
    uint32_t end_x;
    uint32_t end_y;
    int32_t rotation;
}DataPiece_t;

typedef struct{
    uint8_t orginal_header;
    bool SixPiece;
}DataHeader_t;

typedef struct{
    DataHeader_t header;
    DataPiece_t pieces[6]; 
    uint8_t checksum;/* checksum CRC-16 */
}DataPackage_t;



bool GetDataPackage(DataPackage_t* package);


#endif /* __PARSER_H__ */