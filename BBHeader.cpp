#include <memory.h>
#include "BBHeader.h"
#include "alignment.h"

//////////////////////////////////////////////////////////////////////////////
// Block Header

// Lengths
#define BBH__W_SIZE_LEN         sizeof (uint16_t)
#define BBH__W_OPSYS_LEN        sizeof (uint16_t)
#define BBH__W_SUBSYS_LEN       sizeof (uint16_t)
#define BBH__W_APPLIC_LEN       sizeof (uint16_t)
#define BBH__L_NUMBER_LEN       sizeof (uint32_t)
#define BBH__FILL_1_LEN         (20 * sizeof (int8_t))
#define BBH__W_STRUCLEV_LEN     sizeof (uint16_t)
#define BBH__B_STRUCVER_LEN     sizeof (uint8_t)
#define BBH__B_STRUCLEV_LEN     sizeof (uint8_t)
#define BBH__W_VOLNUM_LEN       sizeof (uint16_t)
#define BBH__L_CRC_LEN          sizeof (uint32_t)
#define BBH__L_BLOCKSIZE_LEN    sizeof (uint32_t)
#define BBH__L_FLAGS_LEN        sizeof (uint32_t)
#define BBH__V_NOCRC_LEN        sizeof (uint8_t)
#define BBH__T_SSNAME_LEN       (32 * sizeof (int8_t))
#define BBH__W_FID_LEN          (3 * sizeof (uint16_t))
#define BBH__W_FID_NUM_LEN      sizeof (uint16_t)
#define BBH__W_FID_SEQ_LEN      sizeof (uint16_t)
#define BBH__W_FID_RVN_LEN      sizeof (uint16_t)
#define BBH__B_FID_RVN_LEN      sizeof (uint8_t)
#define BBH__B_FID_NMX_LEN      sizeof (uint8_t)
#define BBH__W_DID_LEN          (3 * sizeof (uint16_t))
#define BBH__W_DID_NUM_LEN      sizeof (uint16_t)
#define BBH__W_DID_SEQ_LEN      sizeof (uint8_t)
#define BBH__W_DID_RVN_LEN      sizeof (uint8_t)
#define BBH__B_DID_RVN_LEN      sizeof (uint8_t)
#define BBH__B_DID_NMX_LEN      sizeof (uint8_t)
#define BBH__T_FILENAME_LEN     (128 * sizeof (int8_t))
#define BBH__B_RTYPE_LEN        sizeof (uint8_t)
#define BBH__B_RATTRIB_LEN      sizeof (uint8_t)
#define BBH__W_RSIZE_LEN        sizeof (uint16_t)
#define BBH__B_BKTSIZE_LEN      sizeof (uint8_t)
#define BBH__B_VFCSIZE_LEN      sizeof (uint8_t)
#define BBH__W_MAXREC_LEN       sizeof (uint16_t)
#define BBH__L_FILESIZE_LEN     sizeof (uint32_t)
#define BBH__T_RESERVED2_LEN    (22 * sizeof (int8_t))
#define BBH__W_CHECKSUM_LEN     sizeof (uint16_t)

