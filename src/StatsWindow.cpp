
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <math.h>
#include <time.h>

#include <iostream>
#include <algorithm>

#include <FL/Fl_Box.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Check_Button.H> 
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Chart.H>
#include <FL/Fl_Group.H>

#include "StatsWindow.h"
#include "StructureManager.h"
#include "RNAStructViz.h"
#include "RNAStructure.h"
#include "InputWindow.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"

#include "pixmaps/StatsFormula.c"
#include "pixmaps/StatsWindowIcon.xbm"
#include "pixmaps/StatsOverviewLegend.c"

Fl_RGB_Image * StatsWindow::overviewLegendImage = new Fl_RGB_Image(
               StatsOverviewLegend.pixel_data, 
               StatsOverviewLegend.width, 
               StatsOverviewLegend.height, 
               StatsOverviewLegend.bytes_per_pixel
);

void RocBoxPlot::roc_box_draw(int x, int y, int w, int h, Fl_Color bgcolor) {
  fl_color(RocBoxPlot::draw_it_active ? FL_BLACK : fl_inactive(FL_BLACK));
  fl_rect(x, y, w, h);
  fl_color(RocBoxPlot::draw_it_active ? bgcolor : fl_inactive(bgcolor));
  fl_rectf(x + 1, y + 1, w - 2, h - 2);
  fl_color(RocBoxPlot::draw_it_active ? FL_BLACK : fl_inactive(FL_BLACK));
  fl_line(x, y, x + w - 1, y + h - 1);
  fl_line(x, y + h - 1, x + w - 1, y);
}

void RocBoxPlot::roc_box_init() {
    Fl::set_boxtype(RocBoxPlot::ROC_BOX, roc_box_draw, 1, 1, 2, 2);
}

