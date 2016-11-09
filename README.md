About mca2nbt
=============
A simple utility to convert a Minecraft .mca region file (in anvil format) to a directory with the same basename containing an uncompressed NBT file for each of its chunks.

From this point, these NBT files can be processed with other external utilities, such as nbt2yaml (with the -n option).

I didn't need to build back an MCA file from these NBT files but this would be easy to do from here (especially as i saved the MCA header for this purpose)...


Installation
============
Typing "make install fullclean" should do it on a FreeBSD system.

Unlucky users of other operating systems might have to manually download a copy of zpipe from the following address: http://www.zlib.net/zpipe.c 


Usage
=====
$ cd world/region
$ mca2nbt *.mca


Versions and changelog
======================
1.00	2014-05-05	Initial release


License
=======
This open source software is distributed under a BSD license (see the "License" file for details).


Credits
=======
This utility was made with information from the Minecraft wiki web site.
Cf. http://minecraft.gamepedia.com/Region_file_format

(although the following information was misleading and according to my tests just wrong:
    "If you prefer to read Zlib-compressed chunk data with Deflate (RFC1951), just skip the first two bytes and leave off the last 4 bytes before decompressing.")

It includes code from the zpipe utility from Mark Adler
Cf. http://en.wikipedia.org/wiki/Mark_Adler
Cf. http://www.zlib.net/


Author
======
Hubert Tournier

5th May 2014
