#include "MainWindow.h"
#include "RNAStructViz.h"
#include "DisplayConfigWindow.h"
#include "ConfigOptions.h"
#include "ConfigParser.h"

#include <unistd.h>
#include <iostream>
#include <vector>

#include <FL/Fl.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Round_Button.H>
#include <FL/Fl_Pixmap.H>
#include <FL/Fl_PNG_Image.H>

#include "pixmaps/RNAStructVizLogo.c"
#include "pixmaps/RNAWindowIcon.xbm"
#include "pixmaps/HelpIcon.c"

MainWindow* MainWindow::ms_instance = 0;

Fl_RGB_Image * MainWindow::helpIconImage = new Fl_RGB_Image( 
	       HelpIcon.pixel_data, 
	       HelpIcon.width, HelpIcon.height, HelpIcon.bytes_per_pixel);

MainWindow::MainWindow(int argc, char **argv)
          : m_fileChooser(NULL), selectedFolderBtn(NULL), 
	    selectedFolderIndex(-1)
{
    m_mainWindow = new Fl_Double_Window(650, 450, RNASTRUCTVIZ_VSTRING);
    m_mainWindow->size_range(650, 450, 650, 450);
    m_mainWindow->callback(CloseCallback);
    m_mainWindow->color(GUI_WINDOW_BGCOLOR);
    m_mainWindow->begin();    

    mainMenuPane = new Fl_Group(0,0,300,450,"");
    {
        
	    // setup the program logo (for now dynamically loaded):
            Fl_RGB_Image *appLogo = new Fl_RGB_Image( 
			  RNAStructVizLogo.pixel_data, 
			  RNAStructVizLogo.width, 
			  RNAStructVizLogo.height, 
			  RNAStructVizLogo.bytes_per_pixel);
	    Fl_Box *appLogoCont = new Fl_Box(NAVBUTTONS_OFFSETX, 
	           NAVBUTTONS_OFFSETY, appLogo->w(), appLogo->h());
	    appLogoCont->image(appLogo);

	    int helpButtonDims = 26;
	    int helpButtonOffsetX = 300 - helpButtonDims - NAVBUTTONS_SPACING;
	    Fl_Button *helpButton = new Fl_Button(helpButtonOffsetX, NAVBUTTONS_SPACING, 
			                          helpButtonDims, helpButtonDims, "");
	    helpButton->color(GUI_WINDOW_BGCOLOR);
	    helpButton->labelcolor(GUI_BTEXT_COLOR);
	    helpButton->labelsize(2 * LOCAL_TEXT_SIZE);
	    helpButton->image(helpIconImage);
	    helpButton->deimage(helpIconImage);
	    helpButton->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
	    helpButton->labeltype(_FL_ICON_LABEL);
	    helpButton->shortcut(FL_CTRL + 'h');
	    helpButton->box(FL_NO_BOX);
	    helpButton->callback(HelpButtonCallback);
            helpButton->redraw();

	// consistent alignment with the folder window display:
	int upperYOffset = NAVBUTTONS_OFFSETY + appLogo->h() + 5; //49;
	int dividerTextHeight = 4;

    	// make it more user friendly by including some help text: 
	const char *navInstText = "@refresh Actions.\nEach expands into a new window.";
    	int navButtonsLabelHeight = 2 * NAVBUTTONS_BHEIGHT;
        actionsLabel = new Fl_Box(NAVBUTTONS_OFFSETX, upperYOffset, 
		                  2 * NAVBUTTONS_BWIDTH + 2 * NAVBUTTONS_SPACING, 
	                          navButtonsLabelHeight, navInstText); 	
        actionsLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
        actionsLabel->color(GUI_BGCOLOR);
	actionsLabel->labelcolor(GUI_BTEXT_COLOR);
	actionsLabel->labelfont(LOCAL_BFFONT);
	actionsLabel->labelsize(LOCAL_TEXT_SIZE);
	actionsLabel->box(FL_RSHADOW_BOX);

        //Open button
        openButton = new Fl_Button(NAVBUTTONS_OFFSETX, 
		                        NAVBUTTONS_OFFSETY + upperYOffset + navButtonsLabelHeight, 
		                        NAVBUTTONS_BWIDTH, NAVBUTTONS_BHEIGHT, 
		                        "@search   Load Files @>|");
        openButton->callback(OpenFileCallback);
        openButton->labelcolor(GUI_BTEXT_COLOR);

       	// The button to open configuration settings:
	configOptionsButton = new Fl_Button( 
	          NAVBUTTONS_OFFSETX + NAVBUTTONS_BWIDTH + NAVBUTTONS_SPACING, 
		  NAVBUTTONS_OFFSETY + upperYOffset + navButtonsLabelHeight, 
		          NAVBUTTONS_BWIDTH, NAVBUTTONS_BHEIGHT, 
		  "@menu   Config Options @>|");
	configOptionsButton->callback(ConfigOptionsCallback);
        configOptionsButton->labelcolor(GUI_BTEXT_COLOR);

	const char *foldersInstText = "@fileopen Folders.\nA list of structures for which\nCT files are currently loaded.";
        columnLabel = new Fl_Box(NAVBUTTONS_OFFSETX, 
		      50 + upperYOffset + navButtonsLabelHeight + 
	              dividerTextHeight, 2 * NAVBUTTONS_BWIDTH + 2 * NAVBUTTONS_SPACING, 
		      navButtonsLabelHeight, foldersInstText);
        columnLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
        columnLabel->color(GUI_BGCOLOR);
	columnLabel->labelcolor(GUI_BTEXT_COLOR);
	columnLabel->labelfont(LOCAL_BFFONT);
	columnLabel->labelsize(LOCAL_TEXT_SIZE);
	columnLabel->box(FL_RSHADOW_BOX);

        m_structureInfo = new Fl_Scroll(0, 105 + upperYOffset + navButtonsLabelHeight + 
			              dividerTextHeight + 15, 290, 360 - 
				      upperYOffset - navButtonsLabelHeight - 
			              dividerTextHeight - 50);
        m_structureInfo->type(Fl_Scroll::VERTICAL_ALWAYS);
        m_packedInfo = new Fl_Pack(0, 105 + upperYOffset + navButtonsLabelHeight + 
			           dividerTextHeight + 15, 270,
			           360 - upperYOffset - navButtonsLabelHeight - dividerTextHeight - 50);
        m_packedInfo->type(Fl_Pack::VERTICAL);
        
        m_packedInfo->end();
        m_packedInfo->color(GUI_WINDOW_BGCOLOR);
        m_structureInfo->end();
        m_structureInfo->color(GUI_WINDOW_BGCOLOR);
        
        mainMenuPane->resizable(m_structureInfo);
    
    }
    mainMenuPane->end();
    mainMenuPane->color(GUI_WINDOW_BGCOLOR);
    
    menu_collapse = new Fl_Button(300, 0, 15, 450, "@>");
    menu_collapse->callback(CollapseMainCallback);
    menu_collapse->labelcolor(GUI_BTEXT_COLOR);
    folder_collapse = new Fl_Button(315, 0, 15, 450, "@<");
    folder_collapse->callback(CollapseFolderCallback);
    folder_collapse->labelcolor(GUI_BTEXT_COLOR);
    
    folderWindowPane = new Fl_Group(330, 0, 320, 450, "");
    {
    }
    folderWindowPane->end();
    folderWindowPane->color(GUI_WINDOW_BGCOLOR);
    
    m_mainWindow->resizable(mainMenuPane);
    m_mainWindow->size_range(650, 450, 650, 0);
    m_mainWindow->end();

    //#ifndef __APPLE__
    //fl_open_display();
    //Pixmap iconPixmap = XCreateBitmapFromData(fl_display, 
    //		        DefaultRootWindow(fl_display),
    //                    RNAWindowIcon_bits, RNAWindowIcon_width, 
    //			RNAWindowIcon_height);
    //m_mainWindow->icon((const void *) iconPixmap);
    //#endif

    m_mainWindow->show(argc, argv);
    
}

