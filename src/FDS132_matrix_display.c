 
/*
FDS132_matrix_display.c

Raspberry Pi version B via GPIO driving a FDS132  LED matrix display.
On the display print the CPLD has been removed.
 
Copyright Jan Panteltje 2013-always

Released under GPL.

 email: panteltje@yahoo.com

Start GPL license:
    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.
    
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    
    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

*/


				/************************** SET TABS TO 4 TO READ ANY OF THIS  *********************************/


/*
Compile this source with:
 gcc -O2 -Wall -o FDS132_matrix_display FDS132_matrix_display.c ; strip FDS132_matrix_display

Install (as root, perhaps use sudo)
 cp FDS132_matrix_display /usr/local/bin/
 
*/


#define PROGRAM_VERSION 	"0.4.3"


/*
Changes:
0.1:
First release

0.2:
Added exit on EOF command line option. default off.
Added -l keep display flag.
Changed delay so it uses feof() call, clean compile.

0.3:
Added -d flag, -e flag. -t flag, -e flag.
Fixed array length.

0.4:
Moved hardware specific I/O to separate functions.
Changed delay positions and names.
Added examples in help.
Cleane up some code.
Added some text here and there.
Totally dropped SPI and now send bit by bit.
Using 6 pixels wide characters, exactly 15 per line, 45 total, 1 pixel horizontal character spacing.
Added vertical scroll, CR LF ignored in that case.
Added linefeed handling in vertical scroll.
Protected against out of array boundary for non ASCI characters > 127.

0.4.1:
Changed loop_conter to reset if match, fixed code error for detection of > 127 ASCII (array boundary).
Moved some code around.
Added compile and install instructions.
Added GPL license, email.

0.4.2:
Changes character count compare in vertical scroll to 16 so full line is allowed.

0.4.3:
Fixed some array numbers.
Added up and down scroll modes (-u flag).
Added form feed cls processing.
Added -w flag, programmable delay after 3 lines scroll, can be used as sort of a form feed.
Added snow and fireworks effects (-x flag).
*/



#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <unistd.h>
#include <errno.h>
//#include <sys/types.h>
#include <stdint.h>
#include <string.h>
#include <getopt.h>
#include <ctype.h>
#include <time.h>
//#include <math.h>


#define IO_DELAY	5	/* delay loop */


/* character font for Panteltje (c) FDS132 LED matrix display */

/* there are differences with ASCII */

#define MATRIX_CHAR_WIDTH	8
#define MATRIX_CHAR_HEIGHT	7

