/* This file was added by Nathan Stenzel <nathanstenzel@users.sourceforge.net>
   It will only get rough rough images, but might do for now
   I may be working on the header by decoding the image into a stream "manually"
   This is the Polaroid Fun! 320's header
   It should be modified for the Polaroid 640SE
   For the bennifit of anyone else that wants to try to figure improve this....
     ...I split the jpegheder into the different sections.
  I believe I did this right. I split it at FF markers that were not ended by OO
  Please email me if you can help me out.
  I need to figure out how to decode the image data by "hand"
     and then edit this header to fit
 */

// New table
#define  quantization_chroma_new \
    "\x03\x02\x02\x03\x02\x02\x03\x03" "\x03\x03\x04\x03\x03\x04\x05\x08" \
    "\x05\x05\x04\x04\x05\x0A\x07\x07" "\x06\x08\x0C\x0A\x0C\x0C\x0B\x0A" \
    "\x0B\x0B\x0D\x0E\x12\x10\x0D\x0E" "\x11\x0E\x0B\x0B\x10\x16\x10\x11" \
    "\x13\x14\x15\x15\x15\x0C\x0F\x17" "\x18\x16\x14\x18\x12\x14\x15\x14"

#define quantization_luma_new \
    "\x03\x04\x04\x05\x04\x05\x09\x05" "\x05\x09\x14\x0D\x0B\x0D\x14\x14" \
    "\x14\x14\x14\x14\x14\x14\x14\x14" "\x14\x14\x14\x14\x14\x14\x14\x14" \
    "\x14\x14\x14\x14\x14\x14\x14\x14" "\x14\x14\x14\x14\x14\x14\x14\x14" \
    "\x14\x14\x14\x14\x14\x14\x14\x14" "\x14\x14\x14\x14\x14\x14\x14\x14"

#define  quantization_table_1s \
    "\x01\x01\x01\x01\x01\x01\x01\x01" "\x01\x01\x01\x01\x01\x01\x01\x01" \
    "\x01\x01\x01\x01\x01\x01\x01\x01" "\x01\x01\x01\x01\x01\x01\x01\x01" \
    "\x01\x01\x01\x01\x01\x01\x01\x01" "\x01\x01\x01\x01\x01\x01\x01\x01" \
    "\x01\x01\x01\x01\x01\x01\x01\x01" "\x01\x01\x01\x01\x01\x01\x01\x01"

#define  quantization_table_6s \
    "\x02\x03\x03\x05\x05\x06\x06\x06" "\x06\x06\x06\x06\x06\x06\x06\x06" \
    "\x06\x06\x06\x06\x06\x06\x06\x06" "\x06\x06\x06\x06\x06\x06\x06\x06" \
    "\x06\x06\x06\x06\x06\x06\x06\x06" "\x06\x06\x06\x06\x06\x06\x06\x06" \
    "\x06\x06\x06\x06\x06\x06\x06\x06" "\x06\x06\x06\x06\x06\x06\x06\x06"

#define  quantization_table_14s \
    "\x06\x08\x08\x0a\x07\x0a\x0d\x0a" "\x0a\x0d\x13\x0f\x0c\x0f\x13\x14" \
    "\x0f\x0d\x0d\x0f\x14\x14\x14\x14" "\x14\x14\x14\x14\x14\x14\x14\x14" \
    "\x14\x14\x14\x14\x14\x14\x14\x14" "\x14\x14\x14\x14\x14\x14\x14\x14" \
    "\x14\x14\x14\x14\x14\x14\x14\x14" "\x14\x14\x14\x14\x14\x14\x14\x14"

#define quantization_table_36s \
    "\x04\x05\x08\x16\x36\x36\x36\x36" "\x36\x36\x36\x36\x36\x36\x36\x36" \
    "\x36\x36\x36\x36\x36\x36\x36\x36" "\x36\x36\x36\x36\x36\x36\x36\x36" \
    "\x36\x36\x36\x36\x36\x36\x36\x36" "\x36\x36\x36\x36\x36\x36\x36\x36" \
    "\x36\x36\x36\x36\x36\x36\x36\x36" "\x36\x36\x36\x36\x36\x36\x36\x36"