MainWindow::~MainWindow()
{
    if (m_fileChooser) {
        delete m_fileChooser;
    }
    delete m_mainWindow;
    delete m_packedInfo;
    delete m_structureInfo;
    delete columnLabel;
    delete actionsLabel;
    delete topTextDivider;
    delete midTextDivider;
    delete openButton;
    delete configOptionsButton;
    for(int w = 0; w < folderDataBtns.size(); w++) {
        delete folderDataBtns[w];
	folderDataBtns[w] = NULL;
    }
}

bool MainWindow::Initialize(int argc, char **argv)
{
    if(ms_instance)
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

void MainWindow::AddFolder(const char* foldername, const int index, 
                           const bool isSelected)
{
    
    Fl_Pack* pack = ms_instance->m_packedInfo;
    pack->begin();
    int vertPosn = pack->children() * 30 + pack->y();
    
    Fl_Group* group = new Fl_Group(pack->x(), vertPosn, pack->w(),30);
    
    Fl_Button* label = new Fl_Button(pack->x() + 10, vertPosn, 
		                     pack->w() - 70, 30, "");
    label->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    label->callback(MainWindow::ShowFolderCallback);
    label->user_data((void*)foldername);
    Folder* folder = RNAStructViz::GetInstance()->GetStructureManager()->GetFolderAt(index);
    sprintf(folder->folderNameFileCount, "(+% 2d) %-.48s", 
            folder->structCount, folder->folderName);
    label->label(folder->folderNameFileCount);
    label->labelcolor(GUI_BTEXT_COLOR);
    
    ms_instance->folderDataBtns.push_back(label);

    Fl_Button* moveUpButton = new Fl_Button(pack->x()+pack->w() - 60, vertPosn + 5, 20, 20, "@8>");
    moveUpButton->callback(MainWindow::MoveFolderUp);
    moveUpButton->labelcolor(GUI_BTEXT_COLOR);
    //ms_instance->folderDataBtns.push_back(moveUpButton);

    Fl_Button* moveDownButton = new Fl_Button(pack->x()+pack->w() - 40, vertPosn + 5, 20, 20, "@2>");
    moveDownButton->callback(MainWindow::MoveFolderDown);
    moveDownButton->labelcolor(GUI_BTEXT_COLOR);
    //ms_instance->folderDataBtns.push_back(moveDownButton);

    Fl_Button* removeButton = new Fl_Button(pack->x() + pack->w() - 20, vertPosn + 5, 20, 20);
    removeButton->callback(MainWindow::RemoveFolderCallback);
    removeButton->label("@1+");
    removeButton->labelcolor(GUI_BTEXT_COLOR);
    //ms_instance->folderDataBtns.push_back(removeButton);

    group->resizable(label);
    pack->end();
    
    ms_instance->m_structureInfo->redraw();
    
}

void MainWindow::OpenFileCallback(Fl_Widget* widget, void* userData)
{
    ms_instance->CreateFileChooser();
    int fileChooserReturnCode = ms_instance->m_fileChooser->show();
    if(fileChooserReturnCode == -1 || fileChooserReturnCode == 1) { 
        if(fileChooserReturnCode == -1) { // display weird exceptional error case:
	    fl_alert("Unable to open new CT Files: %s\n", 
		     ms_instance->m_fileChooser->errmsg());
	}
        return;
    }

    const char *nextWorkingDir = ms_instance->m_fileChooser->directory();
    if(nextWorkingDir != NULL && strcmp(nextWorkingDir, CTFILE_SEARCH_DIRECTORY)) { 
	// update the working directory:
        strncpy(CTFILE_SEARCH_DIRECTORY, nextWorkingDir, MAX_BUFFER_SIZE - 1);
	ConfigParser::nullTerminateString(CTFILE_SEARCH_DIRECTORY);
    }

    for (int i = 0; i < ms_instance->m_fileChooser->count(); ++i)
    {
    	const char *nextFilename = ms_instance->m_fileChooser->filename(i);
	if(!strcmp(nextFilename, "") || !strcmp(nextFilename, ".") || 
	   !strcmp(nextFilename, "..")) { // invalid file to parser, so ignore it:
	    continue;
	}
        RNAStructViz::GetInstance()->GetStructureManager()->AddFile(nextFilename);
    }
    
    ms_instance->m_packedInfo->redraw();
    ms_instance->folderWindowPane->redraw();

}

void MainWindow::ConfigOptionsCallback(Fl_Widget* widget, void* userData) {
    
     DisplayConfigWindow *cfgWindow = new DisplayConfigWindow(); 
     cfgWindow->show();
     while(!cfgWindow->isDone()) 
          Fl::wait();
     delete cfgWindow;

}

void MainWindow::TestCallback(Fl_Widget* widget, void* userData)
{
    RNAStructViz::GetInstance()->TestFolders();
}

void MainWindow::HelpButtonCallback(Fl_Widget *btn, void *udata) {
     
     const char *helpTextBuffer[6] = {
          "A detailed user manual and explanation of core features ", 
	  "of RNAStructViz is available on the project WIKI: \n", 
	  "https://github.com/gtDMMB/RNAStructViz/wiki\n\n", 
	  "Additional runtime diagnostic information can be obtained by ", 
	  "running the application with the \"--about\" option from ", 
	  "your terminal:\n$ RNAStructViz --about"
     };
     char fullHelpText[4 * MAX_BUFFER_SIZE];
     fullHelpText[0] = '\0';
     for(int bufline = 0; bufline < 6; bufline++) {
          strcat(fullHelpText, helpTextBuffer[bufline]); 
     }
     fl_message_title("RNAStructViz Help ...");
     fl_message_icon()->image(MainWindow::helpIconImage);
     fl_message_icon()->label("");
     fl_message_icon()->color(FL_LIGHT2);
     fl_message_icon()->box(FL_NO_BOX);
     fl_message_icon()->align(FL_ALIGN_IMAGE_BACKDROP | FL_ALIGN_INSIDE | FL_ALIGN_CENTER);
     int userHelpSelection = fl_choice(fullHelpText, "Copy WIKI Link", "OK", NULL);
     switch(userHelpSelection) {
          case 0: { // copy the WIKI link to the clipboard:
               const char *wikiLink = "https://github.com/gtDMMB/RNAStructViz/wiki";
               Fl::copy(wikiLink, strlen(wikiLink), 1, Fl::clipboard_plain_text);
	       break;
          }
	  default:
	       break;
     }

}

void MainWindow::CollapseMainMenu()
{
    Fl_Group* mainMenu = ms_instance->mainMenuPane;
    Fl_Button* button = ms_instance->menu_collapse;
    if (ms_instance->folderWindowPane->visible())
    {
        if (mainMenu->visible())
        {
            mainMenu->hide();
            button->label("@<");
    	    button->labelcolor(GUI_TEXT_COLOR);
            
            int x = ms_instance->m_mainWindow->x() + 300;
            int y = ms_instance->m_mainWindow->y();
            ms_instance->m_mainWindow->resizable(NULL);
            ms_instance->m_mainWindow->size(350, 450);
            ms_instance->m_mainWindow->position(x, y);
            
            ms_instance->menu_collapse->position(0, 0);
            ms_instance->folder_collapse->position(15, 0);
            ms_instance->folderWindowPane->position(30, 0);
            
            ms_instance->folder_collapse->deactivate();
        }
        else 
        {
            mainMenu->show();
            button->label("@>");
	        button->labelcolor(GUI_TEXT_COLOR);
            
            int x = ms_instance->m_mainWindow->x() - 300;
            if (x < 0)
                x = 0;
            int y = ms_instance->m_mainWindow->y();            
            ms_instance->m_mainWindow->resizable(NULL);
            ms_instance->m_mainWindow->position(x,y);
            ms_instance->m_mainWindow->size(650, 450);
            
            ms_instance->menu_collapse->position(300, 0);
            ms_instance->folder_collapse->position(315, 0);
            ms_instance->folderWindowPane->position(330, 0);
            
            ms_instance->folder_collapse->activate();
        }
    }
}

void MainWindow::CollapseMainCallback(Fl_Widget* widget, void* userData)
{
    ms_instance->CollapseMainMenu();
}

void MainWindow::CollapseFolderPane()
{
    Fl_Group* folderMenu = ms_instance->folderWindowPane;
    Fl_Button* button = ms_instance->folder_collapse;
    if (ms_instance->mainMenuPane->visible())
    {
        if (folderMenu->visible())
        {
            folderMenu->hide();
            button->label("@>");
            ms_instance->m_mainWindow->resizable(NULL);
            ms_instance->m_mainWindow->size(330, 450);
            ms_instance->m_mainWindow->resizable(ms_instance->mainMenuPane);
            ms_instance->menu_collapse->deactivate();
        }
        else 
        {
            ms_instance->m_mainWindow->resizable(NULL);
            ms_instance->m_mainWindow->size(650, 450);
            button->label("@<");
            folderMenu->show();
            ms_instance->menu_collapse->activate();
        }
    }
}

void MainWindow::CollapseAlwaysFolderPane()
{
    Fl_Group* folderMenu = ms_instance->folderWindowPane;
    Fl_Button* button = ms_instance->folder_collapse;
    if (!ms_instance->mainMenuPane->visible())
    {
        MainWindow::CollapseMainMenu();
    }
    if (folderMenu->visible())
    {
        folderMenu->hide();
        button->label("@>");
        ms_instance->m_mainWindow->resizable(NULL);
        ms_instance->m_mainWindow->size(330, 450);
        ms_instance->m_mainWindow->resizable(ms_instance->mainMenuPane);
        ms_instance->menu_collapse->deactivate();
    }
}

void MainWindow::ExpandAlwaysFolderPane()
{
    Fl_Group* folderMenu = ms_instance->folderWindowPane;
    Fl_Button* button = ms_instance->folder_collapse;
    ms_instance->m_mainWindow->resizable(NULL);
    ms_instance->m_mainWindow->size(650, 450);
    button->label("@<");
    folderMenu->show();
    ms_instance->menu_collapse->activate();
}

void MainWindow::CollapseFolderCallback(Fl_Widget* widget, void* userData)
{
    ms_instance->CollapseFolderPane();
}

void MainWindow::MoveFolderUp(Fl_Widget *widget, void* userData)
{
    Fl_Pack* pack = ms_instance->m_packedInfo;
    Fl_Group* folderGroup = (Fl_Group*)widget->parent();
    
    int index = pack->find(folderGroup);
    if (index > 0)
        pack->insert(*folderGroup,index-1);
    
    pack->redraw();
}

void MainWindow::MoveFolderDown(Fl_Widget *widget, void* userData)
{
    Fl_Pack* pack = ms_instance->m_packedInfo;
    Fl_Group* folderGroup = (Fl_Group*)widget->parent();
    
    int index = pack->find(folderGroup);
    pack->insert(*folderGroup,index + 2);
    
    pack->redraw();
}

bool MainWindow::CreateFileChooser()
{
    if(m_fileChooser) {
	delete m_fileChooser;
        m_fileChooser = NULL;
    }
    
    // Get the current working directory.
    char currentWD[MAX_BUFFER_SIZE];
    if(ConfigParser::directoryExists(CTFILE_SEARCH_DIRECTORY)) { 
        strncpy(currentWD, CTFILE_SEARCH_DIRECTORY, MAX_BUFFER_SIZE - 1); 
    }
    else if (!getcwd(currentWD, MAX_BUFFER_SIZE)) {
        std::cerr << "Error: getcwd failed. Cannot create file chooser.\n";
        return false;
    }
    
    m_fileChooser = new Fl_Native_File_Chooser(Fl_Native_File_Chooser::BROWSE_MULTI_FILE);
    m_fileChooser->title("Select RNA Sequences / Load New CT Files ...");
    m_fileChooser->filter("*.{ct,nopct,bpseq}");
    m_fileChooser->directory(currentWD);
    m_fileChooser->options(Fl_Native_File_Chooser::NO_OPTIONS | 
		           Fl_Native_File_Chooser::PREVIEW);

    return true;
}

void MainWindow::ShowFolderCallback(Fl_Widget* widget, void* userData)
{
    //Find the folderName label in the contentsButton widget's group
    Fl_Button* folderLabel = (Fl_Button*)(widget->parent()->child(0));
    ms_instance->selectedFolderBtn = (Fl_Button *) widget;

    Fl_Pack* pack = ms_instance->m_packedInfo;
    for (int i = 0; i < pack->children(); ++i) {
        Fl_Button* childLabel = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0));
        childLabel->color(FL_BACKGROUND_COLOR);
    }
    folderLabel->color(FL_LIGHT2);
    folderLabel->labelcolor(FL_DARK1);
    
    const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
    int index;

    for (index = 0; index < folders.size(); ++index)
    {
        if (!strcmp(folders[index]->folderName, (char*)(folderLabel->user_data())))
            break;
    }
    if(index == ms_instance->selectedFolderIndex) { // nothing to display:
        return;
    }
    ShowFolderByIndex(index);

}