void StatsWindow::Construct(int w, int h, const std::vector<int>& structures)
{

    folderIndex = -1;
    referenceIndex = -1;
    numStats = 0;
    statistics = NULL;
    color(GUI_WINDOW_BGCOLOR);
    
    /* Create the menu section on the left */
    menu_window = new Fl_Group(0, 0, 300, h, "Menu Group");
    {
        mwx = menu_window->x();
        mwy = menu_window->y();
        mww = menu_window->w();
        mwh = menu_window->h();
        ref_menu = new Fl_Choice(mwx+20,mwy+ 60, mww-65, 30, 
                                 "Select Reference Structure:");
        ref_menu->labelcolor(GUI_TEXT_COLOR);
        ref_menu->textcolor(GUI_BTEXT_COLOR);
        ref_menu->labelfont(FL_HELVETICA_BOLD_ITALIC);
        ref_menu->align(FL_ALIGN_TOP);    
        
        const char *dividerText = "--------------------------------------";
        dividerTextBox = new Fl_Box(mwx + 20, mwy + 95, mww - 65, 
                 5, dividerText);
        dividerTextBox->labelcolor(GUI_TEXT_COLOR);
	dividerTextBox->labelfont(FL_HELVETICA_BOLD_ITALIC);
        dividerTextBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);

        comp_menu = new Fl_Scroll(mwx+20,mwy+ 120, mww-65, 
                                  mwh-120-90 - 50, 
                                  "Structures for Comparison:");
        {    
            comp_pack = new Fl_Pack(mwx+20,mwy+120,mww-60,150);
            comp_pack->type(Fl_Pack::VERTICAL);
            comp_pack->end();
            comp_pack->color(GUI_WINDOW_BGCOLOR);
        }
        comp_menu->type(Fl_Scroll::VERTICAL);
        comp_menu->labelcolor(GUI_TEXT_COLOR);
        comp_menu->labelfont(FL_HELVETICA_BOLD_ITALIC);
        comp_menu->align(FL_ALIGN_TOP);
        comp_menu->color(GUI_WINDOW_BGCOLOR);
        comp_menu->end();
        
        calc_button = new Fl_Toggle_Button(mwx+20, mwy+ mwh-60, 205, 
                                           30, "@refresh   Calculate");
        calc_button->callback(CalcCallback);
        calc_button->labelcolor(GUI_BTEXT_COLOR); 
        calc_button->labelfont(FL_HELVETICA);

        selectAllCompsBtn = new Fl_Button(mwx + 20, mwy + mwh - 100, 
                                          205, 30, "@redo   Compare All");
        selectAllCompsBtn->callback(SelectAllButtonCallback);
        selectAllCompsBtn->labelcolor(GUI_BTEXT_COLOR);
        selectAllCompsBtn->labelfont(FL_HELVETICA);

        //menu_window->resizable(comp_menu);
    }
    menu_window->end();
    ref_menu->callback(ReferenceCallback, &comp_pack);
    
    /* Create the tabs section on the right */
    tab_window = new Fl_Tabs(270, 0, w-275, h, "Tabs Container");
    {
        overview_tab = new Fl_Group(270,20,w-275,h-20,"@filenew Summary"); 
        {
            Fl_Group *ov_charts_group = new Fl_Group(310,30,w-520,h-40,"");
            {
                                int ovcx = ov_charts_group->x();
                                int ovcy = ov_charts_group->y();
                                int ovcw = ov_charts_group->w();
                                int ovch = ov_charts_group->h();
                bp_chart = new Fl_Group(ovcx-10,ovcy+10,(ovcw-60)/2,(ovch-60)/2, "Base Pairs (BP)");
                bp_chart->box(FL_RSHADOW_BOX);
                bp_chart->align(FL_ALIGN_BOTTOM);
                bp_chart->labelcolor(GUI_TEXT_COLOR);
                bp_chart->labelfont(FL_HELVETICA);
                bp_chart->color(GUI_WINDOW_BGCOLOR);
                bp_chart->end();
                
                tp_chart = new Fl_Group(ovcx+30+(ovcw-60)/2,ovcy+10,(ovcw-60)/2,(ovch-60)/2,"True Positives (TP)");
                tp_chart->box(FL_RSHADOW_BOX);
                tp_chart->align(FL_ALIGN_BOTTOM);
                tp_chart->labelcolor(GUI_TEXT_COLOR);
                tp_chart->labelfont(FL_HELVETICA);
                tp_chart->color(GUI_WINDOW_BGCOLOR);
                tp_chart->end();
                
                fp_chart = new Fl_Group(ovcx-10,ovcy+40+(ovch-60)/2,(ovcw-60)/2,(ovch-60)/2,"False Positives (FP)");
                fp_chart->box(FL_RSHADOW_BOX);
                fp_chart->align(FL_ALIGN_BOTTOM);
                fp_chart->labelcolor(GUI_TEXT_COLOR);
                fp_chart->labelfont(FL_HELVETICA);
                fp_chart->color(GUI_WINDOW_BGCOLOR);
                fp_chart->end();
                
                fn_chart = new Fl_Group(ovcx+30+(ovcw-60)/2,ovcy+40+(ovch-60)/2,(ovcw-60)/2,(ovch-60)/2, "False Negatives (FN)");
                fn_chart->box(FL_RSHADOW_BOX);
                fn_chart->align(FL_ALIGN_BOTTOM);
                fn_chart->labelcolor(GUI_TEXT_COLOR);
                fn_chart->labelfont(FL_HELVETICA);
                fn_chart->color(GUI_WINDOW_BGCOLOR);
                fn_chart->end();
                
            }
            ov_charts_group->end();
            ov_charts_group->resizable(ov_charts_group);
            
            leg1_group = new Fl_Group(w-200, 30, 195, h-40, "");
            {
                int leg1x = leg1_group->x();
                int leg1y = leg1_group->y();
                int leg1w = leg1_group->w();
                int leg1h = leg1_group->h();
                
                Fl_Box* leg_label = new Fl_Box(leg1x - 15, leg1y + 10, leg1w, 30, 
                                                               "Plot Colors Legend:");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                leg_label->labelfont(FL_HELVETICA_BOLD_ITALIC);
                
                leg_label = new Fl_Box(leg1x-15,leg1y+ 60, leg1w, 30, "Reference Structure:");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                leg_label->labelfont(FL_HELVETICA_BOLD_ITALIC);
                
                leg1_ref = new Fl_Box(FL_UP_BOX,leg1x-15,leg1y+90, leg1w, 20,"");
                leg1_ref->color(GUI_TEXT_COLOR);
                leg1_ref->labelcolor(GUI_WINDOW_BGCOLOR);
                leg1_ref->labelfont(FL_HELVETICA);
                leg1_ref->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
                leg1_ref->hide();
                
                leg_label = new Fl_Box(leg1x-15,leg1y+ 120, leg1w, 30, "Predicted Structures:");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                leg_label->labelfont(FL_HELVETICA_BOLD_ITALIC);

                leg1_scroll = new Fl_Scroll(leg1x-15,leg1y+ 150, leg1w, leg1h-320);
                {    
                    leg1_pack = new Fl_Pack(leg1x-15,leg1y+150,leg1w,leg1h-320);
                    leg1_pack->type(Fl_Pack::VERTICAL);
                    leg1_pack->end();
                                        leg1_pack->color(GUI_WINDOW_BGCOLOR);
                }
                leg1_scroll->type(Fl_Scroll::VERTICAL);
                leg1_scroll->end();
                leg1_scroll->color(GUI_WINDOW_BGCOLOR);
                
                Fl_Box *legendExplImageBox = new Fl_Box(leg1x - 10, leg1y + leg1h - 125, 
				                        overviewLegendImage->w(), 
							overviewLegendImage->h());
                legendExplImageBox->image(overviewLegendImage);
            }
            leg1_group->end();
            leg1_group->resizable(leg1_scroll);
	    overview_tab->resizable(ov_charts_group);    
        }
        overview_tab->end();
        overview_tab->color(GUI_WINDOW_BGCOLOR);
        overview_tab->labelcolor(GUI_BTEXT_COLOR);
        overview_tab->labelfont(FL_HELVETICA);

        perc_tab = new Fl_Group(270,20,w-275,h-20,
                                "@+ Percentage Values");
        {
            Fl_Group *perc_charts_group = new Fl_Group(310,30,w-520,h-40,""); // DOUBLEWINDOW
            {
                int percx = perc_charts_group->x();
                        int percy = perc_charts_group->y();
                        int percw = perc_charts_group->w();
                        int perch = perc_charts_group->h();
                
                sens_chart = new Fl_Group(percx+10,percy+10,(percw-60)/2,(perch-60)/2, "Sensitivity");
                sens_chart->box(FL_RSHADOW_BOX);
            sens_chart->align(FL_ALIGN_BOTTOM);
            sens_chart->labelcolor(GUI_TEXT_COLOR);
                        sens_chart->labelfont(FL_HELVETICA);
            sens_chart->color(GUI_WINDOW_BGCOLOR);
            sens_chart->end();
                
            sel_chart = new Fl_Group(percx+50+(percw-60)/2,percy+10,(percw-60)/2,(perch-60)/2,"Selectivity");
            sel_chart->box(FL_RSHADOW_BOX);
            sel_chart->align(FL_ALIGN_BOTTOM);
            sel_chart->labelcolor(GUI_TEXT_COLOR);
                        sel_chart->labelfont(FL_HELVETICA);
            sel_chart->color(GUI_WINDOW_BGCOLOR);
            sel_chart->end();
                
        // Display formulas for the different percentage value statistics
        int vgap = 20;
        int hgap = 3; // Should be between 0 and 40
        statsFormulasImage = new Fl_RGB_Image(StatsFormula.pixel_data, 
                             StatsFormula.width, StatsFormula.height, 
                             StatsFormula.bytes_per_pixel);
        statsFormulasBox = new Fl_Box(percx+10+(percw-60)/2+statsFormulasImage->w()/2, percy+75+(perch-60)/2, 
                                      statsFormulasImage->w(), statsFormulasImage->h());
        statsFormulasBox->image(statsFormulasImage);
                
        ppv_chart = new Fl_Group(percx+10,percy+40+(perch-60)/2,(percw-60)/2,(perch-60)/2,"Positive Predictive Value");
        ppv_chart->box(FL_RSHADOW_BOX);
        ppv_chart->align(FL_ALIGN_BOTTOM);
        ppv_chart->labelcolor(GUI_TEXT_COLOR);
        ppv_chart->labelfont(FL_HELVETICA);
        ppv_chart->color(GUI_WINDOW_BGCOLOR);
        ppv_chart->end();
                
     }
         perc_charts_group->resizable(perc_charts_group);
         perc_charts_group->end();
            
         leg2_group = new Fl_Group(w-200,30,195,h-40,""); // DOUBLEWINDOW
            {
                int leg2x = leg2_group->x();
                int leg2y = leg2_group->y();
                int leg2w = leg2_group->w();
                int leg2h = leg2_group->h();
                
                Fl_Box* leg_label = new Fl_Box(leg2x-15,leg2y+10,leg2w-15, 30, 
                                               "Plot Colors Legend:");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                leg_label->labelfont(FL_HELVETICA_BOLD_ITALIC);

                leg_label = new Fl_Box(leg2x-15,leg2y+ 60, leg2w-15, 30, "Reference Structure:");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                leg_label->labelfont(FL_HELVETICA_BOLD_ITALIC);

                leg2_ref = new Fl_Box(FL_UP_BOX,
                                      leg2x-15, 
                                      leg2y+90, 
                                      leg2w-15, 
                                      20,
                                      "");
                leg2_ref->color(GUI_TEXT_COLOR);
                leg2_ref->labelcolor(GUI_WINDOW_BGCOLOR);
                leg2_ref->labelfont(FL_HELVETICA);
                leg2_ref->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
                leg2_ref->hide();
                
                leg_label = new Fl_Box(leg2x-15,leg2y+ 120,leg2w-15, 30, "Predicted Structures:");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                leg_label->labelfont(FL_HELVETICA);

                leg2_scroll = new Fl_Scroll(leg2x-15,leg2y+ 150,leg2w-15, leg2h-320);
                {    
                    leg2_pack = new Fl_Pack(leg2x-15,leg2y+150,leg2w-15,leg2h-320);
                    leg2_pack->type(Fl_Pack::VERTICAL);
                    leg2_pack->color(GUI_WINDOW_BGCOLOR);
                    leg2_pack->end();
                }
                leg2_scroll->type(Fl_Scroll::VERTICAL);
                //leg2_scroll->box(FL_UP_BOX);
                leg2_scroll->color(GUI_WINDOW_BGCOLOR);
                leg2_scroll->end();
                
             }
             leg2_group->end();
             leg2_group->resizable(leg2_scroll);
	     perc_tab->resizable(perc_charts_group);
        }
        perc_tab->end();
        perc_tab->color(GUI_WINDOW_BGCOLOR);
        perc_tab->labelcolor(GUI_BTEXT_COLOR);
        perc_tab->labelfont(FL_HELVETICA);

        pair_tab = new Fl_Group(270, 20, w-275, h-20, "@menu Base Pairings");
        {
            Fl_Group *pair_charts_group = new Fl_Group(310,30,w-520,h-40,""); // DOUBLEWINDOW
            {
                                int pairx = pair_charts_group->x();
                                int pairy = pair_charts_group->y();
                                int pairw = pair_charts_group->w();
                                int pairh = pair_charts_group->h();
                
                    gc_chart = new Fl_Group(pairx+10,pairy+10,(pairw-60)/2,
                                                        (pairh-60)/2,"G-C Base Pairs");
                gc_chart->box(FL_RSHADOW_BOX);
                gc_chart->align(FL_ALIGN_BOTTOM);
                gc_chart->labelcolor(GUI_TEXT_COLOR);
                                gc_chart->labelfont(FL_HELVETICA);
                gc_chart->color(GUI_WINDOW_BGCOLOR);
                gc_chart->end();
                
                au_chart = new Fl_Group(pairx+50+(pairw-60)/2,pairy+10,
                                        (pairw-60)/2,(pairh-60)/2,
                                        "A-U Base Pairs");
                au_chart->box(FL_RSHADOW_BOX);
                au_chart->align(FL_ALIGN_BOTTOM);
                au_chart->labelcolor(GUI_TEXT_COLOR);
                                au_chart->labelfont(FL_HELVETICA);
                au_chart->color(GUI_WINDOW_BGCOLOR);
                au_chart->end();
                
                gu_chart = new Fl_Group(pairx+10,pairy+40+(pairh-60)/2,
                                        (pairw-60)/2,(pairh-60)/2,
                                        "G-U Base Pairs");
                gu_chart->box(FL_RSHADOW_BOX);
                gu_chart->align(FL_ALIGN_BOTTOM);
                gu_chart->labelcolor(GUI_TEXT_COLOR);
                                gu_chart->labelfont(FL_HELVETICA);
                gu_chart->color(GUI_WINDOW_BGCOLOR);
                gu_chart->end();
                
                non_canon_chart = new Fl_Group(pairx+50+(pairw-60)/2,
                                               pairy+40+(pairh-60)/2,(pairw-60)/2,
                                               (pairh-60)/2, "Non-Canonical Base Pairs");
                non_canon_chart->box(FL_RSHADOW_BOX);
                non_canon_chart->align(FL_ALIGN_BOTTOM);
                non_canon_chart->labelcolor(GUI_TEXT_COLOR);
                                non_canon_chart->labelfont(FL_HELVETICA);
                non_canon_chart->color(GUI_WINDOW_BGCOLOR);
                non_canon_chart->end();
                
            }
            pair_charts_group->end();
            pair_charts_group->resizable(pair_charts_group);
            
            leg3_group = new Fl_Group(w-200,30,195,h-40,""); // DOUBLEWINDOW
            {
                int leg3x = leg3_group->x();
                int leg3y = leg3_group->y();
                int leg3w = leg3_group->w();
                int leg3h = leg3_group->h();
                
                Fl_Box* leg_label = new Fl_Box(leg3x+10,leg3y+ 10, leg3w-15, 30, 
                                               "Legend");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                leg_label->labelfont(FL_HELVETICA);

                leg_label = new Fl_Box(leg3x+10,leg3y+ 60,leg3w-15, 30, "Reference Structure");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                leg_label->labelfont(FL_HELVETICA);

                leg3_ref = new Fl_Box(FL_UP_BOX,
                                      leg3x+10, 
                                      leg3y+90,leg3w-15, 20,
                                      "");
                leg3_ref->color(GUI_TEXT_COLOR);
                leg3_ref->labelcolor(GUI_WINDOW_BGCOLOR);
                leg3_ref->labelfont(FL_HELVETICA);
                leg3_ref->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
                leg3_ref->hide();
                
                leg_label = new Fl_Box(leg3x+10,leg3y+ 120, leg3w-15, 30, "Predicted Structures");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                leg_label->labelfont(FL_HELVETICA);
                
                leg3_scroll = new Fl_Scroll(leg3x+10,leg3y+ 150,leg3w-15, leg3h-200);
                {    
                    leg3_pack = new Fl_Pack(leg3x+10,leg3y+150,leg3w-15,leg3h-200);
                    leg3_pack->type(Fl_Pack::VERTICAL);
                    leg3_pack->end();
                }
                leg3_scroll->type(Fl_Scroll::VERTICAL);
                //leg3_scroll->box(FL_UP_BOX);
                leg3_scroll->color(GUI_WINDOW_BGCOLOR);
                leg3_scroll->end();
                
            }
            leg3_group->end();
            leg3_group->resizable(leg3_scroll);
	    pair_tab->resizable(pair_charts_group);    
    }
    pair_tab->end();
    pair_tab->color(GUI_WINDOW_BGCOLOR);
    pair_tab->labelcolor(GUI_BTEXT_COLOR);
    pair_tab->labelfont(FL_HELVETICA);    

    roc_tab = new Fl_Group(270,20,w-275,h-20,"@<-> ROC Plot");
        {
            Fl_Group *roc_plot_group = new Fl_Group(280,30,w-508,h-40,""); // DOUBLEWINDOW
            {
                               int rpx = roc_plot_group->x();
                               int rpy = roc_plot_group->y();
                               int rpw = roc_plot_group->w();
                               int rph = roc_plot_group->h();
                
                roc_plot = new Fl_Group(rpx+40,rpy+20,rpw-50,
                                        rph-60,"ROC Plot");
                RocBoxPlot::roc_box_init();
                roc_plot->box(RocBoxPlot::ROC_BOX);
                roc_plot->align(FL_ALIGN_TOP);
                roc_plot->labelcolor(GUI_TEXT_COLOR);
                roc_plot->labelfont(FL_HELVETICA);
                roc_plot->color(GUI_WINDOW_BGCOLOR);
                roc_plot->end();
                
                /* Draw the labels for the y-axis */
                Fl_Box* text_rotate = new Fl_Box(rpx+0,roc_plot->y(),20,roc_plot->h());
                rocplot_rotated_text = new Fl_Rotated_Text("Sensitivity (%)",
                                                            FL_HELVETICA, 14, 0, 1);
                text_rotate->image(rocplot_rotated_text);
                
                // Put the number to the left of the axis, the dash to the right
                Fl_Box* label = new Fl_Box(FL_NO_BOX, roc_plot->x(),
                                           rpy+15,10,10,"100");
                label->align(FL_ALIGN_LEFT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);
                
                label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                   roc_plot->y()+roc_plot->h()-5-(int)(9*roc_plot->h()/10.0),10,10,"90");
                label->align(FL_ALIGN_LEFT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);
                
                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                   roc_plot->y()+roc_plot->h()-5-(int)(9*roc_plot->h()/10.0),10,10,"@-9line");
                label->align(FL_ALIGN_RIGHT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                                   roc_plot->y()+roc_plot->h()-5-(int)(8*roc_plot->h()/10.0),10,10,"80");
                label->align(FL_ALIGN_LEFT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);
                
                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                   roc_plot->y()+roc_plot->h()-5-(int)(8*roc_plot->h()/10.0),10,10,"@-9line");
                label->align(FL_ALIGN_RIGHT);
                label->labelcolor(GUI_TEXT_COLOR);
                    label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                                   roc_plot->y()+roc_plot->h()-5-(int)(7*roc_plot->h()/10.0),10,10,"70");
                label->align(FL_ALIGN_LEFT);
                label->labelcolor(GUI_TEXT_COLOR);
                                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                                   roc_plot->y()+roc_plot->h()-5-(int)(7*roc_plot->h()/10.0),10,10,"@-9line");
                label->align(FL_ALIGN_RIGHT);
                label->labelcolor(GUI_TEXT_COLOR);
                                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                                   roc_plot->y()+roc_plot->h()-5-(int)(6*roc_plot->h()/10.0),10,10,"60");
                label->align(FL_ALIGN_LEFT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);
                
                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                                   roc_plot->y()+roc_plot->h()-5-(int)(6*roc_plot->h()/10.0),10,10,"@-9line");
                label->align(FL_ALIGN_RIGHT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);
                
                label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                                   roc_plot->y()+roc_plot->h()-5-(int)(5*roc_plot->h()/10.0),10,10,"50");
                label->align(FL_ALIGN_LEFT);
                label->labelcolor(GUI_TEXT_COLOR);
                                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                                   roc_plot->y()+roc_plot->h()-5-(int)(5*roc_plot->h()/10.0),10,10,"@-9line");
                label->align(FL_ALIGN_RIGHT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                                   roc_plot->y()+roc_plot->h()-5-(int)(4*roc_plot->h()/10.0),10,10,"40");
                label->align(FL_ALIGN_LEFT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                                   roc_plot->y()+roc_plot->h()-5-(int)(4*roc_plot->h()/10.0),10,10,"@-9line");
                label->align(FL_ALIGN_RIGHT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                                   roc_plot->y()+roc_plot->h()-5-(int)(3*roc_plot->h()/10.0),10,10,"30");
                label->align(FL_ALIGN_LEFT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);
                
                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                                   roc_plot->y()+roc_plot->h()-5-(int)(3*roc_plot->h()/10.0),10,10,"@-9line");
                label->align(FL_ALIGN_RIGHT);
                label->labelcolor(GUI_TEXT_COLOR);
                    label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                                   roc_plot->y()+roc_plot->h()-5-(int)(2*roc_plot->h()/10.0),10,10,"20");
                label->align(FL_ALIGN_LEFT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                   roc_plot->y()+roc_plot->h()-5-(int)(2*roc_plot->h()/10.0),10,10,"@-9line");
                label->align(FL_ALIGN_RIGHT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                   roc_plot->y()+roc_plot->h()-5-(int)(roc_plot->h()/10.0),10,10,"10");
                label->align(FL_ALIGN_LEFT);
                label->labelfont(FL_HELVETICA);
                label->labelcolor(GUI_TEXT_COLOR);
                
                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                   roc_plot->y()+roc_plot->h()-5-(int)(roc_plot->h()/10.0),10,10,"@-9line");
                label->align(FL_ALIGN_RIGHT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                   roc_plot->y()+roc_plot->h()-5,10,10,"0");
                label->align(FL_ALIGN_LEFT);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                // Draw the labels for the x-axis
                label = new Fl_Box(FL_NO_BOX,roc_plot->x(),rpy+rph-20,
                                   roc_plot->w(),20,"Selectivity (%)");
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5,
                                   roc_plot->y()-10+roc_plot->h(),10,10,"0");
                label->align(FL_ALIGN_BOTTOM);
                label->labelcolor(GUI_TEXT_COLOR);
                    label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"10");
                label->align(FL_ALIGN_BOTTOM);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
                label->align(FL_ALIGN_TOP);
                label->labelcolor(GUI_TEXT_COLOR);
                    label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(2*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"20");
                label->align(FL_ALIGN_BOTTOM);
                label->labelcolor(GUI_TEXT_COLOR);
                                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(2*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
                label->align(FL_ALIGN_TOP);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);
                
                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(3*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"30");
                label->align(FL_ALIGN_BOTTOM);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);
                
                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(3*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
                label->align(FL_ALIGN_TOP);
                label->labelcolor(GUI_TEXT_COLOR);
                    label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(4*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"40");
                label->align(FL_ALIGN_BOTTOM);
                label->labelcolor(GUI_TEXT_COLOR);
                                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(4*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
                label->align(FL_ALIGN_TOP);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(5*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"50");
                label->align(FL_ALIGN_BOTTOM);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(5*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
                label->align(FL_ALIGN_TOP);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(6*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"60");
                label->align(FL_ALIGN_BOTTOM);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);
                
                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(6*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
                label->align(FL_ALIGN_TOP);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(7*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"70");
                label->align(FL_ALIGN_BOTTOM);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);
                
                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(7*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
                label->align(FL_ALIGN_TOP);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(8*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"80");
                label->align(FL_ALIGN_BOTTOM);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA); 

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(8*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
                label->align(FL_ALIGN_TOP);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(9*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"90");
                label->align(FL_ALIGN_BOTTOM);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);
                
                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(9*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
                label->align(FL_ALIGN_TOP);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);

                label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+roc_plot->w()-6,
                                   roc_plot->y()-10+roc_plot->h(),10,10,"100");
                label->align(FL_ALIGN_BOTTOM);
                label->labelcolor(GUI_TEXT_COLOR);
                label->labelfont(FL_HELVETICA);
                
            }
            roc_plot_group->end();
            roc_plot_group->resizable(roc_plot);
            
            leg4_group = new Fl_Group(w-210,30,205,h-40,""); // DOUBLEWINDOW
            {
                int leg4x = leg4_group->x();
                int leg4y = leg4_group->y();
                int leg4w = leg4_group->w();
                int leg4h = leg4_group->h();
                
                Fl_Box* leg_label = new Fl_Box(leg4x+20,leg4y+ 10, leg4w-25, 30, 
                                               "Legend");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                leg_label->labelfont(FL_HELVETICA);

                leg_label = new Fl_Box(leg4x+20,leg4y+ 60, leg4w-25, 30, "Reference Structure");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                leg_label->labelfont(FL_HELVETICA);

                leg4_ref = new Fl_Box(FL_UP_BOX,
                                      leg4x+20, 
                                      leg4y+90,leg4x-25, 20,
                                      "");
                leg4_ref->color(GUI_TEXT_COLOR);
                leg4_ref->labelcolor(GUI_WINDOW_BGCOLOR);
                leg4_ref->labelfont(FL_HELVETICA);
                leg4_ref->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
                leg4_ref->hide();
                leg4_ref_symbol = new Fl_Box(FL_NO_BOX,
                                             leg4x, 
                                             leg4y+90, 20, 20,
                                             "");
                leg4_ref_symbol->labelcolor(GUI_TEXT_COLOR);
                leg4_ref_symbol->labelfont(FL_HELVETICA);
                leg4_ref_symbol->hide();
                
                leg_label = new Fl_Box(leg4x+20,leg4y+ 120,leg4w-25, 30, "Predicted Structures");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                leg_label->labelfont(FL_HELVETICA);
                
                leg4_scroll = new Fl_Scroll(leg4x,leg4y+ 150,leg4w-5, leg4h-200);
                {    
                    leg4_pack = new Fl_Pack(leg4x,leg4y+150,leg4w-5,leg4h-200);
                    leg4_pack->type(Fl_Pack::VERTICAL);
                    leg4_pack->end();
                }
                leg4_scroll->type(Fl_Scroll::VERTICAL);
                //leg4_scroll->box(FL_UP_BOX);
                leg4_scroll->color(GUI_WINDOW_BGCOLOR);
                leg4_scroll->end();
                
            }
            leg4_group->end();
            leg4_group->resizable(leg4_scroll);
            roc_tab->resizable(roc_plot_group);
        }
        roc_tab->end();
        roc_tab->color(GUI_WINDOW_BGCOLOR);
        roc_tab->labelcolor(GUI_BTEXT_COLOR);
                roc_tab->labelfont(FL_HELVETICA);

        table_tab = new Fl_Group(270,20,w-275,h-20,"@line Table");
        {
            // *** Is the text_display_group group necessary? *** //
            
        Fl_Group *text_display_group = new Fl_Group(310,30,w-320,h-80,""); // DOUBLEWINDOW
            {
                int tdx = text_display_group->x();
                int tdy = text_display_group->y();
                int tdw = text_display_group->w();
                int tdh = text_display_group->h();
                
                buff = new Fl_Text_Buffer();
                buff->tab_distance(7);
                text_display = new Fl_Text_Display(tdx+10,tdy+10,tdw-20,tdh-10);
                text_display->buffer(buff);
                text_display->color(GUI_WINDOW_BGCOLOR);
                text_display->textfont(FL_SCREEN);
                text_display->textcolor(GUI_TEXT_COLOR);
                text_display->labelcolor(GUI_TEXT_COLOR);
                text_display->labelfont(FL_SCREEN);
                text_display->labelsize(9);
		text_display_group->resizable(text_display);
            }
            text_display_group->end();
            
            exp_button = new Fl_Button(w-200,h-40,175,30, 
                                       "@filesaveas   Export to CSV @->");
            exp_button->callback(ExportCallback);
            exp_button->value(1);
            exp_button->deactivate();
            
            table_tab->resizable(text_display_group);
        }
        table_tab->end();
        table_tab->color(GUI_WINDOW_BGCOLOR);
        table_tab->labelcolor(GUI_BTEXT_COLOR);
        table_tab->labelfont(FL_HELVETICA);
    
    }
    tab_window->labeltype(FL_NO_LABEL);
    tab_window->labelcolor(Darker(GUI_BTEXT_COLOR, 0.65f));
    tab_window->labelfont(FL_HELVETICA);
    tab_window->end();
    
    this->resizable(tab_window);
    this->end();
    
    title = (char*)malloc(sizeof(char) * 64);
    SetStructures(structures);
}

