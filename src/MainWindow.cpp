#include "MainWindow.h"
#include "RNAStructViz.h"
#include <unistd.h>
#include <iostream>
#include <vector>
#include <FL/Fl_Box.H>
#include <FL/Fl_Check_Button.H>
#include <FL/Fl_Round_Button.H>

MainWindow* MainWindow::ms_instance = 0;

MainWindow::MainWindow(int argc, char **argv)
: m_fileChooser(0)
{
    m_mainWindow = new Fl_Window(650, 450, "RNAStructViz");
    m_mainWindow->callback(CloseCallback);
    m_mainWindow->color(FL_WHITE);
    
    //Fl_Box* resizableBoxOld = new Fl_Box(0, 60, 300, 340);
    //m_mainWindow->resizable(resizableBoxOld);
    
    m_mainWindow->begin();
    
    mainMenuPane = new Fl_Group(0,0,300,450,"");
    {
        
        //Open button
        Fl_Button* openButton = new Fl_Button(10, 10, 90, 30, "Load Files");
        openButton->callback(OpenFileCallback);
        
        Fl_Box* columnLabel = new Fl_Box(20,50,120,30,"Folders");
        columnLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
        
        m_structureInfo = new Fl_Scroll(0,80,290,360);
        m_structureInfo->type(Fl_Scroll::VERTICAL_ALWAYS);
        m_packedInfo = new Fl_Pack(0,80,270,360);
        m_packedInfo->type(Fl_Pack::VERTICAL);
        
        
        m_packedInfo->end();
        m_packedInfo->color(FL_WHITE);
        m_structureInfo->end();
        m_structureInfo->color(FL_WHITE);
        
        mainMenuPane->resizable(m_structureInfo);
    }
    mainMenuPane->end();
    mainMenuPane->color(FL_WHITE);
    
    menu_collapse = new Fl_Button(300,0,15,450,"@>");
    menu_collapse->callback(CollapseMainCallback);
    folder_collapse = new Fl_Button(315,0,15,450,"@<");
    folder_collapse->callback(CollapseFolderCallback);
    
    folderWindowPane = new Fl_Group(330,0,320,450,"");
    {
        
        //Fl_Box* tabsLabel = new Fl_Box(340,0,120,30,"Recently Opened Folders");
        //tabsLabel->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
        
        /*folder_tabs = new Fl_Tabs(340,10,300,430,"");
        {
            
        }
        folder_tabs->labeltype(FL_NO_LABEL);
        folder_tabs->end();*/
    }
    folderWindowPane->end();
    folderWindowPane->color(FL_WHITE);
    
    m_mainWindow->resizable(mainMenuPane);
    m_mainWindow->size_range(650,450,650,0);
    m_mainWindow->end();
    
    m_mainWindow->show(argc, argv);
    
}

MainWindow::~MainWindow()
{
    if (m_fileChooser)
    {
		delete m_fileChooser;
	}
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

void MainWindow::AddFolder(const char* foldername,const int index, 
                           const bool isSelected)
{
    
    Fl_Pack* pack = ms_instance->m_packedInfo;
    pack->begin();
    int vertPosn = pack->children() * 30 + pack->y();
    
    Fl_Group* group = new Fl_Group(pack->x(), vertPosn, pack->w(),30);
    
    Fl_Button* label = new Fl_Button(pack->x() + 10, vertPosn, pack->w() - 70, 30, "");
    label->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    label->callback(MainWindow::ShowFolderCallback);
    label->user_data((void*)foldername);
    
    Folder* folder = RNAStructViz::GetInstance()->GetStructureManager()->GetFolderAt(index);
    sprintf(folder->folderNameFileCount, "%-.48s (%d)", folder->folderName, folder->structCount);
    label->label(folder->folderNameFileCount);
    
    Fl_Button* moveUpButton = new Fl_Button(pack->x()+pack->w()-60,vertPosn+5,20,20,"@8>");
    moveUpButton->callback(MainWindow::MoveFolderUp);
    Fl_Button* moveDownButton = new Fl_Button(pack->x()+pack->w()-40,vertPosn+5,20,20,"@2>");
    moveDownButton->callback(MainWindow::MoveFolderDown);
    
    Fl_Button* removeButton = new Fl_Button(pack->x() + pack->w() - 20, vertPosn + 5, 20, 20);
    removeButton->callback(MainWindow::RemoveFolderCallback);
    //removeButton->user_data((void*)index);
    removeButton->label("@1+");
    
    group->resizable(label);
    pack->end();
    
    //ms_instance->m_structureInfoOld->redraw();
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
		RNAStructViz::GetInstance()->GetStructureManager()->
        AddFile(ms_instance->m_fileChooser->value(i));
    }
    
    ms_instance->m_packedInfo->redraw();
    ms_instance->folderWindowPane->redraw();
}