void MainWindow::ShowFolderByIndex(int index) { 

    const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->
                                          GetStructureManager()->GetFolders();
    FolderWindow* fwindow;
    if (folders[index]->folderWindow == NULL)
    {
    	fwindow = new FolderWindow(340, 40, 300, 390, 
			           folders[index]->folderName, index);
    }
    else
    {
        fwindow = folders[index]->folderWindow;
    }
    
    if (ms_instance->folderWindowPane->children() > 0) {
        ms_instance->folderWindowPane->remove(0);
    }
    ms_instance->selectedFolderIndex = index;
    ms_instance->folderWindowPane->add((Fl_Group*)fwindow);

    ExpandAlwaysFolderPane();
    ms_instance->folderWindowPane->hide();
    ms_instance->folderWindowPane->show();

}

void MainWindow::ShowFolderSelected()
{
    //Find the folderName label in the contentsButton widget's group
    Fl_Button* folderLabel = NULL;
    Fl_Pack* pack = ms_instance->m_packedInfo;
    
    for (int i = 0; i < pack->children(); ++i) {

        Fl_Button* childLabel = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0));
        if (childLabel->color() == FL_LIGHT2) {
            folderLabel = childLabel;
            break;
        }

    }
    
    if (folderLabel != NULL) {
        
        const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
        int index;
        for (index = 0; index < folders.size(); ++index)
        {
            if (!strcmp(folders[index]->folderName,(char*)(folderLabel->user_data())))
                break;
        }
       
        FolderWindow* fwindow;
        if (folders[index]->folderWindow == NULL)
        {
            fwindow = new FolderWindow(340, 40, 300, 390, folders[index]->folderName, index);
        }
        else
        {
            fwindow = folders[index]->folderWindow;
        }
        
        if (ms_instance->folderWindowPane->children() > 0) {
            ms_instance->folderWindowPane->remove(0);
        }

        ms_instance->folderWindowPane->add((Fl_Group*)fwindow);

        ExpandAlwaysFolderPane();
        ms_instance->folderWindowPane->hide();
        ms_instance->folderWindowPane->show();

    }
}