StatsWindow::StatsWindow(int w, int h, const char *label, 
                         const std::vector<int>& structures) : 
	Fl_Window(w, h, label), statsFormulasImage(NULL), statsFormulasBox(NULL), 
	input_window(NULL), rocplot_rotated_text(NULL) {
    Construct(w, h, structures);
}

StatsWindow::StatsWindow(int x, int y, int w, int h, const char *label, 
                         const std::vector<int>& structures) : 
	Fl_Window(x, y, w, h, label), statsFormulasImage(NULL), statsFormulasBox(NULL), 
	input_window(NULL) {
    Construct(w, h, structures);
}

StatsWindow::~StatsWindow()
{
    Free(title); 
    Free(statistics);
    if(statsFormulasImage != NULL) {
             delete statsFormulasImage;
    }
    if(statsFormulasBox != NULL) {
         delete statsFormulasBox;
    }
    Delete(input_window, InputWindow);
    Delete(rocplot_rotated_text, Fl_Rotated_Text);
    Delete(buff, Fl_Text_Buffer);
}

void StatsWindow::ResetWindow()
{
    this->size(DEFAULT_STATSWIN_WIDTH, DEFAULT_STATSWIN_HEIGHT);
    
    ClearStats();
    
    bp_chart->clear();
    tp_chart->clear();
    fp_chart->clear();
    fn_chart->clear();
    sens_chart->clear();
    sel_chart->clear();
    ppv_chart->clear();
    gc_chart->clear();
    au_chart->clear();
    gu_chart->clear();
    non_canon_chart->clear();
    
    roc_plot->clear();
    
    leg1_ref->hide();
    leg1_pack->clear();
    leg2_ref->hide();
    leg2_pack->clear();
    leg3_ref->hide();
    leg3_pack->clear();
    leg4_ref->hide();
    leg4_ref_symbol->hide();
    leg4_pack->clear();
    
    buff->remove(0, buff->length());
    exp_button->value(1);
    exp_button->deactivate();
    
    tab_window->value(overview_tab);
}

