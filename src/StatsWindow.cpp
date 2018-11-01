#include "StatsWindow.h"
#include "StructureManager.h"
#include "RNAStructViz.h"
#include "RNAStructure.h"
#include <FL/Fl_Box.H>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <string.h>
#include <FL/Fl_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Check_Button.H> 
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_Chart.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Group.H>
#include "Fl_Rotated_Text.H"
#include <math.h>
#include "rocbox.h"
#include "InputWindow.h"
#include <algorithm>
#include "ConfigOptions.h"
#include <time.h>

#include "pixmaps/StatsFormula.c"
#include "pixmaps/StatsWindowIcon.xbm"

void StatsWindow::Construct(int w, int h, const std::vector<int>& structures)
{

	/* TODO: look into creating an offscreen version of the graph if calculation/
     creation makes it do odd things. May only work the openGL */
	
	folderIndex = -1;
	referenceIndex = -1;
	numStats = 0;
	statistics = NULL;
    color(GUI_WINDOW_BGCOLOR);
	
	/* Create the menu section on the left */
	menu_window = new Fl_Group(0,0,300,h,"Menu Group");
	{
        int mwx = menu_window->x();
        int mwy = menu_window->y();
        int mww = menu_window->w();
        int mwh = menu_window->h();
		ref_menu = new Fl_Choice(mwx+20,mwy+ 60, mww-40, 30, 
		           "Select Reference Structure:");
		ref_menu->labelcolor(GUI_TEXT_COLOR);
		ref_menu->textcolor(GUI_BTEXT_COLOR);
		ref_menu->align(FL_ALIGN_TOP);    
		
		const char *dividerText = "--------------------------------------------";
		dividerTextBox = new Fl_Box(mwx + 20, mwy + 95, mww - 40, 
				 5, dividerText);
		dividerTextBox->labelcolor(GUI_TEXT_COLOR);
		dividerTextBox->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | 
				      FL_ALIGN_CENTER);

		comp_menu = new Fl_Scroll(mwx+20,mwy+ 120, mww-40, 
			    mwh-120-90, 
			    "Select Comparison Structures:");
		{	
			comp_pack = new Fl_Pack(mwx+20,mwy+120,mww-60,150);
			comp_pack->type(Fl_Pack::VERTICAL);
			comp_pack->end();
                        comp_pack->color(GUI_WINDOW_BGCOLOR);
		}
		comp_menu->type(Fl_Scroll::VERTICAL);
		comp_menu->box(FL_FLAT_BOX);
		comp_menu->labelcolor(GUI_TEXT_COLOR);
		comp_menu->align(FL_ALIGN_TOP);
                comp_menu->color(GUI_WINDOW_BGCOLOR);
		comp_menu->end();
		
		calc_button = new Fl_Toggle_Button(mwx+20,mwy+ mwh-60, mww-40, 
                                           30, "@refresh Calculate");
		calc_button->callback(CalcCallback);
		calc_button->labelcolor(GUI_BTEXT_COLOR); 
		
		menu_window->resizable(comp_menu);
	}
	menu_window->end();
	ref_menu->callback(ReferenceCallback, &comp_pack);
	
	/* Create the tabs section on the right */
	tab_window = new Fl_Tabs(300,0,w-300,h,"Tabs Container");
	{
		overview_tab = new Fl_Group(300,20,w-300,h-20,"@filenew Overview"); 
		{
			Fl_Group *ov_charts_group = new Fl_Group(310,30,w-520,h-40,"");
			{
                int ovcx = ov_charts_group->x();
                int ovcy = ov_charts_group->y();
                int ovcw = ov_charts_group->w();
                int ovch = ov_charts_group->h();
				bp_chart = new Fl_Group(ovcx+10,ovcy+10,(ovcw-60)/2,(ovch-60)/2, "Base Pairs");
				bp_chart->box(FL_BORDER_BOX);
				bp_chart->align(FL_ALIGN_BOTTOM);
				bp_chart->labelcolor(GUI_TEXT_COLOR);
                                bp_chart->color(GUI_WINDOW_BGCOLOR);
				bp_chart->end();
				
				tp_chart = new Fl_Group(ovcx+50+(ovcw-60)/2,ovcy+10,(ovcw-60)/2,(ovch-60)/2,"True Positives");
				tp_chart->box(FL_BORDER_BOX);
				tp_chart->align(FL_ALIGN_BOTTOM);
				tp_chart->labelcolor(GUI_TEXT_COLOR);
                                tp_chart->color(GUI_WINDOW_BGCOLOR);
				tp_chart->end();
				
				fp_chart = new Fl_Group(ovcx+10,ovcy+40+(ovch-60)/2,(ovcw-60)/2,(ovch-60)/2,"False Positives");
				fp_chart->box(FL_BORDER_BOX);
				fp_chart->align(FL_ALIGN_BOTTOM);
				fp_chart->labelcolor(GUI_TEXT_COLOR);
                                fp_chart->color(GUI_WINDOW_BGCOLOR);
				fp_chart->end();
				
				fn_chart = new Fl_Group(ovcx+50+(ovcw-60)/2,ovcy+40+(ovch-60)/2,(ovcw-60)/2,(ovch-60)/2, "False Negatives");
				fn_chart->box(FL_BORDER_BOX);
				fn_chart->align(FL_ALIGN_BOTTOM);
				fn_chart->labelcolor(GUI_TEXT_COLOR);
                                fn_chart->color(GUI_WINDOW_BGCOLOR);
				fn_chart->end();
                
			}
			ov_charts_group->end();
			ov_charts_group->resizable(ov_charts_group);
			
			leg1_group = new Fl_Group(w-200,30,195,h-40,"");
			{
                int leg1x = leg1_group->x();
                int leg1y = leg1_group->y();
                int leg1w = leg1_group->w();
                int leg1h = leg1_group->h();
                
				Fl_Box* leg_label = new Fl_Box(leg1x+10,leg1y+ 10, leg1w-15, 30, 
                                               "Legend");
				leg_label->labelcolor(GUI_TEXT_COLOR);
				
				leg_label = new Fl_Box(leg1x+10,leg1y+ 60, leg1w-15, 30, "Reference Structure");
				leg_label->labelcolor(GUI_TEXT_COLOR);
				
				leg1_ref = new Fl_Box(FL_FLAT_BOX,leg1x+10,leg1y+90, leg1w-15, 20,"");
				leg1_ref->color(GUI_TEXT_COLOR);
				leg1_ref->labelcolor(GUI_WINDOW_BGCOLOR);
				leg1_ref->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
				leg1_ref->hide();
				
				leg_label = new Fl_Box(leg1x+10,leg1y+ 120, leg1w-15, 30, "Predicted Structures");
				leg_label->labelcolor(GUI_TEXT_COLOR);
				
				leg1_scroll = new Fl_Scroll(leg1x+10,leg1y+ 150, leg1w-15, leg1h-200);
				{	
					leg1_pack = new Fl_Pack(leg1x+10,leg1y+150,leg1w-15,leg1h-200);
					leg1_pack->type(Fl_Pack::VERTICAL);
					leg1_pack->end();
                    leg1_pack->color(GUI_WINDOW_BGCOLOR);
				}
				leg1_scroll->type(Fl_Scroll::VERTICAL);
				leg1_scroll->box(FL_FLAT_BOX);
				leg1_scroll->end();
                leg1_scroll->color(GUI_WINDOW_BGCOLOR);
                
			}
			leg1_group->end();
			leg1_group->resizable(leg1_scroll);
			overview_tab->resizable(ov_charts_group);	
		}
		overview_tab->end();
        overview_tab->color(GUI_WINDOW_BGCOLOR);
	overview_tab->labelcolor(GUI_BTEXT_COLOR);
		
		perc_tab = new Fl_Group(300,20,w-300,h-20,
                                "@+ Percentage Values");
		{
			Fl_Group *perc_charts_group = new Fl_Group(310,30,w-520,h-40,""); // DOUBLEWINDOW
		    {
		    	int percx = perc_charts_group->x();
                int percy = perc_charts_group->y();
                int percw = perc_charts_group->w();
                int perch = perc_charts_group->h();
                
		    	sens_chart = new Fl_Group(percx+10,percy+10,(percw-60)/2,(perch-60)/2, "Sensitivity");
				sens_chart->box(FL_BORDER_BOX);
				sens_chart->align(FL_ALIGN_BOTTOM);
				sens_chart->labelcolor(GUI_TEXT_COLOR);
                sens_chart->color(GUI_WINDOW_BGCOLOR);
				sens_chart->end();
				
				sel_chart = new Fl_Group(percx+50+(percw-60)/2,percy+10,(percw-60)/2,(perch-60)/2,"Selectivity");
				sel_chart->box(FL_BORDER_BOX);
				sel_chart->align(FL_ALIGN_BOTTOM);
				sel_chart->labelcolor(GUI_TEXT_COLOR);
                sel_chart->color(GUI_WINDOW_BGCOLOR);
				sel_chart->end();
				
				// Display formulas for the different percentage value statistics
                int vgap = 20;
                int hgap = 10; // Should be between 0 and 40
                //fprintf(stderr, "%d; %d\n", percx+50+(percw-60)/2,percy+40+(perch-60)/2);
		statsFormulasImage = new Fl_RGB_Image(StatsFormula.pixel_data, 
		                     StatsFormula.width, StatsFormula.height, 
		                     StatsFormula.bytes_per_pixel);
                statsFormulasBox = new Fl_Box(percx+30+(percw-60)/2,percy+75+(perch-60)/2, 
				   statsFormulasImage->w(), statsFormulasImage->h());
		statsFormulasBox->image(statsFormulasImage);
		//tp_label = new Fl_Box(percx+10+hgap+(percw-60)/2,percy+40+(perch-60)/2+vgap,(percw-60)/2+40-hgap,20,"TP: true positive");
		//fp_label = new Fl_Box(percx+10+hgap+(percw-60)/2,percy+40+(perch-60)/2+vgap+30,(percw-60)/2+40-hgap,20,"FP: false positive");
		//fp_equ_label = new Fl_Box(percx+10+hgap+(percw-60)/2,percy+40+(perch-60)/2+vgap+80,(percw-60)/2+40-hgap,20,"FP = conflict + contradict + compatible");
                //fp_equ_label->align(FL_ALIGN_WRAP);
		//sens_label = new Fl_Box(percx+10+hgap+(percw-60)/2,percy+40+(perch-60)/2+vgap+130,(percw-60)/2+40-hgap,20,"sensitivity = TP / (TP + FN)");
                //sens_label->align(FL_ALIGN_WRAP);
		//sel_label = new Fl_Box(percx+10+hgap+(percw-60)/2,percy+40+(perch-60)/2+vgap+170,(percw-60)/2+40-hgap,20,"selectivity = TP / (TP + FP - compatible)");
                //sel_label->align(FL_ALIGN_WRAP);
		//ppv_label = new Fl_Box(percx+10+hgap+(percw-60)/2,percy+40+(perch-60)/2+vgap+210,(percw-60)/2+40-hgap,20,"PPV: positive predictive value");
		//ppv_equ_label = new Fl_Box(percx+10+hgap+(percw-60)/2,percy+40+(perch-60)/2+vgap+240,(percw-60)/2+40-hgap,20,"PPV = TP / (TP + FP)");
                //ppv_equ_label->align(FL_ALIGN_WRAP);
                
				ppv_chart = new Fl_Group(percx+10,percy+40+(perch-60)/2,(percw-60)/2,(perch-60)/2,"Positive Predictive Value");
				ppv_chart->box(FL_BORDER_BOX);
				ppv_chart->align(FL_ALIGN_BOTTOM);
				ppv_chart->labelcolor(GUI_TEXT_COLOR);
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
                
                Fl_Box* leg_label = new Fl_Box(leg2x+10,leg2y+10,leg2w-15, 30, 
                                               "Legend");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                
                leg_label = new Fl_Box(leg2x+10,leg2y+ 60, leg2w-15, 30, "Reference Structure");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                
                leg2_ref = new Fl_Box(FL_FLAT_BOX,
                                      leg2x+10, 
                                      leg2y+90, 
                                      leg2w-15, 
                                      20,
                                      "");
                leg2_ref->color(GUI_TEXT_COLOR);
                leg2_ref->labelcolor(GUI_WINDOW_BGCOLOR);
                leg2_ref->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
                leg2_ref->hide();
                
                leg_label = new Fl_Box(leg2x+10,leg2y+ 120,leg2w-15, 30, "Predicted Structures");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                
                leg2_scroll = new Fl_Scroll(leg2x+10,leg2y+ 150,leg2w-15, leg2h-200);
                {	
                    leg2_pack = new Fl_Pack(leg2x+10,leg2y+150,leg2w-15,leg2h-200);
                    leg2_pack->type(Fl_Pack::VERTICAL);
                    leg2_pack->color(GUI_WINDOW_BGCOLOR);
                    leg2_pack->end();
                }
                leg2_scroll->type(Fl_Scroll::VERTICAL);
                leg2_scroll->box(FL_FLAT_BOX);
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

		pair_tab = new Fl_Group(300,20,w-300,h-20,"@menu Base Pairings");
		{
			Fl_Group *pair_charts_group = new Fl_Group(310,30,w-520,h-40,""); // DOUBLEWINDOW
			{
                int pairx = pair_charts_group->x();
                int pairy = pair_charts_group->y();
                int pairw = pair_charts_group->w();
                int pairh = pair_charts_group->h();
                
				gc_chart = new Fl_Group(pairx+10,pairy+10,(pairw-60)/2,
                                        (pairh-60)/2,"G-C Base Pairs");
				gc_chart->box(FL_BORDER_BOX);
				gc_chart->align(FL_ALIGN_BOTTOM);
				gc_chart->labelcolor(GUI_TEXT_COLOR);
                gc_chart->color(GUI_WINDOW_BGCOLOR);
				gc_chart->end();
				
				au_chart = new Fl_Group(pairx+50+(pairw-60)/2,pairy+10,
                                        (pairw-60)/2,(pairh-60)/2,
                                        "A-U Base Pairs");
				au_chart->box(FL_BORDER_BOX);
				au_chart->align(FL_ALIGN_BOTTOM);
				au_chart->labelcolor(GUI_TEXT_COLOR);
                au_chart->color(GUI_WINDOW_BGCOLOR);
				au_chart->end();
				
				gu_chart = new Fl_Group(pairx+10,pairy+40+(pairh-60)/2,
                                        (pairw-60)/2,(pairh-60)/2,
                                        "G-U Base Pairs");
				gu_chart->box(FL_BORDER_BOX);
				gu_chart->align(FL_ALIGN_BOTTOM);
				gu_chart->labelcolor(GUI_TEXT_COLOR);
                gu_chart->color(GUI_WINDOW_BGCOLOR);
				gu_chart->end();
				
				non_canon_chart = new Fl_Group(pairx+50+(pairw-60)/2,
                                               pairy+40+(pairh-60)/2,(pairw-60)/2,
                                               (pairh-60)/2, "Non-Canonical Base Pairs");
				non_canon_chart->box(FL_BORDER_BOX);
				non_canon_chart->align(FL_ALIGN_BOTTOM);
				non_canon_chart->labelcolor(GUI_TEXT_COLOR);
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
                
                leg_label = new Fl_Box(leg3x+10,leg3y+ 60,leg3w-15, 30, "Reference Structure");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                
                leg3_ref = new Fl_Box(FL_FLAT_BOX,
                                      leg3x+10, 
                                      leg3y+90,leg3w-15, 20,
                                      "");
                leg3_ref->color(GUI_TEXT_COLOR);
                leg3_ref->labelcolor(GUI_WINDOW_BGCOLOR);
                leg3_ref->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
                leg3_ref->hide();
                
                leg_label = new Fl_Box(leg3x+10,leg3y+ 120, leg3w-15, 30, "Predicted Structures");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                
                leg3_scroll = new Fl_Scroll(leg3x+10,leg3y+ 150,leg3w-15, leg3h-200);
                {	
                    leg3_pack = new Fl_Pack(leg3x+10,leg3y+150,leg3w-15,leg3h-200);
                    leg3_pack->type(Fl_Pack::VERTICAL);
                    leg3_pack->end();
                }
                leg3_scroll->type(Fl_Scroll::VERTICAL);
                leg3_scroll->box(FL_FLAT_BOX);
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
		
		roc_tab = new Fl_Group(300,20,w-300,h-20,"@<-> ROC Plot");
		{
			Fl_Group *roc_plot_group = new Fl_Group(310,30,w-520,h-40,""); // DOUBLEWINDOW
			{
                int rpx = roc_plot_group->x();
                int rpy = roc_plot_group->y();
                int rpw = roc_plot_group->w();
                int rph = roc_plot_group->h();
                
				roc_plot = new Fl_Group(rpx+40,rpy+20,rpw-50,
                                        rph-60,"ROC Plot");
				//roc_plot->box(FL_BORDER_BOX);
				roc_box_init();
				roc_plot->box(ROC_BOX);
				roc_plot->align(FL_ALIGN_TOP);
				roc_plot->labelcolor(GUI_TEXT_COLOR);
                roc_plot->color(GUI_WINDOW_BGCOLOR);
				roc_plot->end();
				
				/* Draw the labels for the y-axis */
				Fl_Box* text_rotate = new Fl_Box(rpx+0,roc_plot->y(),20,roc_plot->h());
				Fl_Rotated_Text* text = new Fl_Rotated_Text("Sensitivity (%)",
                                                            FL_HELVETICA,14,0,1);
				text_rotate->image(text);
				
				// Put the number to the left of the axis, the dash to the right
				Fl_Box* label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                           rpy+15,10,10,"100");
				label->align(FL_ALIGN_LEFT);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                   roc_plot->y()+roc_plot->h()-5-(int)(9*roc_plot->h()/10.0),10,10,"90");
				label->align(FL_ALIGN_LEFT);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                   roc_plot->y()+roc_plot->h()-5-(int)(9*roc_plot->h()/10.0),10,10,"@-9line");
				label->align(FL_ALIGN_RIGHT);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                   roc_plot->y()+roc_plot->h()-5-(int)(8*roc_plot->h()/10.0),10,10,"80");
				label->align(FL_ALIGN_LEFT);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                   roc_plot->y()+roc_plot->h()-5-(int)(8*roc_plot->h()/10.0),10,10,"@-9line");
				label->align(FL_ALIGN_RIGHT);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                   roc_plot->y()+roc_plot->h()-5-(int)(7*roc_plot->h()/10.0),10,10,"70");
				label->align(FL_ALIGN_LEFT);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                   roc_plot->y()+roc_plot->h()-5-(int)(7*roc_plot->h()/10.0),10,10,"@-9line");
				label->align(FL_ALIGN_RIGHT);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                   roc_plot->y()+roc_plot->h()-5-(int)(6*roc_plot->h()/10.0),10,10,"60");
				label->align(FL_ALIGN_LEFT);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                   roc_plot->y()+roc_plot->h()-5-(int)(6*roc_plot->h()/10.0),10,10,"@-9line");
				label->align(FL_ALIGN_RIGHT);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                   roc_plot->y()+roc_plot->h()-5-(int)(5*roc_plot->h()/10.0),10,10,"50");
				label->align(FL_ALIGN_LEFT);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                   roc_plot->y()+roc_plot->h()-5-(int)(5*roc_plot->h()/10.0),10,10,"@-9line");
				label->align(FL_ALIGN_RIGHT);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                   roc_plot->y()+roc_plot->h()-5-(int)(4*roc_plot->h()/10.0),10,10,"40");
				label->align(FL_ALIGN_LEFT);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                   roc_plot->y()+roc_plot->h()-5-(int)(4*roc_plot->h()/10.0),10,10,"@-9line");
				label->align(FL_ALIGN_RIGHT);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                   roc_plot->y()+roc_plot->h()-5-(int)(3*roc_plot->h()/10.0),10,10,"30");
				label->align(FL_ALIGN_LEFT);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                   roc_plot->y()+roc_plot->h()-5-(int)(3*roc_plot->h()/10.0),10,10,"@-9line");
				label->align(FL_ALIGN_RIGHT);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                   roc_plot->y()+roc_plot->h()-5-(int)(2*roc_plot->h()/10.0),10,10,"20");
				label->align(FL_ALIGN_LEFT);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                   roc_plot->y()+roc_plot->h()-5-(int)(2*roc_plot->h()/10.0),10,10,"@-9line");
				label->align(FL_ALIGN_RIGHT);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                   roc_plot->y()+roc_plot->h()-5-(int)(roc_plot->h()/10.0),10,10,"10");
				label->align(FL_ALIGN_LEFT);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-15,
                                   roc_plot->y()+roc_plot->h()-5-(int)(roc_plot->h()/10.0),10,10,"@-9line");
				label->align(FL_ALIGN_RIGHT);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x(),
                                   roc_plot->y()+roc_plot->h()-5,10,10,"0");
				label->align(FL_ALIGN_LEFT);
				label->labelcolor(GUI_TEXT_COLOR);
				
				// Draw the labels for the x-axis
				label = new Fl_Box(FL_NO_BOX,roc_plot->x(),rpy+rph-20,
                                   roc_plot->w(),20,"Selectivity (%)");
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5,
                                   roc_plot->y()-10+roc_plot->h(),10,10,"0");
				label->align(FL_ALIGN_BOTTOM);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"10");
				label->align(FL_ALIGN_BOTTOM);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
				label->align(FL_ALIGN_TOP);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(2*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"20");
				label->align(FL_ALIGN_BOTTOM);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(2*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
				label->align(FL_ALIGN_TOP);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(3*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"30");
				label->align(FL_ALIGN_BOTTOM);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(3*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
				label->align(FL_ALIGN_TOP);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(4*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"40");
				label->align(FL_ALIGN_BOTTOM);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(4*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
				label->align(FL_ALIGN_TOP);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(5*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"50");
				label->align(FL_ALIGN_BOTTOM);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(5*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
				label->align(FL_ALIGN_TOP);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(6*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"60");
				label->align(FL_ALIGN_BOTTOM);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(6*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
				label->align(FL_ALIGN_TOP);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(7*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"70");
				label->align(FL_ALIGN_BOTTOM);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(7*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
				label->align(FL_ALIGN_TOP);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(8*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"80");
				label->align(FL_ALIGN_BOTTOM);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(8*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
				label->align(FL_ALIGN_TOP);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(9*roc_plot->w()/10.0),
                                   roc_plot->y()-10+roc_plot->h(),10,10,"90");
				label->align(FL_ALIGN_BOTTOM);
				label->labelcolor(GUI_TEXT_COLOR);
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+(int)(9*roc_plot->w()/10.0),
                                   roc_plot->y()+3+roc_plot->h(),10,10,"@-98line");
				label->align(FL_ALIGN_TOP);
				label->labelcolor(GUI_TEXT_COLOR);
				
				label = new Fl_Box(FL_NO_BOX,roc_plot->x()-5+roc_plot->w()-6,
                                   roc_plot->y()-10+roc_plot->h(),10,10,"100");
				label->align(FL_ALIGN_BOTTOM);
				label->labelcolor(GUI_TEXT_COLOR);
                
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
                
                leg_label = new Fl_Box(leg4x+20,leg4y+ 60, leg4w-25, 30, "Reference Structure");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                
                leg4_ref = new Fl_Box(FL_FLAT_BOX,
                                      leg4x+20, 
                                      leg4y+90,leg4x-25, 20,
                                      "");
                leg4_ref->color(GUI_TEXT_COLOR);
                leg4_ref->labelcolor(GUI_WINDOW_BGCOLOR);
                leg4_ref->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
                leg4_ref->hide();
                leg4_ref_symbol = new Fl_Box(FL_NO_BOX,
                                             leg4x, 
                                             leg4y+90, 20, 20,
                                             "");
                leg4_ref_symbol->labelcolor(GUI_TEXT_COLOR);
                leg4_ref_symbol->hide();
                
                leg_label = new Fl_Box(leg4x+20,leg4y+ 120,leg4w-25, 30, "Predicted Structures");
                leg_label->labelcolor(GUI_TEXT_COLOR);
                
                leg4_scroll = new Fl_Scroll(leg4x,leg4y+ 150,leg4w-5, leg4h-200);
                {	
                    leg4_pack = new Fl_Pack(leg4x,leg4y+150,leg4w-5,leg4h-200);
                    leg4_pack->type(Fl_Pack::VERTICAL);
                    leg4_pack->end();
                }
                leg4_scroll->type(Fl_Scroll::VERTICAL);
                leg4_scroll->box(FL_FLAT_BOX);
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
        
		table_tab = new Fl_Group(300,20,w-300,h-20,"@line Table");
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
                text_display->textfont(LOCAL_RMFONT);
		text_display->textcolor(GUI_TEXT_COLOR);
                text_display_group->resizable(text_display);
            }
			text_display_group->end();
			
			exp_button = new Fl_Button(w-100,h-40,80,30,"@filesaveas Export @->");
			exp_button->callback(ExportCallback);
			exp_button->value(1);
                        exp_button->deactivate();
			
			table_tab->resizable(text_display_group);
		}
		table_tab->end();
        table_tab->color(GUI_WINDOW_BGCOLOR);
	table_tab->labelcolor(GUI_BTEXT_COLOR);
	}
	tab_window->labeltype(FL_NO_LABEL);
	tab_window->labelcolor(Darker(FL_LIGHT3, 0.95f));
	tab_window->end();
    
	this->resizable(tab_window);
	this->size_range(800,600);
	this->end();
    
    input_window = NULL;
	
	title = (char*)malloc(sizeof(char) * 64);
	SetStructures(structures);
}

StatsWindow::StatsWindow(int w, int h, const char *label, 
                         const std::vector<int>& structures)
: Fl_Window(w, h, label), statsFormulasImage(NULL), statsFormulasBox(NULL)
{
     
    fl_open_display();
    Pixmap iconPixmap = XCreateBitmapFromData(fl_display, 
		        DefaultRootWindow(fl_display),
                        StatsWindowIcon_bits, StatsWindowIcon_width, 
			StatsWindowIcon_height);
    this->icon((const void *) iconPixmap);
    Construct(w, h, structures);
}

StatsWindow::StatsWindow(int x, int y, int w, int h, const char *label, 
                         const std::vector<int>& structures)
: Fl_Window(x, y, w, h, label), statsFormulasImage(NULL), statsFormulasBox(NULL)
{
    Construct(w, h, structures);
}

StatsWindow::~StatsWindow()
{
	/* memory management */
	free (title); 
	delete[] statistics;
	if(statsFormulasImage != NULL) {
             delete statsFormulasImage;
	}
	if(statsFormulasBox != NULL) {
	     delete statsFormulasBox;
	}
}

void StatsWindow::ResetWindow()
{
	this->size(1000,700);
    
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
	
	buff->remove(0,buff->length());
	exp_button->value(1);
    exp_button->deactivate();
	
	tab_window->value(overview_tab);
}

void StatsWindow::SetFolderIndex(int index)
{
    folderIndex = index;
    
    sprintf(title, "Statistics: %-.48s", structureManager->GetFolderAt(index)->folderName);
    label(title);
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
    if(strcmp(window->ref_menu->mvalue()->label(),
              "Please select a reference"))
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
		Fl_Check_Button* button = (Fl_Check_Button*)window->comp_pack->child(i);
		button->labelcolor(GUI_TEXT_COLOR); 
		if (!strcmp(button->label(),window->ref_menu->mvalue()->label()))
		{
			button->value(1);
			button->deactivate();
		}
		else
		{
			button->activate();
		}
	}	
}

