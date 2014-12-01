#ifndef HEX_H
#define HEX_H

int hex_decode( const unsigned char *input, unsigned char **decoded );
void show_hex_str( const unsigned char *array, int length , CString& str);
void show_hex_char( const unsigned char *array, int length , char*str);
void show_hex( const unsigned char *array, int length);
#endif
