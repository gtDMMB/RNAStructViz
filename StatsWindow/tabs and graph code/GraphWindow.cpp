#include "GraphWindow.h"
#include <FL/Fl_Window.H>
#include <FL/fl_draw.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Group.H>
#include <stdio.h>
#include <stdlib.h>
#include <vector>

GraphWindow::GraphWindow(int wid, int hgt, const char *label) : Fl_Window(wid, hgt, label)
{
    t_structs = (TestStruct*)malloc(sizeof(TestStruct)*3);
    t_structs[0].name = (char*)malloc(sizeof(char)*16);
    sprintf(t_structs[0].name, "%s", "struct1");
    t_structs[0].basePairs = 32;
    t_structs[0].falsePositives = 0;
    t_structs[0].truePositives = 32;
    t_structs[0].referenceStruct = true;
    
    t_structs[1].name = (char*)malloc(sizeof(char)*16);
    sprintf(t_structs[1].name, "%s", "struct2");
    t_structs[1].basePairs = 24;
    t_structs[1].falsePositives = 12;
    t_structs[1].truePositives = 12;
    t_structs[1].referenceStruct = false;

    t_structs[2].name = (char*)malloc(sizeof(char)*16);
    sprintf(t_structs[2].name, "%s", "struct3");
    t_structs[2].basePairs = 53;
    t_structs[2].falsePositives = 32;
    t_structs[2].truePositives = 21;
    t_structs[2].referenceStruct = false;
    
    splitGraph = (int)((w()/2)/3);
    graphX = (int)(w()/2-w()/4);
    graphY = (int)(h()/2-h()/4);
    
    //fl_color(FL_BLACK);
    tabs = new Fl_Tabs(0, 0, wid, hgt);
    { group[0] = new Fl_Group(0, 20, wid-10, hgt-10, "Base Pairs");
        {            
            Fl_Box* box = new Fl_Box((int)(w()/2-w()/4), (int)(h()/2-h()/4), w()/2 - (splitGraph*2), h()/2);
            box->box(FL_BORDER_BOX);
            resizable(box);
            Fl_Box* innerBox1 = new Fl_Box((int)(w()/2-w()/4)+ splitGraph, (int)(h()/2-h()/4), w()/2 - (splitGraph*2), h()/2);
            innerBox1->box(FL_BORDER_BOX);
            resizable(innerBox1);
            Fl_Box* innerBox2 = new Fl_Box((int)(w()/2-w()/4) + (splitGraph*2), (int)(h()/2-h()/4), w()/2 - (splitGraph*2), h()/2);
            innerBox2->box(FL_BORDER_BOX);
            resizable(innerBox2);
            
            Fl_Box* bases = new Fl_Box(graphX + 40,(int)(h()/2 + h()/4) + 5 ,10,10, "Base Pairs");
            bases->box(FL_NO_BOX);
            resizable(bases);
            Fl_Box* trues = new Fl_Box(graphX + splitGraph + 40,(int)(h()/2 + h()/4) + 5 ,10,10, "True Pos");
            trues->box(FL_NO_BOX);
            resizable(trues);
            Fl_Box* falses = new Fl_Box(graphX + (splitGraph*2) + 40,(int)(h()/2 + h()/4) + 5 ,10,10, "False Pos");
            falses->box(FL_NO_BOX);
            resizable(falses);
        }
        group[0]->end();
        resizable(group[0]);
    }
    {   group[1] = new Fl_Group(0, 20, 540, 125, "tab2");
        group[1]->hide();
        {   
            Fl_Box *box2 = new Fl_Box(50, 40, 70, 20,"box2");
            box2->box(FL_NO_BOX);
        }
        group[1]->end();
        resizable(group[1]);
    }
    tabs->end();
    resizable(tabs);
    selectedTab = 1;
    show();
}

void GraphWindow::draw()
{
	Fl_Window::draw();
    if((Fl_Group*)tabs->value() == group[0])
    {
        selectedTab = 1;
    }
    else if ((Fl_Group*)tabs->value() == group[1])
    {
        selectedTab = 2;
    }
    //printf("selectedTab: %d\n", selectedTab);
    DrawTab(selectedTab);
}

void GraphWindow::DrawTab(int sel)
{
    if(sel == 1)
    {
        DrawGraphLabels();
        DrawGraph();
    }
    
}