static unsigned char matrixfont[128 * MATRIX_CHAR_HEIGHT]=
{
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 0 ^@ */
0b00000000,
0b00000000,
0b01001000,
0b00000000,
0b10000100,
0b01001000,
0b00110000,
/* 1 ^A */
0b00010000,
0b00111000,
0b00010000,
0b01111100,
0b00010000,
0b01111110,
0b00010000,
/* 2 ^B */
0b00010000,
0b01010100,
0b00111000,
0b01010100,
0b10111010,
0b01010100,
0b00111000,
/* 3 ^C */
0b00010000,
0b00010000,
0b00010000,
0b00010000,
0b00010000,
0b00010000,
0b00010000,
/* 4 ^D */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 5 ^E */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 6 ^F */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 7 ^G */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 8 ^H */ 
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 9 ^I */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 10 ^J */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 11 ^K */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 12 ^L */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 13 ^M */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 14 ^N */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 05 ^O */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 06 ^P */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 07 ^Q */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 08 ^R */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 19 ^S */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 20 ^T */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 21 ^U */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 22 ^V */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 23 ^V */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 24 ^W */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 25 ^X */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 26 ^Y */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 27 ^[ ESCAPE */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 28 ^\ FILE SEPARATOR */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 29 ^] GROUP SEPARATOR */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 30 ^^ RECORD SEPARATOR */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 31 ^_ UNIT SEPARATOR */
/* start of characters */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 32 space */
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b00000000,
0b00000000,
0b01000000,
/* 33 ! */
0b01010000,
0b01010000,
0b01010000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 34 ~ */
0b00101000,
0b00101000,
0b01111100,
0b00101000,
0b01111100,
0b00101000,
0b00101000,
/* 35 # */
0b00010000,
0b00111100,
0b01010000,
0b00111000,
0b00010100,
0b01111000,
0b00010000,
/* 36 $ */
0b01100000,
0b01100100,
0b00001000,
0b00010000,
0b00100000,
0b01001100,
0b00001100,
/* 37 % */
0b00110000,
0b01001000,
0b01010000,
0b00100000,
0b01010100,
0b01001000,
0b00110100,
/* 38 & */
0b00010000,
0b00100000,
0b01000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 39 ' */
0b00010000,
0b00100000,
0b01000000,
0b01000000,
0b01000000,
0b00100000,
0b00010000,
/* 40 ( */
0b01000000,
0b00100000,
0b00010000,
0b00010000,
0b00010000,
0b00100000,
0b01000000,
/* 41 ) */
0b00000000,
0b00010000,
0b01010100,
0b00111000,
0b01010100,
0b00010000,
0b00000000,
/* 42 * */
0b00000000,
0b00010000,
0b00010000,
0b01111100,
0b00010000,
0b00010000,
0b00000000,
/* 43 + */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b01100000,
0b00100000,
0b01000000,
/* 44 , */
0b00000000,
0b00000000,
0b00000000,
0b01111100,
0b00000000,
0b00000000,
0b00000000,
/* 45 - */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b01100000,
0b01100000,
/* 46 . */
0b00000000,
0b00000100,
0b00001000,
0b00010000,
0b00100000,
0b01000000,
0b00000000,
/* 47 / */
0b00111000,
0b01000100,
0b01001100,
0b01010100,
0b01100100,
0b01000100,
0b00111000,
/* 48 0 */
0b01100000,
0b00100000,
0b00100000,
0b00100000,
0b00100000,
0b00100000,
0b01110000,
/* 49 1 */
0b00111000,
0b01000100,
0b00000100,
0b00011000,
0b00100000,
0b01000000,
0b01111100,
/* 50 2 */
0b00111000,
0b01000100,
0b00000100,
0b00011000,
0b00000100,
0b01000100,
0b00111000,
/* 51 3 */
0b00011000,
0b00101000,
0b01001000,
0b01001000,
0b01111100,
0b00001000,
0b00001000,
/* 52 4 */
0b01111100,
0b01000000,
0b01000000,
0b01111000,
0b00000100,
0b00000100,
0b01111000,
/* 53 5 */
0b00111000,
0b01000000,
0b01000000,
0b01111000,
0b01000100,
0b01000100,
0b00111000,
/* 54 6 */
0b01111100,
0b01000100,
0b00001000,
0b00010000,
0b00010000,
0b00010000,
0b00010000,
/* 55 7 */
0b00111000,
0b01000100,
0b01000100,
0b00111000,
0b01000100,
0b01000100,
0b00111000,
/* 56 8 */
0b00111000,
0b01000100,
0b01000100,
0b00111100,
0b00000100,
0b00000100,
0b00111000,
/* 57 9 */
0b00000000,
0b01100000,
0b01100000,
0b00000000,
0b01100000,
0b01100000,
0b00000000,
/* 58 : */
0b00000000,
0b01100000,
0b01100000,
0b00000000,
0b01100000,
0b00100000,
0b01000000,
/* 59 ; */
0b00001000,
0b00010000,
0b00100000,
0b01000000,
0b00100000,
0b00010000,
0b00001000,
/* 60 < */
0b00000000,
0b00000000,
0b01111100,
0b00000000,
0b01111100,
0b00000000,
0b00000000,
/* 61 = */
0b01000000,
0b00100000,
0b00010000,
0b00001000,
0b00010000,
0b00100000,
0b01000000,
/* 62 > */
0b00111000,
0b01000100,
0b00000100,
0b00001000,
0b00010000,
0b00000000,
0b00010000,
/* 63 ? */
0b00111000,
0b01000100,
0b00000100,
0b00110100,
0b01010100,
0b01010100,
0b00111000,
/* 64 @ */
0b00111000,
0b01000100,
0b01000100,
0b01000100,
0b01111100,
0b01000100,
0b01000100,
/* 65 A */
0b01111000,
0b01000100,
0b01000100,
0b01111000,
0b01000100,
0b01000100,
0b01111000,
/* 66 B */
0b00111000,
0b01000100,
0b01000000,
0b01000000,
0b01000000,
0b01000100,
0b00111000,
/* 67 C */
0b01110000,
0b01001000,
0b01000100,
0b01000100,
0b01000100,
0b01001000,
0b01110000,
/* 68 D */
0b01111100,
0b01000000,
0b01000000,
0b01111000,
0b01000000,
0b01000000,
0b01111100,
/* 69 E */
0b01111100,
0b01000000,
0b01000000,
0b01111000,
0b01000000,
0b01000000,
0b01000000,
/* 70 F */
0b00111000,
0b01000100,
0b01000000,
0b01001100,
0b01000100,
0b01000100,
0b00111100,
/* 71 G */
0b01000100,
0b01000100,
0b01000100,
0b01111100,
0b01000100,
0b01000100,
0b01000100,
/* 72 H */
0b00111000,
0b00010000,
0b00010000,
0b00010000,
0b00010000,
0b00010000,
0b00111000,
/* 73 I */
0b00011100,
0b00001000,
0b00001000,
0b00001000,
0b01001000,
0b01001000,
0b00110000,
/* 74 J */
0b01000100,
0b01001000,
0b01010000,
0b01100000,
0b01010000,
0b01001000,
0b01000100,
/* 75 K */
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b01111100,
/* 76 L */
0b01000100,
0b01101100,
0b01010100,
0b01010100,
0b01000100,
0b01000100,
0b01000100,
/* 77 M */
0b01000100,
0b01000100,
0b01100100,
0b01010100,
0b01001100,
0b01000100,
0b01000100,
/* 78 N */
0b00111000,
0b01000100,
0b01000100,
0b01000100,
0b01000100,
0b01000100,
0b00111000,
/* 79 O */
0b01111000,
0b01000100,
0b01000100,
0b01111000,
0b01000000,
0b01000000,
0b01000000,
/* 80 P */
0b00111000,
0b01000100,
0b01000100,
0b01000100,
0b01010100,
0b01001000,
0b00110100,
/* 81 Q */
0b01111000,
0b01000100,
0b01000100,
0b01111000,
0b01010000,
0b01001000,
0b01000100,
/* 82 R */
0b00111100,
0b01000100,
0b01000000,
0b00111000,
0b00000100,
0b01000100,
0b01111000,
/* 83 S */
0b01111100,
0b00010000,
0b00010000,
0b00010000,
0b00010000,
0b00010000,
0b00010000,
/* 84 T */
0b01000100,
0b01000100,
0b01000100,
0b01000100,
0b01000100,
0b01000100,
0b00111000,
/* 85 U */
0b01000100,
0b01000100,
0b01000100,
0b01000100,
0b01000100,
0b00101000,
0b00010000,
/* 86 V */
0b01000100,
0b01000100,
0b01000100,
0b01010100,
0b01010100,
0b01010100,
0b00101000,
/* 87 W */
0b01000100,
0b01000100,
0b00101000,
0b00010000,
0b00101000,
0b01000100,
0b01000100,
/* 88 X */
0b01000100,
0b01000100,
0b01000100,
0b00101000,
0b00010000,
0b00010000,
0b00010000,
/* 89 Y */
0b01111100,
0b00000100,
0b00001000,
0b00010000,
0b00100000,
0b01000000,
0b01111100,
/* 90 Z */
0b00101000,
0b00000000,
0b00111000,
0b01000100,
0b01111100,
0b01000100,
0b01000100,
/* 91 A umlaut, was [ in ASCII */
0b00101000,
0b00000000,
0b00111000,
0b01000100,
0b01000100,
0b01000100,
0b00111000,
/* 92 O umlaut, was \ in ASCII */
0b00101000,
0b00000000,
0b01000100,
0b01000100,
0b01000100,
0b01000100,
0b00111000,
/* 93 U umlaut, was ] in ASCCI */
0b00000000,
0b00000000,
0b00010000,
0b00101000,
0b01000100,
0b01000100,
0b00000000,
/* 94 ^ */
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
0b01111111,
/* 95 _ */
0b00011000,
0b00100100,
0b00011000,
0b00000000,
0b00000000,
0b00000000,
0b00000000,
/* 96 degrees sym, was 'in ASCII */
0b00000000,
0b00000000,
0b00111000,
0b01001000,
0b01001000,
0b01001000,
0b00110100,
/* 97 a */
0b01000000,
0b01000000,
0b01111000,
0b01000100,
0b01000100,
0b01000100,
0b00111000,
/* 98 b */
0b00000000,
0b00000000,
0b00111100,
0b01000000,
0b01000000,
0b01000000,
0b00111100,
/* 99 c */
0b00000100,
0b00000100,
0b00111100,
0b01000100,
0b01000100,
0b01000100,
0b00111100,
/* 100 d */
0b00000000,
0b00000000,
0b00111000,
0b01000100,
0b01111100,
0b01000000,
0b00111100,
/* 101 e */
0b00001100,
0b00010000,
0b00010000,
0b01111100,
0b00010000,
0b00010000,
0b00010000,
/* 102 f */
0b00000000,
0b00111100,
0b01000100,
0b01000100,
0b00111100,
0b00000100,
0b00111000,
/* 103 g */
0b01000000,
0b01000000,
0b01011000,
0b01100100,
0b01000100,
0b01000100,
0b01000100,
/* 104 h */
0b00100000,
0b00000000,
0b01100000,
0b00100000,
0b00100000,
0b00100000,
0b01110000,
/* 105 i */
0b00010000,
0b00000000,
0b00110000,
0b00010000,
0b00010000,
0b00010000,
0b01100000,
/* 106 j */
0b01000000,
0b01000000,
0b01001000,
0b01010000,
0b01100000,
0b01010000,
0b01001000,
/* 107 k */
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b01000000,
0b01000000,
/* 108 l */
0b00000000,
0b00000000,
0b01101000,
0b01010100,
0b01010100,
0b01010100,
0b01010100,
/* 109 m */
0b00000000,
0b00000000,
0b01011000,
0b01100100,
0b01000100,
0b01000100,
0b01000100,
/* 110 n */
0b00000000,
0b00000000,
0b00111000,
0b01000100,
0b01000100,
0b01000100,
0b00111000,
/* 111 o */
0b00000000,
0b01111000,
0b01000100,
0b01000100,
0b01111000,
0b01000000,
0b01000000,
/* 112 p */
0b00000000,
0b00111100,
0b01000100,
0b01000100,
0b00111100,
0b00000100,
0b00000100,
/* 113 q */
0b00000000,
0b00000000,
0b01011000,
0b01100100,
0b01000000,
0b01000000,
0b01000000,
/* 114 r */
0b00000000,
0b00000000,
0b00111100,
0b01000000,
0b00111000,
0b00000100,
0b01111000,
/* 115 s */
0b00010000,
0b00010000,
0b01111100,
0b00010000,
0b00010000,
0b00010000,
0b00001100,
/* 116 t */
0b00000000,
0b00000000,
0b01000100,
0b01000100,
0b01000100,
0b01001100,
0b00110100,
/* 117 u */
0b00000000,
0b00000000,
0b01000100,
0b01000100,
0b01000100,
0b00101000,
0b00010000,
/* 118 v */
0b00000000,
0b00000000,
0b01000100,
0b01000100,
0b01010100,
0b01010100,
0b00101000,
/* 119 w */
0b00000000,
0b00000000,
0b01000100,
0b00101000,
0b00010000,
0b00101000,
0b01000100,
/* 120 x */
0b00000000,
0b01000100,
0b01000100,
0b01000100,
0b00111100,
0b00000100,
0b00111000,
/* 121 y */
0b00000000,
0b00000000,
0b01111100,
0b00001000,
0b00010000,
0b00100000,
0b01111100,
/* 122 z */
0b00101000,
0b00000000,
0b00111000,
0b01001000,
0b01001000,
0b01001000,
0b00110100,
/* 123 a umlaut, was { in ASCII */
0b00101000,
0b00000000,
0b00111000,
0b01000100,
0b01000100,
0b01000100,
0b00111000,
/* 124 o umlaut, was | in ASCII */
0b00101000,
0b00000000,
0b01000100,
0b01000100,
0b01000100,
0b01001100,
0b00110100,
/* 125 u umlaut, was } in ASCII */
0b00000000,
0b00110000,
0b01001000,
0b01011000,
0b01000100,
0b01011000,
0b01000000,
/* 126 beta, was ~ in ASCII */
0b00000000,
0b01111110,
0b01111110,
0b01111110,
0b01111110,
0b01111110,
0b00000000,
/* 127 blokje upper sepa,  was DEL*/

}; /* end matrixfont */




