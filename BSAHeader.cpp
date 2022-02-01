#include <memory.h>
#include "BSAHeader.h"
#include "alignment.h"

//////////////////////////////////////////////////////////////////////////////
// Block Header

// Lengths
#define BSA__W_SIZE_LEN         sizeof (uint16_t)
#define BSA__W_TYPE_LEN         sizeof (uint16_t)
#define BSA__T_TEXT_LEN         sizeof (uint16_t)

// Addresses
#define BSA__W_SIZE_ADDR        0
#define BSA__W_TYPE_ADDR        (BSA__W_SIZE_ADDR + BSA__W_SIZE_LEN)
#define BSA__T_TEXT_ADDR        (BSA__W_TYPE_ADDR + BSA__W_TYPE_LEN)

// Minimum Header Size
#define BSA__LENGTH             (W_SIZE () + 4)

// Purely for the purposes of copying data from the C Buffer
#define W_SIZE_FROM_X(x)        SWAPSHORT (x) + 4

BSAHeader::BSAHeader ()
{
}

void BSAHeader::Clean ()
{
    delete (m_cBuffer);
}

void BSAHeader::LoadBSAHeader (uint8_t* cBuffer)
{
    uint32_t        uiBufferSize    = 512;
    uint16_t        pBuffer;

    /* Interestingly on some systems, it's impossible to overlay a short
       buffer on a byte buffer due to alignment constraints.  So lets
       handle this manually */
    memcpy (&pBuffer, cBuffer, sizeof (uint16_t));

    // Allocate the Char Buffer
    // 512 is seen as a "safe" size
    if ((W_SIZE_FROM_X(pBuffer) * sizeof (uint8_t) + 2) < uiBufferSize)
    {
        uiBufferSize    = (W_SIZE_FROM_X(pBuffer) * sizeof (uint8_t) + 2);
    }

    m_cBuffer           = new uint8_t[uiBufferSize];
    memset (m_cBuffer, 0, uiBufferSize);

    // Map each data element
    m_W_SIZE            = (uint16_t*) &m_cBuffer[BSA__W_SIZE_ADDR];
    m_W_TYPE            = (uint16_t*) &m_cBuffer[BSA__W_TYPE_ADDR];
    m_T_TEXT            = (uint8_t*) &m_cBuffer[BSA__T_TEXT_ADDR];

    memcpy (m_cBuffer, cBuffer, (uiBufferSize - 2) * sizeof (uint8_t));
}

uint16_t BSAHeader::W_SIZE ()
{
    return SWAPSHORT (*m_W_SIZE);
}

uint16_t BSAHeader::W_TYPE ()
{
    return SWAPSHORT (*m_W_TYPE);
}

uint8_t* BSAHeader::T_TEXT ()
{
    return m_T_TEXT;
}

uint32_t BSAHeader::GetLength ()
{
    return (uint32_t) BSA__LENGTH;
}
