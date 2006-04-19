
/*	$Id$    */

/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#ifndef appleticonsinclude
#define appleticonsinclude



#define	atNone 0x00
#define	atVerticalCenter 0x01
#define atTop 0x02
#define atBottom 0x03
#define atHorizontalCenter 0x04
#define atLeft 0x08
#define atRight 0x0C


extern unsigned short ttLabel [8];


boolean newiconsuite (Handle *);

boolean addicontosuite (Handle, Handle, ResType);

boolean disposeiconsuite (Handle, Boolean);
 
boolean ploticonsuite (Rect *, short, short, Handle);

boolean geticonfromsuite (Handle *, Handle, ResType);

boolean ploticonresource (Rect *, short, short, short);

boolean ploticonresource (Rect *, short, short, short);

ResType icontypetorestype (short);

boolean packiconsuite (Handle, Handle *);

boolean unpackiconsuite (Handle, Handle *);

boolean geticonsuite (short, Handle *);

#endif