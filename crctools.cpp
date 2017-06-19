#include "crctools.h"





crcTools::crcTools()
{

}

crcTools::~crcTools()
{

}

/*
 *实现crc计算
 *
 *
 *
 *
 */
uint16_t crcTools::ucMBCRC16(char* pucFrame, uint16_t usLen)
{
    uint8_t    ucCRCHi = 0xFF;
    uint8_t    ucCRCLo = 0xFF;
    int        iIndex;

    while( usLen-- )
    {
        iIndex = ucCRCLo ^ *( pucFrame++ );
        ucCRCLo = ( uint8_t )( ucCRCHi ^ aucCRCHi[iIndex] );
        ucCRCHi = aucCRCLo[iIndex];
    }
    return ( uint16_t )( ucCRCHi << 8 | ucCRCLo );
}