/*
I/O mapping:

outputs:
MATRIX_SHIFT_REGISTER_CLOCK				GPIO_11		pin 23
MATRIX_SHIFT_REGISTER_DATA				GPIO_9		pin 21
MATRIX_STROBE							GPIO_8		pin 24
MATRIX_ROW_SELECT_A						GPIO_22		pin 15
MATRIX_ROW_SELECT_B						GPIO_23		pin 16
MATRIX_ROW_SELECT_C						GPIO_24		pin 18

free:
		GPIO_25		pin 22
		GPIO_11		pin 23
		GPIO_2		pin  3
		GPIO_3		pin  5
		GPIO_4		pin	 7	
		GPIO_2		pin  3
		GPIO_17		pin 11
		GPIO_14		pin  8
*/


#define BUFFER_SIZE		512*1024 //8192 // default Linux fread() buffer size?
//#define BUFFER_SIZE		8000000 //

#define MATRIX_SHIFT_REGISTER_CLOCK		1<<11
#define MATRIX_SHIFT_REGISTER_DATA		1<<9
#define MATRIX_STROBE					1<<8
#define MATRIX_ROW_SELECT_A				1<<22
#define MATRIX_ROW_SELECT_B				1<<23
#define MATRIX_ROW_SELECT_C				1<<24


