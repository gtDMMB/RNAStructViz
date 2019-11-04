#include "rocbox.h"

static int draw_it_active = 1;

void roc_box_draw(int x, int y, int w, int h, Fl_Color bgcolor) {

  fl_color(draw_it_active ? FL_BLACK : fl_inactive(FL_BLACK));
  fl_rect(x, y, w, h);
  fl_color(draw_it_active ? bgcolor : fl_inactive(bgcolor));
  fl_rectf(x + 1, y + 1, w - 2, h - 2);
  fl_color(draw_it_active ? FL_BLACK : fl_inactive(FL_BLACK));
  fl_line(x, y, x + w - 1, y + h - 1);
  fl_line(x, y + h - 1, x + w - 1, y);

}

void roc_box_init()
{
	Fl::set_boxtype(ROC_BOX, roc_box_draw, 1, 1, 2, 2);
}
