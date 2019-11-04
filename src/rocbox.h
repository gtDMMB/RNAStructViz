#ifndef ROCBOX_H
#define ROCBOX_H

#include <FL/Fl.H>
#include <FL/fl_draw.H>

#define ROC_BOX FL_FREE_BOXTYPE

void roc_box_draw(int x, int y, int w, int h, Fl_Color bgcolor);
void roc_box_init();

#endif