// Access from ARM Running Linux
//Variables used by auto Pi model detection
static volatile uint32_t piModel = 1;
static volatile uint32_t piPeriphBase = 0x20000000;
static volatile uint32_t piBusAddr = 0x40000000;
static volatile uint32_t piGpioBase;

int verbose;

#define PAGE_SIZE (4*1024)
#define BLOCK_SIZE (4*1024)

int  mem_fd;
void *gpio_map;

// I/O access
volatile unsigned *gpio;


// GPIO setup macros. Always use INP_GPIO(x) before using OUT_GPIO(x) or SET_GPIO_ALT(x,y)
#define INP_GPIO(g) *(gpio+((g)/10)) &= ~(7<<(((g)%10)*3))
#define OUT_GPIO(g) *(gpio+((g)/10)) |=  (1<<(((g)%10)*3))
#define SET_GPIO_ALT(g,a) *(gpio+(((g)/10))) |= (((a)<=3?(a)+4:(a)==4?3:2)<<(((g)%10)*3))

#define GPIO_SET *(gpio+7)  // sets   bits which are 1 ignores bits which are 0
#define GPIO_CLR *(gpio+10) // clears bits which are 1 ignores bits which are 0

#define GPIO_CLR0 *(gpio+10) // Set GPIO low bits 0-31
#define GPIO_CLR1 *(gpio+11) // Set GPIO low bits 32-53