void MainWindow::TestCallback(Fl_Widget* widget, void* userData)
{
    RNAStructViz::GetInstance()->TestFolders();
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
            
            int x = ms_instance->m_mainWindow->x() + 300;
            int y = ms_instance->m_mainWindow->y();
            ms_instance->m_mainWindow->resizable(NULL);
            ms_instance->m_mainWindow->size(350,450);
            ms_instance->m_mainWindow->position(x,y);
            
            ms_instance->menu_collapse->position(0,0);
            ms_instance->folder_collapse->position(15,0);
            ms_instance->folderWindowPane->position(30,0);
            
            ms_instance->folder_collapse->deactivate();
        }
        else 
        {
            mainMenu->show();
            button->label("@>");
            
            int x = ms_instance->m_mainWindow->x() - 300;
            if (x < 0)
                x = 0;
            int y = ms_instance->m_mainWindow->y();            
            ms_instance->m_mainWindow->resizable(NULL);
            ms_instance->m_mainWindow->position(x,y);
            ms_instance->m_mainWindow->size(650,450);
            
            
            ms_instance->menu_collapse->position(300,0);
            ms_instance->folder_collapse->position(315,0);
            ms_instance->folderWindowPane->position(330,0);
            
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
            ms_instance->m_mainWindow->size(330,450);
            ms_instance->m_mainWindow->resizable(ms_instance->mainMenuPane);
            ms_instance->menu_collapse->deactivate();
            //ms_instance->mainMenuPane->resize(0,0,300,450);
            //ms_instance->menu_collapse->position(300,0);
            //ms_instance->folder_collapse->position(315,0);
            //ms_instance->m_mainWindow->resizable(ms_instance->m_structureInfo);
            //ms_instance->m_mainWindow->init_sizes();
        }
        else 
        {
            ms_instance->m_mainWindow->resizable(NULL);
            ms_instance->m_mainWindow->size(650,450);
            button->label("@<");
            folderMenu->show();
            ms_instance->menu_collapse->activate();
            //ms_instance->mainMenuPane->resize(0,0,300,450);
            //ms_instance->menu_collapse->position(300,0);
            //ms_instance->folder_collapse->position(315,0);
            
            //folderMenu->position(330,0);
            
            //ms_instance->m_mainWindow->resizable(ms_instance->m_structureInfo);
            //ms_instance->m_mainWindow->init_sizes();
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
        ms_instance->m_mainWindow->size(330,450);
        ms_instance->m_mainWindow->resizable(ms_instance->mainMenuPane);
        ms_instance->menu_collapse->deactivate();
    }
}

