#ifndef __BSAHeader_H__
#define __BSAHeader_H__

#include <stdio.h>
#include <stdint.h>

class BSAHeader
{
    public:
                        BSAHeader           (                       );
        void            Clean               (                       );

        void            LoadBSAHeader       (   uint8_t* cBuffer    );

        uint16_t        W_SIZE              (                       );
        uint16_t        W_TYPE              (                       );
        uint8_t*        T_TEXT              (                       );

        uint32_t        GetLength           (                       );

    private:
        uint8_t*        m_cBuffer;

        uint16_t*       m_W_SIZE;
        uint16_t*       m_W_TYPE;
        uint8_t*        m_T_TEXT;
};

#endif // __BSAHeader_H__