void MainWindow::HideFolderByIndex(const int index)
{
    RNAStructViz* appInstance = RNAStructViz::GetInstance();
    Folder* folder = appInstance->GetStructureManager()->GetFolderAt(index);
    Fl_Group* pane = ms_instance->folderWindowPane;
    Fl_Pack* pack = ms_instance->m_packedInfo;
    
    for (int i = 0; i < pack->children(); ++i) {
        Fl_Button* childLabel = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0));
        if (!strcmp((char*)(childLabel->user_data()),folder->folderName))
            childLabel->color(FL_BACKGROUND_COLOR);
	    childLabel->labelcolor(GUI_BTEXT_COLOR);
    }
    
    if (pane->children() > 0)
    {
        Fl_Group* childGroup = (Fl_Group*)(pane->child(0));
        if (!strcmp(childGroup->label(),folder->folderName)) {
	    pane->remove(0);
        }
    }
    
    pane->hide();
    pane->show();
}

void MainWindow::HideFolderByName(const char* foldername)
{   
    Fl_Pack* pack = ms_instance->m_packedInfo;
    
    for (int i = 0; i < pack->children(); ++i) {
        Fl_Button* childLabel = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0));
        if (!strcmp((char*)(childLabel->user_data()), foldername + 5))
            childLabel->color(FL_BACKGROUND_COLOR);
	    childLabel->labelcolor(GUI_BTEXT_COLOR);
    }
    
    Fl_Group* pane = ms_instance->folderWindowPane;
    if (pane->children() > 0)
    {
        Fl_Group* childGroup = (Fl_Group*)(pane->child(0));
        
        if (!strcmp(childGroup->label(),foldername)) {
            pane->remove(0);
        }
    }
    
    pane->hide();
    pane->show();
}

