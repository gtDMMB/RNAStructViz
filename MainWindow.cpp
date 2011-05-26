#include "MainWindow.h"
#include "RNAStructViz.h"
#include <unistd.h>
#include <iostream>
#include <FL/Fl_Box.h>
#include <FL/Fl_Check_Button.h>
#include <FL/Fl_Round_Button.h>

MainWindow* MainWindow::ms_instance = 0;

MainWindow::MainWindow(int argc, char **argv)
    : m_fileChooser(0)
{
    m_mainWindow = new Fl_Window(300, 400);
    m_mainWindow->callback(CloseCallback);

    Fl_Box* resizableBox = new Fl_Box(0, 60, 300, 340);
    m_mainWindow->resizable(resizableBox);

    Fl_Button* openButton = new Fl_Button(0, 0, 75, 30, "Open");
    openButton->callback(OpenFileCallback);

    Fl_Button* diagramButton = new Fl_Button(75, 0, 75, 30, "Diagram");
    diagramButton->callback(DiagramCallback);

    // The structure manager information
    Fl_Box* label1 = new Fl_Box(0, 30, 30, 30, "Ref");
    Fl_Box* label2 = new Fl_Box(30, 30, 150, 30, "File name");

    m_mainWindow->begin();
    m_structureInfo = new Fl_Scroll(0, 60, 300, 340);
    m_structureInfo->type(Fl_Scroll::VERTICAL_ALWAYS);
    m_packedInfo = new Fl_Pack(0, 60, 280, 340);
    m_packedInfo->type(Fl_Pack::VERTICAL);

    m_mainWindow->size_range(300, 400, 350);

    m_mainWindow->show(argc, argv);
}

MainWindow::~MainWindow()
{
    if (m_fileChooser)
	delete m_fileChooser;
    delete m_mainWindow;
}

bool MainWindow::Initialize(int argc, char **argv)
{
    if (ms_instance)
	return true;

    ms_instance = new MainWindow(argc, argv);

    return true;
}

void MainWindow::Shutdown()
{
    if (ms_instance)
    {
	delete ms_instance;
	ms_instance = 0;
    }
}

void MainWindow::AddStructure(const char* filename, const int index, const bool isReference)
{
    Fl_Pack* pack = ms_instance->m_packedInfo;
    pack->begin();

    int vertPosn = pack->children() * 30 + pack->y();

    Fl_Group* group = new Fl_Group(pack->x(), vertPosn, pack->w(), 30);

    Fl_Round_Button* refButton = new Fl_Round_Button(pack->x() + 5, vertPosn + 5, 20, 20);
    if (isReference)
	refButton->set();
    refButton->callback(MainWindow::ReferenceCallback);
    refButton->user_data((void*)index);
   
    Fl_Button* label = new Fl_Button(pack->x() + 30, vertPosn, pack->w() - 50, 30, filename);
    label->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    label->callback(MainWindow::ShowFileCallback);
    label->user_data((void*)index);

    Fl_Button* removeButton = new Fl_Button(pack->x() + pack->w() - 20, vertPosn + 5, 20, 20);
    removeButton->callback(MainWindow::RemoveCallback);
    removeButton->user_data((void*)index);
    removeButton->label("@1+");

    group->resizable(label);

    const std::vector<DiagramWindow*>& diagrams = RNAStructViz::GetInstance()->GetDiagramWindows();
    for (unsigned int ui = 0; ui < diagrams.size(); ++ui)
    {
	diagrams[ui]->AddStructure(index);
    }

    ms_instance->m_structureInfo->redraw();
}

void MainWindow::OpenFileCallback(Fl_Widget* widget, void* userData)
{
    if (!ms_instance->m_fileChooser)
	ms_instance->CreateFileChooser();

    ms_instance->m_fileChooser->show();

    while (ms_instance->m_fileChooser->visible())
	Fl::wait();

    for (int i = 1; i <= ms_instance->m_fileChooser->count(); ++i)
    {
	RNAStructViz::GetInstance()->GetStructureManager()->AddFile(ms_instance->m_fileChooser->value(i));
    }
}

void MainWindow::DiagramCallback(Fl_Widget* widget, void* userData)
{
    RNAStructViz::GetInstance()->AddDiagramWindow();
}

bool MainWindow::CreateFileChooser()
{
    if (m_fileChooser)
	return true;

    // Get the current working directory.
    char currentWD[4096];
    if (!getcwd(currentWD, 4096))
    {
	std::cerr << "Error: getcwd failed. Cannot create file chooser.\n";
	return false;
    }

    m_fileChooser = new Fl_File_Chooser(currentWD, "*.{ct,bpseq}", Fl_File_Chooser::MULTI, "Select RNA Sequences");

    return true;
}

void MainWindow::ReferenceCallback(Fl_Widget* widget, void* userData)
{
    Fl_Button* newActive = (Fl_Button*)widget;
    if (newActive->value() == 0)
    {
	// Unset the current active.
	RNAStructViz::GetInstance()->GetStructureManager()->SetReferenceStructure(-1);
	return;
    }

    Fl_Pack* pack = ms_instance->m_packedInfo;

    Fl_Button* currentActive = 0;
    for (int i = 0; i < pack->children(); ++i)
    {
	Fl_Button* childButton = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0));
	if (childButton != newActive && ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0))->value() == 1)
	{
	    currentActive = childButton;
	}
    }

    if (currentActive)
	currentActive->value(0);

    RNAStructViz::GetInstance()->GetStructureManager()->SetReferenceStructure((int)userData);
}

void MainWindow::ShowFileCallback(Fl_Widget* widget, void* userData)
{
    RNAStructViz::GetInstance()->GetStructureManager()->DisplayFileContents((int)userData);
}

void MainWindow::RemoveCallback(Fl_Widget* widget, void* userData)
{
    // Find the group with this child
    Fl_Pack* pack = ms_instance->m_packedInfo;
    for (int i = 0; i < pack->children(); ++i)
    {
	Fl_Button* childButton = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(2));
	if (childButton == widget)
	{
	    RNAStructViz* appInstance = RNAStructViz::GetInstance();

	    const std::vector<DiagramWindow*>& diagrams = appInstance->GetDiagramWindows();
	    for (unsigned int ui = 0; ui < diagrams.size(); ++ui)
	    {
		diagrams[ui]->RemoveStructure((int)userData);
	    }

	    appInstance->GetStructureManager()->RemoveStructure((int)userData);

	    Fl_Group* toRemove = (Fl_Group*)pack->child(i);
	    pack->remove(toRemove);
	    ms_instance->m_structureInfo->redraw();

	    Fl::delete_widget(toRemove);
	}
    }
}

void MainWindow::CloseCallback(Fl_Widget* widget, void* userData)
{
    exit(0);
}
