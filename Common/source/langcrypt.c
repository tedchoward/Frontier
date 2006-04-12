
/*	$Id$    */

/******************************************************************************

    UserLand Frontier(tm) -- High performance Web content management,
    object database, system-level and Internet scripting environment,
    including source code editing and debugging.

    Copyright (C) 1992-2004 UserLand Software, Inc.

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
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

******************************************************************************/

#include "frontier.h"
#include "standard.h"

#include "langinternal.h"
#include "kernelverbs.h"
#include "kernelverbdefs.h"
#include "resources.h"
#include "tablestructure.h"
#include "lang.h"
#include "frontierconfig.h"

#include "langcrypt.h"

#include "md5.h"
#include "whirlpool.h"
#include "sha.h"
#include "sha_locl.h"


typedef enum tycryptverbtoken { /* verbs that are processed by crypt.c */

	whirlpoolfunc,

	hmacmd5func,
	
	md5func,
	
	sha1func,
	
	hmacsha1func,
	
	ctcryptverbs
	} tycryptverbtoken;


static boolean cryptfunctionvalue (short token, hdltreenode hparam1, tyvaluerecord *vreturned, bigstring bserror) {
	
	/*
	2006-03-07 creedon: created, cribbed from htmlfunctionvalue
	*/
	
	hdltreenode hp1 = hparam1;
	tyvaluerecord *v = vreturned;
	
	setbooleanvalue (false, v); /* by default, crypt functions return false */
	
	switch (token) {
		
		case whirlpoolfunc: { /* 2006-03-04 creedon: Whirlpool hash */

			Handle h;
			struct NESSIEstruct w;
			u8 digest [DIGESTBYTES];
			short ctconsumed = 1;
			short ctpositional = 1;
			tyvaluerecord vtranslate;

			setbooleanvalue (true, &vtranslate);

			if (!getreadonlytextvalue (hp1, 1, &h))
				return (false);

			flnextparamislast = true;

			if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x0b""flTranslate"), &vtranslate)) 
				return (false);

			lockhandle (h);

			NESSIEinit (&w);
			NESSIEadd ((unsigned char *)(*h), 8 * gethandlesize (h), &w);
			NESSIEfinalize (&w, digest);

			unlockhandle (h);

			if (vtranslate.data.flvalue) { /* return a hex string */
				bigstring bs;
				unsigned char enc [] = "0123456789abcdef";
				long ix;
				Handle hresult;

				for (ix = 0; ix < DIGESTBYTES; ix++) {
					setstringcharacter (bs, 2 * ix, enc [(int) ((digest [ix]) / 16)]);
					setstringcharacter (bs, 2 * ix + 1, enc [(int) ((digest [ix]) % 16)]);
					} /* for */

				setstringlength (bs, DIGESTBYTES * 2);

				if (!newtexthandle (bs, &hresult))
					return (false);
				
				return (setheapvalue (hresult, stringvaluetype, v));
				}
				
			else { /* return a binary of unknown type */
				Handle hresult;

				if (!newfilledhandle (digest, DIGESTBYTES, &hresult))
					return (false);

				return (setbinaryvalue (hresult, typeunknown, v));
				}
			
			return (true);
			} /* whirlpoolfunc */

		case hmacmd5func: { /* 2006-03-05 creedon: keyed-hashing for message authentication using md5 */

			Handle hdata, hkey;
			unsigned char digest [16];
			short ctconsumed = 2;
			short ctpositional = 2;
			tyvaluerecord vtranslate;

			setbooleanvalue (true, &vtranslate);

			if (!getreadonlytextvalue (hp1, 1, &hdata))
				return (false);
			
			if (!getreadonlytextvalue (hp1, 2, &hkey))
				return (false);

			flnextparamislast = true;

			if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x0b""flTranslate"), &vtranslate)) 
				return (false);

			lockhandle (hkey);
			lockhandle (hdata);
			
			hmacmd5 ((unsigned char *)(*hdata), gethandlesize (hdata), (unsigned char *)(*hkey), gethandlesize (hkey), (unsigned char *) digest);

			unlockhandle (hkey);
			unlockhandle (hdata);
			
			if (vtranslate.data.flvalue) { /* return a hex string */
				bigstring bs;
				unsigned char enc [] = "0123456789abcdef";
				long ix;
				Handle hresult;

				for (ix = 0; ix < 16; ix++) {
					setstringcharacter (bs, 2 * ix, enc [(int) ((digest [ix]) / 16)]);
					setstringcharacter (bs, 2 * ix + 1, enc [(int) ((digest [ix]) % 16)]);
					} /* for */

				setstringlength (bs, 32);

				if (!newtexthandle (bs, &hresult))
					return (false);
				
				return (setheapvalue (hresult, stringvaluetype, v));
				}
				
			else { /* return a binary of unknown type */
				Handle hresult;

				if (!newfilledhandle (digest, 16, &hresult))
					return (false);

				return (setbinaryvalue (hresult, typeunknown, v));
				}
			
			return (true);
			} /* hmacmd5func */

		case md5func: { /* 2006-03-10 creedon: cribbed from stringverbs.c */
			Handle x;
			MD5_CTX hashcontext; /* MD5 context. */
			unsigned char checksum [16];
			short ctconsumed = 1;
			short ctpositional = 1;
			tyvaluerecord vtranslate;
		
			if (!getreadonlytextvalue (hp1, 1, &x))
				return (false);

			setbooleanvalue (true, &vtranslate);

			flnextparamislast = true;

			if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x0b""flTranslate"), &vtranslate)) 
				return (false);

			lockhandle (x);
	
			MD5Init (&hashcontext);
			
			MD5Update (&hashcontext, (unsigned char *)(*x), gethandlesize (x));
			
			MD5Final (checksum, &hashcontext);
			
			unlockhandle (x);

			if (vtranslate.data.flvalue) { /* return a hex string */
				bigstring bs;
				unsigned char enc [] = "0123456789abcdef";
				long ix;
				Handle h;

				for(ix = 0; ix < 16; ix++) {
					setstringcharacter (bs, 2 * ix, enc [(int)((checksum [ix]) / 16)]);
					setstringcharacter (bs, 2 * ix + 1, enc [(int)((checksum [ix]) % 16)]);
					} /* for */

				setstringlength (bs, 32);

				if (!newtexthandle (bs, &h))
					return (false);

				return (setheapvalue (h, stringvaluetype, v));
				}

			else { /* return a binary of unknown type */
				Handle h;

				if (!newfilledhandle (checksum, 16, &h))
					return (false);

				return (setbinaryvalue (h, typeunknown, v));
				}
			
			return (true);
			} /* md5func */
			
		case sha1func: { /* 2006-03-11 creedon: SHA1 hash */
			Handle h;
			SHA_CTX hashcontext; /* SHA context */
			unsigned char digest [SHA_DIGEST_LENGTH];
			short ctconsumed = 1;
			short ctpositional = 1;
			tyvaluerecord vtranslate;

			if (!getreadonlytextvalue (hp1, 1, &h))
				return (false);

			setbooleanvalue (true, &vtranslate);

			flnextparamislast = true;

			if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x0b""flTranslate"), &vtranslate)) 
				return (false);

			lockhandle (h);
	
			SHA1_Init (&hashcontext);
			
			SHA1_Update (&hashcontext, (unsigned char *)(*h), gethandlesize (h));
			
			SHA1_Final (digest, &hashcontext);

			unlockhandle (h);

			if (vtranslate.data.flvalue) { /* return a hex string */
				bigstring bs;
				unsigned char enc [] = "0123456789abcdef";
				long ix;
				Handle hresult;

				for(ix = 0; ix < SHA_DIGEST_LENGTH; ix++) {
					setstringcharacter (bs, 2 * ix, enc [(int)((digest [ix]) / 16)]);
					setstringcharacter (bs, 2 * ix + 1, enc [(int)((digest [ix]) % 16)]);
					} /* for */

				setstringlength (bs, SHA_DIGEST_LENGTH * 2);

				if (!newtexthandle (bs, &hresult))
					return (false);

				return (setheapvalue (hresult, stringvaluetype, v));
				}

			else { /* return a binary of unknown type */
				Handle hresult;

				if (!newfilledhandle (digest, SHA_DIGEST_LENGTH, &hresult))
					return (false);

				return (setbinaryvalue (hresult, typeunknown, v));
				}
			
			return (true);
			} /* sha1func */
			
		case hmacsha1func: { /* 2006-03-11 creedon: keyed-hashing for message authentication using sha1 */

			Handle hdata, hkey;
			unsigned char digest [SHA_DIGEST_LENGTH];
			short ctconsumed = 2;
			short ctpositional = 2;
			tyvaluerecord vtranslate;

			setbooleanvalue (true, &vtranslate);

			if (!getreadonlytextvalue (hp1, 1, &hdata))
				return (false);
			
			if (!getreadonlytextvalue (hp1, 2, &hkey))
				return (false);

			flnextparamislast = true;

			if (!getoptionalparamvalue (hp1, &ctconsumed, &ctpositional, BIGSTRING ("\x0b""flTranslate"), &vtranslate)) 
				return (false);

			lockhandle (hkey);
			lockhandle (hdata);
			
			hmacsha1 ((unsigned char *)(*hdata), gethandlesize (hdata), (unsigned char *)(*hkey), gethandlesize (hkey), (unsigned char *) digest);

			unlockhandle (hkey);
			unlockhandle (hdata);
			
			if (vtranslate.data.flvalue) { /* return a hex string */
				bigstring bs;
				unsigned char enc [] = "0123456789abcdef";
				long ix;
				Handle hresult;

				for (ix = 0; ix < SHA_DIGEST_LENGTH; ix++) {
					setstringcharacter (bs, 2 * ix, enc [(int) ((digest [ix]) / 16)]);
					setstringcharacter (bs, 2 * ix + 1, enc [(int) ((digest [ix]) % 16)]);
					} /* for */

				setstringlength (bs, SHA_DIGEST_LENGTH * 2);

				if (!newtexthandle (bs, &hresult))
					return (false);
				
				return (setheapvalue (hresult, stringvaluetype, v));
				}
				
			else { /* return a binary of unknown type */
				Handle hresult;

				if (!newfilledhandle (digest, SHA_DIGEST_LENGTH, &hresult))
					return (false);

				return (setbinaryvalue (hresult, typeunknown, v));
				}
			
			return (true);
			} /* hmacsha1func */

		default:
			getstringlist (langerrorlist, unimplementedverberror, bserror);
			
			return (false);
		} /* switch */
	} /* cryptfunctionvalue */


