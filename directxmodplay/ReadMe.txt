DirectMODPlay README
====================
Important info goes here.



Modified version of rateconv
----------------------------

DirectXMODPlay uses code from "rateconv", a utility originally written by Dr.-Ing. Markus Mummert, as can be seen in the header of rateconv.c
A description can be found here:
http://www.tnt.uni-hannover.de/soft/audio/tools/ff_convert/rateconv/
and Mummert's own homepage seems to be here:
http://www.mmk.ei.tum.de/~rue/mum/

It can easily be found using Google, and downloaded from various places. The version used by DirectXMODPlay is 1.5, but there are newer versions.

I have made some smaller modifications of rateconv, so that it better suite my needs, but the algorithm remains the same.
The modifications are as follows:
- File operations converted to memory operations
- Rewritten to a library, with one main externally visible routine, rather than a "main" function.
- Removed "main" function and other command-line routines.
- Moved includes to VC precompiled header filed stdafx.h 

All the original files from the rateconv package are included, and only the file rateconv.c is modified.
