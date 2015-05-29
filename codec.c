/*********************************************************************************
Fuji Xerox Printer Driver for Linux
(C) Fuji Xerox Co.,Ltd. 2006

This program is free software; you can redistribute it and/or
modify it under the terms of the GNU General Public License
as published by the Free Software Foundation; either version 2
of the License, or (at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
***********************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "codec.h"

#define	numof(x)	(int)(sizeof(x)/sizeof(*x))

#define MAXBUFFLEN_ENCODED 32
#define MAXBUFFLEN_DECODED 24

#define FALSE 0
#define TRUE 1

typedef struct {
	char	before;
	char	after;
} CNV_T;

const CNV_T convTbl[] =
{
	{0x22, 0x76},
	{0x27, 0x77},
	{0x2c, 0x78},
	{0x3d, 0x79},
	{0x5c, 0x7b}
};

static int intpow(int x, int y);

/*********************************************************************
 * function : Decode
 *
 * input    : unsigned char*  encData        : encoded data
 * output   : unsigned char*  decData        : decoded data
 *          : int*            decDataLen     : bytes of decoded data
 *********************************************************************
 */

int
Decode (unsigned char* encData,	/* encoded data */
	 unsigned char* decData,	/* decoded data */
	 int*           decDataLen)	/* length of decode data */
{
    unsigned int sum;
    int ei = 0,
        di = 0,
        i,
		j, 
        validEncDigits,
        validDecDigits;

    while (encData[ei] != 0) {		/* calculate sum until NULL */
	sum     = 0;
	validEncDigits = 0;
	for (i=0 ; i<5 ; i++) {
		for (j=0; j < numof(convTbl); j++) {
			if (encData[ei] == convTbl[j].after) {
				encData[ei] = convTbl[j].before;
				break;
			}
		}                         	 
    	if ((encData[ei] >= (unsigned char) '!') && (encData[ei] <= (unsigned char) 'u')) { 
		sum = sum*85 + (int) encData[ei] - 33; 
		ei++;
		validEncDigits++;
	    } else {
		if (encData[ei] == 0) {
		    sum = sum*85 + 84;
		} else {
		    return FALSE;
		}
	    }
	}

	validDecDigits = validEncDigits - 1; 
	for (i=0 ; i < validDecDigits ; i++) {	/* write data */
	    if (di < MAXBUFFLEN_DECODED) {
		decData[di] = (unsigned char) ((sum / intpow(256, 3-i)) % 256);
		di++;
	    } else {
		return FALSE;
	    }
	}
    }
    *decDataLen = di;

    return TRUE;
}	    

static int intpow(int x, int y)
{
    int ret = 1;
    
    if (y < 0) {
	return 0;
    }
    if (y == 0) {
	return 1;
    }
    while (y != 0) {
	ret = ret * x;
	y--;
    }
    return ret;
}
