#include <FL/Fl_Input.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Box.H>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <string>

#include "InputWindow.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"

InputWindow::InputWindow(int w, int h, const char *label, 
	const char *defaultName, InputWindowType type) : Fl_Window(w, h, label)
{	
    string = (char*)malloc(sizeof(char)*90);
    color(GUI_WINDOW_BGCOLOR);
    set_modal();
    
    std::string actualStructName = ExtractStructureNameFromCTName(defaultName);
    const char *actualStructNameCStr = actualStructName.c_str();
    inputText = (char *) malloc(MAX_BUFFER_SIZE * sizeof(char));
    strncpy(inputText, actualStructNameCStr, actualStructName.size() + 1);
    ConfigParser::nullTerminateString(inputText, actualStructName.size());

    if (type == InputWindow::FILE_INPUT)
    {
    	    sprintf(string, "Current name: %s", defaultName);
	    input = new Fl_Input(160, 50, 100, 30, "Export File Name:");
	    input->when(FL_WHEN_ENTER_KEY);
	    input->value(inputText);
	    Fl_Box *box = new Fl_Box(150, 20, 100, 30, (const char*)string);
	    box->box(FL_NO_BOX);
	    box->align(FL_ALIGN_CENTER);
	    new Fl_Box(260,50,30,30,".txt");
	    Fl_Button *button = new Fl_Button(305, 50, 50, 30, "OK");
	    button->callback(InputCallback, (void*)0);
	    button->set_active();
	    input->callback(InputCallback, (void*)0);
	    callback(CloseCallback);
    }
    else
    {
	    sprintf(string, "Creating new folder for the structure %s", defaultName);
	    input = new Fl_Input(160, 50, 100, 30, "@fileopen  New Folder Name:");
	    input->when(FL_WHEN_ENTER_KEY);
            input->maximum_size(60);
	    input->value(inputText);
	    input->color(GUI_BGCOLOR);
	    input->textcolor(GUI_BTEXT_COLOR);
	    Fl_Box *box = new Fl_Box(50, 1, 300, 40, (const char*)string);
	    box->box(FL_OSHADOW_BOX);
	    box->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_WRAP);
	    box->color(GUI_BGCOLOR);
	    box->labelcolor(GUI_BTEXT_COLOR);
	    Fl_Button *button = new Fl_Button(265, 50, 100, 30, "Add Folder @|>");
	    button->callback(InputCallback, (void*)0);
	    button->labelcolor(GUI_BTEXT_COLOR);
	    button->set_active();
	    input->callback(InputCallback, (void*)0);
	    input->labelcolor(GUI_TEXT_COLOR);
	    const char *cbText = " Use only default names for structure folders";
	    cbUseDefaultNames = new Fl_Check_Button(30, 100, 325, 30, cbText);
	    cbUseDefaultNames->box(FL_PLASTIC_UP_BOX);
	    cbUseDefaultNames->color(GUI_BGCOLOR);
	    cbUseDefaultNames->labelcolor(GUI_BTEXT_COLOR);
	    cbUseDefaultNames->down_color(GUI_WINDOW_BGCOLOR);
	    cbUseDefaultNames->align(FL_ALIGN_LEFT | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	    callback(CloseCallback);
	}
	if(!GUI_USE_DEFAULT_FOLDER_NAMES) {
            show();
	}
	else {
	    show();
	    InputCallback((Fl_Widget *) cbUseDefaultNames, (void *) NULL);
	}
}

InputWindow::~InputWindow() {
    delete input;
    delete cbUseDefaultNames;
    free(inputText);
}

void InputWindow::InputCallback(Fl_Widget *widget, void *userdata)
{
    InputWindow *window = (InputWindow*)widget->parent();
    if(window->inputText != (char*)window->input->value()) {
        strcpy(window->inputText, (char*)window->input->value());
    }
    window->name = window->inputText;
    if(window->cbUseDefaultNames->value()) {
        GUI_USE_DEFAULT_FOLDER_NAMES = true;
    }
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

std::string InputWindow::ExtractStructureNameFromCTName(const char *ctPath) {
    std::string structName(ctPath);
    int structureNameNoPrefix = structName.find_first_of('_');
    if(structureNameNoPrefix >= 0) {
        structName = structName.substr(structureNameNoPrefix + 1);;
    }
    int dotIndexPtr = structName.find_last_of('.');
    if(dotIndexPtr >= 0) {
        structName = structName.substr(0, dotIndexPtr);
    }
    return structName; 
}