#define GPIO_PULL   *(gpio+37) // Pull up/pull down
#define GPIO_PULLCLK0 *(gpio+38) // Pull up/pull down clock

#define GPIO_IN0 *(gpio+13)  // Reads GPIO input bits 0-31

//Code to automatically sense the Pi version
//Thanks to http://abyz.co.uk/rpi/pigpio/examples.html#Misc_code
unsigned gpioHardwareRevision(void)
{
   static unsigned rev = 0;

   FILE * filp;
   char buf[512];
   char term;
   int chars=4; /* number of chars in revision string */

   if (rev) return rev;

   piModel = 0;

   filp = fopen ("/proc/cpuinfo", "r");

   if (filp != NULL)
   {
      while (fgets(buf, sizeof(buf), filp) != NULL)
      {
         if (piModel == 0)
         {
            if (!strncasecmp("model name", buf, 10))
            {
               if (strstr (buf, "ARMv6") != NULL)
               {
                  piModel = 1;
                  chars = 4;
                  piPeriphBase = 0x20000000;
                  piBusAddr = 0x40000000;
               }
               else if (strstr (buf, "ARMv7") != NULL)
               {
                  piModel = 2;
                  chars = 6;
                  piPeriphBase = 0x3F000000;
                  piBusAddr = 0xC0000000;
               }
               else if (strstr (buf, "ARMv8") != NULL)
               {
                  piModel = 2;
                  chars = 6;
                  piPeriphBase = 0x3F000000;
                  piBusAddr = 0xC0000000;
               }
            }
         }

        piGpioBase = (piPeriphBase + 0x200000);

         if (!strncasecmp("revision", buf, 8))
         {
            if (sscanf(buf+strlen(buf)-(chars+1),
               "%x%c", &rev, &term) == 2)
            {
               if (term != '\n') rev = 0;
            }
         }
      }

      fclose(filp);
   }
   return rev;
}


char *strsave(char *s) /* save char array s somewhere */
{
char *p;

p = (char *) malloc(strlen(s) +  1);
if(p) strcpy(p, s);
return p;
} /* end function strsave */



void io_delay(int delays)
{
int i;

for(i = 0; i < delays; i++)
	{
	/* some delay */
	feof(stdin);
	}
	
} /* end function io_delay */



void so_h()
{
*(gpio + 7) = MATRIX_SHIFT_REGISTER_DATA;
io_delay(IO_DELAY);

} /* end functiom so_h */



void so_l()
{
*(gpio + 10) = MATRIX_SHIFT_REGISTER_DATA;
io_delay(IO_DELAY);

} /* end function so_l */



void sck_h()
{
*(gpio + 7) = MATRIX_SHIFT_REGISTER_CLOCK;
io_delay(IO_DELAY);

} /* end function sck_h */



void sck_l()
{
*(gpio + 10) = MATRIX_SHIFT_REGISTER_CLOCK;
io_delay(IO_DELAY);

} /* end function sck_l */



void strobe_h()
{
*(gpio + 7) = MATRIX_STROBE;
io_delay(IO_DELAY);

} /* end function strobe_h */



void strobe_l()
{
*(gpio + 10) = MATRIX_STROBE;
io_delay(IO_DELAY);

} /* end function  strobe_l */



void row_select_a_high()
{
*(gpio + 7) = MATRIX_ROW_SELECT_A;
io_delay(IO_DELAY);

} /* end function row_select_a_high */



void row_select_a_low()
{
*(gpio + 10) = MATRIX_ROW_SELECT_A;
io_delay(IO_DELAY);

} /* end function row_select_a_low */

	

void row_select_b_high()
{
*(gpio + 7) = MATRIX_ROW_SELECT_B;
io_delay(IO_DELAY);

} /* end function row_select_b_high */



void row_select_b_low()
{
*(gpio + 10) =  MATRIX_ROW_SELECT_B; 
io_delay(IO_DELAY);

} /* end function row_select_b_low */



void row_select_c_high()
{
*(gpio + 7) = MATRIX_ROW_SELECT_C;
io_delay(IO_DELAY);

} /* end function row_select_c_high */



void row_select_c_low()
{
*(gpio + 10) = MATRIX_ROW_SELECT_C;
io_delay(IO_DELAY);

} /* end function row_select_c_low */



void setup_io()
{
/* open /dev/mem */
if ((mem_fd = open("/dev/mem", O_RDWR|O_SYNC) ) < 0)
	{
	printf("can't open /dev/mem \n");
	exit(-1);
   }

/* mmap GPIO */
gpio_map = mmap(
     NULL,             //Any adddress in our space will do
     BLOCK_SIZE,       //Map length
     PROT_READ|PROT_WRITE,// Enable reading & writting to mapped memory
     MAP_SHARED,       //Shared with other processes
     mem_fd,           //File to map
     piGpioBase        //Offset to GPIO peripheral
   );

close(mem_fd); // No need to keep mem_fd open after mmap

if(gpio_map == MAP_FAILED)
	{
	printf("mmap error %d\n", (int)gpio_map);//errno also set!
	exit(-1);
	}

// Always use volatile pointer!
gpio = (volatile unsigned *)gpio_map;

} /* end function setup_io */



