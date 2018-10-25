#include "InputWindow.h"
#include "ConfigOptions.h"
#include "FL/Fl_Input.H"
#include "FL/Fl_Button.H"
#include "FL/Fl_Box.H"
#include <stdio.h>
#include <stdlib.h>
#include <string>

InputWindow::InputWindow(int w, int h, const char *label, 
	const char *defaultName, InputWindowType type) : Fl_Window(w, h, label)
{	
    string = (char*)malloc(sizeof(char)*90);
    color(GUI_WINDOW_BGCOLOR);
    
    if (type == InputWindow::FILE_INPUT)
    {
    	sprintf(string, "Current name: %s", defaultName);
		input = new Fl_Input(160, 50, 100, 30, "Export File Name:");
		input->when(FL_WHEN_ENTER_KEY);
	    Fl_Box *box = new Fl_Box(150, 20, 100, 30, (const char*)string);
	    box->box(FL_NO_BOX);
	    box->align(FL_ALIGN_CENTER);
	    new Fl_Box(260,50,30,30,".txt");
	    Fl_Button *button = new Fl_Button(305, 50, 50, 30, "OK");
	    button->callback(ButtonCallback, (void*)0);
	    input->callback(InputCallback, (void*)0);
	    callback(CloseCallback);
    }
    else
    {
	    sprintf(string, "Creating folder for the RNA sequence of %s", defaultName);
	    input = new Fl_Input(160, 50, 100, 30, "@fileopen New Folder Name:");
	    input->when(FL_WHEN_ENTER_KEY);
            input->maximum_size(60);
	    Fl_Box *box = new Fl_Box(50, 10, 300, 40, (const char*)string);
	    box->box(FL_NO_BOX);
	    box->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
	    Fl_Button *button = new Fl_Button(265, 50, 100, 30, "Add Folder @arrow");
	    button->callback(ButtonCallback, (void*)0);
	    button->labelcolor(GUI_BTEXT_COLOR);
	    input->callback(InputCallback, (void*)0);
	    callback(CloseCallback);
            Fl_Box* message = new Fl_Box(50,100,300,40,"All structures with the same underlying RNA sequence will be put in this folder.");
            message->box(FL_NO_BOX);
	    message->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
	}
	show();
}

void InputWindow::InputCallback(Fl_Widget *widget, void *userdata)
{
    InputWindow *window = (InputWindow*)widget->parent();
    window->name = (char*)window->input->value();
    //int dotIndex = std::string(window->name).rfind('.');
    //if(dotIndex > 0) {
    //    window->name[dotIndex] = '\0';
    //}
    free(window->string);
    window->hide();
}

void InputWindow::ButtonCallback(Fl_Widget *widget, void *userdata)
{
    InputWindow *window = (InputWindow*)widget->parent();
    window->name = (char*)window->input->value();
    free(window->string);
    window->hide();
}

void InputWindow::CloseCallback(Fl_Widget* widget, void* userData)
{
    InputWindow *window = (InputWindow*)widget;
    window->name = "";
    free(window->string);    
    window->hide();
}
