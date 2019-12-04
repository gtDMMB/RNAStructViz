/*
    The window that computes the stats of a folder.
 */

#ifndef STATSWINDOW_H
#define STATSWINDOW_H

#include <FL/Fl.H>
#include <FL/fl_draw.H>
#include <FL/Enumerations.H>
#include <FL/Fl_Window.H>
#include <FL/Fl_Tabs.H>
#include <FL/Fl_Choice.H>
#include <FL/Fl_Pack.H>
#include <FL/Fl_Scroll.H>
#include <FL/Fl_Button.H>
#include <FL/Fl_Toggle_Button.H>
#include <FL/Fl_Chart.H>
#include <FL/Fl_Box.H>
#include <FL/Fl_Text_Display.H>
#include <FL/Fl_Text_Buffer.H>
#include <FL/Fl_RGB_Image.H>

#include <vector>

#include "RNAStructure.h"
#include "StructureManager.h"
#include "InputWindow.h"
#include "ConfigOptions.h"

#define DEFAULT_STATSWIN_WIDTH        (1400)
#define DEFAULT_STATSWIN_HEIGHT       (700)


namespace RocBoxPlot {

     inline static const Fl_Boxtype ROC_BOX = FL_ROUNDED_FRAME;
     inline static const int draw_it_active = 1;

     void roc_box_draw(int x, int y, int w, int h, Fl_Color bgcolor);
     void roc_box_init();

}

class StatsWindow : public Fl_Window
{
public:
    void Construct(int w, int h, const std::vector<int>& structures);
    StatsWindow(int w, int h, const char *label, 
    	const std::vector<int>& structures);
    StatsWindow(int x, int y, int w, int h, const char *label, 
    	const std::vector<int>& structures);
    
    virtual ~StatsWindow();
    
    
    /*
     Hides the window and closes any InputWindows
     */
    void hide();
    
    //Manages structures
    void AddStructure(const int index);
    void RemoveStructure(const int index);
    void SetStructures(const std::vector<int>& structures);
    const std::vector<int>& GetStructures();
    
    /*
     Set the reference structure.
     
     Giving an index of -1 clears the reference structure.
     */
    void SetReferenceStructure(const int index);
    
    /*
    An index of -1 means there is no reference structure.
    */
    inline int GetReferenceStructure()
    {
        return referenceIndex;
    }
    
    inline int GetFolderIndex()
    {
        return folderIndex;
    }
    
    void SetFolderIndex(int index);
    int GetFolderIndex() const;

    /* Populates the reference structure dropdown list with options from the 
    current structures folder */
    void BuildRefMenu();
    
    /* Populates the comparison structure list with options from the structures
    folder */
    void BuildCompMenu();
    
    void ResetWindow();
    
    struct StatData
    {
        const char *filename; // The filename of the structure the stats correspond to
        bool ref; // True if this structure is the reference
        bool isValid; // whether the structure has been initialized
	int color; // Color assigned to the structure for the histograms
        unsigned int base_pair_count; // Number of base pairs in the structure
        unsigned int gc_count; // Number of G-C base pairs
        unsigned int au_count; // Number of A-U base pairs
        unsigned int gu_count; // Number of G-U base pairs
        unsigned int non_canon_count; // Number of non-canonical base pairs
        unsigned int true_pos_count; // Number of true positive base pairs
        unsigned int false_neg_count; // Number of false negative base pairs 
        unsigned int false_pos_count; // Number of false positive base pairs (discounting compatible)
        unsigned int conflict_count; // Number of false positives that 'conflict'
        unsigned int contradict_count; // Number of false positives that 'contradict'
        unsigned int compatible_count; // Number of false positives that are 'compatible'
        float sensitivity; // Sensitivity = TP/(TP+FN)
        float selectivity; // Selectivity = TP/(TP+FP) discounting compatible
        float pos_pred_value; // Positive predictive value, TP/(TP+FP) including 
    	
	// compatible char* versions of each value:
        char bp_char [12]; // base_pair_count
        char tp_char [12]; // true_pos_count
        char fn_char [12]; // false_neg_count
        char fp_char [12]; // false_pos_count
        char conf_char [12]; // conflict_count
        char cont_char [12]; // contradict_count
        char comp_char [12]; // compatible_count
        char sens_char [12]; // sensitivity
        char sel_char [12]; // selectivity
        char ppv_char [12]; // pos_pred_value
        char gc_char [12]; // gc_count
        char au_char [12]; // au_count
        char gu_char [12]; // gu_count
        char nc_char [12]; // non_canon_count
    };
    
protected:
    void resize(int x, int y, int w, int h);

private:
    /*
     Callback to make a structure the reference, when selected from the 
     dropdown list.
     */
    static void ReferenceCallback(Fl_Widget* widget, void* userData);
    
