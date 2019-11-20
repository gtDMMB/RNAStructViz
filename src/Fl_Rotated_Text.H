// Fl_Rotated_Text.H,v 0.1
//
// Copyright 2005 by Roman Kantor.
//
// This library is free software; you can redistribute it and/or
// modify it under the terms of the GNU Library General Public License
// version 2 as published by the Free Software Foundation.
//
// This library is distributed  WITHOUT ANY WARRANTY;
// WITHOUT even the implied warranty of MERCHANTABILITY 
// or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
// Library General Public License for more details.
// You should have received a copy of the GNU Library General Public
// License along with this library; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.

#ifndef Fl_Rotated_Text_H 
# define Fl_Rotated_Text_H

#include <FL/Fl_Bitmap.H>



/** Fl_Rotated_Text is a bitmap image for printing of rotated text. Only multiplications of 90 degrees
* are recently implemented. It draws specified text with current color but with pre-defined font,
* size and rotation. When drawn for the first time, it internaly builds a bitmap (and bitmap cache).
* All subsequent calls of draw() method are fast. The text can be changed using set() method: 
* this method causes the bitmap to be rebuild so use it only when you realy want the text to be changed.
*
* How does it work:
* When drawn for the first time, the class draws the text to an offscreen and reads the result using
* fl_read_image() function. This image is then transformed to a standard 1-bit deep bitmap with respect
* to the defined rotation. The bitmap is then drawn with current color whenever needed.
*
* Advantages: it is portable, reasonably fast and "lihtweight".
* Disadvantages: it de-antialiases  the output even if antialiased text is available and used.
*
* Usage:
* The easiest way to use it is to add the instance of this class to a widget using Fl_Widget::image(...)
* method. If the widget label align is FL_ALIGN_LEFT or FL_ALIGN_RIGHT, it nicely draws the text along
* the left or right widget edge. Of course you can use it elsewhere like a standard Fl_Image (Fl_Bitmap)
* class. Note that the size of the resulting image depends on the text itself (the length, the font/size
* and the number of lines) and can be accessed by standard w() and h() methods.
*/

class Fl_Rotated_Text : public Fl_Bitmap{
  uchar rot_;
  char * text_;
  Fl_Font font_;
  short size_;
  uchar align_;
  uchar * ar2; // this points to the same as array but it is not constant - will be probably removed  <const_cast> used
public:

  /** Sets the text, font, size, align and rotation of printed text.
  * Text is copied to internal buffer. Parameter align determines the
  * formating within the box * before rotation * if multi-line text is printed.
  * Only FL_ALIGN_LEFT and FL_ALIGN_RIGHT has a meaning. The rotation is either
  * rotation in degrees counter-clockwise if the value is greater than 10 or,
  * if smaller than 10, a multiplication of 90 deg.
  * 0: no rotation, 1: 90 deg, 2: 180 deg (upside down), 3: 270 (or -90) deg.
  * User should use this function only when he wants to change the text within the image:
  * the function is relatively slow as it causes to rebuild the bitmap.with next redraw.*/
  void set(const char *text, Fl_Font font, int size, uchar align, int rotation);
  
  // returns pointer to internal buffer describing the text
  const char * text() {return text_;}
  
  /** Returns the text font. */
  Fl_Font font(){return font_;}
  
  /** Returns the text size */
  int size(){return size_;}
  
  /** Returns the formating parameter *before* the rotation.
  * Only FL_ALIGN_LEFT and FL_ALIGN_RIGHT has a meaning. */
  Fl_Align align(){return (Fl_Align) align_;}
  
  /** Function returns rotation in degrees, it is always a multiplication of 90 deg */ 
  int rotation(){return rot_ * 90;}
  
  /** The drawing function. */
  void draw(int x, int y, int w, int h, int cx = 0, int cy = 0);
  
  /** The constructor calls also set() sunction.*/
  Fl_Rotated_Text(const char *text = 0, Fl_Font font = FL_HELVETICA, 
  	          int size = 14, uchar align = 0, int rotation = 0);
  ~Fl_Rotated_Text();
};

#endif