void StatsWindow::SetFolderIndex(int index)
{
    folderIndex = index;
    sprintf(title, "Statistics for Sequence: %-.48s", structureManager->GetFolderAt(index)->folderName);
    label(title);
}

int StatsWindow::GetFolderIndex() const {
     return folderIndex;
}

void StatsWindow::SetStructures(const std::vector<int>& structures)
{
    /* Refer to SetStructures in FolderWindow.cpp and DiagramWindow.cpp*/
    m_structures.clear();
    for (unsigned int ui = 0; ui < structures.size(); ++ui)
    {
        m_structures.push_back(structures[ui]);
    }
    
    structureManager = RNAStructViz::GetInstance()->GetStructureManager();
    
    BuildRefMenu();
    BuildCompMenu();
    menu_window->redraw();
}

const std::vector<int>& StatsWindow::GetStructures()
{
    return m_structures;
}

void StatsWindow::AddStructure(const int index)
{
    /* Refer to AddStructure in FolderWindow.cpp */
    if (std::find(m_structures.begin(), m_structures.end(), index) == 
        m_structures.end())
    {
        m_structures.push_back(index);
        
        structureManager = RNAStructViz::GetInstance()->GetStructureManager();
        
        BuildRefMenu();
        BuildCompMenu();
        menu_window->redraw();
    }
}

void StatsWindow::RemoveStructure(const int index)
{
    /* Refer to RemoveStructure in DiagramWindow.cpp*/
    std::vector<int>::iterator iter = std::find(m_structures.begin(), 
                                                m_structures.end(), index);
    
    if (iter != m_structures.end())
    {
        m_structures.erase(iter);
        
        structureManager = RNAStructViz::GetInstance()->GetStructureManager();
        
        BuildRefMenu();
        BuildCompMenu();
        menu_window->redraw();
    }
}

void StatsWindow::ReferenceCallback(Fl_Widget* widget, void* userData)
{
    
    StatsWindow* window = (StatsWindow*)widget->parent()->parent();
    if(strncmp(window->ref_menu->mvalue()->label(),
              "Please select a reference", 25) && window->GetCheckedStructuresCount() > 0)
    {
        window->calc_button->value(0);
        window->calc_button->activate();
    }
    else
    {
        window->calc_button->value(1);
        window->calc_button->deactivate();
    }
    
    const std::vector<int>& m_structures = window->GetStructures();
    for (unsigned int ui = 0; ui < m_structures.size(); ++ui)
    {
        RNAStructure* structure = 
        window->structureManager->GetStructure(m_structures[ui]);
        if (!strcmp(structure->GetFilename(), 
                    window->ref_menu->mvalue()->label())) 
        {
            window->SetReferenceStructure(ui);
        }
    }
    for (int i=0; i < window->comp_pack->children(); i++)
    {
        Fl_Check_Button* button = (Fl_Check_Button *) window->comp_pack->child(i);
        button->box(FL_PLASTIC_UP_BOX);
	button->down_box(FL_PLASTIC_DOWN_BOX);
	button->color(GUI_WINDOW_BGCOLOR);
        button->labelcolor(GUI_TEXT_COLOR);
        button->labelfont(FL_HELVETICA);
        button->selection_color(GUI_WINDOW_BGCOLOR);    
        if (!strcmp(button->label(), window->ref_menu->mvalue()->label()))
        {
            button->value(1);
            button->deactivate();
        }
        else
        {
             //button->value(1);
             button->activate();
        }
	if(button->value()) {
	     char tempBtnLabel[MAX_BUFFER_SIZE];
	     strcpy(tempBtnLabel, button->label());
	     button->label("");
	     button->redraw();
	     button->labelfont(FL_HELVETICA_BOLD_ITALIC);
	     button->copy_label(tempBtnLabel);
	}
	button->redraw();
    }
    
}

void StatsWindow::MenuCallback(Fl_Widget* widget, void* userData)
{
    StatsWindow* window = (StatsWindow *) widget->parent()->parent()->parent()->parent();
    if(!strncmp(window->ref_menu->mvalue()->label(),
                "Please select a reference", 25) || window->GetCheckedStructuresCount() == 0)
    {
        window->calc_button->value(1);
        window->calc_button->deactivate();
    }
    else if(window->GetCheckedStructuresCount() > 0) 
    {
	window->calc_button->value(0);
        window->calc_button->activate();
    }
    Fl_Check_Button *cb = (Fl_Check_Button *) widget;
    if(cb->value() && strcmp(cb->label(), window->ref_menu->mvalue()->label())) {
         char tempBtnLabel[MAX_BUFFER_SIZE];
	 strcpy(tempBtnLabel, cb->label());
	 cb->label("");
	 cb->redraw();
	 cb->labelfont(FL_HELVETICA_BOLD_ITALIC);
	 cb->copy_label(tempBtnLabel);
	 //cb->activate();
    }
    else {
	 char tempBtnLabel[MAX_BUFFER_SIZE];
	 strcpy(tempBtnLabel, cb->label());
	 cb->label("");
	 cb->redraw();
	 cb->labelfont(FL_HELVETICA);
         cb->copy_label(tempBtnLabel);
    }
    cb->redraw();

}

void StatsWindow::CalcCallback(Fl_Widget* widget, void* userData)
{    
    StatsWindow* window = (StatsWindow*)widget->parent()->parent();
    Fl_Toggle_Button *cb = (Fl_Toggle_Button*) widget;
    if(cb->value() == 1 && window->GetCheckedStructuresCount() > 0)
    {
        if(strncmp(window->ref_menu->mvalue()->label(),
                  "Please select a reference", 25))
        {
            for (int i=0; i < window->comp_pack->children(); i++)
            {
                //Fl_Check_Button* button = 
                //        (Fl_Check_Button*) window->comp_pack->child(i);
		//button->value(0);
		//button->labelfont(FL_HELVETICA);
            }
            
            
        }
        window->ComputeStats();
    }
    else
    {
        //((Fl_Toggle_Button*)widget)->value(1);
    }
    ((Fl_Check_Button*) widget)->deactivate();
    Fl_Window *parentWin = widget->parent()->as_window();
    if(parentWin != NULL) {
         StatsWindow *statsWin = (StatsWindow *) parentWin;
	 statsWin->selectAllCompsBtn->deactivate();
    }
}

void StatsWindow::ExportCallback(Fl_Widget* widget, void* userData)
{
    if (((Fl_Toggle_Button*)widget)->value() == 0)
    {
        StatsWindow* window = (StatsWindow*)widget->parent()->parent()->parent();
        window->ExportTable();
    }
}

void StatsWindow::SelectAllButtonCallback(Fl_Widget *saBtn, void *udata) { 
    Fl_Widget *curParentWin = (Fl_Widget *) saBtn->parent();
    while(curParentWin && !curParentWin->as_window()) {
        curParentWin = curParentWin->parent();
    }
    StatsWindow *window = (StatsWindow *) curParentWin; 
    bool noRefMsgDisplayed = true;
    for(int cbidx = 0; cbidx < window->comp_pack->children(); cbidx++) { 
        Fl_Check_Button *cb = (Fl_Check_Button *) window->comp_pack->child(cbidx); 
        if(cbidx != window->referenceIndex && strncmp(window->ref_menu->mvalue()->label(),
                                                      "Please select a reference", 25)) { 
            cb->value(1);
	    //cb->activate();
	    char tempBtnLabel[MAX_BUFFER_SIZE];
	    strcpy(tempBtnLabel, cb->label());
	    cb->label("");
	    cb->redraw();
	    cb->labelfont(FL_HELVETICA_BOLD_ITALIC);
	    cb->copy_label(tempBtnLabel);
        }
        else {
	    cb->value(1);
            //cb->deactivate();
	    char tempBtnLabel[MAX_BUFFER_SIZE];
	    strcpy(tempBtnLabel, cb->label());
	    cb->label("");
	    cb->redraw();
	    cb->labelfont(FL_HELVETICA);
	    cb->copy_label(tempBtnLabel);
	    if(cbidx != window->referenceIndex && noRefMsgDisplayed) {
	         fl_alert("Please select a reference structure first.");
		 noRefMsgDisplayed = false;
	    }
	}
        cb->redraw();	
    }
    if(window->GetCheckedStructuresCount() > 0) {
         window->calc_button->value(0);
	 window->calc_button->activate();
    }
} 