void MainWindow::RemoveFolderByIndex(const int index, bool selectNext)
{
    RNAStructViz* appInstance = RNAStructViz::GetInstance();
    const std::vector<Folder*>& folders = appInstance->GetStructureManager()->GetFolders();
    Fl_Pack* pack = ms_instance->m_packedInfo;
    
    for (int i = 0; i < pack->children(); ++i)
    {
        Fl_Button* childButton = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0));
        if (!strcmp((char*)(childButton->user_data()),folders[index]->folderName)) {
            
            for(int btn = 0; btn < ms_instance->folderDataBtns.size(); btn++) {
	         if(ms_instance->folderDataBtns[btn] == childButton) {
		      ms_instance->folderDataBtns.erase( 
				   ms_instance->folderDataBtns.begin() + btn);
		      break;
		 }
	    }

	    Fl_Group* toRemove = (Fl_Group*)pack->child(i);
            pack->insert(*toRemove,pack->children());
            
            const std::vector<DiagramWindow*>& diagrams = appInstance->
	    	GetDiagramWindows();
            const std::vector<StatsWindow*>& stats = appInstance->
        	GetStatsWindows();
            int structNum = folders[index]->structCount;
            
            for (unsigned int ui = 0; ui < diagrams.size(); ++ui)
            {
                if(diagrams[ui]->GetFolderIndex() == index)
                {
                    for(int j = structNum - 1; j >= 0; j--)
                    {
                        int structIndex = folders[index]->folderStructs[j];
                        diagrams[ui]->RemoveStructure(structIndex);
                    }
                    diagrams[ui]->hide();
                }
            }
            for (unsigned int ui = 0; ui < stats.size(); ++ui)
            {
                if(stats[ui]->GetFolderIndex() == index)
                {
                    for(int j = structNum - 1; j >= 0; j--)
                    {
                        int structIndex = folders[index]->folderStructs[j];
                        stats[ui]->RemoveStructure(structIndex);
                    }
                    stats[ui]->hide();
                }
            }
            HideFolderByIndex(index);
            
            int shift = 0;
            for (int j = 0; j < folders[index]->structCount; j++)
            {
                if(folders[index]->folderStructs[(j+shift)] == -1)
                {
                    shift++;
                }
                if(folders[index]->folderStructs[(j+shift)] != -1)
                {   
                    int structIndex = folders[index]->folderStructs[(j+shift)];
                    appInstance->GetStructureManager()->RemoveStructure((structIndex));
                }
            }
            folders[index]->structCount = 0;
            
            appInstance->GetStructureManager()->RemoveFolder(index , i);
            pack->remove(pack->child(pack->children()-1));
            ms_instance->m_structureInfo->redraw();
            break;
        }
        
    }
    if(index == ms_instance->selectedFolderIndex) {
         ms_instance->selectedFolderIndex = -1;
    }

    if(selectNext) { // select the next folder in line:
        Fl_Group* pane = ms_instance->folderWindowPane;
        pane->hide();
	pane->show();
        int nextIndex = (index + 1) % folders.size();	
	while(nextIndex != index) { 
             if(folders[nextIndex]->structCount > 0) { 
                  ShowFolderByIndex(nextIndex);
                  int labelIndex = 0;
		  for(int lbl = 0; lbl < ms_instance->folderDataBtns.size(); lbl++) {
                       Fl_Button *folderLabel = ms_instance->folderDataBtns[lbl];
	               if(!strcmp(folders[nextIndex]->folderName, 
			          (char*) (folderLabel->user_data()))) {
		            labelIndex = lbl;
			    break;
		       }
		  }
		  Fl_Button *folderLabel = ms_instance->folderDataBtns[labelIndex];
		  folderLabel->color(FL_LIGHT2);
		  folderLabel->labelcolor(FL_DARK1);
		  ms_instance->selectedFolderBtn = folderLabel;
		  break;
	     }
             nextIndex = (nextIndex + 1) % folders.size();
	}
    }
}

