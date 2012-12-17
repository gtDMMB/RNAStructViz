// Fl_Rotated_Text.cxx,v 0.1
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


#include "Fl_Rotated_Text.H"
#include <string.h>
#include <FL/x.H>
#include <FL/fl_draw.H>
#include <FL/Fl.H>
#include <FL/Enumerations.H>

Fl_Rotated_Text::~Fl_Rotated_Text(){
  delete[] text_;
}
Fl_Rotated_Text::Fl_Rotated_Text(const char *text , Fl_Font font , int size , uchar align , int rotation ):Fl_Bitmap((const uchar *)0, 0, 0), ar2(0){
  text_ = 0;
  set(text, font, size, align, rotation);
}


void Fl_Rotated_Text::set(const char *text, Fl_Font font, int size, uchar align, int rotation){

  uncache();
  if(alloc_array) delete[] ar2;
  ar2 = 0;
  array = 0;
  alloc_array = 0;


  delete[] text_;
  text_=0;
  font_ = font;
  size_ = size;
  align_ = align;

  if(!text || !(*text)){
    w(0);
    h(0);
    return;
  }

  text_ = new char[strlen(text) + 1];
  strcpy(text_, text);

  if(rotation >9)
    rot_ = (uchar)(((rotation+45)/90) % 4);
  else
    rot_ = (uchar) (rotation & 3);
  int w_ = 0;
  int h_ = 0;
  int old_font = fl_font();
  int old_size = fl_size();
  fl_font(font,size);
  fl_measure(text_,w_,h_,0); // assure that w() and h() are always available
  fl_font(old_font,old_size);
  h_ += fl_height()/2;
  if(rot_ & 1){
    w(h_);
    h(w_);
  }else{
    w(w_);
    h(h_);
  }
 
};

void Fl_Rotated_Text::draw(int x, int y, int W, int H, int cx, int cy){
  
  if(!text_) return;
  if(!rot_){ // standard drawing)
    int olf_font = fl_font();
    int old_size = fl_size();
    fl_font(font_,size_);
    fl_push_clip(x, y, W-cx, H-cy);
    fl_draw(text_, x - cx, y - cy, w(), h(), (Fl_Align)align_, (Fl_Image *) 0, 0);
    fl_pop_clip();
    fl_font(olf_font, old_size);
    return;
  }
  if(!array){ // not drawn yet, building rotated bitmap "cache"
    int w_, h_;
    if(rot_ & 1){
      w_ = h();
      h_ = w();
    }else{
      w_ = w();
      h_ = h();
    }
    int bsize = ((w()+7)/8)  * h();
    array = ar2 = new uchar[bsize];
    alloc_array = 1;
    //memset(ar2, 0, bsize);

    int old_font = fl_font();
    int old_size = fl_size();

    Fl_Color old_color = fl_color();
    Fl_Offscreen offscreen = fl_create_offscreen(w_,h_);
    fl_begin_offscreen(offscreen);
    fl_color(0x000000);
    fl_rectf(0,0,w_,h_);
    fl_font(font_, size_);
    fl_color(0x00FF0000); // we use green color to plot to the offscreen

    fl_draw(text_,0, 0, w_, h_, (Fl_Align)align_, (Fl_Image *)0, 0);
    uchar * rgb = fl_read_image(0, 0, 0, w_, h_, 0);
    fl_end_offscreen();
    fl_delete_offscreen(offscreen);
    fl_font(old_font, old_size);
    fl_color(old_color);

    int i,j;

    uchar * start = rgb;
    int i_iter = 0;
    int j_iter = 0;
    switch(rot_){
    case 3:
      start +=  w_ * (h_ - 1)*3 + 1;
      i_iter = - w_ * 3;
      j_iter =  3;
      break;
    case 2:
      start +=  (w_ * h_ - 1)*3 + 1;
      i_iter = -3;
      j_iter = - w_ * 3;
      break;
    case 1:
      start +=  (w_ - 1)*3 + 1;
      i_iter = w_ * 3;
      j_iter = -3;
      break;
    }

    uchar * stj = start;
    uchar c;
    uchar * where;
    uchar * sti;

    for(j = 0; j< h(); j++, stj += j_iter){
      uchar val = 0;
      c = 8;
      where = ar2 + j*((w()+7)/8);
      for(i = 0, sti = stj; i< w(); i++, sti +=i_iter){
        if(*sti >127) val |= c;
        if(c & (uchar)128){ // pushing value to the array
          *where = val;
          where++;
          c =1;
          val = 0;
        }else
          c <<=1;
      }
      if(w() % 8){ //need to push last byte
        * where = val;
        where++;
      }
    }
    
    delete[] rgb;
  }

  Fl_Bitmap::draw(x, y, W, H, cx, cy); // finaly drawing the bitmap

};

        
    