// Addresses
#define BBH__W_SIZE_ADDR        0
#define BBH__W_OPSYS_ADDR       (BBH__W_SIZE_ADDR + BBH__W_SIZE_LEN)
#define BBH__W_SUBSYS_ADDR      (BBH__W_OPSYS_ADDR + BBH__W_OPSYS_LEN)
#define BBH__W_APPLIC_ADDR      (BBH__W_SUBSYS_ADDR + BBH__W_SUBSYS_LEN)
#define BBH__L_NUMBER_ADDR      (BBH__W_APPLIC_ADDR + BBH__W_APPLIC_LEN)
#define BBH__FILL_1_ADDR        (BBH__L_NUMBER_ADDR + BBH__L_NUMBER_LEN)
#define BBH__W_STRUCLEV_ADDR    (BBH__FILL_1_ADDR + BBH__FILL_1_LEN)
#define BBH__B_STRUCVER_ADDR    BBH__W_STRUCLEV_ADDR
#define BBH__B_STRUCLEV_ADDR    (BBH__B_STRUCVER_ADDR + BBH__B_STRUCVER_LEN)
#define BBH__W_VOLNUM_ADDR      (BBH__W_STRUCLEV_ADDR + BBH__W_STRUCLEV_LEN)
#define BBH__L_CRC_ADDR         (BBH__W_VOLNUM_ADDR + BBH__W_VOLNUM_LEN)
#define BBH__L_BLOCKSIZE_ADDR   (BBH__L_CRC_ADDR + BBH__L_CRC_LEN)
#define BBH__L_FLAGS_ADDR       (BBH__L_BLOCKSIZE_ADDR + BBH__L_BLOCKSIZE_LEN)
#define BBH__V_NOCRC_ADDR       BBH__L_FLAGS_ADDR
#define BBH__T_SSNAME_ADDR      (BBH__L_FLAGS_ADDR + BBH__L_FLAGS_LEN)
#define BBH__W_FID_ADDR         (BBH__T_SSNAME_ADDR + BBH__T_SSNAME_LEN)
#define BBH__W_FID_NUM_ADDR     BBH__W_FID_ADDR
#define BBH__W_FID_SEQ_ADDR     (BBH__W_FID_NUM_ADDR + BBH__W_FID_NUM_LEN)
#define BBH__W_FID_RVN_ADDR     (BBH__W_FID_SEQ_ADDR + BBH__W_FID_SEQ_LEN)
#define BBH__B_FID_RVN_ADDR     BBH__W_FID_RVN_ADDR
#define BBH__B_FID_NMX_ADDR     (BBH__B_FID_RVN_ADDR + BBH__B_FID_RVN_LEN)
#define BBH__W_DID_ADDR         BBH__W_FID_ADDR
#define BBH__W_DID_NUM_ADDR     BBH__W_DID_ADDR
#define BBH__W_DID_SEQ_ADDR     (BBH__W_DID_NUM_ADDR + BBH__W_DID_NUM_LEN)
#define BBH__W_DID_RVN_ADDR     (BBH__W_DID_SEQ_ADDR + BBH__W_DID_SEQ_LEN)
#define BBH__B_DID_RVN_ADDR     BBH__W_DID_RVN_ADDR
#define BBH__B_DID_NMX_ADDR     (BBH__B_DID_RVN_ADDR + BBH__B_DID_RVN_LEN)
#define BBH__T_FILENAME_ADDR    (BBH__W_FID_ADDR + BBH__W_FID_LEN)
#define BBH__B_RTYPE_ADDR       (BBH__T_FILENAME_ADDR + BBH__T_FILENAME_LEN)
#define BBH__B_RATTRIB_ADDR     (BBH__B_RTYPE_ADDR + BBH__B_RTYPE_LEN)
#define BBH__W_RSIZE_ADDR       (BBH__B_RATTRIB_ADDR + BBH__B_RATTRIB_LEN)
#define BBH__B_BKTSIZE_ADDR     (BBH__W_RSIZE_ADDR + BBH__W_RSIZE_LEN)
#define BBH__B_VFCSIZE_ADDR     (BBH__B_BKTSIZE_ADDR + BBH__B_BKTSIZE_LEN)
#define BBH__W_MAXREC_ADDR      (BBH__B_VFCSIZE_ADDR + BBH__B_VFCSIZE_LEN)
#define BBH__L_FILESIZE_ADDR    (BBH__W_MAXREC_ADDR + BBH__W_MAXREC_LEN)
#define BBH__T_RESERVED2_ADDR   (BBH__L_FILESIZE_ADDR + BBH__L_FILESIZE_LEN)
#define BBH__W_CHECKSUM_ADDR    (BBH__T_RESERVED2_ADDR + BBH__T_RESERVED2_LEN)

// Minimum Header Size
#define BBH__LENGTH             (BBH__W_CHECKSUM_ADDR + BBH__W_CHECKSUM_LEN)

