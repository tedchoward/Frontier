
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

/*
    MPEG Audio Layer I/II/III frame header

    There is no main file header in an MPEG audio file. An MPEG
    audio file is built up from a succession of smaller parts called
    frames. A frame is a datablock with its own header and audio
    information.

    In the case of Layer I or Layer II, frames are some totally
    independent items, so you can cut any part of MPEG file and play
    it correctly. The player will then play the music starting to
    the first plain valid frame founded. However, in the case of
    Layer III, frames are not always independant. Due to the possible
    use of the "byte reservoir", wich is a kind of buffer, frames
    are often dependent of each other. In the worst case, 9 frames
    may be needed before beeing able to decode one frame.

    When you want to read info about an MPEG audio file, it is usually
    enough to find the first frame, read its header and assume that
    the other frames are the same. But this is not always the case,
    as variable bitrate (VBR) files may be encountered. In a VBR
    file, the bitrate can be changed in each frame. It can be used,
    as an exemple to keep a constant sound quality during the whole
    file, by using more bits where the music need more to be
    encoded.

    The frame header is 32 bits (4 bytes) length. The first eleven
    bits (or first twelve bits in the case of the MPEG 2.5 extension) of
    a frame header are always set to 1 and are called "frame sync".

    Frames may have an optional CRC checksum. It is 16 bits long and, if
    it exists, follows the frame header. After the CRC comes the
    audio data. By re-calculating the CRC and comparing its value to
    the sored one, you can check if the frame has been altered during
    transmission of the bitstream.

    Here is a presentation of the frame header content. Characters A
    to M are used to indicate different fields. In the table below,
    you can see details about the content of each field.

            AAAAAAAA AAABBCCD EEEEFFGH IIJJKLMM
            
    
    Sign    Length (bits)   Position (bits)     Description
    
      A        11               (31-21)         Frame sync (all bits must be set)
      
      B         2               (20,19)         MPEG Audio version ID
      
                                                  00 - MPEG Version 2.5 (later extension of MPEG 2)
                                                  01 - reserved
                                                  10 - MPEG Version 2 (ISO/IEC 13818-3)
                                                  11 - MPEG Version 1 (ISO/IEC 11172-3) 
                                                  
                                                Note: MPEG Version 2.5 was added lately to the MPEG 2 standard.
                                                It is an extension used for very low bitrate files, allowing the
                                                use of lower sampling frequencies. If your decoder does not
                                                support this extension, it is recommended for you to use 12 bits
                                                for synchronization instead of 11 bits.
                                                  
      C         2               (18,17)         Layer description
      
                                                  00 - reserved
                                                  01 - Layer III
                                                  10 - Layer II
                                                  11 - Layer I
    
      D         1                 (16)          Protection bit
      
                                                  0 - Protected by CRC (16bit CRC follows header)
                                                  1 - Not protected
                                                  
      E         4               (15,12)         Bitrate index
    
                                                    bits    V1,L1   V1,L2   V1,L3   V2,L1   V2, L2 & L3
                                                    
                                                    0000    free    free    free    free    free
                                                    0001     32      32       32     32       8
                                                    0010     64      48       40     48      16
                                                    0011     96      56       48     56      24
                                                    0100    128      64       56     64      32
                                                    0101    160      80       64     80      40
                                                    0110    192      96       80     96      48
                                                    0111    224     112       96    112      56
                                                    1000    256     128      112    128      64
                                                    1001    288     160      128    144      80
                                                    1010    320     192      160    160      96
                                                    1011    352     224      192    176     112
                                                    1100    384     256      224    192     128
                                                    1101    416     320      256    224     144
                                                    1110    448     384      320    256     160
                                                    1111    bad     bad      bad    bad     bad
    
                                                    NOTES: All values are in kbps
                                                    V1 - MPEG Version 1
                                                    V2 - MPEG Version 2 and Version 2.5
                                                    L1 - Layer I
                                                    L2 - Layer II
                                                    L3 - Layer III
    
                                                "free" means free format. The free bitrate must remain constant,
                                                an must be lower than the maximum allowed bitrate. Decoders are
                                                not required to support decoding of free bitrate streams.
                                                
                                                "bad" means that the value is unallowed. 

                                                MPEG files may feature variable bitrate (VBR). Each frame may then
                                                be created with a different bitrate. It may be used in all layers.
                                                Layer III decoders must support this method.
                                                Layer I & II decoders may support it. 

                                                For Layer II there are some combinations of bitrate and mode
                                                which are not allowed. Here is a list of allowed combinations. 
    
                                                    bitrate   single channel   stereo   intensity stereo   dual channel
                                                    
                                                    free           yes          yes             yes           yes
                                                     32            yes          no              no            no 
                                                     48            yes          no              no            no 
                                                     56            yes          no              no            no 
                                                     64            yes          yes             yes           yes
                                                     80            yes          no              no            no 
                                                     96            yes          yes             yes           yes
                                                    112            yes          yes             yes           yes
                                                    128            yes          yes             yes           yes
                                                    160            yes          yes             yes           yes
                                                    192            yes          yes             yes           yes
                                                    224            no           yes             yes           yes
                                                    256            no           yes             yes           yes
                                                    320            no           yes             yes           yes
                                                    384            no           yes             yes           yes
    
      F         2               (11,10)         Sampling rate frequency index
      
                                                    bits   MPEG1      MPEG2      MPEG2.5
                                                    
                                                     00    44100 Hz   22050 Hz   11025 Hz
                                                     01    48000 Hz   24000 Hz   12000 Hz
                                                     10    32000 Hz   16000 Hz    8000 Hz
                                                     11    reserv.    reserv.     reserv.
    
      G         1                 (9)           Padding bit
      
                                                    0 - frame is not padded
                                                    1 - frame is padded with one extra slot
    
                                                Padding is used to exactly fit the bitrate.
                                                As an example: 128kbps 44.1kHz layer II uses a lot of 418 bytes
                                                and some of 417 bytes long frames to get the exact 128k bitrate.
                                                For Layer I slot is 32 bits long, for Layer II and Layer III slot is 8 bits long.
    
      H         1                 (8)           Private bit. This one is only informative.
      
      I         2                (7,6)          Channel Mode
      
                                                    00 - Stereo
                                                    01 - Joint stereo (Stereo)
                                                    10 - Dual channel (2 mono channels)
                                                    11 - Single channel (Mono)
                                                    
                                                Note: Dual channel files are made of two independant mono channel.
                                                Each one uses exactly half the bitrate of the file. Most decoders
                                                output them as stereo, but it might not always be the case.
                                                One example of use would be some speech in two different languages
                                                carried in the same bitstream, and then an appropriate decoder would
                                                decode only the choosen language. 
    
      J         2                (5,4)          Mode extension (Only used in Joint stereo) 
    
                                                Mode extension is used to join informations that are of no use
                                                for stereo effect, thus reducing needed bits. These bits are dynamically
                                                determined by an encoder in Joint stereo mode, and Joint Stereo can be
                                                changed from one frame to another, or even switched on or off. 
                                                    
                                                Complete frequency range of MPEG file is divided in subbands.
                                                There are 32 subbands. For Layer I & II these two bits determine
                                                frequency range (bands) where intensity stereo is applied.
                                                For Layer III these two bits determine which type of joint stereo
                                                is used (intensity stereo or m/s stereo).
                                                Frequency range is determined within decompression algorithm.
                                                    
                                                                                        Layer III
                                                    value   Layer I & II    Intensity stereo  MS stereo
                                                    
                                                     00     bands  4 to 31          off          off
                                                     01     bands  8 to 31          on           off
                                                     10     bands 12 to 31          off          on
                                                     11     bands 16 to 31          on           on
         
      K         1                 (3)           Copyright
      
                                                    0 - Audio is not copyrighted
                                                    1 - Audio is copyrighted
    
                                                The copyright has the same meaning as the copyright bit on CDs and DAT tapes,
                                                i.e. telling that it is illegal to copy the contents if the bit is set.
    
      L         1                 (2)           Original
      
                                                    0 - Copy of original media
                                                    1 - Original media
    
                                                The original bit indicates, if it is set, that the frame is located
                                                on its original media.
    
      M         2                (1,0)          Emphasis
      
                                                    00 - none
                                                    01 - 50/15 ms
                                                    10 - reserved
                                                    11 - CCIT J.17
    
                                                The emphasis indication is here to tell the decoder that the file
                                                must be de-emphasized, ie the decoder must 're-equalize' the sound
                                                after a Dolby-like noise supression. It is rarely used. 
    
      (c) 1999-2000 Gabriel Bouvigne for MP3'Tech - www.mp3-tech.org
      
      Source: http://www.mp3-tech.org/programmer/frame_header.html
 */