void StatsWindow::SetReferenceStructure(const int index)
{
    if (index != referenceIndex)
    {
        referenceIndex = index;
    }
}

void StatsWindow::resize(int x, int y, int w, int h)
{
    Fl_Window::resize(x,y,w,h);
    
    // Check whether any boxes get outsized, change the text
    if (ppv_chart->w() < 160) {
        ppv_chart->label("Pos. Pred. Value");
    } else {
        ppv_chart->label("Positive Predictive Value");
    }
    if (non_canon_chart->w() < 160) {
        non_canon_chart->label("Non-Canoncial BPs");
    } else {
        non_canon_chart->label("Non-Canonical Base Pairs");
    }
    
}

void StatsWindow::hide() {
    Fl_Window::hide();
}

void StatsWindow::ClearStats()
{
    if (statistics != NULL) 
    {
        Free(statistics);
        statistics = NULL;
    }
    numStats = 0;
}

void StatsWindow::ComputeStats()
{
    ClearStats();
    
    buff->remove(0, buff->length());    
    
    for (int i=0; i < comp_pack->children(); i++)
    {
        Fl_Check_Button* button = (Fl_Check_Button *) comp_pack->child(i);
        if (button->value() == 1)
        {
            numStats++;        
        }
    }
    
    statistics = (StatData *) malloc(comp_pack->children() * sizeof(StatData));
    memset(statistics, 0x00, comp_pack->children() * sizeof(StatData));

    // Find the reference structure
    RNAStructure* reference = 
                      structureManager->GetStructure(m_structures[referenceIndex]);
    SetReferenceStructure(referenceIndex);
    
    // Compute the number of base pairs in the reference structure
    unsigned int ref_base_pair_count = 0;
    for (unsigned int i=0; i < reference->GetLength(); i++)
    {
        if (reference->GetBaseAt(i)->m_pair != RNAStructure::UNPAIRED && 
            reference->GetBaseAt(i)->m_pair > i)
        {
            ref_base_pair_count++;
        }
    }
    
    // Compute statistics for selected structures
    int statsIndex;
    int counter = 1; 
    // Use a different counter so statsIndex will be 0 if it's the reference and counter otherwise
    for (int i = comp_pack->children() - 1; i >= 0; i--)
    {
        Fl_Check_Button* button = (Fl_Check_Button *) comp_pack->child(i);
	if(button->value() == 1)
        {
            // Find the corresponding structure
            // index corresponds to index of checkbox
            RNAStructure* predicted = structureManager->GetStructure(m_structures[i]);
            
            // Initialize values
            if (strcmp(predicted->GetFilename(), reference->GetFilename()))
            {
                statsIndex = counter;
                statistics[statsIndex].ref = false;
            }
            else
            {
                statsIndex = 0;
                statistics[statsIndex].ref = true;
            }
            statistics[statsIndex].isValid = true;
	    statistics[statsIndex].filename = predicted->GetFilenameNoExtension();
            statistics[statsIndex].base_pair_count = 0;
            statistics[statsIndex].gc_count = 0;
            statistics[statsIndex].au_count = 0;
            statistics[statsIndex].gu_count = 0;
            statistics[statsIndex].non_canon_count = 0;
            statistics[statsIndex].true_pos_count = 0;
            statistics[statsIndex].false_neg_count = 0;
            statistics[statsIndex].false_pos_count = 0;
            statistics[statsIndex].conflict_count = 0;
            statistics[statsIndex].contradict_count = 0;
            statistics[statsIndex].compatible_count = 0;
            statistics[statsIndex].sensitivity = 0;
            statistics[statsIndex].selectivity = 0;
            statistics[statsIndex].pos_pred_value = 0;
            

            // Compute counts
            // Go through reference strand, comparing to predicted
            for (unsigned int uj = 0; uj < reference->GetLength(); uj++)
            {
                // Increment if there is a base pair in predicted
                if(predicted->GetBaseAt(uj)->m_pair != RNAStructure::UNPAIRED &&
                                   predicted->GetBaseAt(uj)->m_pair > uj)
                {
                    statistics[statsIndex].base_pair_count++;
                    
                    RNAStructure::Base base1 = predicted->GetBaseAt(uj)->m_base;
                    RNAStructure::Base base2 = predicted->GetBaseAt(predicted->GetBaseAt(uj)->m_pair)->m_base;
                    
                    if ((base1 == RNAStructure::G && base2 == RNAStructure::C) 
                        || (base1 == RNAStructure::C && base2 == RNAStructure::G))
                    {
                        statistics[statsIndex].gc_count++;
                    }
                    else if ((base1 == RNAStructure::A && base2 == RNAStructure::U) 
                             || (base1 == RNAStructure::U && base2 == RNAStructure::A))
                    {
                        statistics[statsIndex].au_count++;
                    }
                    else if ((base1 == RNAStructure::G && base2 == RNAStructure::U) 
                             || (base1 == RNAStructure::U && base2 == RNAStructure::G))
                    {
                        statistics[statsIndex].gu_count++;
                    }
                    else
                    {
                        statistics[statsIndex].non_canon_count++;
                    }
                }
                
                // If the base pair in reference & predicted match, increment TP
                if(reference->GetBaseAt(uj)->m_pair == predicted->GetBaseAt(uj)->m_pair
                   && reference->GetBaseAt(uj)->m_pair != RNAStructure::UNPAIRED
                   && reference->GetBaseAt(uj)->m_pair > uj)
                {
                    statistics[statsIndex].true_pos_count++;
                }
                // Else, look for false positives
                else if(reference->GetBaseAt(uj)->m_pair != predicted->GetBaseAt(uj)->m_pair
                        && predicted->GetBaseAt(uj)->m_pair != RNAStructure::UNPAIRED
                        && predicted->GetBaseAt(uj)->m_pair > uj)
                {

                    
                    // Remember the current predicted base pair
                    unsigned int prime_5 = uj;
                    unsigned int prime_3 = predicted->GetBaseAt(uj)->m_pair;
                    

                    
                    // Look for contradicting base pairs: a different pair at that index
                    if (reference->GetBaseAt(prime_3)->m_pair != RNAStructure::UNPAIRED
                        || reference->GetBaseAt(prime_5)->m_pair != RNAStructure::UNPAIRED)
                    {
                        statistics[statsIndex].contradict_count++;

                    }
                    // Look for conflicting base pairs: a pairing with one base inside and one outside the loop
                    else
                    {
                        unsigned int k = prime_5; // loop through the remaining bases
                        bool b = false; // whether you've found a conflicting base pair
                        while(!b && k < prime_3)
                        {
                            // Conflicting if the reference has a base pair 
                            if(reference->GetBaseAt(k)->m_pair != RNAStructure::UNPAIRED
                               && (reference->GetBaseAt(k)->m_pair < prime_5 
                                   || reference->GetBaseAt(k)->m_pair > prime_3))
                            {
                                b = true;
                                statistics[statsIndex].conflict_count++;
;
                            }
                            k++;
                        }
                        
                    }
                }
            }
            
            // Compute aggregates
            statistics[statsIndex].false_pos_count =
                        statistics[statsIndex].base_pair_count -
                        statistics[statsIndex].true_pos_count;
            statistics[statsIndex].false_neg_count = 
                                               ref_base_pair_count - statistics[statsIndex].true_pos_count;
            statistics[statsIndex].compatible_count = 
                                               statistics[statsIndex].false_pos_count -
                                               statistics[statsIndex].conflict_count - 
                                               statistics[statsIndex].contradict_count;
            
            // Compute statistics
            if (ref_base_pair_count > 0)
            {
                statistics[statsIndex].sensitivity = 
                (float)statistics[statsIndex].true_pos_count /
                ((float)statistics[statsIndex].true_pos_count + 
                 (float)statistics[statsIndex].false_neg_count);
            }
            if (statistics[statsIndex].base_pair_count > 0)
            {
            statistics[statsIndex].selectivity = 
                (float)statistics[statsIndex].true_pos_count /
                ((float)statistics[statsIndex].true_pos_count + 
                 (float)statistics[statsIndex].false_pos_count -
                 (float)statistics[statsIndex].compatible_count);
                statistics[statsIndex].pos_pred_value = 
                (float)statistics[statsIndex].true_pos_count /
                ((float)statistics[statsIndex].true_pos_count + 
                 (float)statistics[statsIndex].false_pos_count);
            }
            
            // Increment which statistics struct is being accessed
            if (statistics[statsIndex].ref == false) {
                counter++;
            }
        }
    }
    
    buff->append("Reference structure: ");
    buff->append(reference->GetFilenameNoExtension());
    buff->append("\n\n");
    buff->append("Comparison structures:\n\n");
    buff->append(
                     "Filename\t\t\t\tPairs\tTPs\tFPs\tFNs\tSensi.\tSelec.\tPPV\tConfl.\tContr.\tCompa.\tG-C\tA-U\tG-U\tOther\n" 
                );
    
    unsigned int activeStatsCount = 0;
    for (unsigned int ui=0; ui < comp_pack->children(); ui++)
    {
        
        if(!statistics[ui].isValid) {
	     continue;
	}
	char tempc[38];
        strncpy(tempc, statistics[ui].filename, 36);
        tempc[37] = '\0';
        if (strlen(statistics[ui].filename) >= 36) 
        {
            strcpy(tempc + 31, "... ");
	    buff->append(tempc);
            buff->append("\t");
        }
        else
        {
            buff->append(tempc);
            buff->append("\t\t");
            if (strlen(tempc) < 21)
            {
                buff->append("\t");
            }
            if (strlen(tempc) < 14)
            {
                buff->append("\t");
            }
            if (strlen(tempc) < 7)
            {
                buff->append("\t");
            }
        }
        sprintf(statistics[ui].bp_char,"%d",statistics[ui].base_pair_count);
        sprintf(tempc,"%d\t",statistics[ui].base_pair_count);
        buff->append(tempc);
        sprintf(statistics[ui].tp_char,"%d",statistics[ui].true_pos_count);
        sprintf(tempc,"%d\t",statistics[ui].true_pos_count);
        buff->append(tempc);
        sprintf(statistics[ui].fp_char,"%d",statistics[ui].false_pos_count);
        sprintf(tempc,"%d\t",statistics[ui].false_pos_count);
        buff->append(tempc);
        sprintf(statistics[ui].fn_char,"%d",statistics[ui].false_neg_count);
        sprintf(tempc,"%d\t",statistics[ui].false_neg_count);
        buff->append(tempc);
        sprintf(statistics[ui].sens_char,"%.3f",statistics[ui].sensitivity);
        sprintf(tempc,"%.4f\t",statistics[ui].sensitivity);
        buff->append(tempc);
        sprintf(statistics[ui].sel_char,"%.3f",statistics[ui].selectivity);
        sprintf(tempc,"%.4f\t",statistics[ui].selectivity);
        buff->append(tempc);
        sprintf(statistics[ui].ppv_char,"%.3f",statistics[ui].pos_pred_value);
        sprintf(tempc,"%.4f\t",statistics[ui].pos_pred_value);
        buff->append(tempc);
        sprintf(statistics[ui].conf_char,"%d",statistics[ui].conflict_count);
        sprintf(tempc,"%d\t",statistics[ui].conflict_count);
        buff->append(tempc);
        sprintf(statistics[ui].cont_char,"%d",statistics[ui].contradict_count);
        sprintf(tempc,"%d\t",statistics[ui].contradict_count);
        buff->append(tempc);
        sprintf(statistics[ui].comp_char,"%d",statistics[ui].compatible_count);
        sprintf(tempc,"%d\t",statistics[ui].compatible_count);
        buff->append(tempc);
        sprintf(statistics[ui].gc_char,"%d",statistics[ui].gc_count);
        sprintf(tempc,"%d\t",statistics[ui].gc_count);
        buff->append(tempc);
        sprintf(statistics[ui].au_char,"%d",statistics[ui].au_count);
        sprintf(tempc,"%d\t",statistics[ui].au_count);
        buff->append(tempc);
        sprintf(statistics[ui].gu_char,"%d",statistics[ui].gu_count);
        sprintf(tempc,"%d\t",statistics[ui].gu_count);
        buff->append(tempc);
        sprintf(statistics[ui].nc_char,"%d",statistics[ui].non_canon_count);
        if (activeStatsCount++ == 0) {
            sprintf(tempc,"%d",statistics[ui].non_canon_count);
        }
        else {
            sprintf(tempc,"%d\n",statistics[ui].non_canon_count);
        }
        buff->append(tempc);
    }
    
    int colors[7] = {
        Lighter(RGBColor(32, 74, 135), 0.61f), // FL_BLUE, 
        Lighter(RGBColor(115, 210, 22), 0.61f),  // FL_GREEN, 
        Lighter(RGBColor(164, 0, 0), 0.61f), // FL_RED, 
        Lighter(RGBColor(196, 160, 0), 0.61f), // FL_YELLOW, 
        Lighter(RGBColor(0, 195, 215), 0.61f), // FL_CYAN, 
        Lighter(RGBColor(239, 41, 159), 0.61f), // FL_MAGENTA,         
        Lighter(RGBColor(0xaa, 0xaa, 0xaa), 0.61f),
    };
    
    int haveSeenRefStruct = 0;
    for (unsigned int ui = 0; ui < comp_pack->children(); ui++)
    {
	if(!statistics[ui].isValid) continue;
        if (statistics[ui].ref) 
        {
            statistics[ui].color = GUI_TEXT_COLOR;
	    haveSeenRefStruct = 1;
        }
        else
        {
            float darkerPct = 1.0 - 0.25 * ((ui - haveSeenRefStruct) / 7);
            statistics[ui].color = Darker(colors[(ui - haveSeenRefStruct) % 7], darkerPct);
        }
    }
    
    exp_button->value(0);
    exp_button->activate();
    
    DrawHistograms();
    DrawRoc();
    DrawLegend();
    
    redraw();
}

