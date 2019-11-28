/* PixelDataStruct.h : Define a reusable structure definition for the pixel-based image data 
 *                     generated when saving to C-Source files in the GIMP;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.11.28
 */

#ifndef __PIXEL_DATA_STRUCT_TYPE_H__
#define __PIXEL_DATA_STRUCT_TYPE_H__

typedef struct {
     unsigned int width;
     unsigned int height;
     unsigned int bytes_per_pixel;
     const unsigned char *pixel_data;
} PixelDataStruct_t;

#endif
