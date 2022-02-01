#ifndef __BBHeader_H__
#define __BBHeader_H__

#include <stdio.h>
#include <stdint.h>

class BBHeader
{
    public:
                        BBHeader            (                               );

        void            LoadBBHeader        (   uint8_t* cBuffer            );
        void            LoadBBHeader        (   FILE*   pBuffer             );
        void            Clean               (                               );

        uint16_t        W_SIZE              (                               );
        uint16_t        W_OPSYS             (                               );
        uint16_t        W_SUBSYS            (                               );
        uint16_t        W_APPLIC            (                               );
        uint32_t        L_NUMBER            (                               );
        uint16_t        W_STRUCLEV          (                               );
        uint8_t         B_STRUCVER          (                               );
        uint8_t         B_STRUCLEV          (                               );
        uint16_t        W_VOLNUM            (                               );
        uint32_t        L_CRC               (                               );
        uint32_t        L_BLOCKSIZE         (                               );
        uint32_t        L_FLAGS             (                               );
        bool            V_NOCRC             (                               );
        uint8_t*        T_SSNAME            (                               );
        uint16_t*       W_FID               (                               );
        uint16_t        W_FID_NUM           (                               );
        uint16_t        W_FID_SEQ           (                               );
        uint16_t        W_FID_RVN           (                               );
        uint8_t         B_FID_RVN           (                               );
        uint8_t         B_FID_NMX           (                               );
        uint16_t*       W_DID               (                               );
        uint16_t        W_DID_NUM           (                               );
        uint16_t        W_DID_SEQ           (                               );
        uint16_t        W_DID_RVN           (                               );
        uint8_t         B_DID_RVN           (                               );
        uint8_t         B_DID_NMX           (                               );
        int8_t*         T_FILENAME          (                               );
        uint8_t         B_RTYPE             (                               );
        uint8_t         B_RATTRIB           (                               );
        uint16_t        W_RSIZE             (                               );
        uint8_t         B_BKTSIZE           (                               );
        uint8_t         B_VFCSIZE           (                               );
        uint16_t        W_MAXREC            (                               );
        uint32_t        L_FILESIZE          (                               );
        uint16_t        W_CHECKSUM          (                               );

        uint32_t        GetLength           (                               );

        bool            Validate            (   BBHeader*   pBaseHeader     );

    private:

        bool            ValidateBlockSize   (   BBHeader*   pBaseHeader     );

        uint8_t*        m_cBuffer;

        uint16_t*       m_W_SIZE;
        uint16_t*       m_W_OPSYS;
        uint16_t*       m_W_SUBSYS;
        uint16_t*       m_W_APPLIC;
        uint32_t*       m_L_NUMBER;
        uint16_t*       m_W_STRUCLEV;
        uint8_t*        m_B_STRUCVER;
        uint8_t*        m_B_STRUCLEV;
        uint16_t*       m_W_VOLNUM;
        uint32_t*       m_L_CRC;
        uint32_t*       m_L_BLOCKSIZE;
        uint32_t*       m_L_FLAGS;
        uint8_t*        m_V_NOCRC;
        uint8_t*        m_T_SSNAME;
        uint16_t*       m_W_FID;
        uint16_t*       m_W_FID_NUM;
        uint16_t*       m_W_FID_SEQ;
        uint16_t*       m_W_FID_RVN;
        uint8_t*        m_B_FID_RVN;
        uint8_t*        m_B_FID_NMX;
        uint16_t*       m_W_DID;
        uint16_t*       m_W_DID_NUM;
        uint16_t*       m_W_DID_SEQ;
        uint16_t*       m_W_DID_RVN;
        uint8_t*        m_B_DID_RVN;
        uint8_t*        m_B_DID_NMX;
        int8_t*         m_T_FILENAME;
        uint8_t*        m_B_RTYPE;
        uint8_t*        m_B_RATTRIB;
        uint16_t*       m_W_RSIZE;
        uint8_t*        m_B_BKTSIZE;
        uint8_t*        m_B_VFCSIZE;
        uint16_t*       m_W_MAXREC;
        uint32_t*       m_L_FILESIZE;
        uint16_t*       m_W_CHECKSUM;
};

#endif // __BBHeader_H__