void StatsWindow::DrawHistograms()
{
    bp_chart->clear();
    tp_chart->clear();
    fp_chart->clear();
    fn_chart->clear();
    
    sens_chart->clear();
    sel_chart->clear();
    ppv_chart->clear();
    
    gc_chart->clear();
    au_chart->clear();
    gu_chart->clear();
    non_canon_chart->clear();
    
    unsigned int bound = 0;
    unsigned int activeBarsCount = 0;
    for (unsigned int ui = 0; ui < comp_pack->children(); ui++)
    {
        if(!statistics[ui].isValid) continue;
	if (statistics[ui].base_pair_count > bound)
        {
            bound = statistics[ui].base_pair_count;
        }
    }
    
    bp_chart->begin();
    {        
        int cbx = bp_chart->x();
        int cby = bp_chart->y();
        int cbw = bp_chart->w();
        int cbh = bp_chart->h();
        
        // Set the width of each column
        // Divide the chart width by the number of columns to display
        int col_w = cbw/numStats;
        
        // Set the scale for the heights of columns
        // Divide the total height of the chart (with space for column labels)
        // by the largest base pair count
        double hscale = (cbh - 20.0)/bound;
        
        for (unsigned int ui = 0; ui < comp_pack->children(); ui++)
        {
            if(!statistics[ui].isValid) continue;
            Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx+ activeBarsCount++ * col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].base_pair_count),
                                     col_w,
                                     (int)(hscale*statistics[ui].base_pair_count),statistics[ui].bp_char);
            box->align(FL_ALIGN_TOP);
            box->color(statistics[ui].color);
        }
        
    }
    bp_chart->end();
    
    tp_chart->begin();
    {
        
        int cbx = tp_chart->x();
        int cby = tp_chart->y();
        int cbw = tp_chart->w();
        int cbh = tp_chart->h();
        
        // Set the width of each column
        // Divide the chart width by the number of columns to display
        int col_w = cbw/numStats;
        
        // Set the scale for the heights of columns
        // Divide the total height of the chart (with space for column labels)
        // by the largest base pair count
        double hscale = (cbh - 20.0)/bound;
        
	activeBarsCount = 0;
        for (unsigned int ui = 0; ui < comp_pack->children(); ui++)
        {
	    if(!statistics[ui].isValid) continue;
            Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx+ activeBarsCount++ * col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].true_pos_count),
                                     col_w,
                                     (int)(hscale*statistics[ui].true_pos_count), statistics[ui].tp_char);
            box->color(statistics[ui].color);
            box->align(FL_ALIGN_TOP);
        }
        
    }
    tp_chart->end();
    
    fp_chart->begin();
    {
        
        int cbx = fp_chart->x();
        int cby = fp_chart->y();
        int cbw = fp_chart->w();
        int cbh = fp_chart->h();
        
        // Set the width of each column
        // Divide the chart width by the number of columns to display
        int col_w = cbw/numStats;
        
        // Set the scale for the heights of columns
        // Divide the total height of the chart (with space for column labels)
        // by the largest base pair count
        double hscale = (cbh - 20.0)/bound;
        
	activeBarsCount = 0;
        for (unsigned int ui = 0; ui < comp_pack->children(); ui++)
        {
	    if(!statistics[ui].isValid) continue;
            Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx + activeBarsCount++ * col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].false_pos_count),
                                     col_w,
                                     (int)(hscale*statistics[ui].false_pos_count),statistics[ui].fp_char);
            box->color(statistics[ui].color);
            box->align(FL_ALIGN_TOP);
        }
        
    }
    fp_chart->end();
    
    fn_chart->begin();
    {
        
        int cbx = fn_chart->x();
        int cby = fn_chart->y();
        int cbw = fn_chart->w();
        int cbh = fn_chart->h();
        
        // Set the width of each column
        // Divide the chart width by the number of columns to display
        int col_w = cbw/numStats;
        
        // Set the scale for the heights of columns
        // Divide the total height of the chart (with space for column labels)
        // by the largest base pair count
        double hscale = (cbh - 20.0)/bound;
        
	activeBarsCount = 0;
        for (unsigned int ui = 0; ui < comp_pack->children(); ui++)
        {
	    if(!statistics[ui].isValid) continue;
            Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx + activeBarsCount++ * col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].false_neg_count),
                                     col_w,
                                     (int)(hscale*statistics[ui].false_neg_count),statistics[ui].fn_char);
            box->color(statistics[ui].color);
            box->align(FL_ALIGN_TOP);
        }
        
    }
    fn_chart->end();
    
    sens_chart->begin();
    {
        
        int cbx = sens_chart->x();
        int cby = sens_chart->y();
        int cbw = sens_chart->w();
        int cbh = sens_chart->h();
        
        // Set the width of each column
        // Divide the chart width by the number of columns to display
        int col_w = cbw/numStats;
        
        // Set the scale for the heights of columns
        // Divide the total height of the chart (with space for column labels)
        // by the largest base pair count
        double hscale = cbh - 20.0;
        
	activeBarsCount = 0;
        for (unsigned int ui = 0; ui < comp_pack->children(); ui++)
        {
	    if(!statistics[ui].isValid) continue;
            Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx + activeBarsCount++ * col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].sensitivity),
                                     col_w,
                                     (int)(hscale*statistics[ui].sensitivity),statistics[ui].sens_char);
            box->color(statistics[ui].color);
            box->labelsize(MAX(MIN(16 / numStats * 6, 6), 3));
	    box->labelfont(FL_HELVETICA);
            box->align(FL_ALIGN_TOP);
            box->copy_tooltip((const char *) statistics[ui].sens_char);

	}
        
    }
    sens_chart->end();
    
    sel_chart->begin();
    {
        
        int cbx = sel_chart->x();
        int cby = sel_chart->y();
        int cbw = sel_chart->w();
        int cbh = sel_chart->h();
        
        // Set the width of each column
        // Divide the chart width by the number of columns to display
        int col_w = cbw/numStats;
        
        // Set the scale for the heights of columns
        // Divide the total height of the chart (with space for column labels)
        // by the largest base pair count
        double hscale = cbh - 20.0;
        
	activeBarsCount = 0;
        for (unsigned int ui = 0; ui < comp_pack->children(); ui++)
        {
	    if(!statistics[ui].isValid) continue;
            Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx + activeBarsCount++ * col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].selectivity),
                                     col_w,
                                     (int)(hscale*statistics[ui].selectivity),statistics[ui].sel_char);
            box->color(statistics[ui].color);
            box->labelsize(MAX(MIN(16 / numStats * 6, 6), 3));
	    box->labelfont(FL_HELVETICA);
	    box->align(FL_ALIGN_TOP);
	    box->copy_tooltip((const char *) statistics[ui].sel_char);
        }
        
    }
    sel_chart->end();
    
    ppv_chart->begin();
    {
        
        int cbx = ppv_chart->x();
        int cby = ppv_chart->y();
        int cbw = ppv_chart->w();
        int cbh = ppv_chart->h();
        
        // Set the width of each column
        // Divide the chart width by the number of columns to display
        int col_w = cbw/numStats;
        
        // Set the scale for the heights of columns
        // Divide the total height of the chart (with space for column labels)
        // by the largest base pair count
        double hscale = cbh - 20.0;
        
	activeBarsCount = 0;
        for (unsigned int ui = 0; ui < numStats; ui++)
        {
	    if(!statistics[ui].isValid) continue;
            Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx + activeBarsCount++ * col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].pos_pred_value),
                                     col_w,
                                     (int)(hscale*statistics[ui].pos_pred_value),statistics[ui].ppv_char);
            box->labelsize(MAX(MIN(16 / numStats * 6, 6), 3));
	    box->labelfont(FL_HELVETICA);
	    box->color(statistics[ui].color);
            box->align(FL_ALIGN_TOP);
            box->copy_tooltip((const char *) statistics[ui].ppv_char);
	}
        
    }
    ppv_chart->end();
    
    gc_chart->begin();
    {        
        int cbx = gc_chart->x();
        int cby = gc_chart->y();
        int cbw = gc_chart->w();
        int cbh = gc_chart->h();
        
        // Set the width of each column
        // Divide the chart width by the number of columns to display
        int col_w = cbw/numStats;
        
        // Set the scale for the heights of columns
        // Divide the total height of the chart (with space for column labels)
        // by the largest base pair count
        double hscale = (cbh - 20.0)/bound;
        
	activeBarsCount = 0;
        for (unsigned int ui = 0; ui < comp_pack->children(); ui++)
        {
	    if(!statistics[ui].isValid) continue;
            Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx + activeBarsCount * col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].base_pair_count),
                                     col_w,
                                     (int)(hscale*statistics[ui].base_pair_count),statistics[ui].bp_char);
            box->color();
            box->align(FL_ALIGN_TOP);
            
            box = new Fl_Box(FL_BORDER_BOX,cbx + activeBarsCount++ * col_w,
                             cbh + cby - (int)(hscale * statistics[ui].gc_count),
                             col_w,
                             (int)(hscale*statistics[ui].gc_count),statistics[ui].gc_char);
            box->color(statistics[ui].color);
            box->align(FL_ALIGN_TOP);
        }
        
    }
    gc_chart->end();
    
    au_chart->begin();
    {        
        int cbx = au_chart->x();
        int cby = au_chart->y();
        int cbw = au_chart->w();
        int cbh = au_chart->h();
        
        // Set the width of each column
        // Divide the chart width by the number of columns to display
        int col_w = cbw/numStats;
        
        // Set the scale for the heights of columns
        // Divide the total height of the chart (with space for column labels)
        // by the largest base pair count
        double hscale = (cbh - 20.0)/bound;
        
	activeBarsCount = 0;
        for (unsigned int ui = 0; ui < comp_pack->children(); ui++)
        {
	    if(!statistics[ui].isValid) continue;
            Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx + activeBarsCount * col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].base_pair_count),
                                     col_w,
                                     (int)(hscale*statistics[ui].base_pair_count),statistics[ui].bp_char);
            box->color(FL_BACKGROUND_COLOR);
            box->align(FL_ALIGN_TOP);
            
            box = new Fl_Box(FL_BORDER_BOX,cbx + activeBarsCount++ * col_w,
                             cbh + cby - (int)(hscale * statistics[ui].au_count),
                             col_w,
                             (int)(hscale*statistics[ui].au_count),statistics[ui].au_char);
            box->color(statistics[ui].color);
            box->align(FL_ALIGN_TOP);
        }
        
    }
    au_chart->end();
    
    gu_chart->begin();
    {        
        int cbx = gu_chart->x();
        int cby = gu_chart->y();
        int cbw = gu_chart->w();
        int cbh = gu_chart->h();
        
        // Set the width of each column
        // Divide the chart width by the number of columns to display
        int col_w = cbw/numStats;
        
        // Set the scale for the heights of columns
        // Divide the total height of the chart (with space for column labels)
        // by the largest base pair count
        double hscale = (cbh - 20.0)/bound;
        
	activeBarsCount = 0;
        for (unsigned int ui = 0; ui < numStats; ui++)
        {
	    if(!statistics[ui].isValid) continue;
            Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx + activeBarsCount * col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].base_pair_count),
                                     col_w,
                                     (int)(hscale*statistics[ui].base_pair_count),statistics[ui].bp_char);
            box->color(FL_BACKGROUND_COLOR);
            box->align(FL_ALIGN_TOP);
            
            box = new Fl_Box(FL_BORDER_BOX,cbx + activeBarsCount++ * col_w,
                             cbh + cby - (int)(hscale * statistics[ui].gu_count),
                             col_w,
                             (int)(hscale*statistics[ui].gu_count),statistics[ui].gu_char);
            box->color(statistics[ui].color);
            box->align(FL_ALIGN_TOP);
        }
        
    }
    gu_chart->end();
    
    non_canon_chart->begin();
    {        
        int cbx = non_canon_chart->x();
        int cby = non_canon_chart->y();
        int cbw = non_canon_chart->w();
        int cbh = non_canon_chart->h();
        
        // Set the width of each column
        // Divide the chart width by the number of columns to display
        int col_w = cbw/numStats;
        
        // Set the scale for the heights of columns
        // Divide the total height of the chart (with space for column labels)
        // by the largest base pair count
        double hscale = (cbh - 20.0)/bound;
        
	activeBarsCount = 0;
        for (unsigned int ui = 0; ui < comp_pack->children(); ui++)
        {
	    if(!statistics[ui].isValid) continue;
            Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx + activeBarsCount * col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].base_pair_count),
                                     col_w,
                                     (int)(hscale*statistics[ui].base_pair_count),statistics[ui].bp_char);
            box->color(FL_BACKGROUND_COLOR);
            box->align(FL_ALIGN_TOP);
            
            box = new Fl_Box(FL_BORDER_BOX,cbx + activeBarsCount++ * col_w,
                             cbh + cby - (int)(hscale * statistics[ui].non_canon_count),
                             col_w,
                             (int)(hscale*statistics[ui].non_canon_count),statistics[ui].nc_char);
            box->color(statistics[ui].color);
            box->align(FL_ALIGN_TOP);
        }
        
    }
    gc_chart->end();
    
    bp_chart->redraw();
    tp_chart->redraw();
    fp_chart->redraw();
    fn_chart->redraw();
    sens_chart->redraw();
    sel_chart->redraw();
    ppv_chart->redraw();
    gc_chart->redraw();
    au_chart->redraw();
    gu_chart->redraw();
    non_canon_chart->redraw();
}

