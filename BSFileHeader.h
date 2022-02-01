#ifndef __BSFileHeader_H__
#define __BSFileHeader_H__

#include <stdio.h>
#include <stdint.h>
#include "BSAHeader.h"

// Enumerations for RECATTR[0] : Record Format
#define RECORD_FORMAT_UDF       0   /* undefined */
#define RECORD_FORMAT_FIX       1   /* fixed-length record */
#define RECORD_FORMAT_VAR       2   /* variable-length record */
#define RECORD_FORMAT_VFC       3   /* variable-length with fixed-length control record */
#define RECORD_FORMAT_STM       4   /* RMS-11 stream record (valid only for sequential org) */
#define RECORD_FORMAT_STMLF     5   /* stream record delimited by LF (sequential org only) */
#define RECORD_FORMAT_STMCR     6   /* stream record delimited by CR (sequential org only) */

// Enumerations for RECATTR[1] : Record Attributes
#define RECORD_ATTRIBUTE_FTN    0   /* FORTRAN carriage control character */
#define RECORD_ATTRIBUTE_CR     1   /* line feed - record -carriage return */
#define RECORD_ATTRIBUTE_CRN    2   /* carriage control */
#define RECORD_ATTRIBUTE_BLK    3   /* records don't cross block boundaries */
#define RECORD_ATTRIBUTE_PRN    4   /* print-file carriage control */

class BSFileHeader
{
    public:
                        BSFileHeader        (                       );
        void            Clean               (                       );

        void            LoadBSFileHeader    (   uint8_t*    cBuffer,
                                                uint32_t    uiRSize     );

        uint8_t*        FILENAME            (                           );
        uint8_t*        STRUCLEV            (                           );
        uint8_t*        FID                 (                           );
        uint8_t*        BACKLINK            (                           );
        uint8_t*        FILESIZE            (                           );
        uint8_t*        UIC                 (                           );
        uint8_t*        FPRO                (                           );
        uint8_t*        RPRO                (                           );
        uint8_t*        ACLEVEL             (                           );
        uint8_t*        UCHAR               (                           );
        uint8_t*        RECATTR             (                           );
        uint16_t*       RECSIZE             (                           );
        uint8_t*        REVISION            (                           );
        uint8_t*        CREDATE             (                           );
        uint8_t*        REVDATE             (                           );
        uint8_t*        EXPDATE             (                           );
        uint8_t*        BAKDATE             (                           );
        uint8_t*        VERLIMIT            (                           );
        uint8_t*        HIGHWATER           (                           );

    private:
        BSAHeader*      m_pUnidentified;

        BSAHeader*      m_pcFILENAME;
        BSAHeader*      m_pcSTRUCLEV;
        BSAHeader*      m_pcFID;
        BSAHeader*      m_pcBACKLINK;
        BSAHeader*      m_pcFILESIZE;
        BSAHeader*      m_pcUIC;
        BSAHeader*      m_pcFPRO;
        BSAHeader*      m_pcRPRO;
        BSAHeader*      m_pcACLEVEL;
        BSAHeader*      m_pcUCHAR;
        BSAHeader*      m_pcRECATTR;
        BSAHeader*      m_pcREVISION;
        BSAHeader*      m_pcCREDATE;
        BSAHeader*      m_pcREVDATE;
        BSAHeader*      m_pcEXPDATE;
        BSAHeader*      m_pcBAKDATE;

        BSAHeader*      m_pcVERLIMIT;
        BSAHeader*      m_pcHIGHWATER;
};

#endif // __BSFileHeader_H__
