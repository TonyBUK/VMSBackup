#include <memory.h>
#include "BRHeader.h"
#include "alignment.h"

//////////////////////////////////////////////////////////////////////////////
// Block Header

// Lengths
#define BRH__W_RSIZE_LEN            sizeof (uint16_t)
#define BRH__W_RTYPE_LEN            sizeof (uint16_t)
#define BRH__L_FLAGS_LEN            sizeof (uint32_t)
#define BRH__V_BADDATA_LEN          sizeof (uint8_t)
#define BRH__V_DIRECTORY_LEN        sizeof (uint8_t)
#define BRH__V_NONSEQUENTIAL_LEN    sizeof (uint8_t)
#define BRH__V_BLOCKERRS_LEN        sizeof (uint8_t)
#define BRH__V_ALIAS_ENTRY_LEN      sizeof (uint8_t)
#define BRH__V_HEADONLY_LEN         sizeof (uint8_t)
#define BRH__L_ADDRESS_LEN          sizeof (uint32_t)
#define BRH__W_BLOCKFLAGS_LEN       sizeof (uint16_t)
#define BRH__W_RESERVED_LEN         sizeof (uint16_t)

// Addresses
#define BRH__W_RSIZE_ADDR           0
#define BRH__W_RTYPE_ADDR           (BRH__W_RSIZE_ADDR + BRH__W_RSIZE_LEN)
#define BRH__L_FLAGS_ADDR           (BRH__W_RTYPE_ADDR + BRH__W_RTYPE_LEN)
#define BRH__V_BADDATA_ADDR         BRH__L_FLAGS_ADDR
#define BRH__V_DIRECTORY_ADDR       BRH__L_FLAGS_ADDR
#define BRH__V_NONSEQUENTIAL_ADDR   BRH__L_FLAGS_ADDR
#define BRH__V_BLOCKERRS_ADDR       BRH__L_FLAGS_ADDR
#define BRH__V_ALIAS_ENTRY_ADDR     BRH__L_FLAGS_ADDR
#define BRH__V_HEADONLY_ADDR        BRH__L_FLAGS_ADDR
#define BRH__L_ADDRESS_ADDR         (BRH__L_FLAGS_ADDR + BRH__L_FLAGS_LEN)
#define BRH__W_BLOCKFLAGS_ADDR      (BRH__L_ADDRESS_ADDR + BRH__L_ADDRESS_LEN)
#define BRH__W_RESERVED_ADDR        (BRH__W_BLOCKFLAGS_ADDR + BRH__W_BLOCKFLAGS_LEN)

#define BRH__LENGTH                 (BRH__W_RESERVED_ADDR + BRH__W_RESERVED_LEN)

BRHeader::BRHeader ()
{
}

void BRHeader::Clean ()
{
    delete (m_cBuffer);
}

void BRHeader::LoadBRHeader (uint8_t* cBuffer)
{
    // Allocate the Char Buffer
    m_cBuffer           = new uint8_t[BRH__LENGTH];

    // Map each data element
    m_W_RSIZE           = (uint16_t*) &m_cBuffer[BRH__W_RSIZE_ADDR];
    m_W_RTYPE           = (uint16_t*) &m_cBuffer[BRH__W_RTYPE_ADDR];
    m_L_FLAGS           = (uint32_t*)  &m_cBuffer[BRH__L_FLAGS_ADDR];
    m_V_BADDATA         = (uint8_t*)  &m_cBuffer[BRH__V_BADDATA_ADDR];
    m_V_DIRECTORY       = (uint8_t*)  &m_cBuffer[BRH__V_DIRECTORY_ADDR];
    m_V_NONSEQUENTIAL   = (uint8_t*)  &m_cBuffer[BRH__V_NONSEQUENTIAL_ADDR];
    m_V_BLOCKERRS       = (uint8_t*)  &m_cBuffer[BRH__V_BLOCKERRS_ADDR];
    m_V_ALIAS_ENTRY     = (uint8_t*)  &m_cBuffer[BRH__V_ALIAS_ENTRY_ADDR];
    m_V_HEADONLY        = (uint8_t*)  &m_cBuffer[BRH__V_HEADONLY_ADDR];
    m_L_ADDRESS         = (uint32_t*)  &m_cBuffer[BRH__L_ADDRESS_ADDR];
    m_W_BLOCKFLAGS      = (uint16_t*) &m_cBuffer[BRH__W_BLOCKFLAGS_ADDR];

    memcpy (m_cBuffer, cBuffer, BRH__LENGTH * sizeof (uint8_t));
}

uint16_t BRHeader::W_RSIZE ()
{
    return SWAPSHORT (*m_W_RSIZE);
}

uint16_t BRHeader::W_RTYPE ()
{
    return SWAPSHORT (*m_W_RTYPE);
}

uint32_t BRHeader::L_FLAGS ()
{
    return SWAPLONG (*m_L_FLAGS);
}

bool BRHeader::V_BADDATA ()
{
    return (*m_V_BADDATA & 0x80) == 0x80;
}

bool BRHeader::V_DIRECTORY ()
{
    return (*m_V_DIRECTORY & 0x40) == 0x40;
}

bool BRHeader::V_NONSEQUENTIAL ()
{
    return (*m_V_NONSEQUENTIAL & 0x20) == 0x20;
}

bool BRHeader::V_BLOCKERRS ()
{
    return (*m_V_BLOCKERRS & 0x10) == 0x10;
}

bool BRHeader::V_ALIAS_ENTRY ()
{
    return (*m_V_ALIAS_ENTRY & 0x08) == 0x08;
}

bool BRHeader::V_HEADONLY ()
{
    return (*m_V_HEADONLY & 0x04) == 0x04;
}

uint32_t BRHeader::L_ADDRESS ()
{
    return SWAPLONG (*m_L_ADDRESS);
}

uint16_t BRHeader::W_BLOCKFLAGS ()
{
    return SWAPSHORT (*m_W_BLOCKFLAGS);
}

uint32_t BRHeader::GetLength ()
{
    return BRH__LENGTH;
}