//-c            reads temperature in Celcius from /tmp/temperature, displayed in -d.


void print_usage()
{
fprintf(stderr,\
"\nPanteltje FDS132_matrix_diplay-%s\n\
Usage:\nmatrix_diplay [-e] [-h] [-l] [-v] [t]\n\
\n\
-d            display date and time.\n\
-e            exit on EOF, display will go black, else last text will be displayed.\n\
-h            help (this help).\n\
-s int        scroll delay, default 40.\n\
-t text       text to display.\n\
-u int        scroll mode:\n\
                0 horizontal left.\n\
                1 vertically up.\n\
                2 vertical down.\n\
                default 0.\n\
-v            verbose, prints functions and arguments.\n\
-w int        delay time to wait after displaying 3 lines in vertical scroll, default 0.\n\
-x int        special effects:\n\
                0 off.\n\
                1 snow.\n\
                2 fireworks.\n\
                default 0.\n\\n\
\n",\
PROGRAM_VERSION);

fprintf(stderr,\
"Examples, \n\
 Note that there 3 lines of 15 characters available.\n\n\
Horizontal Scrolling text:\n\
FDS132_matrix_display < example.txt\n\n\
Vertical scrolling text, keep your lines shorter than 15 characters, LF works, or 15 charaters without LF:\n\
FDS132_matrix_display -u -s 400 < example.txt\n\n\
Display date and time:\n\
FDS132_matrix_display -d\n\n\
Put a specific text on the display, use spaces to format:\n\
FDS132_matrix_display -t \"Hello world\"\n\
\n"\
);

} /* end function print_usage */


char text[60];

uint8_t data[7][50];


#define SCROLL_LEFT		0
#define SCROLL_UP		1
#define	SCROLL_DOWN		2


#define EFFECT_OFF 							0
#define EFFECT_SNOW							1
#define EFFECT_FIREWORKS					2


