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
#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define		READ_DATA_SIZE		(1024*256)

//==============================================
// main ( )
//==============================================
int main (int argc, const char* argv[])
{
	static char 	ch[READ_DATA_SIZE+1];
	int 			fd_in;
	unsigned int 	len = 0;

	if (argc >= 7)
	{
		if ((fd_in = open (argv[6], O_RDONLY)) < 0) {
			return (-2);
		}	
	}
	else
	{
		fd_in = 0;
	}

	while ((len = read (fd_in, ch, READ_DATA_SIZE)) > 0) {
		write (1, ch, len);
	}

	if (fd_in)
		close (fd_in);

	return (0);
}
