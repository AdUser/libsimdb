SIMDB version 2
===============

Database header format - fixed length, 48 bytes

    Bits  | Contents
    ------+------------------------------------------
     0-15 : "IMDB vXX, CAPS: "
    16-23 : capabilities, terminated with ';'
    24-48 : padding with null's

Database record format - also fixed length, 48 bytes:

     # | off | len | description
    ---+-----+-----+-------------------------------------------------------
     1 |   0 |   1 | record is used
     2 |   1 |   1 | overall level of color: R--
     3 |   2 |   1 | overall level of color: -G-
     4 |   3 |   1 | overall level of color: --B
     5 |   4 |   2 | image width
     6 |   6 |   2 | image height
     - |   8 |   8 | reserved for future use
     7 |  16 |  32 | bitmap, each 2 bytes is row of monochrome image 16x16

    field |  12345 6 +       7
    map   |  XRGBWWHH________MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM
    sect  |  [     0-15     ][    16-31     ][    32-48     ]
