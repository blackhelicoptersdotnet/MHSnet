#	Copyright 2012 Piers Lauder

#	This file is part of MHSnet.

#	MHSnet is free software: you can redistribute it and/or modify
#	it under the terms of the GNU General Public License as published by
#	the Free Software Foundation, either version 3 of the License, or
#	(at your option) any later version.

#	MHSnet is distributed in the hope that it will be useful,
#	but WITHOUT ANY WARRANTY; without even the implied warranty of
#	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#	GNU General Public License for more details.

#	You should have received a copy of the GNU General Public License
#	along with MHSnet.  If not, see <http://www.gnu.org/licenses/>.


#include	"global.h"



/*
**	Return string version of CRC16.
*/

char *
StrCRC16(s)
	char *		s;
{
	register int	i;
	Ulong		c;
	static char	r[8];

	c = acrc((Crc_t)0, s, strlen(s));

	r[0] = ' ';
	r[1] = '{';
	i = EncodeNum(&r[2], c, -1) + 2;
	r[i++] = '}';
	r[i] = '\0';

	return r;
}
