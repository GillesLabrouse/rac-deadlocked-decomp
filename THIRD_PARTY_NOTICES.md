# Third-party notices

## GCC runtime library (`src/libgcc/`)

`libgcc2.c`, `gbl-ctors.h`, `longlong.h` (GCC trunk, 1999-09-09) and
`fp-bit.c` (GCC 2.95.3): Copyright Free Software Foundation, Inc. Licensed
under the GNU General Public License version 2 or later with the libgcc
linking exception, as stated in each file's header. They are built unchanged
(except two edits marked in place in `fp-bit.c`) to reproduce the library
that Sony's toolchain linked into the game.

The selection and build recipe come from
[rac1-decomp](https://github.com/Lynder063/rac1-decomp) (MIT).

## Math library (`src/libm/`)

Sources from newlib (snapshot 2000-02-17), derived from Sun Microsystems' fdlibm:
"Copyright (C) 1993 by Sun Microsystems, Inc. All rights reserved. Developed at
SunSoft, a Sun Microsystems, Inc. business. Permission to use, copy, modify, and
distribute this software is freely granted, provided that this notice is
preserved." Other files carry the notices of their respective authors in their
headers. They are built unchanged to reproduce the library that Sony's
toolchain linked into the game.

## Public architecture references (no vendored code)

The bounded integer instruction oracle and native indexed-pixel permutation
were independently authored and cross-checked against public PCSX2 architecture
facts: [R5900 operations](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/R5900OpcodeImpl.cpp),
[GS layouts](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/GS/GSTables.cpp) and
[GS address composition](https://github.com/PCSX2/pcsx2/blob/master/pcsx2/GS/GSLocalMemory.h).
PCSX2 source is copyright its developers and licensed GPL-3.0+; no implementation,
lookup table or generated copy is included here. Local validation references
retain their original notices under ignored private storage.
