#ifndef __BRHeader_H__
#define __BRHeader_H__

#include <stdio.h>
#include <stdint.h>

#define RECORD_NULL 0                    /* null record                      */
#define RECORD_SUMMARY 1                 /* BACKUP summary record            */
#define RECORD_VOLUME 2                  /* volume summary record            */
#define RECORD_FILE 3                    /* file attribute record            */
#define RECORD_VBN 4                     /* file virtual block record        */
#define RECORD_PHYSVOL 5                 /* physical volume attribute record  */
#define RECORD_LBN 6                     /* physical volume logical block record  */
#define RECORD_FID 7                     /* file ID record                   */
#define RECORD_FILE_EXT 8                /* file attribute extension record  */
#define RECORD_LBN_576 9                 /* 576 byte LBN record              */
#define RECORD_RS_DIRATTR 10             /* RSTS directory attribute record  */
#define RECORD_ALIAS 11
#define RECORD_MAX_RTYPE 12              /* max record type                  */

class BRHeader
{
    public:
                        BRHeader            (                       );
        void            Clean               (                       );

        void            LoadBRHeader        (   uint8_t* cBuffer    );

        uint16_t        W_RSIZE             (                       );
        uint16_t        W_RTYPE             (                       );
        uint32_t        L_FLAGS             (                       );
        bool            V_BADDATA           (                       );
        bool            V_DIRECTORY         (                       );
        bool            V_NONSEQUENTIAL     (                       );
        bool            V_BLOCKERRS         (                       );
        bool            V_ALIAS_ENTRY       (                       );
        bool            V_HEADONLY          (                       );
        uint32_t        L_ADDRESS           (                       );
        uint16_t        W_BLOCKFLAGS        (                       );

        uint32_t        GetLength           (                       );

    private:
        uint8_t*        m_cBuffer;

        uint16_t*       m_W_RSIZE;
        uint16_t*       m_W_RTYPE;
        uint32_t*       m_L_FLAGS;
        uint8_t*        m_V_BADDATA;
        uint8_t*        m_V_DIRECTORY;
        uint8_t*        m_V_NONSEQUENTIAL;
        uint8_t*        m_V_BLOCKERRS;
        uint8_t*        m_V_ALIAS_ENTRY;
        uint8_t*        m_V_HEADONLY;
        uint32_t*       m_L_ADDRESS;
        uint16_t*       m_W_BLOCKFLAGS;
};

#endif // __BRHeader_H__