void StatsWindow::MenuCallback(Fl_Widget* widget, void* userData)
{
    StatsWindow* window = (StatsWindow*)widget->parent()->parent()->parent()->
    parent();
    
	if(strcmp(window->ref_menu->mvalue()->label(),
              "Please select a reference"))
    {
    	window->calc_button->value(0);
        window->calc_button->activate();
    }
    else
    {
    	window->calc_button->value(1);
        window->calc_button->deactivate();
    }
}

void StatsWindow::CalcCallback(Fl_Widget* widget, void* userData)
{	
	if(((Fl_Toggle_Button*)widget)->value() == 1)
	{
		StatsWindow* window = (StatsWindow*)widget->parent()->parent();
		if(strcmp(window->ref_menu->mvalue()->label(),
                  "Please select a reference"))
		{
			
			
			for (int i=0; i < window->comp_pack->children(); i++)
			{
				Fl_Check_Button* button = 
                (Fl_Check_Button*)window->comp_pack->child(i);
				
			}
			
            
		}
		
		window->ComputeStats();
		
	}
	else
	{
		((Fl_Toggle_Button*)widget)->value(1);
	}
    ((Fl_Toggle_Button*)widget)->deactivate();
}

void StatsWindow::ExportCallback(Fl_Widget* widget, void* userData)
{
	if (((Fl_Toggle_Button*)widget)->value() == 0)
	{
		StatsWindow* window = (StatsWindow*)widget->parent()->parent()->parent();
		window->ExportTable();
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
    
    //if (ppv_label->w() < 175) {
    //    ppv_label->label("PPV: pos. pred. value");
    //} else {
    //    ppv_label->label("PPV: positive predictive value");
    //}
    
}

/*void StatsWindow::draw()
 {
 Fl_Color priorColor = fl_color();
 
 fl_color(color());
 fl_rectf(0, 0, w(), h());
 fl_color(priorColor);
 
 Fl_Window::draw();
 }*/

void StatsWindow::hide()
{
    if (input_window != NULL)
    {
        delete input_window;
        input_window = NULL;
    }
    
    Fl_Window::hide();
}

void StatsWindow::ClearStats()
{
	if (statistics != NULL) 
	{
		delete[] statistics;
		statistics = NULL;
	}
	numStats = 0;
}

void StatsWindow::ComputeStats()
{
	ClearStats();
	
	buff->remove(0,buff->length());	
	
	for (int i=0; i < comp_pack->children(); i++)
	{
		Fl_Check_Button* button = 
        (Fl_Check_Button*)comp_pack->child(i);
		if (button->value() == 1)
		{
			numStats++;		
		}
	}
	
	statistics = new StatData[numStats];
	
	// Find the reference structure
	RNAStructure* reference = 
    structureManager->GetStructure(m_structures[referenceIndex]);
	
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
	for (int i=0; i< comp_pack->children(); i++)
	{
		Fl_Check_Button* button = 
        (Fl_Check_Button*)comp_pack->child(i);
		if (button->value() == 1)
		{
			// Find the corresponding structure
			// index corresponds to index of checkbox
			RNAStructure* predicted = 
            structureManager->GetStructure(m_structures[i]);
			
			// Initialize values
			if (strcmp(predicted->GetFilename(),reference->GetFilename()))
			{
                statsIndex = counter;
				statistics[statsIndex].ref = false;
			}
			else
			{
                statsIndex = 0;
				statistics[statsIndex].ref = true;
			}
            statistics[statsIndex].filename = predicted->GetFilename();
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
			for (unsigned int uj = 0; uj< reference->GetLength(); uj++)
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
			/*statistics[statsIndex].false_pos_count = 
             statistics[statsIndex].conflict_count + 
             statistics[statsIndex].contradict_count; // discount compatible*/
			statistics[statsIndex].false_neg_count = 
            ref_base_pair_count - statistics[statsIndex].true_pos_count;
			statistics[statsIndex].compatible_count = 
            statistics[statsIndex].false_pos_count -
            statistics[statsIndex].conflict_count - 
            statistics[statsIndex].contradict_count;
			/*statistics[statsIndex].compatible_count =
             statistics[statsIndex].base_pair_count -
             statistics[statsIndex].true_pos_count -
             statistics[statsIndex].false_pos_count;*/
			
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
				/*statistics[statsIndex].selectivity = 
                 (float)statistics[statsIndex].true_pos_count /
                 ((float)statistics[statsIndex].true_pos_count + 
                 (float)statistics[statsIndex].contradict_count +
                 (float)statistics[statsIndex].conflict_count);
                 statistics[statsIndex].pos_pred_value = 
                 (float)statistics[statsIndex].true_pos_count /
                 ((float)statistics[statsIndex].true_pos_count + 
                 (float)statistics[statsIndex].contradict_count +
                 (float)statistics[statsIndex].conflict_count + 
                 (float)statistics[statsIndex].compatible_count);*/
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
	buff->append(reference->GetFilename());
	buff->append("\n\n");
	buff->append(
                 "Filename\t\t\tPairs\tTPs\tFPs\tFNs\tSensi.\tSelec.\tPPV\tConfl.\tContr.\tCompa.\tG-C\tA-U\tG-U\tOther\n"
                 );
	
	for (unsigned int ui=0; ui<numStats; ui++)
	{
		
		char tempc [25];
		strncpy(tempc,statistics[ui].filename,24);
		tempc[24] = '\0';
		buff->append(tempc);
		if (strlen(statistics[ui].filename) > 24) 
		{
			buff->append("...\t");
		}
		else
		{
			buff->append("\t");
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
        if (ui == numStats-1) {
            sprintf(tempc,"%d",statistics[ui].non_canon_count);
        }
        else {
            sprintf(tempc,"%d\n",statistics[ui].non_canon_count);
        }
		buff->append(tempc);
	}
	
	int colors[7] = {FL_BLUE, FL_GREEN, FL_RED, FL_YELLOW, FL_CYAN, FL_MAGENTA,
		         FL_WHITE};
    
	for (unsigned int ui = 0; ui < numStats; ui++)
	{
		if (statistics[ui].ref) 
		{
			statistics[ui].color = GUI_TEXT_COLOR;
		}
		else
		{
			statistics[ui].color = colors[ui%7];
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
	for (unsigned int ui = 0; ui < numStats; ui++)
	{
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
		
		for (unsigned int ui = 0; ui < numStats; ui++)
		{
			Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
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
		
		for (unsigned int ui = 0; ui < numStats; ui++)
		{
			Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].true_pos_count),
                                     col_w,
                                     (int)(hscale*statistics[ui].true_pos_count),statistics[ui].tp_char);
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
		
		for (unsigned int ui = 0; ui < numStats; ui++)
		{
			Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
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
		
		for (unsigned int ui = 0; ui < numStats; ui++)
		{
			Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
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
		
		for (unsigned int ui = 0; ui < numStats; ui++)
		{
			Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].sensitivity),
                                     col_w,
                                     (int)(hscale*statistics[ui].sensitivity),statistics[ui].sens_char);
			box->color(statistics[ui].color);
			box->align(FL_ALIGN_TOP);
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
		
		for (unsigned int ui = 0; ui < numStats; ui++)
		{
			Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].selectivity),
                                     col_w,
                                     (int)(hscale*statistics[ui].selectivity),statistics[ui].sel_char);
			box->color(statistics[ui].color);
			box->align(FL_ALIGN_TOP);
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
		
		for (unsigned int ui = 0; ui < numStats; ui++)
		{
			Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].pos_pred_value),
                                     col_w,
                                     (int)(hscale*statistics[ui].pos_pred_value),statistics[ui].ppv_char);
			box->color(statistics[ui].color);
			box->align(FL_ALIGN_TOP);
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
		
		for (unsigned int ui = 0; ui < numStats; ui++)
		{
			Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].base_pair_count),
                                     col_w,
                                     (int)(hscale*statistics[ui].base_pair_count),statistics[ui].bp_char);
			box->color(FL_BACKGROUND_COLOR);
			box->align(FL_ALIGN_TOP);
            
			box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
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
		
		for (unsigned int ui = 0; ui < numStats; ui++)
		{
			Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].base_pair_count),
                                     col_w,
                                     (int)(hscale*statistics[ui].base_pair_count),statistics[ui].bp_char);
			box->color(FL_BACKGROUND_COLOR);
			box->align(FL_ALIGN_TOP);
            
			box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
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
		
		for (unsigned int ui = 0; ui < numStats; ui++)
		{
			Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].base_pair_count),
                                     col_w,
                                     (int)(hscale*statistics[ui].base_pair_count),statistics[ui].bp_char);
			box->color(FL_BACKGROUND_COLOR);
			box->align(FL_ALIGN_TOP);
            
			box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
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
		
		for (unsigned int ui = 0; ui < numStats; ui++)
		{
			Fl_Box *box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
                                     cbh + cby - (int)(hscale * statistics[ui].base_pair_count),
                                     col_w,
                                     (int)(hscale*statistics[ui].base_pair_count),statistics[ui].bp_char);
			box->color(FL_BACKGROUND_COLOR);
			box->align(FL_ALIGN_TOP);
            
			box = new Fl_Box(FL_BORDER_BOX,cbx+ui*col_w,
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
		
		for (unsigned int ui=0; ui < numStats; ui++)
		{
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
		for (unsigned int ui=0; ui < numStats; ui++)
		{
			if (statistics[ui].ref)
			{	
				leg1_ref->label(statistics[ui].filename);
				leg1_ref->show();
			}
			else
			{
				Fl_Box* entry = new Fl_Box(FL_FLAT_BOX,
                                           leg1_pack->x(),
                                           leg1_pack->y()+k*20,
                                           leg1_pack->w(),
                                           20,
                                           statistics[ui].filename);
				entry->color(statistics[ui].color);
				entry->labelcolor(GUI_TEXT_COLOR);
				entry->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
				k++;
			}
		}
	}
	leg1_pack->end();
    
	leg2_pack->begin();
    {
        unsigned int k = 0;
        for (unsigned int ui=0; ui < numStats; ui++)
        {
            if (statistics[ui].ref)
            {	
                leg2_ref->label(statistics[ui].filename);
                leg2_ref->show();
            }
            else
            {
                Fl_Box* entry = new Fl_Box(FL_FLAT_BOX,
                                           leg2_pack->x(),
                                           leg2_pack->y()+k*20,
                                           leg2_pack->w(),
                                           20,
                                           statistics[ui].filename);
                entry->color(statistics[ui].color);
                entry->labelcolor(GUI_TEXT_COLOR);
                entry->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
                k++;
            }
        }
    }
    leg2_pack->end();
    
    leg3_pack->begin();
    {
        unsigned int k = 0;
        for (unsigned int ui=0; ui < numStats; ui++)
        {
            if (statistics[ui].ref)
            {	
                leg3_ref->label(statistics[ui].filename);
                leg3_ref->show();
            }
            else
            {
                Fl_Box* entry = new Fl_Box(FL_FLAT_BOX,
                                           leg3_pack->x(),
                                           leg3_pack->y()+k*20,
                                           leg3_pack->w(),
                                           20,
                                           statistics[ui].filename);
                entry->color(statistics[ui].color);
                entry->labelcolor(GUI_TEXT_COLOR);
                entry->align(FL_ALIGN_INSIDE|FL_ALIGN_LEFT);
                k++;
            }
        }
    }
    leg3_pack->end();
    
    leg4_pack->begin();
    {
        unsigned int k = 0;
        for (unsigned int ui=0; ui < numStats; ui++)
        {
            if (statistics[ui].ref)
            {	
                leg4_ref->label(statistics[ui].filename);
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
                    Fl_Box* entry = new Fl_Box(FL_FLAT_BOX,
                                               leg4_pack->x()+20,
                                               leg4_pack->y()+k*20,
                                               leg4_pack->w()-20,
                                               20,
                                               statistics[ui].filename);
                    entry->color(statistics[ui].color);
                    entry->labelcolor(GUI_TEXT_COLOR);
                    entry->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
                    // Add symbol label in color
                    entry = new Fl_Box(FL_NO_BOX,leg4_pack->x(),leg4_pack->y()+k*20,
                                       20,20,"");
                    entry->labelcolor(statistics[ui].color);
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
	snprintf(filename, MAX_BUFFER_SIZE - 1, "%s/StatsTableOutput-%s.dat", 
		 PNG_OUTPUT_DIRECTORY, dateStamp);
	input_window = new InputWindow(400, 150, "Export Table To File ...",
                                   filename, InputWindow::FILE_INPUT);
	exp_button->value(1);
        exp_button->deactivate();
	while (input_window != NULL && input_window->visible())
	{
		Fl::wait();
	}
    
    if(input_window != NULL)
    {
        
        if(strcmp(input_window->getName(),""))
        {
            strncpy(filename, input_window->getName(), MAX_BUFFER_SIZE - 1);
            strcat(filename, ".txt");
        }
        else {}

	FILE * expFile = fopen(filename, "a+");
        if (expFile != NULL)
        {
            /* 	Check whether file is empty (i.e. whether it already existed. 
             If it did, add a line (or something) to denote starting a new table */
            rewind(expFile);
            int checkChar = fgetc(expFile);
            if (checkChar != EOF)
            {
                fprintf(expFile,"\n");
            }
            
            // print the column header
            fprintf(expFile,
                    "Filename\tReference\tBase_Pairs\tTrue_Positive\tFalse_Positive\t");
            fprintf(expFile,
                    "False_Negative\tConflict\tContradict\tCompatible\tSensitivity\t");
            fprintf(expFile,
                    "Selectivity\tPositive_Predictive_Value\t");
            fprintf(expFile,
                    "G-C_Pairs\tA-U_Pairs\tG-U_Pairs\tNon-Canonical_Pairs\n"); 
            
            for (unsigned int ui=0; ui<numStats; ui++)
            {
                // print the row of statistics for each structure
                fprintf(expFile,
                        "%s\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%d\t%.10f\t%.10f\t%.10f\t%d\t%d\t%d\t%d\n",
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
        delete input_window;
        input_window = NULL;
    }
    
	exp_button->value(0);
    exp_button->activate();
    
    menu_window->activate();
    tab_window->activate();
}

void StatsWindow::BuildRefMenu()
{
	
	ref_menu->clear();
	ref_menu->add("Please select a reference",0,0,0,0);
	
    
	// Add entries
    for (unsigned int ui = 0; ui < m_structures.size(); ++ui)
    {
		RNAStructure* structure = 
        structureManager->GetStructure(m_structures[ui]);
		
		ref_menu->add(structure->GetFilename(),0,0,0,0);
    }
    
    ref_menu->value(0);
    ref_menu->redraw();
    
    calc_button->value(1);
    calc_button->deactivate();
}

void StatsWindow::BuildCompMenu()
{
	comp_pack->clear();
	comp_pack->begin();
	{
		for (unsigned int ui=0; ui < m_structures.size(); ui++)
		{
			RNAStructure* structure = 
            structureManager->GetStructure(m_structures[ui]);
			if (strcmp(ref_menu->mvalue()->label(),structure->GetFilename()))
			{
				Fl_Check_Button* button = new Fl_Check_Button(comp_pack->x(),
                                                              comp_pack->y()+30, comp_pack->w(), 30, 
                                                              structure->GetFilename());
				button->callback(MenuCallback);
			}
		}
	}
	comp_pack->end();
}