void MainWindow::RemoveFolderCallback(Fl_Widget* widget, void* userData)
{
    // Find the group with this child
    RNAStructViz* appInstance = RNAStructViz::GetInstance();
    Fl_Pack* pack = ms_instance->m_packedInfo;
    Fl_Button* folderLabel = (Fl_Button*)(widget->parent()->child(0));
                
    for(int btn = 0; btn < ms_instance->folderDataBtns.size(); btn++) {
        if(ms_instance->folderDataBtns[btn] == folderLabel) {
	    ms_instance->folderDataBtns.erase(
			 ms_instance->folderDataBtns.begin() + btn);
	    break;
	}
    }

    const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
    int index;
    for (index = 0; index < folders.size(); ++index)
    {
        if (!strcmp(folders[index]->folderName,(char*)(folderLabel->user_data())))
            break;
    }
    for (int i = 0; i < pack->children(); ++i)
    {
        Fl_Button* childButton = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(3));
        if (childButton == widget)
        {
            Fl_Group* toRemove = (Fl_Group*)pack->child(i);
            pack->insert(*toRemove,pack->children());
            Folder* folder = appInstance->GetStructureManager()->
        	GetFolderAt(index);
            const std::vector<DiagramWindow*>& diagrams = appInstance->
	    	GetDiagramWindows();
            const std::vector<StatsWindow*>& stats = appInstance->
        	GetStatsWindows();
            int structNum = folder->structCount;
            
            for (unsigned int ui = 0; ui < diagrams.size(); ++ui)
            {
                if(diagrams[ui]->GetFolderIndex() == index)
                {
                    for(int j = structNum - 1; j >= 0; j--)
                    {
                        int index = folder->folderStructs[j];
                        diagrams[ui]->RemoveStructure(index);
                    }
                    diagrams[ui]->hide();
                }
            }
            
            for (unsigned int ui = 0; ui < stats.size(); ++ui)
            {
                if(stats[ui]->GetFolderIndex() == index)
                {
                    for(int j = structNum - 1; j >= 0; j--)
                    {
                        int index = folder->folderStructs[j];
                        stats[ui]->RemoveStructure(index);
                    }
                    stats[ui]->hide();
                }
            }
            HideFolderByIndex(index);
            
            int shift = 0;
            for (int j = 0; j < folder->structCount; j++)
            {
                if(folder->folderStructs[(j+shift)] == -1)
                {
                    shift++;
                }
                if(folder->folderStructs[(j+shift)] != -1)
                {   
                    int index = folder->folderStructs[(j+shift)];
                    appInstance->GetStructureManager()->RemoveStructure((index));
                }
            }
            folder->structCount = 0;
            appInstance->GetStructureManager()->RemoveFolder(index , i);
            
            for (unsigned int ui = 0; ui < diagrams.size(); ++ui)
            {
                if (diagrams[ui]->GetFolderIndex() > index)
                {
                    int prevIndex = diagrams[ui]->GetFolderIndex();
                    diagrams[ui]->SetFolderIndex(prevIndex - 1);
                }
            }
            for (unsigned int ui = 0; ui < stats.size(); ++ui)
            {
                if(stats[ui]->GetFolderIndex() > index)
                {
                    int prevIndex = stats[ui]->GetFolderIndex();
                    stats[ui]->SetFolderIndex(prevIndex - 1);
                }
            }
            
            pack->remove(pack->child(pack->children()-1));
            ms_instance->m_structureInfo->redraw();
            break;
        }
    }
}

