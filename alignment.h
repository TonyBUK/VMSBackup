#ifndef __alignment_h__
#define __alignment_h__


// Windows

#if 1

#define SWAPSHORT(x) x

#define SWAPLONG(x) x

#define SWAPLONGLONG(x) x

#else

// Solaris

#define SWAPSHORT(x) \
(\
    ((((x) & 0x00ff) << 8) | \
     (((x) & 0xff00) >> 8)) \
)

#define SWAPLONG(x) \
(\
    ((((x) & 0x000000ff) << 24) | \
     (((x) & 0x0000ff00) <<  8) | \
     (((x) & 0x00ff0000) >>  8) | \
     (((x) & 0xff000000) >> 24)) \
)

#define SWAPLONGLONG(x) \
(\
    ((((x) & 0x00000000000000FFull) << 56) | \
     (((x) & 0x000000000000FF00ull) << 40) | \
     (((x) & 0x0000000000FF0000ull) << 24) | \
     (((x) & 0x00000000FF000000ull) <<  8) | \
     (((x) & 0x000000FF00000000ull) >>  8) | \
     (((x) & 0x0000FF0000000000ull) >> 24) | \
     (((x) & 0x00FF000000000000ull) >> 40) | \
     (((x) & 0xFF00000000000000ull) >> 56)) \
)


#endif

#endif // __alignment_h__