    void ClearStats();
    
    void ComputeStats();
    
    void DrawHistograms();
    
    void DrawRoc();
    
    void DrawLegend();
    
    void ExportTable();
    
    // calback for the comparison structure check buttons 
    static void MenuCallback(Fl_Widget* widget, void* userData);
    
    // Callback for the calculate button
    static void CalcCallback(Fl_Widget* widget, void* userData);
    
    // Callback for the export button
    static void ExportCallback(Fl_Widget* widget, void* userData);
    
    // Callback to select all checkboxes for the comparison vs. reference 
    // structures: 
    static void SelectAllButtonCallback(Fl_Widget *widget, void *userData);

    // Holds the title of the window
    char* title; 
    
    /* The tabs sections for the statistics */
    Fl_Tabs *tab_window;
    Fl_Group *overview_tab; // Group for the main histograms
    Fl_Group *perc_tab; // Group for selectivity, sensitivity, and ppv
    Fl_Group *pair_tab; // Group for the base-pairing histograms
    Fl_Group *roc_tab; // Group for the ROC plot
    Fl_Group *table_tab; // Group for the statistics table
    
    /* Overview tab legend explanation image */
    static Fl_RGB_Image *overviewLegendImage;

    /* Holds the set of structures being compared */
    std::vector<int> m_structures;
    
    // global position data storage for the LHS selection widgets:
    int mwx, mwy, mww, mwh;    

    // Index of the folder
    int folderIndex;
    
    // Index of the reference sequence. Can be -1 for no sequence. 
    int referenceIndex;
    
    // Window for menu section on the left of the screen
    Fl_Group *menu_window;
    
    // Menu of options for reference structure
    Fl_Choice* ref_menu;
    
    // Holds menu of structures for comparison
    Fl_Scroll* comp_menu;
    
    // Contains the check buttons for each comparison structure as children
    Fl_Pack* comp_pack;
    
    // "Calculate" and "Select All" buttons
    Fl_Toggle_Button* calc_button;
    Fl_Button *selectAllCompsBtn;
    
    // GUI divider:
    Fl_Box *dividerTextBox;

    StructureManager* structureManager;
    
    // Holds the calculated statistics for the window
    StatData* statistics;
    unsigned int numStats; // Number of structures for which there are stat
    
    // Text display for statistics
    Fl_Text_Display *text_display;
    Fl_Text_Buffer *buff;
        
    // Histogram groups
    Fl_Group *bp_chart; // histogram for base pairs
    Fl_Group *tp_chart; // histogram for true positives
    Fl_Group *fp_chart; // histogram for false positives
    Fl_Group *fn_chart; // histogram for false negatives
    Fl_Group *sens_chart; // histogram for sensitivities
    Fl_Group *sel_chart; // histogram for selectivities
    Fl_Group *ppv_chart; // histogram for positive predictive values
    Fl_Group *gc_chart; // histogram for G-C base pairs
    Fl_Group *au_chart; // histogram for A-U base pairs
    Fl_Group *gu_chart; // histogram for G-U base pairs
    Fl_Group *non_canon_chart; // histogram for non-canonical base pairs
  
    // Formulas image:
    Fl_RGB_Image *statsFormulasImage;
    Fl_Box *statsFormulasBox;

    // ROC Plot group
    Fl_Group *roc_plot;
  
  	// Legends for the charts
  	Fl_Box* leg1_ref; 
  	Fl_Group* leg1_group;
  	Fl_Scroll* leg1_scroll;
  	Fl_Pack* leg1_pack;
  	Fl_Box* leg2_ref;
  	Fl_Group* leg2_group;
  	Fl_Scroll* leg2_scroll;
  	Fl_Pack* leg2_pack;
  	Fl_Box* leg3_ref;
  	Fl_Group* leg3_group;
  	Fl_Scroll* leg3_scroll;
  	Fl_Pack* leg3_pack;
  	Fl_Box* leg4_ref;
  	Fl_Box* leg4_ref_symbol;
  	Fl_Group* leg4_group;
  	Fl_Scroll* leg4_scroll;
  	Fl_Pack* leg4_pack;
  	
  	// Equation legend labels
        Fl_Box* tp_label;
        Fl_Box* fp_label;
        Fl_Box* fp_equ_label;
        Fl_Box* sens_label;
        Fl_Box* sel_label;
        Fl_Box* ppv_label;
        Fl_Box* ppv_equ_label;
    
  	// "Export" button and name InputWindow
  	Fl_Button* exp_button;
        InputWindow* input_window;
    
};

#endif //STATSWINDOW_H