void MainWindow::CloseCallback(Fl_Widget* widget, void* userData) {
    ConfigParser::WriteUserConfigFile(USER_CONFIG_PATH);
    widget->hide();
    exit(0);
}

void MainWindow::ResetThemeColormaps() {
     Fl::set_color(FL_BACKGROUND_COLOR, GUI_WINDOW_BGCOLOR);
     Fl::set_color(FL_BACKGROUND2_COLOR, fl_darker(GUI_WINDOW_BGCOLOR));
     Fl::set_color(FL_FOREGROUND_COLOR, GUI_BTEXT_COLOR);
     Fl::set_color(FL_INACTIVE_COLOR, fl_darker(GUI_BTEXT_COLOR));
     Fl::set_color(FL_SELECTION_COLOR, fl_lighter(GUI_BTEXT_COLOR));
}

void MainWindow::RethemeMainWindow() {
	     
	     //Fl::foreground(GetRed(LOCAL_FGCOLOR), 
	     //		    GetGreen(LOCAL_FGCOLOR), 
	     //		    GetBlue(LOCAL_FGCOLOR));
	     //Fl::background(GetRed(GUI_BGCOLOR), 
	     //		    GetGreen(GUI_BGCOLOR), 
	     //		    GetBlue(GUI_BGCOLOR)); 
             //Fl::background2(GetRed(LOCAL_BG2COLOR), 
	     //		     GetGreen(LOCAL_BG2COLOR), 
	     //		     GetBlue(LOCAL_BG2COLOR));
	     ResetThemeColormaps();

	     if(ms_instance == NULL) {
	          return;
	     }
	     ms_instance->m_mainWindow->color(GUI_WINDOW_BGCOLOR);
             ms_instance->m_mainWindow->redraw();
             if(ms_instance->m_packedInfo) {
	          ms_instance->m_packedInfo->color(GUI_WINDOW_BGCOLOR);
	          ms_instance->m_packedInfo->redraw();
	     }
	     if(ms_instance->m_structureInfo) { 
	          ms_instance->m_structureInfo->color(GUI_WINDOW_BGCOLOR);
	          ms_instance->m_structureInfo->labelcolor(GUI_BTEXT_COLOR);
	          ms_instance->m_structureInfo->redraw();
	     }
	     ms_instance->menu_collapse->color(GUI_BGCOLOR);
	     ms_instance->menu_collapse->labelcolor(GUI_BTEXT_COLOR);
	     ms_instance->menu_collapse->redraw();
	     ms_instance->folder_collapse->color(GUI_BGCOLOR);
	     ms_instance->folder_collapse->labelcolor(GUI_BTEXT_COLOR);
	     ms_instance->folder_collapse->redraw();
	     ms_instance->columnLabel->labelcolor(GUI_TEXT_COLOR);
	     ms_instance->columnLabel->redraw();
	     ms_instance->actionsLabel->labelcolor(GUI_TEXT_COLOR);
	     ms_instance->actionsLabel->redraw();
	     ms_instance->openButton->color(GUI_BGCOLOR);
	     ms_instance->openButton->labelcolor(GUI_BTEXT_COLOR);
	     ms_instance->openButton->redraw();
	     ms_instance->configOptionsButton->color(GUI_BGCOLOR);
	     ms_instance->configOptionsButton->labelcolor(GUI_BTEXT_COLOR);
	     ms_instance->configOptionsButton->redraw();
	     
	     for(int b = 0; b < ms_instance->folderDataBtns.size(); b++) {
	          ms_instance->folderDataBtns[b]->color(GUI_BGCOLOR);
		  ms_instance->folderDataBtns[b]->labelcolor(GUI_BTEXT_COLOR);
	     }
	     if(ms_instance->selectedFolderBtn != NULL) {
                  ms_instance->selectedFolderBtn->color(FL_LIGHT2);
		  ms_instance->selectedFolderBtn->labelcolor(FL_DARK1);
	     }
	     ms_instance->actionsLabel->color(GUI_BGCOLOR);
	     ms_instance->actionsLabel->labelcolor(GUI_BTEXT_COLOR);
	     ms_instance->actionsLabel->redraw();
	     ms_instance->columnLabel->color(GUI_BGCOLOR);
	     ms_instance->columnLabel->labelcolor(GUI_BTEXT_COLOR);
	     ms_instance->columnLabel->redraw();
	     if(ms_instance->selectedFolderIndex >= 0) {
                  FolderWindow *curFolderWin = (FolderWindow *) 
			       RNAStructViz::GetInstance()->
			       GetStructureManager()->
			       GetFolders()[ms_instance->selectedFolderIndex];
	          curFolderWin->RethemeFolderWindow();
		  ShowFolderCallback(ms_instance->selectedFolderBtn, 
		         (void *) ms_instance->selectedFolderBtn->user_data());
	     }
	     Fl::scheme(FLTK_THEME);
	     Fl::redraw();
	     Fl::flush();
}