void MainWindow::ExpandAlwaysFolderPane()
{
    Fl_Group* folderMenu = ms_instance->folderWindowPane;
    Fl_Button* button = ms_instance->folder_collapse;
    ms_instance->m_mainWindow->resizable(NULL);
    ms_instance->m_mainWindow->size(650,450);
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
    pack->insert(*folderGroup,index+2);
    
    pack->redraw();
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
    
    m_fileChooser = new Fl_File_Chooser(currentWD, "*.{ct,nopct,bpseq}", 
                                        Fl_File_Chooser::MULTI, "Select RNA Sequences");
    Fl_Box* message = new Fl_Box(0,0,400,40,"Loaded files will automatically be grouped into folders according to their underlying sequence.");
    message->align(FL_ALIGN_WRAP | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
    m_fileChooser->add_extra(message);
    
    return true;
}

/*void MainWindow::ShowFolderCallbackOld(Fl_Widget* widget, void* userData)
 {
 
 const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
 int index;
 for (index = 0; index < folders.size(); ++index)
 {
 if (!strcmp(folders[index]->folderName,widget->label()))
 break;
 }
 
 RNAStructViz::GetInstance()->GetStructureManager()->DisplayFolderContents(index);
 }*/

void MainWindow::ShowFolderCallback(Fl_Widget* widget, void* userData)
{
    //Find the folderName label in the contentsButton widget's group
    Fl_Button* folderLabel = (Fl_Button*)(widget->parent()->child(0));
    Fl_Pack* pack = ms_instance->m_packedInfo;
    
    for (int i = 0; i < pack->children(); ++i) {
        Fl_Button* childLabel = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0));
        childLabel->color(FL_BACKGROUND_COLOR);
    }
    folderLabel->color(FL_CYAN);
    //printf("Trying to show: %s\n",folderLabel->label());
    
    const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
    int index;
    for (index = 0; index < folders.size(); ++index)
    {
        if (!strcmp(folders[index]->folderName,(char*)(folderLabel->user_data())))
            break;
    }
    
    //printf("Found folder: %s\n",folders[index]->folderName);
    
    //RNAStructViz::GetInstance()->GetStructureManager()->DisplayFolderContents(index);
    
    //printf("Adding folder to tabs\n");
    //Fl_Tabs* tabs = ms_instance->folder_tabs;
    /*
    int present = 0;
    for (int i = 0; i < ms_instance->folder_tabs->children(); ++i) 
    {
        if (!strcmp(tabs->child(i)->label(),folders[index]->folderName))
        {
            tabs->value(tabs->child(i));
            present = 1;
            break;
        }
    }
    if (present == 0) 
    {
        Fl_Group* folderGroup = new FolderWindow(340,40,300,450-60, folders[index]->folderName, index);
        
        tabs->add(folderGroup);
        tabs->value(folderGroup);
    }

    ExpandAlwaysFolderPane();
    tabs->show();
    tabs->redraw();
    */
    Fl_Group* folderGroup;
    if (folders[index]->folderWindow == NULL)
    {
    
        folderGroup = new FolderWindow(340,40,300,450-60, folders[index]->folderName, index);
        folders[index]->folderWindow = (FolderWindow*)folderGroup;
    }
    else
    {
        folderGroup = folders[index]->folderWindow;
    }
        
    if (ms_instance->folderWindowPane->children() > 0) {
        ms_instance->folderWindowPane->remove(0);
    }
    ms_instance->folderWindowPane->add(folderGroup);
    ExpandAlwaysFolderPane();
    ms_instance->folderWindowPane->hide();
    ms_instance->folderWindowPane->show();
}

void MainWindow::HideFolderByIndex(const int index)
{
    RNAStructViz* appInstance = RNAStructViz::GetInstance();
    Folder* folder = appInstance->GetStructureManager()->GetFolderAt(index);
    
    //Fl_Tabs* tabs = ms_instance->folder_tabs;
    Fl_Group* pane = ms_instance->folderWindowPane;
    
    /*for (int i = 0; i < tabs->children(); ++i)
    {
        Fl_Group* childGroup = (Fl_Group*)(tabs->child(i));
        
        if (!strcmp(childGroup->label(),folder->folderName)) {
            tabs->value(childGroup);
            tabs->remove(i);
        }
    }
    tabs->hide();
    if (tabs->children() > 0)
        tabs->show();
    */
    
    Fl_Pack* pack = ms_instance->m_packedInfo;
    
    for (int i = 0; i < pack->children(); ++i) {
        Fl_Button* childLabel = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0));
        if (!strcmp((char*)(childLabel->user_data()),folder->folderName))
            childLabel->color(FL_BACKGROUND_COLOR);
    }
    
    if (pane->children() > 0)
    {
        Fl_Group* childGroup = (Fl_Group*)(pane->child(0));
        
        if (!strcmp(childGroup->label(),folder->folderName)) {
            pane->remove(0);
        }
    }
    //CollapseAlwaysFolderPane();
    
    pane->hide();
    pane->show();
}

void MainWindow::HideFolderByName(const char* foldername)
{
    /*Fl_Tabs* tabs = ms_instance->folder_tabs;
    
    for (int i = 0; i < tabs->children(); ++i)
    {
        Fl_Group* childGroup = (Fl_Group*)(tabs->child(i));
        
        if (!strcmp(childGroup->label(),foldername)) {
            tabs->value(childGroup);
            tabs->remove(i);
        }
    }
    tabs->hide();
    if (tabs->children() > 0)
        tabs->show();*/
    
    Fl_Pack* pack = ms_instance->m_packedInfo;
    
    for (int i = 0; i < pack->children(); ++i) {
        Fl_Button* childLabel = ((Fl_Button*)((Fl_Group*)pack->child(i))->child(0));
        if (!strcmp((char*)(childLabel->user_data()),foldername))
            childLabel->color(FL_BACKGROUND_COLOR);
    }
    
    Fl_Group* pane = ms_instance->folderWindowPane;
    
    if (pane->children() > 0)
    {
        Fl_Group* childGroup = (Fl_Group*)(pane->child(0));
        
        if (!strcmp(childGroup->label(),foldername)) {
            pane->remove(0);
        }
    }
    //CollapseAlwaysFolderPane();
    
    pane->hide();
    pane->show();
}

