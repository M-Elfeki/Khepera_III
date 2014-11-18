/*-------------------------------------------------------------------------------
 * Project: KoreBot Library	
 * Author: Yves Piguet, 2004
 * $Date: 2004/07/29 10:51:54 $
 * $Revision: 1.1 $
 * 
 * 
 * $Header: /home/cvs/libkorebot/src/crystalfontz634.h,v 1.1 2004/07/29 10:51:54 cgaudin Exp $
 */
#if !defined(__crystalfontz634__)
#	define __crystalfontz634__

#ifdef __cplusplus
extern "C" {
#endif

enum
{
	k_c634_cursorHide = 0,
	k_c634_cursorUnderline,
	k_c634_cursorBlock,
	k_c634_cursorInverting
};

int c634_open(char const *path);
void c634_close(int fd);
void c634_home(int fd);
void c634_hide(int fd);
void c634_restore(int fd);
void c634_cursor(int fd, int type);
void c634_backspace(int fd);
void c634_clear(int fd);
void c634_cr(int fd);
void c634_lf(int fd);
void c634_backlight(int fd, int v);	// v = 0..100
void c634_contrast(int fd, int v);	// v = 0..100
void c634_gotoxy(int fd, int x, int y);
void c634_scrollMode(int fd, int m);
void c634_wrapMode(int fd, int m);
void c634_defineChar(int fd, char ch, char bm[8]);	// ch = 128..135, bm[i] = 0..63
void c634_reboot(int fd);
void c634_printf(int fd, char const *format, ...);

#ifdef __cplusplus
}
#endif

#endif