boolean cryptinitverbs (void) {
	
	/*
	2006-03-07 creedon: created, cribbed from htmlinitverbs
	*/
	
	return (loadfunctionprocessor (idcryptverbs, &cryptfunctionvalue));
	} /* cryptinitverbs */


boolean hmacmd5 (unsigned char * text, int text_len, unsigned char * key, int key_len, unsigned char * digest) {

	/*
	2006-03-05 creedon: created, cribbed from rfc2104, with a few alterations to fit our code and style
						aradke had implemented this function as part of his crypto.dll also
	
	text; pointer to data stream
	text_len; length of data stream
	key; pointer to authentication key
	key_len; length of authentication key
	digest; caller digest to be filled in
	*/

        MD5_CTX context;
        unsigned char k_ipad [65];	/* inner padding - key XORd with ipad */
        unsigned char k_opad [65];	/* outer padding - key XORd with opad */
        unsigned char tk [16];
        int i;

	if (key_len > 64) { /* if key is longer than 64 bytes reset it to key=MD5(key) */

		MD5_CTX tctx;

		MD5Init (&tctx);
		MD5Update (&tctx, key, key_len);
		MD5Final (tk, &tctx);

		key = tk;
		key_len = 16;
		}

	/*
	the HMAC_MD5 transform looks like:
	
	MD5(K XOR opad, MD5(K XOR ipad, text))
	
	where K is an n byte key
	ipad is the byte 0x36 repeated 64 times
	opad is the byte 0x5c repeated 64 times
	and text is the data being protected
	*/

	memset (k_ipad, 0, sizeof (k_ipad)); /* start out by storing key in pads */
	memset (k_opad, 0, sizeof (k_opad));
	memcpy (k_ipad, key, key_len);
	memcpy (k_opad, key, key_len);
	
	for (i = 0; i < 64; i++) { /* XOR key with ipad and opad values */
		k_ipad [i] ^= 0x36;
		k_opad [i] ^= 0x5c;
		}
	
        /* perform inner MD5 */
	
	MD5Init (&context);					/* init context for 1st pass */
	MD5Update (&context, k_ipad, 64);		/* start with inner pad */
	MD5Update (&context, text, text_len);	/* then text of datagram */
	MD5Final (digest, &context);			/* finish up 1st pass */
	
	/* perform outer MD5 */
	
	MD5Init (&context);					/* init context for 2nd pass */
	MD5Update (&context, k_opad, 64);		/* start with outer pad */
	MD5Update (&context, digest, 16);		/* then results of 1st hash */
	MD5Final (digest, &context);			/* finish up 2nd pass */
	
	return (true);
	} /* hmacmd5 */


