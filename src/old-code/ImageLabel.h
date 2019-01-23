/* ImageLabel.h : Ties up all of the loose ends around the Fl_Label semi-widget.
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2018.10.22
 */

#ifndef _IMAGE_LABEL_H_
#define _IMAGE_LABEL_H_

#include <stdlib.h>

#include <FL/Enumerations.H>
#include <FL/Fl_Widget.H>

#define DEFAULT_ALIGN          (FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT)

class ImageLabel : public Fl_Widget, Fl_Label {

     protected:
          int xoffset, yoffset;
	  int width, height;

     public:
	  inline ImageLabel(int xo, int yo, int w, int h, const char *labelText) : 
		 Fl_Widget(xo, yo, w, h, labelText), 
		 xoffset(xo), yoffset(yo), width(w), height(h) {
	       
	       Fl_Label::image = NULL;
	       Fl_Label::color = GUI_TEXT_COLOR;
	       align_ = DEFAULT_ALIGN;
	       font = LOCAL_BFFONT;
	       value = this->label();
	       Fl_Label::type = _FL_IMAGE_LABEL;	       
          }

	  inline ~ImageLabel() {
               if(Fl_Label::image != NULL) {
	            delete Fl_Label::image;
	       }
	  }

	  inline void setImage(const unsigned char *pxbuf, int w, int h, int depth) {
		  Fl_Label::image = new Fl_RGB_Image(pxbuf, w, h, depth);
	  }

	  inline void draw() {
		  Fl_Label::draw(xoffset, yoffset, width, height, align_);
	  }

};

#endif
