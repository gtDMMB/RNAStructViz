#include "GraphWindow.h"
#include <FL/Fl.H>
#include <stdio.h>
#include <stdlib.h>
int main(int argc, char **argv)
{
	char *title = (char*)malloc(sizeof(char) * 16);
	sprintf(title, "Statistics");
    //int* arr = { 32, 24, 53 };
    //GraphWindow* graph = new GraphWindow(400, 400, title, arr);
	GraphWindow* graph = new GraphWindow(600,600, title);
	return Fl::run();
}