boolean hmacsha1 (unsigned char * text, int text_len, unsigned char * key, int key_len, unsigned char * digest) {

	/*
	2006-03-10 creedon: created, cribbed from hmacmd5 function
	
	text; pointer to data stream
	text_len; length of data stream
	key; pointer to authentication key
	key_len; length of authentication key
	digest; caller digest to be filled in
	*/

        SHA_CTX context;
        unsigned char k_ipad [SHA_CBLOCK + 1];		/* inner padding - key XORd with ipad */
        unsigned char k_opad [SHA_CBLOCK + 1];	/* outer padding - key XORd with opad */
        unsigned char tk [SHA_DIGEST_LENGTH];
        int i;

	if (key_len > SHA_CBLOCK) { /* if key is longer than 64 bytes reset it to key=SHA1(key) */

		SHA_CTX tctx;

		SHA1_Init (&tctx);
		SHA1_Update (&tctx, key, key_len);
		SHA1_Final (tk, &tctx);

		key = tk;
		key_len = SHA_DIGEST_LENGTH;
		}

	/*
	the HMAC_SHA1 transform looks like:
	
	SHA1(K XOR opad, SHA1(K XOR ipad, text))
	
	where K is an n byte key
	ipad is the byte 0x36 repeated 64 times
	opad is the byte 0x5c repeated 64 times
	and text is the data being protected
	*/

	memset (k_ipad, 0, sizeof (k_ipad)); /* start out by storing key in pads */
	memset (k_opad, 0, sizeof (k_opad));
	memcpy (k_ipad, key, key_len);
	memcpy (k_opad, key, key_len);
	
	for (i = 0; i < SHA_CBLOCK; i++) { /* XOR key with ipad and opad values */
		k_ipad [i] ^= 0x36;
		k_opad [i] ^= 0x5c;
		}
	
        /* perform inner SHA1 */
	
	SHA1_Init (&context);						/* init context for 1st pass */
	SHA1_Update (&context, k_ipad, SHA_CBLOCK);	/* start with inner pad */
	SHA1_Update (&context, text, text_len);			/* then text of datagram */
	SHA1_Final (digest, &context);					/* finish up 1st pass */
	
	/* perform outer SHA1 */
	
	SHA1_Init (&context);							/* init context for 2nd pass */
	SHA1_Update (&context, k_opad, SHA_CBLOCK);		/* start with outer pad */
	SHA1_Update (&context, digest, SHA_DIGEST_LENGTH);	/* then results of 1st hash */
	SHA1_Final (digest, &context);						/* finish up 2nd pass */
	
	return (true);
	} /* hmacsha1 */

