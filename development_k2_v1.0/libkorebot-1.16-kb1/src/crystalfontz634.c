/*-------------------------------------------------------------------------------
 * Project: KoreBot Library     
 * Author: Yves Piguet, 2004
 * $Date: 2004/07/29 10:51:54 $
 * $Revision: 1.1 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/crystalfontz634.c,v 1.1 2004/07/29 10:51:54 cgaudin Exp $
 */
#include <stdlib.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <termio.h>
#include <string.h>
#include <stdarg.h>

#include "crystalfontz634.h"

int c634_open(char const *path)
{
	int fd;
	struct termios termios_p;
	
	fd = open(path, O_WRONLY);
	if (fd == -1)
		return -1;
	
	tcgetattr(fd, &termios_p);
	termios_p.c_cflag &= ~CSTOPB & ~CRTSCTS;
	termios_p.c_cflag |= CLOCAL | CREAD;	// don't own the port and accept reading
	termios_p.c_oflag &= ~OPOST;	// raw output
	tcgetattr(fd, &termios_p);
	termios_p.c_cflag = CS8 | CLOCAL | CREAD | B19200;
	termios_p.c_iflag = IGNPAR;
	termios_p.c_oflag = 0;
	termios_p.c_lflag = 0;
	termios_p.c_cc[VMIN] = 0;
	termios_p.c_cc[VTIME] = 0;
	(void)tcflush(fd, TCIFLUSH);
	tcsetattr(fd, TCSANOW, &termios_p);
	
	return fd;
}

void c634_close(int fd)
{
	close(fd);
}

void c634_home(int fd)
{
	char b = 1;
	write(fd, &b, 1);
}

void c634_hide(int fd)
{
	char b = 2;
	write(fd, &b, 1);
}

void c634_restore(int fd)
{
	char b = 3;
	write(fd, &b, 1);
}

void c634_cursor(int fd, int type)
{
	char b = type == k_c634_cursorHide ? 4 : type == k_c634_cursorUnderline ? 5
			: type == k_c634_cursorInverting ? 7 : 6;
	write(fd, &b, 1);
}

void c634_backspace(int fd)
{
	char b = 8;
	write(fd, &b, 1);
}

void c634_clear(int fd)
{
	char b = 12;
	write(fd, &b, 1);
}

void c634_cr(int fd)
{
	char b = 13;
	write(fd, &b, 1);
}

void c634_lf(int fd)
{
	char b = 10;
	write(fd, &b, 1);
}

void c634_backlight(int fd, int v)
{
	char b[2] = {14};
	b[1] = v;
	write(fd, b, 2);
}

void c634_contrast(int fd, int v)
{
	char b[2] = {15};
	b[1] = v;
	write(fd, b, 2);
}

void c634_gotoxy(int fd, int x, int y)
{
	char b[3] = {17};
	b[1] = x;
	b[2] = y;
	write(fd, b, 3);
}

void c634_scrollMode(int fd, int m)
{
	char b = m ? 19 : 20;
	write(fd, &b, 1);
}

void c634_wrapMode(int fd, int m)
{
	char b = m ? 23 : 24;
	write(fd, &b, 1);
}

void c634_defineChar(int fd, char ch, char bm[8])
{
	char b[2] = {25};
	b[1] = ch - 128;
	write(fd, b, 2);
	write(fd, bm, 8);
}

void c634_reboot(int fd)
{
	char b = 26;
	write(fd, &b, 1);
}

void c634_printf(int fd, char const *format, ...)
{
	char *str, *str1;
	int i, j, len;
	va_list ap;
	
	str = malloc(1024);
	if (str)
	{
		va_start(ap, format);
		vsnprintf(str, 1024, format, ap);
		va_end(ap);
		len = strlen(str);
		for (i = 0; i < len; i += j + 1)
		{
			str1 = strchr(str + i, '\n');
			j = str1 ? str1 - str : len - i;
			write(fd, str + i, j);
			if (i + j < len)
				write(fd, "\r\n", 2);
		}
		free(str);
	}
}
