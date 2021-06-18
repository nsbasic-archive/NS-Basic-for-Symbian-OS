/*
    Copyright (c) 2004, Alexander R. Pruss
    All rights reserved.

    This license applies to palmbitmap.h only.

    Redistribution and use in source and binary forms, with or without modification,
    are permitted provided that the following conditions are met:

        Redistributions of source code must retain the above copyright notice, this
        list of conditions and the following disclaimer.

        Redistributions in binary form must reproduce the above copyright notice, this
        list of conditions and the following disclaimer in the documentation and/or
        other materials provided with the distribution.

        Neither the name of the PalmResize Project nor the names of its
        contributors may be used to endorse or promote products derived from this
        software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
    ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
    WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
    DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR
    ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
    (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
    LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON
    ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
    SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */


/* This header file defines various structures for Palm bitmaps.
   There is no guarantee that OS functions will generate bitmaps
   in these formats, but they accept bitmaps in these formats
   (with the correct version numbers). */

#ifndef PLUCKER_PALMBITMAP_H
#define PLUCKER_PALMBITMAP_H

#define palmDirectColor              0x04
#define palmHasTransparency          0x20
#define palmHasColorTable            0x40  /* constants for bitmap flags */
#define palmCompressed               0x80


#define RED_BITS                     5
#define GREEN_BITS                   6
#define BLUE_BITS                    5
#define MAX_RED                      ( ( 1 << RED_BITS ) - 1 )
#define MAX_GREEN                    ( ( 1 << GREEN_BITS ) - 1 )
#define MAX_BLUE                     ( ( 1 << BLUE_BITS ) - 1 )

typedef enum {
        palmPixelFormatIndexed = 0,
        palmPixelFormat565,
        palmPixelFormat565LE,
        palmPixelFormatIndexedLE,
} PalmPixelFormatType;

typedef struct {
        Int16  width;
        Int16  height;
        UInt16 rowBytes;
        UInt8  flags;
        UInt8  reserved0;
        UInt8  pixelSize;
        UInt8  version;
} PalmBitmapType;

typedef struct {
        Int16  width;
        Int16  height;
        UInt16 rowBytes;
        UInt8  flags;
        UInt8  reserved0;
        UInt16 reserved[4];
} PalmBitmapTypeV0;

typedef struct {
        Int16  width;
        Int16  height;
        UInt16 rowBytes;
        UInt8  flags;
        UInt8  reserved0;
        UInt8  pixelSize;
        UInt8  version;
        UInt16 nextDepthOffset;
        UInt16 reserved[2];
} PalmBitmapTypeV1;

typedef struct {
        Int16  width;
        Int16  height;
        UInt16 rowBytes;
        UInt8  flags;
        UInt8  reserved0;
        UInt8  pixelSize;
        UInt8  version;
        UInt16 nextDepthOffset;
        UInt8  transparentIndex;
        UInt8  compressionType;
        UInt16 reserved;
} PalmBitmapTypeV2;

typedef struct {
       Int16  width;
       Int16  height;
       UInt16 rowBytes;
       UInt8  flags;
       UInt8  reserved0;
       UInt8  pixelSize;
       UInt8  version;
       UInt8  size;
       UInt8  pixelFormat;
       UInt8  unused;
       UInt8  compressionType;
       UInt16 density;
       UInt32 transparentValue;
       UInt32 nextBitmapOffset;
} PalmBitmapTypeV3;

typedef enum {
    palmBitmapCompressionTypeScanLine = 0,
    palmBitmapCompressionTypeRLE,
    palmBitmapCompressionTypePackBits,
    palmBitmapCompressionTypeEnd,
    palmBitmapCompressionTypeBest = 0x64,
    palmBitmapCompressionTypeNone = 0xFF
} PalmBitmapCompressionType;

typedef struct {
        UInt16 numEntries;
        /* RGBColorType entry[]; */
} PalmColorTableType;

typedef struct {
    UInt8 redBits;
    UInt8 greenBits;
    UInt8 blueBits;
    UInt8 reserved;
    RGBColorType transparentColor;
} PalmBitmapDirectInfoType;

#endif /* PALM_BITMAP_H */