BBHeader::BBHeader ()
{
    // Allocate the Char Buffer
    m_cBuffer           = new uint8_t[BBH__LENGTH];

    // Map each data element
    m_W_SIZE            = (uint16_t*) &m_cBuffer[BBH__W_SIZE_ADDR];
    m_W_SUBSYS          = (uint16_t*) &m_cBuffer[BBH__W_SUBSYS_ADDR];
    m_W_OPSYS           = (uint16_t*) &m_cBuffer[BBH__W_OPSYS_ADDR];
    m_W_APPLIC          = (uint16_t*) &m_cBuffer[BBH__W_APPLIC_ADDR];
    m_L_NUMBER          = (uint32_t*)  &m_cBuffer[BBH__L_NUMBER_ADDR];
    m_W_STRUCLEV        = (uint16_t*) &m_cBuffer[BBH__W_STRUCLEV_ADDR];
    m_B_STRUCVER        = (uint8_t*)  &m_cBuffer[BBH__B_STRUCVER_ADDR];
    m_B_STRUCLEV        = (uint8_t*)  &m_cBuffer[BBH__B_STRUCLEV_ADDR];
    m_W_VOLNUM          = (uint16_t*) &m_cBuffer[BBH__W_VOLNUM_ADDR];
    m_L_CRC             = (uint32_t*)  &m_cBuffer[BBH__L_CRC_ADDR];
    m_L_BLOCKSIZE       = (uint32_t*)  &m_cBuffer[BBH__L_BLOCKSIZE_ADDR];
    m_L_FLAGS           = (uint32_t*)  &m_cBuffer[BBH__L_FLAGS_ADDR];
    m_V_NOCRC           = (uint8_t*)  &m_cBuffer[BBH__V_NOCRC_ADDR];
    m_T_SSNAME          = (uint8_t*)  &m_cBuffer[BBH__T_SSNAME_ADDR];
    m_W_FID             = (uint16_t*) &m_cBuffer[BBH__W_FID_ADDR];
    m_W_FID_NUM         = (uint16_t*) &m_cBuffer[BBH__W_FID_NUM_ADDR];
    m_W_FID_SEQ         = (uint16_t*) &m_cBuffer[BBH__W_FID_SEQ_ADDR];
    m_W_FID_RVN         = (uint16_t*) &m_cBuffer[BBH__W_FID_RVN_ADDR];
    m_B_FID_RVN         = (uint8_t*)  &m_cBuffer[BBH__B_FID_RVN_ADDR];
    m_B_FID_NMX         = (uint8_t*)  &m_cBuffer[BBH__B_FID_NMX_ADDR];
    m_W_DID             = (uint16_t*) &m_cBuffer[BBH__W_DID_ADDR];
    m_W_DID_NUM         = (uint16_t*) &m_cBuffer[BBH__W_DID_NUM_ADDR];
    m_W_DID_SEQ         = (uint16_t*) &m_cBuffer[BBH__W_DID_SEQ_ADDR];
    m_W_DID_RVN         = (uint16_t*) &m_cBuffer[BBH__W_DID_RVN_ADDR];
    m_B_DID_RVN         = (uint8_t*)  &m_cBuffer[BBH__B_DID_RVN_ADDR];
    m_B_DID_NMX         = (uint8_t*)  &m_cBuffer[BBH__B_DID_NMX_ADDR];
    m_T_FILENAME        = (int8_t*)  &m_cBuffer[BBH__T_FILENAME_ADDR];
    m_B_RTYPE           = (uint8_t*)  &m_cBuffer[BBH__B_RTYPE_ADDR];
    m_B_RATTRIB         = (uint8_t*)  &m_cBuffer[BBH__B_RATTRIB_ADDR];
    m_W_RSIZE           = (uint16_t*) &m_cBuffer[BBH__W_RSIZE_ADDR];
    m_B_BKTSIZE         = (uint8_t*)  &m_cBuffer[BBH__B_BKTSIZE_ADDR];
    m_B_VFCSIZE         = (uint8_t*)  &m_cBuffer[BBH__B_VFCSIZE_ADDR];
    m_W_MAXREC          = (uint16_t*) &m_cBuffer[BBH__W_MAXREC_ADDR];
    m_L_FILESIZE        = (uint32_t*)  &m_cBuffer[BBH__L_FILESIZE_ADDR];
    m_W_CHECKSUM        = (uint16_t*) &m_cBuffer[BBH__W_CHECKSUM_ADDR];
}

void BBHeader::Clean ()
{
    delete (m_cBuffer);
}

void BBHeader::LoadBBHeader (uint8_t* cBuffer)
{
    memcpy (m_cBuffer, cBuffer, BBH__LENGTH * sizeof (uint8_t));
}

void BBHeader::LoadBBHeader (FILE* pBuffer)
{
    fread (m_cBuffer, sizeof (uint8_t), BBH__LENGTH, pBuffer);
}

uint16_t BBHeader::W_SIZE ()
{
    return SWAPSHORT (*m_W_SIZE);
}

uint16_t BBHeader::W_OPSYS ()
{
    return SWAPSHORT (*m_W_OPSYS);
}

uint16_t BBHeader::W_SUBSYS ()
{
    return SWAPSHORT (*m_W_SUBSYS);
}

