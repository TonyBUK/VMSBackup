# VMSBackup (C++)
OpenVMS Save Set Reader (C++)

This is a pretty old reverse engineering project of mine.  This will take a valid OpenVMS tape set (.BCK) file and extract the contents.  Primarily this is designed to produce the least mangled files on a modern OS, but there are comprimises to be made.  The Tape Set format is a wrapper for the PDP-11 file system, which stores as meta-data within the file system itself critical data to a file that would these days form part of the file data itself, primarily relating to record structures.

For the most part, this is a relatively useful way of grabbing text/binary data from a save set archive, but I've also left a raw mode that will embed file system data as well which could be useful if trying to rehost one of the more esoteric file formats.

This was one of my first attempts to try and write something to work in different architectures, and whilst not automatic, I have heard of this working on PPC, Intel, Sparc architectures amongst others.

One very nice feature of this tool that I'm pretty proud of is the auto mode (by default) where it will attempt to auto-detect between text/binary files to produce something that's compatible with your OS of choice.

An alternative to this is the Python version, which you can grab here: https://github.com/TonyBUK/VMSBackupPython

Effectively the same tool, but a little more portable, a little slower, but with a couple of extra features.  I'll be maintaining both.

***Updates***

1.7 is a very tiny bugfix to remove superflous new lines appearing in rare cases in ASCII mode files.  My next goal after this is a code cleanup, to remove alot of the C'isms littered throughout the code.

1.6 is mostly minor changes brought about by comparing with the Python Port.

- Fixed a crashing bug where one of the backup save sets was actually crashing due to the record length apparently being larger than a block.
- Fixed an issue where some files had record legnths of 0xFFFF.
- Fixed some minor output issues.
- Replaced some nonsensical linked list processing with STL Maps.  Over time I'm probably going to leverage STL more to simplify the code, especially with string processing, as well as bring the code base more in-line with the Python Port.
- Fixed issue in wildcards where [] was interpetted as a set, for VMS files it makes more sense to process these as literally ASCII characters.
- Fixed issue where file formats seemed to have data in the upper nibble, masked for now.
- Added a simple python script to make the "-F" flag more tolerable, as this will remove the file path from the file name, and actually move the files into the associated paths.  Main reason I skipped this before was due to how OS specific some of the operations are, whereas at least with Python, someone else has dealt with all of that for me.


***Usage***

    VMSBackup [FILE] [-L:listoption] [-N] [-X:extractmode] [-M:mask] [-F] [-V] [-D] [-?]

      FILE           Backup Data Set
      -L             Selects output list
      listoptions     S  Suppress Output             B  Brief Output (default)
                      F  Full Output                 C  CSV Output
      -N             Don't Extract File Contents
      -X             Selects Extraction Mode
      extractmode     S  Smart/Auto Mode (default)   A  ASCII Mode
                      B  Binary Mode                 R  Raw Mode
      -M             File Extraction Mask
                      e.g. *.*, *.bin;*, *a*.*;-1 etc.
                      Default is *.*;0.
      -F             Extract with full path (default off)
      -V             Extract with version numbers in the filename (default off)
      -D             Debug Mode (default off)
      -DD            Enhanced Debug Mode (default off)
      -?             Display this help

The default usage will be:

VMSBackup archive.bck

Which will extract the latest (only) revision of each file to the current folder using automatic extraction rules, where it will either try to convert the file into ASCII, or extract the file as pure binary.


**Extracting a Single File**

Currently the file mask *considers* the directory as part of the filename, even if by default it's stripped out.  So if you need a specific file, you may need to use part of the folder name.  i.e.

VMSBackup archive.bck -M:*]filename.txt;0

Note: The version after the semi-colon follows OpenVMS rules, meaning:

    ;0  - Latest
    ;-1 - Previous version
    ;1  - Version 1
    
And so on...  If there's enough demand I can probably tweak this.  However in my use case, I effectively always just extracted everything, making this mask more an afterthought I'm afraid.


**Folder Handling**

Because of the relative annoyance of a truly portable OS directory handling, this version doesn't support path preservation, however, this can be baked into the filename itself and post-processed if needed.  To include the full path in the filename, use the "-F" flag.  i.e.

VMSBackup archive.bck -F

This will then cause files extracted to look like:

[mypath.in.vax.format]myfile.txt

To try and help mitigate this a bit, I've created a relatively simplistic Python script that will take an output which used the "-F" flag, and use that to put the file into an appropriate folder.  In the example above, it would move the file to:

~/mypath/in/vax/format/myfile.txt


**Version Handling**

Most file systems these days don't support file versioning natively within the file system itself.  Meaning if you want to extract a specific version, you *can* use the method outlined in *Extracting a Single File*, but if you need multiple versions extracted in one pass, the "-V" flag will come into play, which will append the file version onto the file name.  And chances are if you're doing so, you'll want to re-adjust the mask as follows:

VMSBackup archive.bck -M:*.*;* -V

The will cause extracted files to look like this:

myfile.txt;2
myfile.txt;1

But once extracted, remember you're now at the mercy of the host OS.  And the version number will cause the OS to misinterpret the file extention if you're hoping to double click etc.  But if you're dealing with a Vax archive, I'm sure you knew that already!


**Extraction Modes**

So by default, smart extract is used.  This will essentailly analyse a file to determine if it thinks it's plain text, and if so, try and decode it in a way that's amiable to your host OS.  So regardless as to whether your file was a Stream, a Record (Fixed/Variable Length) or any other esoteric format, the tool will try and determine how best to re-intepret the file such that you can open it in your text editor of choice, otherwise it will just dump the file contents as-is.

You can force a file to use ASCII or Binary if needed, where ASCII will force the ASCII conversion to take place, and binary will bypass this.  But there is a third option, Raw.  In Raw mode, the File System record data will be interleaved.  It's not a feature I've ever used, but you could use it to try and convert a native VAX record structure into something that can be handled with a more modern OS, but that's a task I leave to you.
