/* XMLExportButton.h : Define the machinery to handle exporting to XML files, and the widgets that 
 *                     trigger this action on a loaded structure from some folder in the GUI;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.12.03
 */

#ifndef __XMLEXPORT_BUTTON_H__
#define __XMLEXPORT_BUTTON_H__

#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <string>
#include <vector>

namespace XMLExport {
     
     typedef enum {
	  XMLEXPORT_OPTIONS_NONE         = 0x00,
          XMLEXPORT_STRUCTURE_FULL       = 0x01, 
	  XMLEXPORT_STRUCTURE_STANDARD   = 0x02,
	  XMLEXPORT_STRUCTURE_MINIMAL    = 0x04,
	  XMLEXPORT_VERBOSE              = 0x08,
	  XMLEXPORT_ALLOW_OVERWRITES     = 0x10,
	  XMLEXPORT_INCLUDE_EXPERIMENTAL = 0x20,
     } XMLExportOption_t;

     static bool XMLExportOptionSet(int optionSet, XMLExportOption_t optionFlag); 

     typedef enum {
          XMLEXPORTFMT_RNASTRUCTVIZ    = 0, 
	  XMLEXPORTFMT_STRUCTURE_ONLY  = 1, 
	  XMLEXPORTFMT_FULL_FOLDER     = 2,
     } XMLExportFormat_t;

     static std::string GetXMLExportPathFromUser(unsigned int options);
     static int ExportStructureToXML(std::string outFilePath, XMLExportFormat_t xmlExportFmt, 
		                     unsigned int dataIndex = -1, unsigned int optionFlags = 0x00);

     class XMLExportWriter {

	  public:
	       static inline unsigned int LEVEL_INDENT = 5;
	       static inline const char *TAG_SPACING = "\n";
	       static const inline const char *GLOBAL_META_TAG = "<?xml version=\"1.0\" encoding=\"UTF-8\" ?>";

	  protected:
               typedef enum {
	            XMLDATAELM_DATATYPE_INT, 
		    XMLDATAELM_DATATYPE_FLOAT,
		    XMLDATAELM_DATATYPE_STRING,
		    XMLDATAELM_DATATYPE_HEXBUF,
		    XMLDATAELM_DATATYPE_UNICODE,
		    XMLDATAELM_DATATYPE_TEXT,
		    XMLDATAELM_DATATYPE_POSTSCRIPT,
		    XMLDATAELM_DATATYPE_MIXED,
		    XMLDATAELM_DATATYPE_NESTED_TAG
	       } XMLElementDataType_t;

	       typedef struct {
                    std::string elementTagName;
                    bool isSingletonTag;
                    std::string *tagMetaAttrNames;
		    std::string *tagMetaAttralues;
		    unsigned int mdataAttrCount;
		    XMLElementDataType_t tagDataType;
	       } XMLDataElement_t;

	       bool outputFileOpen;
	       FILE *fpOutputFile;
               XMLExportFormat_t xmlFormatType;

          public:
               XMLExportWriter(); // TODO: params list
	       ~XMLExportWriter();

	       int OpenFile(std::string outputFile, unsigned int optionFlags);
	       int CloseFile();
               
	       int WriteXMLHeaderData(int level = 0);
	       int WriteXMLTag(XMLDataElement_t tagData, int level = 1, bool tagOpenStatus = true);
	       int WriteXMLFooterData(int level = 0);
	       int WriteXMLComment(std::string commentField, bool multiLine = false);
               int WriteXMLFile(unsigned int dataIndex = -1, unsigned int optionFlags = 0x00);

	       static XMLExportWriter* GetXMLExportWriterByType(XMLExportFormat_t xmldocFmtType);
     
     };

     class XMLExportWriter_RNAStructViz : public XMLExportWriter {};
     class XMLExportWriter_StructureOnly : public XMLExportWriter {};
     class XMLExportWriter_FullSequenceFolder : public XMLExportWriter {};

}

#include <FL/Enumerations.H>
#include <FL/Fl_Button.H>

#include "pixmaps/XMLExportButtonIcon.c"

public class XMLExportButton : public Fl_Button {
};

#endif