uint16_t BBHeader::W_APPLIC ()
{
    return SWAPSHORT (*m_W_APPLIC);
}

uint32_t BBHeader::L_NUMBER ()
{
    return SWAPLONG (*m_L_NUMBER);
}

uint16_t BBHeader::W_STRUCLEV ()
{
    return SWAPSHORT (*m_W_STRUCLEV);
}

uint8_t BBHeader::B_STRUCVER ()
{
    return *m_B_STRUCVER;
}

uint8_t BBHeader::B_STRUCLEV ()
{
    return *m_B_STRUCLEV;
}

uint16_t BBHeader::W_VOLNUM ()
{
    return SWAPSHORT (*m_W_VOLNUM);
}

uint32_t BBHeader::L_CRC ()
{
    return SWAPLONG (*m_L_CRC);
}

uint32_t BBHeader::L_BLOCKSIZE ()
{
    return SWAPLONG (*m_L_BLOCKSIZE);
}

uint32_t BBHeader::L_FLAGS ()
{
    return SWAPLONG (*m_L_FLAGS);
}

bool BBHeader::V_NOCRC ()
{
    return ((*m_V_NOCRC) & 0x80) == 0x80;
}

uint8_t* BBHeader::T_SSNAME ()
{
    // Skip past the first element, which is the length
    return &m_T_SSNAME[1];
}

uint16_t* BBHeader::W_FID ()
{
    return m_W_FID;
}

uint16_t BBHeader::W_FID_NUM ()
{
    return SWAPSHORT (*m_W_FID_NUM);
}

uint16_t BBHeader::W_FID_SEQ ()
{
    return SWAPSHORT (*m_W_FID_SEQ);
}

uint16_t BBHeader::W_FID_RVN ()
{
    return SWAPSHORT (*m_W_FID_RVN);
}

uint8_t BBHeader::B_FID_RVN ()
{
    return *m_B_FID_RVN;
}

uint8_t BBHeader::B_FID_NMX ()
{
    return *m_B_FID_NMX;
}

uint16_t* BBHeader::W_DID ()
{
    return m_W_DID;
}

uint16_t BBHeader::W_DID_NUM ()
{
    return SWAPSHORT (*m_W_DID_NUM);
}

uint16_t BBHeader::W_DID_SEQ ()
{
    return SWAPSHORT (*m_W_DID_SEQ);
}

uint16_t BBHeader::W_DID_RVN ()
{
    return SWAPSHORT (*m_W_DID_RVN);
}

uint8_t BBHeader::B_DID_RVN ()
{
    return *m_B_DID_RVN;
}

uint8_t BBHeader::B_DID_NMX ()
{
    return *m_B_DID_NMX;
}

int8_t* BBHeader::T_FILENAME ()
{
    return m_T_FILENAME;
}

uint8_t BBHeader::B_RTYPE ()
{
    return *m_B_RTYPE;
}

uint8_t BBHeader::B_RATTRIB ()
{
    return *m_B_RATTRIB;
}

uint16_t BBHeader::W_RSIZE ()
{
    return SWAPSHORT (*m_W_RSIZE);
}

uint8_t BBHeader::B_BKTSIZE ()
{
    return *m_B_BKTSIZE;
}

uint8_t BBHeader::B_VFCSIZE ()
{
    return *m_B_VFCSIZE;
}

uint16_t BBHeader::W_MAXREC ()
{
    return SWAPSHORT (*m_W_MAXREC);
}

uint32_t BBHeader::L_FILESIZE ()
{
    return SWAPLONG (*m_L_FILESIZE);
}

uint16_t BBHeader::W_CHECKSUM ()
{
    return SWAPSHORT (*m_W_CHECKSUM);
}

uint32_t BBHeader::GetLength ()
{
    return (uint32_t) W_SIZE ();
}

bool BBHeader::Validate (BBHeader* pBaseHeader)
{
    //////////////////////////////////////////////////////////////////////////
    // Variables

    // Block Header Validity
    bool        bValid          = true;

    // Validate the Current Block
    bValid  = bValid && ValidateBlockSize (pBaseHeader);

    return bValid;
}

bool BBHeader::ValidateBlockSize (BBHeader* pBaseHeader)
{
    // Valid Block Sizes are 0 or the same size as the first block
    return (L_BLOCKSIZE () == 0) || (L_BLOCKSIZE () == pBaseHeader->L_BLOCKSIZE ());
}