#include "frontier.h"
#include "standard.h"

#include "file.h"
#include "shell.h"


typedef struct {
	long version;
	long layer;
	long bitrate;
	long frequency;
	long channelmode;
	long modeextension;
	long emphasis;
	boolean flchecksum;
	boolean flpadding;
	boolean flprivate;
	boolean flcopyright;
	boolean floriginal;
	} tympeginfostruct;


static const long mpegsamplingfrequency[3][4] = {
		{44100, 48000, 32000, 0}, // MPEG 1
		{22050, 24000, 16000, 0}, // MPEG 2
		{11025, 12000,  8000, 0}  // MPEG 2.5
	};
	

static const long mpegbitrate[2][3][16] = {
	{ // MPEG 1
		{0, 32, 64, 96, 128, 160, 192, 224, 256, 288, 320, 352, 384, 416, 448, 0}, // Layer 1
		{0, 32, 48, 56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 384, 0}, // Layer 2
		{0, 32, 40, 48,  56,  64,  80,  96, 112, 128, 160, 192, 224, 256, 320, 0}  // Layer 3
	},{ // MPEG 2
		{0, 32, 48, 56,  64,  80,  96, 112, 128, 144, 160, 176, 192, 224, 256, 0}, // Layer 1
		{0,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0}, // Layer 2
		{0,  8, 16, 24,  32,  40,  48,  56,  64,  80,  96, 112, 128, 144, 160, 0}  // Layer 3
	}
};