void GraphWindow::DrawGraph()
{
    int yPos = 5;
    int wid = (int)(splitGraph/3) - 5;
    char *datalength = (char*)malloc(sizeof(char)*16);
    splitGraph = (int)((w()/2)/3);
    //printf("splitGraph: %d\n", splitGraph);
    
    //Base Pairs
	fl_color(FL_RED);
	fl_rectf((int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(t_structs[0].basePairs*2+1), wid, t_structs[0].basePairs*2);
    //fl_color(FL_BLACK);
    //fl_rectf((int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(2*10), wid, 1);
	yPos += wid + 1;
	fl_color(FL_BLUE);	
	fl_rectf((int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(t_structs[1].basePairs*2+1), wid, t_structs[1].basePairs*2);
	yPos += wid + 1;
	fl_color(160,32, 240); //Purple
	fl_rectf((int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(t_structs[2].basePairs*2+1), wid, t_structs[2].basePairs*2);
    sprintf(datalength, "%d", t_structs[2].basePairs);
    fl_draw(datalength,(int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(t_structs[2].basePairs*2+5));
    //fl_color(FL_BLACK);
    //fl_rectf((int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(2*50), wid, 1);
    
    //True Positives
    yPos = splitGraph + 5;
    fl_color(FL_RED);
	fl_rectf((int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(t_structs[0].truePositives*2+1), wid, t_structs[0].truePositives*2);
	yPos += wid + 1;
	fl_color(FL_BLUE);	
	fl_rectf((int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(t_structs[1].truePositives*2+1), wid, t_structs[1].truePositives*2);
	yPos += wid + 1;
	fl_color(160,32, 240); //Purple
	fl_rectf((int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(t_structs[2].truePositives*2+1), wid, t_structs[2].truePositives*2);
    sprintf(datalength, "%d", t_structs[2].truePositives);
    fl_draw(datalength,(int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(t_structs[2].truePositives*2+5));
    
    //False Positives
    yPos = (splitGraph*2) + 5;
    fl_color(FL_RED);
	fl_rectf((int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(t_structs[0].falsePositives*2+1), wid, t_structs[0].falsePositives*2);
	yPos += wid + 1;
	fl_color(FL_BLUE);	
	fl_rectf((int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(t_structs[1].falsePositives*2+1), wid, t_structs[1].falsePositives*2);
	yPos += wid + 1;
	fl_color(160,32, 240); //Purple
	fl_rectf((int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(t_structs[2].falsePositives*2+1), wid, t_structs[2].falsePositives*2);
    sprintf(datalength, "%d", t_structs[2].falsePositives);
    fl_draw(datalength,(int)(w()/2-w()/4)+yPos, (int)(h()/2-h()/4)+h()/2-(t_structs[2].falsePositives*2+5));
    
}

void GraphWindow::DrawGraphLabels()
{
    int num = 0;
    char *graphLabel = (char*)malloc(sizeof(char)*16);
    sprintf(graphLabel, "%d", num);
    
    fl_color(FL_BLACK);
    fl_draw(graphLabel,(int)(w()/2-w()/4)-10, (int)(h()/2-h()/4)+h()/2+5);
    num += 10;
    sprintf(graphLabel, "%d", num);
    fl_draw(graphLabel,(int)(w()/2-w()/4)-20, (int)(h()/2-h()/4)+h()/2+5-(num*2));
    num += 10;
    sprintf(graphLabel, "%d", num);
    fl_draw(graphLabel,(int)(w()/2-w()/4)-20, (int)(h()/2-h()/4)+h()/2+5-(num*2));
    num += 10;
    sprintf(graphLabel, "%d", num);
    fl_draw(graphLabel,(int)(w()/2-w()/4)-20, (int)(h()/2-h()/4)+h()/2+5-(num*2));
    num += 10;
    sprintf(graphLabel, "%d", num);
    fl_draw(graphLabel,(int)(w()/2-w()/4)-20, (int)(h()/2-h()/4)+h()/2+5-(num*2));
    num += 10;
    sprintf(graphLabel, "%d", num);
    fl_draw(graphLabel,(int)(w()/2-w()/4)-20, (int)(h()/2-h()/4)+h()/2+5-(num*2));
    num += 10;
    sprintf(graphLabel, "%d", num);
    fl_draw(graphLabel,(int)(w()/2-w()/4)-20, (int)(h()/2-h()/4)+h()/2+5-(num*2));
    num += 10;
    sprintf(graphLabel, "%d", num);
    fl_draw(graphLabel,(int)(w()/2-w()/4)-20, (int)(h()/2-h()/4)+h()/2+5-(num*2));
    num += 10;
    sprintf(graphLabel, "%d", num);
    fl_draw(graphLabel,(int)(w()/2-w()/4)-20, (int)(h()/2-h()/4)+h()/2+5-(num*2));
    num += 10;
    sprintf(graphLabel, "%d", num);
    fl_draw(graphLabel,(int)(w()/2-w()/4)-20, (int)(h()/2-h()/4)+h()/2+5-(num*2));
    num += 10;
    sprintf(graphLabel, "%d", num);
    fl_draw(graphLabel,(int)(w()/2-w()/4)-30, (int)(h()/2-h()/4)+h()/2+5-(num*2));
    
}
