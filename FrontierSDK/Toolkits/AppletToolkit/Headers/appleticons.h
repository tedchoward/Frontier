
/*© copyright 1991-96 UserLand Software, Inc. All Rights Reserved.*/


#ifndef appleticonsinclude
#define appleticonsinclude



#define	atNone 0x0
#define	atVerticalCenter 0x1
#define atTop 0x2
#define atBottom 0x3
#define atHorizontalCenter 0x4
#define atLeft 0x8
#define atRight 0xC


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