// Default luminance table
#define  quantization_luma \
    "\x10\x0B\x0C\x0E\x0C\x0A\x10\x0E" "\x0D\x0E\x12\x11\x10\x13\x18\x28" \
    "\x1A\x18\x16\x16\x18\x31\x23\x25" "\x1D\x28\x3A\x33\x3D\x3C\x39\x33" \
    "\x38\x37\x40\x48\x5C\x4E\x40\x44" "\x57\x45\x37\x38\x50\x6D\x51\x57" \
    "\x5F\x62\x67\x68\x67\x3E\x4D\x71" "\x79\x70\x64\x78\x5C\x65\x67\x63"

// Default Chrominance table
#define quantization_chroma \
    "\x11\x12\x12\x18\x15\x18\x2f\x1a" "\x1a\x2f\x63\x63\x63\x63\x63\x63" \
    "\x63\x63\x63\x63\x63\x63\x63\x63" "\x63\x63\x63\x63\x63\x63\x63\x63" \
    "\x63\x63\x63\x63\x63\x63\x63\x63" "\x63\x63\x63\x63\x63\x63\x63\x63" \
    "\x63\x63\x63\x63\x63\x63\x63\x63" "\x63\x63\x63\x63\x63\x63\x63\x63"

#define JPEG_Q0 "\xFF\xDB\x00\x43\x00"
#define JPEG_Q1 "\xFF\xDB\x00\x43\x01"

/* This is the old table that might be faulty
#define  quantization_table2 \
    "\xFF\xDB\x00\x43\x01" \
    "\x01\x01\x01\x01\x01\x01\x01\x01" "\x01\x01\x01\x01\x01\x01\x01\x01" \
    "\x01\x01\x01\x01\x01\x01\x01\x01" "\x01\x01\x01\x01\x01\x01\x01\x01" \
    "\x01\x01\x01\x01\x01\x01\x01\x01" "\x01\x01\x01\x01\x01\x01\x01\x01" \
    "\x01\x01\x01\x01\x01\x01\x01\x01" "\x01\x01\x01\x01\x01\x01\x01\x01"
*/

/* I am trying setting the SOFC0 so that component one uses quantization table 2
 * This seems to give it higher contrast that would be needed to understand the image.
 */
#define SOFCO_320x128 \
    "\xFF\xC0\x00\x11" \
    "\x08\x00\x80\x01\x40\x03" "\x01\x11\x01" "\x02\x11\x00" "\x03\x21\x00"

#define SOFC0_640x248 \
    "\xFF\xC0\x00\x11" \
    "\x08\x00\xf8\x02\x80\x03" "\x01\x11\x00" "\x02\x11\x00" "\x03\x21\x00"