int main(int argc, char **argv)
{
int a, c, i, j, r;
int loop_counter;
int exit_on_eof_flag;
int text_flag;
int date_flag;
int scroll_delay;
int scroll_mode;
int three_line_delay;
int line_cnt;
int effect_mode;
int input_line_cnt;
float fa;
char fireworks_landscape[] = 	" * **  * *"; // landscape on bottom line, 15 characters wide, the control B is a christmass tree, use hexedit for example to make these strings
char snow_landscape[] = 		" * **  * *";
//int get_temperature_flag;
//int temperature;
//FILE *fptr;


setbuf(stdout, NULL);
setbuf(stdin, NULL);

//strcpy(text, "   PANTELTJE                               ");
strcpy(text, "                                           ");

gpioHardwareRevision(); /* sets piModel, needed for peripherals address */

// Set up gpio pointer for direct register access
setup_io();


/*
GPIO	header Pin
2		3
3		5
4		7		
7		26
8		24
9		21
10		19
11		23
14		8
15		10
17		11
18		12
27		13
22		15
23		16
24		18
25		22
*/


/* defaults */

verbose = 0;
exit_on_eof_flag = 0;
text_flag = 0;
date_flag = 0;
scroll_delay = 40;
three_line_delay = 0;
line_cnt = 0;
//get_temperature_flag = 0;
scroll_mode = SCROLL_LEFT;
effect_mode = EFFECT_OFF;
input_line_cnt = 0;

/* end defaults */


/* proces any command line arguments */
while(1)
	{
	a = getopt(argc, argv, "cdehs:u:vw:t:x:");
	if(a == -1) break;

	switch(a)
		{
//		case 'c': // temperature
//			get_temperature_flag = 1;
//			break;
		case 'd': // dsiplay date and time
			date_flag = 1;
			break;
		case 'e': // exit on EOF
			exit_on_eof_flag = 1;
			break;
		case 'h': // help
			print_usage();
			exit(1);
			break;
		case 's': // scroll delay
			scroll_delay = atoi(optarg);
			break;
		case 't': // text to display
			text_flag = 1;
			strncpy(text, optarg, 45);
			break;
		case 'u': // scroll mode
			a =  atoi(optarg);
			if( (a < 0) || (a > SCROLL_DOWN) )
				{
				print_usage();
				
				exit(1);
				}
			scroll_mode = a;
			break;
		case 'v': // verbose
			verbose = 1;
			break;
		case 'w':// time to wait after 3 lines displayed
			three_line_delay = atoi(optarg);
			break;
 		case 'x': // special effect mode 
			effect_mode = atoi(optarg);
			break;
        case -1:
        	break;
		case '?':
			if (isprint(optopt) )
 				{
 				fprintf(stderr, "send_music: unknown option `-%c'.\n", optopt);
 				}
			else
				{
				fprintf(stderr, "send_music: unknown option character `\\x%x'.\n", optopt);
				}
			print_usage();

			exit(1);
			break;			
		default:
			print_usage();
			exit(1);
			break;
		}/* end switch a */
	}/* end while getopt() */


/* set I/O directions */
/*
Note:
GPIO 2 and GPIO 3 have 1k8 pull up resistors!!!

must use INP_GPIO before we can use OUT_GPIO
*/

/*
8-11, 21-24 are output
no inputs
*/

// Set GPIO pins 8-11 to output
for(i = 8; i <= 11; i++)
	{
    INP_GPIO(i);
    OUT_GPIO(i);
	}

// Set GPIO pins 22-24 to output
for(i = 21; i <= 24; i++)
	{
    INP_GPIO(i);
    OUT_GPIO(i);
	}


// clock and data line low
sck_l();
so_l();


/*
MATRIX_SHIFT_REGISTER_CLOCK
MATRIX_SHIFT_REGISTER_DATA
MATRIX_STROBE
MATRIX_ROW_SELECT_A
MATRIX_ROW_SELECT_B
MATRIX_ROW_SELECT_C
*/


//#define IO_TEST
#ifdef IO_TEST
//a = MATRIX_SHIFT_REGISTER_CLOCK;
//a = MATRIX_SHIFT_REGISTER_DATA;
//a = MATRIX_STROBE;
//a = MATRIX_ROW_SELECT_A;
//a = MATRIX_ROW_SELECT_B;
a = MATRIX_ROW_SELECT_C;

while(1)
	{
	*(gpio + 7) = a;
	usleep(100);
	*(gpio + 10) = a;	
	usleep(100);
	}
#endif // IO_TEST


time_t now;
struct tm *local_time;
char temp[1024];

loop_counter = 0;
while(1)
	{
	// add text

	if(date_flag)
		{
		now = time(0);
		local_time = localtime(&now);
		strftime(temp, 511, "  %d %m %Y      %H:%M:%S       %A    ", local_time);
//		strftime(temp, 511, " %d %m %Y  %H:%M:%S", local_time);
		
		strcpy(text, temp);

/*
		if(get_temperature_flag)
			{
			fptr = fopen("/tmp/temperature", "r");
			if(fptr)
				{
				fscanf(fptr, "%d", &temperature);
				
				fclose(fptr);
				}

			sprintf(temp, "      %d%cC              ", temperature, 96);
			strcat(text, temp);
			}
*/


//fprintf(stderr, "%s\n", temp);
		}

	/*
	The FDS132 has 3 rows of 18  5x7 matrix displays.
	This makes horizontal 18 x 5 = 90 bits
	If we make 6 bit wide and 7 bit high characters, then exactly 15 fit on a 'line',
	so we get 3 rows of 15 characters.
	*/

	int row;
	int bit;
	int k;

	/* process each row in the display */
	for(row = 0; row < 7; row++) // all rows in display
		{

		// set row select lines
		if(row & 1) row_select_a_high();
		else		row_select_a_low();
	
		if(row & 2) row_select_b_high();
		else 		row_select_b_low();

		if(row & 4) row_select_c_high();
		else		row_select_c_low();


		// fix for hardware row counting
		if(row == 6) r = 0; 
		else r = row + 1;


		/* pixels from font to shift registers */
		for(i = 45; i >= 0; i--) // each charater in the input text
			{
			for(k = 2; k < 8; k++) // the bits we use from the font rows
				{
				c = text[i];

				// font array boundary
				if(c > 127) c = 0; // subsitute non ASCII with blanks

				bit = (matrixfont[ (c * MATRIX_CHAR_HEIGHT) + r ] >> k) & 1;
				
				// set shift register data input
				if(bit) so_h();
				else so_l();
				
				// toggle shift register clock
				sck_h();
				sck_l();

				} /* end for all bits in font row */

			} /* end for all characters in text string */


		/* latch shift register data to output */

		// strobe high
		strobe_h();

		// strobe low
		strobe_l();

		} /* end for all rows */

	loop_counter++;

	/* start special effects */
	if(effect_mode == EFFECT_FIREWORKS)
		{
		if(loop_counter == scroll_delay)
			{
			loop_counter = 0;

			/* create a landscape on the bottom line */
			for(j = 0; j < 15; j++)
				{
				text[j + 30] = fireworks_landscape[j];			
				}

			input_line_cnt++;
			if(input_line_cnt == 1)
				{
				/* generate sparse random '|' on line 2 */
				for(j = 0; j < 15; j++)
					{
					fa =  random();

					if(fa > (RAND_MAX / 16) )
						text[j + 15] = ' ';
					else 
						text[j + 15] = 4;
					}
				}
			else if(input_line_cnt == 2)
				{
				/* copy up line and replace '|' code 4 by '*' */
				for(j = 0; j < 15; j++)
					{
					if(text[j + 15] == 4)
						{
						text[j] = '*';
						}
					else
						{
						text[j] = ' ';
						}

					/* clear second line */
					text[j + 15] = 0;
					}		
				
				input_line_cnt = 0;
				}

			}		
		
		continue;

		} /* end if EFFECT_FIREWORKS */
	else if(effect_mode == EFFECT_SNOW)
		{
		/* create a landscape on the bottom line */
		for(j = 0; j < 15; j++)
			{
			text[j + 30] = snow_landscape[j];			
			}

		if(loop_counter == scroll_delay)		
			{
			loop_counter = 0;

			/* copy down topline */
			for(j = 0; j < 15; j++)
				{
				text[j + 15] = text[j];

				/* clear to line */
				text[j] = ' ';
				}		
				
			/* generate random snow on top line */
			for(j = 0; j < 15; j++)
				{
				fa =  random();

				if(fa > (RAND_MAX / 2) )
					text[j]  = ' ';
				else 
					text[j] = '*';
				}

			}		
		
		continue;
		} /* end if EFFECT_SNOW */

	/* end special effects */	

		
	if(date_flag) continue;
	if(text_flag) continue;


	// when time process any scrolling, v or h

	// after line 3 wait longer
	if(line_cnt == 3)
		{
		a = scroll_delay + three_line_delay;
		}
	else  a = scroll_delay;

	if(loop_counter == a)
		{
		loop_counter = 0;

		if(scroll_mode == SCROLL_UP) // vertical scroll up
			{		
			/* copy up one line, make space */
			for(i = 0; i < 30; i++)
				{
				text[i] = text[i + 15];
				}

			/* read in new bottom line */
			// clear line in case input does not fill a line (EOF)
			for(i = 30 ; i < 45; i++)		
				{
				text[i] = 0;
				}

			if(line_cnt == 3) line_cnt = 0;
			line_cnt++;

			// get characters from input to bottom line
			i = 0;
			while(1)
				{
				c = fgetc(stdin);
				if(feof(stdin) )
					{
					if(exit_on_eof_flag)
						{
						exit(0);
						}
					else
						{
						text_flag = 1;

						break;
						}
					} 

				if(c == 10) // LF, line feed
					{
					break;
					}

				if(c == 12) // FF, form feed
					{
					// start again at top
					i = 0;
					line_cnt = 0;
					
					// clear screen
					for(j = 0; j < 45; j++)
						{
						text[j] = ' ';
						}

					break;
					}

				if( (c != 10)  && (c != 13) ) // skip any LF, CR
					{
					// font array boundary, skip non ASCII
					if(c > 127) c = 0;

					text[30 + i] = c;
					i++;
					if(i == 15) break;
					}

				}		

			text[45] = 0;

			// line feed
			if(c == 10) continue;
		
			} /* end if scroll up */
		else if(scroll_mode == SCROLL_DOWN)
			{

			/* copy down one line, make space */

if(verbose) 
	fprintf(stderr, "WAS A input_line_cnt=%d text=%s\n", input_line_cnt, text);
			for(i = 0; i < 30 ; i++)
				{
				text[44 - i ] = text[29 - i];
				}

if(verbose)
	fprintf(stderr, "WAS B input_line_cnt=%d text=%s\n", input_line_cnt, text);

			/* read in new top line */

			// clear line in case input does not fill a line (EOF)
			for(i = 0 ; i < 15; i++)		
				{
				text[i] = 0;
				}

			if(line_cnt == 3) line_cnt = 0;
			line_cnt++;

			// get characters from input to bottom line
			i = 0;
			while(1)
				{
				c = fgetc(stdin);
				if(feof(stdin) )
					{
					if(exit_on_eof_flag)
						{
						exit(0);
						}
					else
						{
						text_flag = 1;

						break;
						}
					} 

				if(c == 10) // LF, line feed
					{
					break;
					}

				if(c == 12) // FF, form feed
					{
					// start again at bottom
					i = 0;
					line_cnt = 0;
					
					// clear screen
					for(j = 0; j < 45; j++)
						{
						text[j] = ' ';
						}

					break;
					}

				if( (c != 10)  && (c != 13) ) // skip any LF, CR
					{
					// font array boundary, skip non ASCII
					if(c > 127) c = 0;

					text[i] = c;
					i++;
					if(i == 15) break;
					}

				}		

			text[45] = 0;

			// line feed
			if(c == 10) continue;
			
			} /* end if scroll down */
		else if(scroll_mode == SCROLL_LEFT)
			{
			// get new character from input
			c = fgetc(stdin);
			if(feof(stdin) )
				{
				if(exit_on_eof_flag)
					{
					exit(0);
					}
				else
					{
					text_flag = 1;
				
					continue;
					}
				}
		
			// font array boundary, replace non ASCII with blanks
			if(c > 127) c = 0;

			text[45] = c;

			// copy down text array
			for(i = 0; i < 45; i++)
				{
				text[i] = text[i + 1];
				}

			text[46] = 0;
			} /* end if scroll left */

		} /* end while scan */	
	} /* end if loop_counter == scroll_delay */

	// end v, h scroll processing

exit(0);
} /* end function main */