void StatsWindow::DrawRoc()
{
    roc_plot->clear();
    roc_plot->begin();
    {
        int rpx = roc_plot->x();
        int rpy = roc_plot->y();
        int rpw = roc_plot->w();
        int rph = roc_plot->h();
        
        for (unsigned int ui=0; ui < comp_pack->children(); ui++)
        {
	    if(!statistics[ui].isValid) continue;
            Fl_Box* box = new Fl_Box(FL_NO_BOX,
                                     (int)(rpw * statistics[ui].selectivity)+rpx-11,
                                     (int)(rph * (1-statistics[ui].sensitivity))+rpy-10,
                                     20,
                                     20,
                                     "");
            if (ui%28 >= 0 && ui%28 <7)
            {
                box->label("@circle");
            } 
            else if (ui%28 >= 7 && ui%28 <14)
            {
                box->label("@square");
            } 
            else if (ui%28 >= 14 && ui%28 <21)
            {
                box->label("@+98>"); // Triangle pointed upwards
            }
            else
            {
                box->label("@+");
            }
            box->labelcolor(statistics[ui].color);
        }    
    }
    roc_plot->end();
    roc_plot->parent()->redraw();
}

void StatsWindow::DrawLegend()
{
    leg1_pack->clear();
    leg2_pack->clear();
    leg3_pack->clear();
    leg4_pack->clear();
    
    leg1_pack->begin();
    {
        unsigned int k = 0;
        for (unsigned int ui=0; ui < comp_pack->children(); ui++)
        {
	    if(!statistics[ui].isValid) continue;
            if (statistics[ui].ref)
            {    
                leg1_ref->label(statistics[ui].filename);
                leg1_ref->color(GUI_TEXT_COLOR);
		leg1_ref->labelcolor(GUI_WINDOW_BGCOLOR);
		leg1_ref->labelsize(11);
		leg1_ref->copy_tooltip(statistics[ui].filename);
		leg1_ref->show();
            }
            else
            {
                Fl_Box* entry = new Fl_Box(FL_UP_BOX,
                                           leg1_pack->x(),
                                           leg1_pack->y()+k*20,
                                           leg1_pack->w(),
                                           20,
                                           statistics[ui].filename);
                entry->color(statistics[ui].color);
                entry->labelcolor(GUI_TEXT_COLOR);
                entry->labelfont(FL_HELVETICA);
		entry->labelsize(11);
		entry->copy_tooltip(statistics[ui].filename);
                entry->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
                k++;
            }
        }
    }
    leg1_pack->end();
    //leg1_pack->color(Darker(GUI_WINDOW_BGCOLOR, 0.25));

    leg2_pack->begin();
        {
        unsigned int k = 0;
        for (unsigned int ui=0; ui < comp_pack->children(); ui++)
        {
	    if(!statistics[ui].isValid) continue;
            if (statistics[ui].ref)
            {    
                leg2_ref->label(statistics[ui].filename);
                leg2_ref->color(GUI_TEXT_COLOR);
		leg2_ref->labelcolor(GUI_WINDOW_BGCOLOR);
		leg2_ref->labelsize(11);
		leg2_ref->copy_tooltip(statistics[ui].filename);
		leg2_ref->show();
            }
            else
            {
                Fl_Box* entry = new Fl_Box(FL_UP_BOX,
                                           leg2_pack->x(),
                                           leg2_pack->y()+k*20,
                                           leg2_pack->w(),
                                           20,
                                           statistics[ui].filename);
                entry->color(statistics[ui].color);
                entry->labelcolor(GUI_TEXT_COLOR);
                entry->labelfont(FL_HELVETICA);
                entry->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
                entry->labelsize(11);
		entry->copy_tooltip(statistics[ui].filename);
		k++;
            }
        }
    }
    leg2_pack->end();
    //leg2_pack->color(Darker(GUI_WINDOW_BGCOLOR, 0.25));

    leg3_pack->begin();
    {
        unsigned int k = 0;
        for (unsigned int ui=0; ui < comp_pack->children(); ui++)
        {
	    if(!statistics[ui].isValid) continue;
            if (statistics[ui].ref)
            {    
                leg3_ref->label(statistics[ui].filename);
                leg3_ref->color(GUI_TEXT_COLOR);
		leg3_ref->labelcolor(GUI_WINDOW_BGCOLOR);
		leg3_ref->labelsize(11);
		leg3_ref->copy_tooltip(statistics[ui].filename);
		leg3_ref->show();
            }
            else
            {
                Fl_Box* entry = new Fl_Box(FL_UP_BOX,
                                           leg3_pack->x(),
                                           leg3_pack->y()+k*20,
                                           leg3_pack->w(),
                                           20,
                                           statistics[ui].filename);
                entry->color(statistics[ui].color);
                entry->labelcolor(GUI_TEXT_COLOR);
                entry->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
                entry->labelfont(FL_HELVETICA);
                entry->labelsize(11);
		entry->copy_tooltip(statistics[ui].filename);
		k++;
            }
        }
    }
    leg3_pack->end();
    //leg3_pack->color(Darker(GUI_WINDOW_BGCOLOR, 0.25));

    leg4_pack->begin();
    {
        unsigned int k = 0;
        for (unsigned int ui=0; ui < comp_pack->children(); ui++)
        {
	    if(!statistics[ui].isValid) continue;
            if (statistics[ui].ref)
            {    
                leg4_ref->label(statistics[ui].filename);
                leg4_ref->color(GUI_TEXT_COLOR);
		leg4_ref->labelcolor(GUI_WINDOW_BGCOLOR);
		leg4_ref->labelsize(11);
		leg4_ref->copy_tooltip(statistics[ui].filename);
		leg4_ref->show();
                if (ui%28 >= 0 && ui%28 <7)
                {
                    leg4_ref_symbol->label("@circle");
                } 
                else if (ui%28 >= 7 && ui%28 <14)
                {
                    leg4_ref_symbol->label("@square");
                } 
                else if (ui%28 >= 14 && ui%28 <21)
                {
                    leg4_ref_symbol->label("@+98>"); // Triangle pointed upwards
                }
                else
                {
                    leg4_ref_symbol->label("@+");
                }
                leg4_ref_symbol->show();
            }
            else
            {
                // Add text label in color with filename
                Fl_Group* entry_group = new Fl_Group(leg4_pack->x(),
                                                     leg4_pack->y()+k*20,leg4_pack->w(),20,"");
                {
                    Fl_Box* entry = new Fl_Box(FL_UP_BOX,
                                               leg4_pack->x()+20,
                                               leg4_pack->y()+k*20,
                                               leg4_pack->w()-20,
                                               20,
                                               statistics[ui].filename);
                    entry->color(statistics[ui].color);
                    entry->labelcolor(GUI_TEXT_COLOR);
                    entry->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
                    entry->labelfont(FL_HELVETICA);
                    entry->labelsize(11);
		    entry->copy_tooltip(statistics[ui].filename);

                    // Add symbol label in color
                    entry = new Fl_Box(FL_NO_BOX,leg4_pack->x(),leg4_pack->y()+k*20,
                                       20,20,"");
                    entry->labelcolor(statistics[ui].color);
                    entry->labelfont(FL_HELVETICA);
		    entry->labelsize(11);
                    entry->copy_tooltip(statistics[ui].filename);
		    if (ui%28 >= 0 && ui%28 <7)
                    {
                        entry->label("@circle");
                    } 
                    else if (ui%28 >= 7 && ui%28 <14)
                    {
                        entry->label("@square");
                    }
                    else if (ui%28 >= 14 && ui%28 <21)
                    {
                        entry->label("@+98>"); // Triangle pointed upwards
                    }
                    else
                    {
                        entry->label("@+");
                    }
                }
                entry_group->end();
                k++;
            }
        }
    }
    leg4_pack->end();
    //leg4_pack->color(Darker(GUI_WINDOW_BGCOLOR, 0.25));

    leg1_scroll->redraw();
    leg2_scroll->redraw();
    leg3_scroll->redraw();
    leg4_scroll->redraw();

}