void MainWindow::RemoveFolderByIndex(const int index)
{
    RNAStructViz* appInstance = RNAStructViz::GetInstance();
    const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
    
    Fl_Pack* pack = ms_instance->m_packedInfo;
    
    //printf("folderName: %s\n",folder->folderName);
    
    for (int i = 0; i < pack->children(); ++i)
    {
        Fl_Button* childButton = ((Fl_Button*)((Fl_Group*)pack->child(i))->
                            child(0));
        //printf("button label: %s\n",childButton->label());
        if (!strcmp((char*)(childButton->user_data()),folders[index]->folderName)) {
            
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
            
            //appInstance->GetStructureManager()->RemoveFolder((int)userData);
            appInstance->GetStructureManager()->
            RemoveFolder(index , i);
            Fl_Group* toRemove = (Fl_Group*)pack->child(i);
            pack->remove(toRemove);
            ms_instance->m_structureInfo->redraw();

            Fl::delete_widget(toRemove);
            break;
        }
        
    }
}

/*void MainWindow::RemoveFolderCallbackOld(Fl_Widget* widget, void* userData)
 {
 
 // Find the group with this child
 Fl_Pack* packOld = ms_instance->m_packedInfoOld;
 
 Fl_Button* nameButton = (Fl_Button*)(widget->parent()->child(1));
 
 const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
 int index;
 for (index = 0; index < folders.size(); ++index)
 {
 if (!strcmp(folders[index]->folderName,nameButton->label()))
 break;
 }
 
 for (int i = 0; i < packOld->children(); ++i)
 {
 Fl_Button* childButton = ((Fl_Button*)((Fl_Group*)packOld->child(i))->
 child(2));
 if (childButton == widget)
 {
 RNAStructViz* appInstance = RNAStructViz::GetInstance();
 
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
 
 //appInstance->GetStructureManager()->RemoveFolder((int)userData);
 appInstance->GetStructureManager()->
 RemoveFolder(index , i);
 
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
 
 
 Fl_Group* toRemove = (Fl_Group*)packOld->child(i);
 packOld->remove(toRemove);
 ms_instance->m_structureInfoOld->redraw();
 
 if(RNAStructViz::GetInstance()->GetStructureManager()->GetSelectedFolder() 
 == -1)
 {
 ms_instance->diagramButtonOld->deactivate();
 ms_instance->statsButtonOld->deactivate();
 }
 
 Fl::delete_widget(toRemove);
 break;
 }
 }
 }*/

void MainWindow::RemoveFolderCallback(Fl_Widget* widget, void* userData)
{
    // Find the group with this child
    RNAStructViz* appInstance = RNAStructViz::GetInstance();
    Fl_Pack* pack = ms_instance->m_packedInfo;
    
    Fl_Button* folderLabel = (Fl_Button*)(widget->parent()->child(0));
    //printf("Removing folder: %s\n",folderLabel->label());
    
    const std::vector<Folder*>& folders = RNAStructViz::GetInstance()->GetStructureManager()->GetFolders();
    int index;
    for (index = 0; index < folders.size(); ++index)
    {
        if (!strcmp(folders[index]->folderName,(char*)(folderLabel->user_data())))
            break;
    }
    for (int i = 0; i < pack->children(); ++i)
    {
        Fl_Button* childButton = ((Fl_Button*)((Fl_Group*)pack->child(i))->
                                  child(3));
        if (childButton == widget)
        {
            
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
            
            //appInstance->GetStructureManager()->RemoveFolder((int)userData);
            appInstance->GetStructureManager()->
            RemoveFolder(index , i);
            
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
            
            
            Fl_Group* toRemove = (Fl_Group*)pack->child(i);
            pack->remove(toRemove);
            ms_instance->m_structureInfo->redraw();
            
            
            Fl::delete_widget(toRemove);
            break;
        }
    }
}


void MainWindow::CloseCallback(Fl_Widget* widget, void* userData)
{
    exit(0);
}