static boolean decodempegheader (unsigned long header, tympeginfostruct *info) {
	
	if ((header >> 20) != 0x00000fff) // bits 20-31 must be set, we don't recognize MPEG 2.5
		return (false);
	
	info->version = 2 - ((header >> 19) & 0x00000001); // 1 = MPEG1, 2 = MPEG2
	
	info->layer = 4 - ((header >> 17) & 0x00000003); // 1 = Layer I, 2 = Layer II, 3 = Layer III

	info->bitrate = mpegbitrate [info->version - 1] [info->layer - 1] [(header >> 12) & 0x0000000f];

	info->frequency = mpegsamplingfrequency [info->version - 1] [(header >> 10) & 0x00000003];

	info->channelmode =  (header >> 6) & 0x00000003;

	info->modeextension =  (header >> 4) & 0x00000003;

	info->emphasis = header & 0x00000003;

	info->flchecksum = ((header >> 16) & 0x00000001) ? false : true;

	info->flpadding = ((header >> 9) & 0x00000001) ? true : false;

	info->flprivate = ((header >> 8) & 0x00000001) ? true : false;

	info->flcopyright = ((header >> 3) & 0x00000001) ? true : false;
	
	info->floriginal =  ((header >> 2) & 0x00000001) ? true : false;

	return (true);
	}/*decodempegheader*/


static long extractfourbytes (Handle h, long ix) {
	
	long n;
	
	n = (unsigned char) (*h)[ix++];
	
	n <<= 8;

	n |= (unsigned char) (*h)[ix++];
	
	n <<= 8;

	n |= (unsigned char) (*h)[ix++];
	
	n <<= 8;

	n |= (unsigned char) (*h)[ix++];

	return (n);
	}/*extractfourbytes*/


static boolean getxingheader (tympeginfostruct *info, Handle h, long ix, long *ctframes, long *ctbytes) {
#pragma unused (info)

	long flags;
	
	ix += 32; //MPEG 1, Layer 3
	
	if ((*h)[ix++] != 'X')
		return (false);
	
	if ((*h)[ix++] != 'i')
		return (false);
	
	if ((*h)[ix++] != 'n')
		return (false);
	
	if ((*h)[ix++] != 'g')
		return (false);
	
	flags = extractfourbytes (h, ix);
	
	ix += sizeof(long);
	
	if ((flags & 0x00000001) == 0) //check for frame count
		return (false);
		
	*ctframes = extractfourbytes (h, ix);
	
	ix += sizeof(long);
	
	if (flags & 0x00000002) { //check for byte count
		*ctbytes = extractfourbytes (h, ix);
		ix += sizeof(long);
		}
	
	return (true);
	}/*getxingheader*/
	

boolean getmp3info (const tyfilespec *fs, long *seconds, long *bitrate, long *frequency, long *offset, boolean *flvariablebitrate) {

	tympeginfostruct mp3;
	long ix = 0;
	long header = 0;
	long oldpos, ctbytes, ctframes;
	Handle h = nil;
	unsigned char c1, c2;
	const long buffersize = 32767;
	
	if (!fifgetposition (fs, &oldpos))
		return (false);

	if (!fifsetposition (fs, 0))
		return (false);
	
	if (!fifreadhandle (fs, buffersize, &h)) //read the first 32k of data
		return (false);
	
	for (ix = 0; ix < buffersize - (long) sizeof (header) - 1; ix++) {
	
		c1 = (unsigned char) (*h)[ix];
			
		if (c1 == 0xff) {
		
			c2 = (unsigned char) (*h)[ix+1];

			c2 >>= 4;
					
			if (c2 == 0x0f) {
				header = extractfourbytes (h, ix);
				break;
				}
			}
		}
	
	if (decodempegheader (header, &mp3) && mp3.version == 1 && mp3.layer == 3) {

		if (!fifgetendoffile (fs, &ctbytes))
			goto exit;
		
		ctbytes -= ix;
		
		*offset = ix;
		
		*frequency = mp3.frequency;
		
		*seconds = (mp3.bitrate != 0) ? (ctbytes / (125 * mp3.bitrate)) : 0;			

		*bitrate = mp3.bitrate;

		*flvariablebitrate = getxingheader (&mp3, h, ix + sizeof (header), &ctframes, &ctbytes);
		
		if (*flvariablebitrate) {
			*seconds = (ctframes * 26) / 1000;			
			*bitrate = (*seconds) ? ((ctbytes) / (*seconds * 125)) : 0;
			}
		}
	else {
		
		shellerrormessage (BIGSTRING ("\x4b" "Can't get the MP3 file info because an MP3 frame header could not be found."));

		goto exit;
		}
	
	if (!fifsetposition (fs, oldpos))
		goto exit;

	return (true);

exit:

	fifsetposition (fs, oldpos);
	
	return (false);
	}/*getmp3info*/
