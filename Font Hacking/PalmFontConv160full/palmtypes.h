/*
 * $Id: palmtypes.h,v 1.9 2004/12/09 14:22:23 arpruss Exp $
 *
 * The true home of this program is palmfontconv.sourceforge.net.  If you
 * find it elsewhere and improve it, please send a patch to
 * Alexander R. Pruss <ap85@georgetown.edu>.
 *
 * fontconv - Copyright (c) 2003, Alexander R. Pruss
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 *
 */

#ifndef PALMFONTCONV_PALMTYPES_H
#define PALMFONTCONV_PALMTYPES_H

typedef unsigned char Boolean;
typedef unsigned char Byte;
typedef unsigned short Word;
typedef signed short SWord;
typedef unsigned long DWord;
typedef struct FontTagV2 {
/* 0 */	Word fontType;
/* 2 */	Word firstChar;
/* 4 */	Word lastChar;
/* 6 */	Word maxWidth;
/* 8 */	Word kernMax;
/* a */	Word nDescent;
/* c */	Word fRectWidth;
/* e */	Word fRectHeight;
/* 10 */ Word owTLoc;
/* 12 */ Word ascent;
/* 14 */ Word descent;
/* 16 */ Word leading;
/* 18 */ Word rowWords;
/* 1a */ Word version;
/* 1c */ Word densityCount;
} FontTypeV2;

typedef struct BitmapTypeV1 {
        SWord width;
        SWord height;
        Word  rowBytes;
        Byte  flags;
        Byte  reserved0;
        Byte  pixelSize;
        Byte  version;
        Word  nextDepthOffset;
        Word  reserved[2];
} BitmapTypeV1;

typedef struct {
        SWord  width;
        SWord  height;
        Word   rowBytes;
        Byte   flags;
        Byte   reserved0;
        Byte   pixelSize;
        Byte   version;
        Word   nextDepthOffset;
        Byte   transparentIndex;
        Byte   compressionType;
        Word   reserved;
} BitmapTypeV2;

typedef struct BitmapTypeV3 {
       SWord width;
       SWord height;
       Word  rowBytes;
       Byte  flags;
       Byte  reserved0;
       Byte  pixelSize;
       Byte  version;
       Byte  size;
       Byte  pixelFormat;
       Byte  unused;
       Byte  compressionType;
       Word  density;
       DWord transparentValue;
       DWord nextBitmapOffset;
} BitmapTypeV3;

typedef enum {
        kDensityLow = 72,
        kDensityOneAndAHalf = 108,
        kDensityDouble = 144,
        kDensityTriple = 216,
        kDensityQuadruple = 288
} DensityType;

#define compressedLE       0x01
#define hasTransparencyLE  0x04

#define compressed         0x80
#define hasColorTable      0x40
#define hasTransparency    0x20
#define indirect           0x10
#define forScreen          0x08
#define directColor        0x04
#define indirectColorTable 0x02
#define noDither           0x01

#define dmDBNameLength 32

typedef struct {
	DWord			nextRecordListID;
	Word			numRecords;
} RecordListType;

typedef struct {
	Byte	name[ dmDBNameLength ];
	Word 	attributes;
	Word 	version;
	DWord	creationDate;
	DWord	modificationDate;
	DWord	lastBackupDate;
	DWord	modificationNumber;
	DWord	appInfoID;
	DWord	sortInfoID;
	Byte	type[4];
	Byte	creator[4];
	DWord	uniqueIDSeed;
	RecordListType	recordList;
} DatabaseHdrType;

#define DatabaseHdrType_sizes  "B32WWD6B8DDW"

typedef struct {
    char	type[ 4 ];
	Word	id;
	Word	localChunkIDHigh;
	Word    localChunkIDLow;
} RsrcEntryType;
#define RsrcEntryType_sizes   "B4WWW"
#define RsrcEntryType_size    10

typedef struct {
	DWord	localChunkID;
	Byte	attributes;
	Byte	uniqueID[3];
} RecordEntryType;
#define RecordEntryType_sizes "DB4"
#define RecordEntryType_size 8

typedef enum {
    BitmapCompressionTypeScanLine = 0,
    BitmapCompressionTypeRLE,
    BitmapCompressionTypePackBits,
    BitmapCompressionTypeEnd,
    BitmapCompressionTypeBest = 0x64,
    BitmapCompressionTypeNone = 0xFF
} BitmapCompressionType;

#define MAX_RESOURCES ( 65536l / sizeof( RsrcEntryType ) - 1 )

#define MAX_RESOURCE_SIZE 65000u
#define MAX_SAFE_RESOURCE_SIZE 65000u

#endif
