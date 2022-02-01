# VMSBackup
OpenVMS Save Set Reader

This is a pretty old reverse engineering project of mine.  This will take a valid OpenVMS tape set (.BCK) file and extract the contents.  Primarily this is designed to produce the least mangled files on a modern OS, but there are comprimises to be made.  The Tape Set format is a wrapper for the PDP-11 file system, which stores as meta-data within the file system itself critical data to a file that would these days form part of the file data itself, primarily relating to record structures.

For the most part, this is a relatively useful way of grabbing text/binary data from a save set archive, but I've also left a raw mode that will embed file system data as well which could be useful if trying to rehost one of the more esoteric file formats.

This was one of my first attempts to try and write something to work in different architectures, and whilst not automatic, I have heard of this working on PPC, Intel, Sparc architectures amongst others.