static struct {
unsigned int size;
char *data;
} jpegheader[] = {
    {2, "\xFF\xD8"},
// JFIF APPO Marker:Version 1.01, density 78x78 (the 00 48 00 48)
    {18, "\xFF\xE0\x00\x10\x4A\x46\x49\x46" "\x00\x01\x01\x01\x00\x48\x00\x48" "\x00\x00"},
    {69, JPEG_Q0 quantization_chroma_new},
    {69, JPEG_Q1 quantization_luma_new},
/*
//	define quantization table 0 precision 0
    {69, jpeg_q0 quantization_table_6s},
//	define quantization table 1 precision 0
    {69, jpeg_q1 quantization_table_14s},
*/
//	Start Of Frame 0: width=320, height=128, components=3
// It seems that the camera makes 320x120 images
// 8 of the 128 is to get the pixels I think are for color correction
// I have to wonder if the camera even makes actual 320x240 images
    {19, SOFCO_320x128},
//Define Huffman Table 0x00 (same in old and new)
    {33, "\xFF\xC4\x00\x1F\x00\x00\x01\x05" "\x01\x01\x01\x01\x01\x01\x00\x00"
         "\x00\x00\x00\x00\x00\x00\x01\x02" "\x03\x04\x05\x06\x07\x08\x09\x0A\x0B"},
//	Define Huffman Table 0x10
    {183, "\xFF\xC4\x00\xB5\x10\x00\x02\x01" "\x03\x03\x02\x04\x03\x05\x05\x04"
          "\x04\x00\x00\x01\x7D\x01\x02\x03" "\x00\x04\x11\x05\x12\x21\x31\x41"
          "\x06\x13\x51\x61\x07\x22\x71\x14" "\x32\x81\x91\xA1\x08\x23\x42\xB1"
          "\xC1\x15\x52\xD1\xF0\x24\x33\x62" "\x72\x82\x09\x0A\x16\x17\x18\x19"
          "\x1A\x25\x26\x27\x28\x29\x2A\x34" "\x35\x36\x37\x38\x39\x3A\x43\x44"
          "\x45\x46\x47\x48\x49\x4A\x53\x54" "\x55\x56\x57\x58\x59\x5A\x63\x64"
          "\x65\x66\x67\x68\x69\x6A\x73\x74" "\x75\x76\x77\x78\x79\x7A\x83\x84"
          "\x85\x86\x87\x88\x89\x8A\x92\x93" "\x94\x95\x96\x97\x98\x99\x9A\xA2"
          "\xA3\xA4\xA5\xA6\xA7\xA8\xA9\xAA" "\xB2\xB3\xB4\xB5\xB6\xB7\xB8\xB9"
          "\xBA\xC2\xC3\xC4\xC5\xC6\xC7\xC8" "\xC9\xCA\xD2\xD3\xD4\xD5\xD6\xD7"
          "\xD8\xD9\xDA\xE1\xE2\xE3\xE4\xE5" "\xE6\xE7\xE8\xE9\xEA\xF1\xF2\xF3"
          "\xF4\xF5\xF6\xF7\xF8\xF9\xFA"},
//	Define Huffman Table 0x01
    {33, "\xFF\xC4\x00\x1F\x01\x00\x03\x01" "\x01\x01\x01\x01\x01\x01\x01\x01"
         "\x00\x00\x00\x00\x00\x00\x01\x02" "\x03\x04\x05\x06\x07\x08\x09\x0A\x0B"},
//	Define Huffman Table 0x11
    {183, "\xFF\xC4\x00\xB5\x11\x00\x02\x01" "\x02\x04\x04\x03\x04\x07\x05\x04"
          "\x04\x00\x01\x02\x77\x00\x01\x02" "\x03\x11\x04\x05\x21\x31\x06\x12"
          "\x41\x51\x07\x61\x71\x13\x22\x32" "\x81\x08\x14\x42\x91\xA1\xB1\xC1"
          "\x09\x23\x33\x52\xF0\x15\x62\x72" "\xD1\x0A\x16\x24\x34\xE1\x25\xF1"
          "\x17\x18\x19\x1A\x26\x27\x28\x29" "\x2A\x35\x36\x37\x38\x39\x3A\x43"
          "\x44\x45\x46\x47\x48\x49\x4A\x53" "\x54\x55\x56\x57\x58\x59\x5A\x63"
          "\x64\x65\x66\x67\x68\x69\x6A\x73" "\x74\x75\x76\x77\x78\x79\x7A\x82"
          "\x83\x84\x85\x86\x87\x88\x89\x8A" "\x92\x93\x94\x95\x96\x97\x98\x99"
          "\x9A\xA2\xA3\xA4\xA5\xA6\xA7\xA8" "\xA9\xAA\xB2\xB3\xB4\xB5\xB6\xB7"
          "\xB8\xB9\xBA\xC2\xC3\xC4\xC5\xC6" "\xC7\xC8\xC9\xCA\xD2\xD3\xD4\xD5"
          "\xD6\xD7\xD8\xD9\xDA\xE2\xE3\xE4" "\xE5\xE6\xE7\xE8\xE9\xEA\xF2\xF3"
          "\xF4\xF5\xF6\xF7\xF8\xF9\xFA"},
//	Ss=0, Se=63, Ah=0, Al=0
    {14, "\xFF\xDA\x00\x0C\x03\x01\x00\x02" "\x00\x03\x00\x00\x3F\x00"}
};

/* djpeg can be used with -debug to show some info on the encoding of a jpeg
 * I fed my modified djpeg (which makes jpegs from my raw camera data) and got:

Start of Image
JFIF APP0 marker: version 1.01, density 72x72  1
Define Quantization Table 0  precision 0
Define Quantization Table 1  precision 0
Start Of Frame 0xc0: width=320, height=240, components=3
    Component 1: 1hx1v q=0
    Component 2: 1hx1v q=0
    Component 3: 2hx1v q=0
Define Huffman Table 0x00
Define Huffman Table 0x10
Define Huffman Table 0x01
Define Huffman Table 0x11
Start Of Scan: 3 components
    Component 1: dc=0 ac=0
    Component 2: dc=0 ac=0
    Component 3: dc=0 ac=0
  Ss=0, Se=63, Ah=0, Al=0

 * I used this info to comment the jpeg header above
 * I honestly still don't understand it very well
 */