void StatsWindow::ExportTable()
{
    menu_window->deactivate();
    tab_window->deactivate();
    
    char filename[MAX_BUFFER_SIZE];
    char dateStamp[MAX_BUFFER_SIZE];
    time_t currentTime = time(NULL);
    struct tm *tmCurrentTime = localtime(&currentTime);
    strftime(dateStamp, MAX_BUFFER_SIZE - 1, "%F-%H%M%S", tmCurrentTime);
    const char *sepChar = PNG_OUTPUT_DIRECTORY[
                strlen((char *) PNG_OUTPUT_DIRECTORY) - 1] == '/' ? 
                "" : "/";
    snprintf(filename, MAX_BUFFER_SIZE - 1, 
             "%s%sRNAStructViz-StatsTableOutput-%s.csv", 
             (char *) PNG_OUTPUT_DIRECTORY, sepChar, dateStamp);
    if(input_window != NULL) {
         Delete(input_window, InputWindow);
    }
    input_window = new InputWindow(450, 150, 
		                   "Export Table To CSV-Fomatted Plaintext File ...",
                                    filename, InputWindow::FILE_INPUT);
    exp_button->value(1);
    exp_button->deactivate();
    while (input_window->visible()) {
        Fl::wait();
    }
    
    if(input_window != NULL)
    {
        FILE * expFile;
        if(strcmp(input_window->getName(), ""))
        {
            strncpy(filename, input_window->getName(), MAX_BUFFER_SIZE - 1);
            expFile = fopen(filename, "a+");
	    const char *dirMarkerPos = strrchr(filename, '/');
	    if(dirMarkerPos != NULL) {
		 unsigned int charsToCopy = (unsigned int) (dirMarkerPos - filename + 1);
	         strncpy((char *) PNG_OUTPUT_DIRECTORY, filename, charsToCopy);
		 PNG_OUTPUT_DIRECTORY[charsToCopy] = '\0';
		 ConfigParser::WriteUserConfigFile(USER_CONFIG_PATH);
	    }
        }
        else {
            expFile = NULL;
        }

        if (expFile != NULL)
        {
            /* Check whether file is empty (i.e. whether it already existed. 
                If it did, add a line (or something) to denote starting a new table */
            rewind(expFile);
            int checkChar = fgetc(expFile);
            if (checkChar != EOF)
            {
                fprintf(expFile,"\n");
            }
            
            // print the column header
            fprintf(expFile,
                    "Filename,Reference,Base_Pairs,True_Positive,False_Positive,");
            fprintf(expFile,
                    "False_Negative,Conflict,Contradict,Compatible,Sensitivity,");
            fprintf(expFile,
                    "Selectivity,Positive_Predictive_Value,");
            fprintf(expFile,
                    "G-C_Pairs,A-U_Pairs,G-U_Pairs,Non-Canonical_Pairs\n"); 
            
            for (int ui = comp_pack->children() - 1; ui >= 0; ui--)
            {
		if(!statistics[ui].isValid) continue;
                // print the row of statistics for each structure
                fprintf(expFile,
                        "%s,%d,%d,%d,%d,%d,%d,%d,%d,%.10f,%.10f,%.10f,%d,%d,%d,%d\n",
                        statistics[ui].filename,
                        statistics[ui].ref,
                        statistics[ui].base_pair_count,
                        statistics[ui].true_pos_count,
                        statistics[ui].false_pos_count,
                        statistics[ui].false_neg_count,
                        statistics[ui].conflict_count,
                        statistics[ui].contradict_count,
                        statistics[ui].compatible_count,
                        statistics[ui].sensitivity,
                        statistics[ui].selectivity,
                        statistics[ui].pos_pred_value,
                        statistics[ui].gc_count,
                        statistics[ui].au_count,
                        statistics[ui].gu_count,
                        statistics[ui].non_canon_count);
            }
            fclose(expFile);
        }
    }
    
    exp_button->value(0);
    exp_button->activate();
    menu_window->activate();
    tab_window->activate();
}

void StatsWindow::BuildRefMenu()
{
    
    ref_menu->clear();
    ref_menu->add("Please select a reference  @|>",0,0,0,0);
    ref_menu->textsize(11);
    ref_menu->textfont(FL_HELVETICA);

    // Add entries
    for (unsigned int ui = 0; ui < m_structures.size(); ++ui)
    {
        RNAStructure* structure = 
        structureManager->GetStructure(m_structures[ui]);
        ref_menu->add(structure->GetFilename(), 0, 0, 0, 0);
    }
    ref_menu->value(0);
    ref_menu->redraw();
    
    calc_button->value(1);
    calc_button->deactivate();
}

void StatsWindow::BuildCompMenu()
{
    int lastBtnX = 0, lastBtnY = 0;
    comp_pack->clear();
    comp_pack->begin();
    {
        for (unsigned int ui=0; ui < m_structures.size(); ui++)
        {
            RNAStructure* structure = 
            structureManager->GetStructure(m_structures[ui]);
            if (strcmp(ref_menu->mvalue()->label(), structure->GetFilename()))
            {
                Fl_Check_Button* button = new Fl_Check_Button(comp_pack->x(),
                                                              comp_pack->y() + 30 + lastBtnY, 218, //comp_pack->w() - 12, 
							      24, structure->GetFilename());
                button->color(GUI_WINDOW_BGCOLOR);
                button->labelcolor(GUI_TEXT_COLOR);
                button->labelfont(FL_HELVETICA);
		button->labelsize(10);
                button->selection_color(GUI_WINDOW_BGCOLOR);
                button->callback(MenuCallback);
                if(button->y() >= lastBtnY) { 
                    lastBtnX = button->x();
                    lastBtnY = button->y();
                }
		else {
		    //lastBtnY += 30;
		}
            }
        }
    }
    comp_pack->end();

}
