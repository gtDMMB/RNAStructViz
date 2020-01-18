/* InputWindowExportImage.cpp : Implementation of the header specs;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2020.01.18
 */

#include <cairo-svg.h>

#include <FL/Enumerations.H>

#include "InputWindowExportImage.h"
#include "ConfigOptions.h"
#include "TerminalPrinting.h"

Fl_Check_Button * InputWindowExportImage::GetCheckButtonReference(const InputWindowExportImage * &inputWin, 
		                                                  LocalCheckboxName_t cbType) {
     if(inputWin == NULL) {
          return NULL;
     }
     switch(cbType) {
          case CBNAME_NONE:
	       return NULL;
	  case CBNAME_OVERWRITE_DISK:
	       return inputWin->cbOverwriteSetting;
	  case CBNAME_UPDATE_SETTINGS:
	       return inputWin->cbUpdateCfgSetting;
	  case CBNAME_INCLUDE_FOOTER_DATA:
	       return inputWin->cbIncludeFooterSetting;
	  case CBNAME_WRITE_FOOTER_DATA_TXT:
	       return inputWin->cbWriteFooterTextSetting;
	  default:
	       return NULL;
     }
     return NULL;
}

InputWindowExportImage::InputWindowExportImage(std::string fullSavePath) : 
	InputWindow(IWIN_WIDTH, IWIN_HEIGHT, 
		    "Select export image type ... ", "Select image type", 
	            InputWindow::EXPORT_IMAGE, false), 
	mainWinInfoBox(NULL), 
	cbOverwriteSetting(NULL), cbUpdateCfgSetting(NULL), cbIncludeFooterSetting(NULL), 
	cbWriteFooterTextSetting(NULL), outputFileDirLabel(NULL), outputFileDirText(NULL), 
	outputFileBasenameLabel(NULL), outputFileBasenameText(NULL), fileFormatLabel(NULL), 
	fileFormatDropDownSelector(NULL), okButton(NULL), cancelButton(NULL), 
	openDocsLinkButton(NULL) {

     size_t lastSlashPos = fullSavePath.find_last_of("/");
     dirPath = lastSlashPos != std::string::npos ? fullSavePath.substr(0, lastSlashPos) : "";
     basePath = lastSlashPos != std::string::npos ? fullSavePath.substr(lastSlashPos + 1) : fullSavePath;
     size_t lastExtPos = basePath.find_last_of(".");
     if(lastExtPos != std::string::npos) {
          basePath = basePath.substr(0, lastExtPos);
     }

     int xoffset = (IWIN_WIDTH - IWIN_LABEL_WIDTH - IWIN_INPUT_WIDTH) / 2;
     int yoffset = 10;
     this->begin();

     mainWinInfoBox = new Fl_Box(xoffset, yoffset, IWIN_WIDTH - 2 * xoffset, IWIN_WIDGET_HEIGHT, 
		                 "Select image output options ...");
     mainWinInfoBox->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
     mainWinInfoBox->color(Darker(GUI_BGCOLOR, 0.85));
     mainWinInfoBox->labelcolor(GUI_BTEXT_COLOR);
     mainWinInfoBox->labelfont(FL_HELVETICA_BOLD);
     mainWinInfoBox->labelsize(1.5 * LOCAL_TEXT_SIZE);
     mainWinInfoBox->box(FL_RSHADOW_BOX);
     mainWinInfoBox->labeltype(FL_SHADOW_LABEL);
     yoffset += IWIN_WIDGET_HEIGHT + 2 * IWIN_SPACING;

     outputFileDirLabel = new Fl_Box(xoffset, yoffset, IWIN_LABEL_WIDTH, IWIN_WIDGET_HEIGHT, 
		                     "Output File Folder:");
     outputFileDirLabel->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
     outputFileDirLabel->color(GUI_BGCOLOR);
     outputFileDirLabel->labelcolor(GUI_BTEXT_COLOR);
     outputFileDirLabel->labelfont(FL_HELVETICA_ITALIC);
     outputFileDirLabel->labelsize(LOCAL_TEXT_SIZE);
     outputFileDirText = new Fl_Input(xoffset + IWIN_LABEL_WIDTH, yoffset, IWIN_INPUT_WIDTH, IWIN_WIDGET_HEIGHT);
     outputFileDirText->value(dirPath.c_str());
     outputFileDirText->deactivate();
     yoffset += IWIN_WIDGET_HEIGHT + IWIN_SPACING;
    
     outputFileBasenameLabel = new Fl_Box(xoffset, yoffset, IWIN_LABEL_WIDTH, IWIN_WIDGET_HEIGHT, 
		                     "Output File Folder:");
     outputFileBasenameLabel->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
     outputFileBasenameLabel->color(GUI_BGCOLOR);
     outputFileBasenameLabel->labelcolor(GUI_BTEXT_COLOR);
     outputFileBasenameLabel->labelfont(FL_HELVETICA_ITALIC);
     outputFileBasenameLabel->labelsize(LOCAL_TEXT_SIZE);
     outputFileBasenameText = new Fl_Input(xoffset + IWIN_LABEL_WIDTH, yoffset, IWIN_INPUT_WIDTH, IWIN_WIDGET_HEIGHT);
     outputFileBasenameText->value(basePath.c_str());
     outputFileBasenameText->deactivate();
     yoffset += IWIN_WIDGET_HEIGHT + IWIN_SPACING;
  
     fileFormatLabel = new Fl_Box(xoffset, yoffset, IWIN_LABEL_WIDTH, IWIN_WIDGET_HEIGHT, 
		                  "File Format Selection:");
     fileFormatLabel->align(FL_ALIGN_INSIDE | FL_ALIGN_LEFT);
     fileFormatLabel->color(GUI_BGCOLOR);
     fileFormatLabel->labelcolor(GUI_BTEXT_COLOR);
     fileFormatLabel->labelfont(FL_HELVETICA_ITALIC);
     fileFormatLabel->labelsize(LOCAL_TEXT_SIZE);
     yoffset += IWIN_WIDGET_HEIGHT + IWIN_SPACING;

     fileFormatDropDownSelector = new Fl_Choice(xoffset, yoffset, IWIN_WIDTH - 2 * xoffset, IWIN_WIDGET_HEIGHT);
     for(int ddIdx = 0; ddIdx < GetArrayLength(IMAGE_TYPE_SPECS); ddIdx++) {
	  fileFormatDropDownSelector->add(IMAGE_TYPE_SPECS[ddIdx].imgTypeLongDesc);
     }
     fileFormatDropDownSelector->value(0);
     fileFormatDropDownSelector->callback(FileFormatDropDownCallback);
     yoffset += IWIN_WIDGET_HEIGHT + 2 * IWIN_SPACING;

     okButton = new Fl_Button(IWIN_WIDTH - 2.5 * xoffset, yoffset, IWIN_BUTTON_WIDTH, IWIN_WIDGET_HEIGHT, 
		              "Ok  @returnarrow");
     okButton->align(FL_ALIGN_CENTER | FL_ALIGN_INSIDE);
     okButton->color(GUI_BGCOLOR);
     okButton->labelcolor(GUI_BTEXT_COLOR);
     okButton->labelfont(FL_HELVETICA_BOLD);
     okButton->labelsize(1.25 * LOCAL_TEXT_SIZE);
     okButton->box(FL_PLASTIC_UP_BOX);
     okButton->labeltype(FL_SHADOW_LABEL);
     okButton->callback(OKButtonCallback);

     this->end();
     this->show();

}

InputWindowExportImage::~InputWindowExportImage() {
     Delete(mainWinInfoBox, Fl_Box);
     Delete(cbOverwriteSetting, Fl_Check_Button);
     Delete(cbUpdateCfgSetting, Fl_Check_Button);
     Delete(cbIncludeFooterSetting, Fl_Check_Button);
     Delete(cbWriteFooterTextSetting, Fl_Check_Button);
     Delete(outputFileDirLabel, Fl_Box);
     Delete(outputFileDirText, Fl_Input);
     Delete(outputFileBasenameLabel, Fl_Box);
     Delete(outputFileBasenameText, Fl_Input);
     Delete(fileFormatDropDownSelector, Fl_Choice);
     Delete(okButton, Fl_Button);
     Delete(cancelButton, Fl_Button);
     Delete(openDocsLinkButton, Fl_Button);
}

ImageType_t InputWindowExportImage::GetImageOutputFormat() const {
     int selectedIdx = fileFormatDropDownSelector->value();
     return IMAGE_TYPE_SPECS[selectedIdx].imgTypeID;
}

std::string InputWindowExportImage::GetOutputFileSavePath() const {
     std::string dirStr = dirPath;
     std::string baseNameStr = basePath;
     std::string fileExt = std::string(".") + IMAGE_TYPE_SPECS[fileFormatDropDownSelector->value()].imgTypeFileExt;
     baseNameStr += fileExt;
     if((dirStr.length() >= 1 && dirStr.at(dirStr.length() - 1) == '/') || 
	(baseNameStr.length() >= 1 && baseNameStr.at(0) == '/')) {
          return dirStr + baseNameStr;
     }
     return dirStr + std::string("/") + baseNameStr;
}

bool InputWindowExportImage::WriteImageToFile(cairo_surface_t *imageDataSurface) {
     if(imageDataSurface == NULL) {
          return false;
     }
     std::string savePath = GetOutputFileSavePath();
     ImageType_t imageFormatType = GetImageOutputFormat();
     return !WriteImageToFile(savePath, imageFormatType, imageDataSurface);
}

bool InputWindowExportImage::WriteImageToFile(std::string savePath, ImageType_t outputType, cairo_surface_t *imageDataSurface) {
     if(imageDataSurface == NULL) {
          return false;
     }
     switch(outputType) {
          case IMGTYPE_PNG:
	       return !WriteCairoToPNGImage(savePath, imageDataSurface);
	  case IMGTYPE_SVG:
	       return !WriteCairoToSVGImage(savePath, imageDataSurface);
	  case IMGTYPE_CSRCHDR:
	       return !WriteCairoToCStyleHeaderFile(savePath, imageDataSurface);
	  default:
	       return false;
     }
     return false;
}

void InputWindowExportImage::FileFormatDropDownCallback(Fl_Widget *ddWidget, void *udata) {
     //Fl_Choice dd = (Fl_Choice *) ddWidget;
     //InputWindowExportImage *imgWin = (InputWindowExportImage *) ddWidget->parent();
     //int selectedIdx = dd->value();
     //const char *fileExt = INAGE_TYPE_SPECS[selectedIdx].imgTypeFileExt;
}

void InputWindowExportImage::OKButtonCallback(Fl_Widget *okBtn, void *udata) {
     InputWindowExportImage *inputWin = (InputWindowExportImage *) okBtn->parent();
     inputWin->windowDone = true;
     inputWin->hide();
}

int InputWindowExportImage::WriteCairoToPNGImage(std::string outputPath, cairo_surface_t *imageDataSurface) {
     bool writeStatus = cairo_surface_write_to_png(imageDataSurface, outputPath.c_str()) != CAIRO_STATUS_SUCCESS;
     return writeStatus;
}

int InputWindowExportImage::WriteCairoToSVGImage(std::string outputPath, cairo_surface_t *imageDataSurface) {
     int imgWidth = cairo_image_surface_get_width(imageDataSurface);
     int imgHeight = cairo_image_surface_get_height(imageDataSurface);
     cairo_surface_t *svgSurface = cairo_svg_surface_create(outputPath.c_str(), imgWidth, imgHeight);
     cairo_svg_surface_restrict_to_version(svgSurface, CAIRO_SVG_VERSION_1_2);
     cairo_t *crContext = cairo_create(svgSurface);
     cairo_set_source_surface(crContext, imageDataSurface, 0, 0);
     cairo_rectangle(crContext, 0, 0, imgWidth, imgHeight);
     cairo_fill(crContext);
     cairo_surface_flush(svgSurface);
     //crContext->show_page();
     cairo_surface_finish(svgSurface);
     cairo_surface_destroy(svgSurface);
     cairo_destroy(crContext);
     return 0;
}

int InputWindowExportImage::WriteCairoToCStyleHeaderFile(std::string outputPath, cairo_surface_t *imageDataSurface) {
     
     unsigned int imgWidth = cairo_image_surface_get_width(imageDataSurface);
     unsigned int imgHeight = cairo_image_surface_get_height(imageDataSurface);
     unsigned char *imgPixelBuf = cairo_image_surface_get_data(imageDataSurface);

     char timeStampHdrDefine[512];
     time_t currentTime = time(NULL);
     struct tm *tmCurrentTime = localtime(&currentTime);
     strftime(timeStampHdrDefine, 511, " __RNASTRUCTVIZ_IMAGE_%Y%m%d%H%M%S_C__\n", tmCurrentTime);

     FILE *outputFP = fopen(outputPath.c_str(), "w+");
     if(outputFP == NULL) {
	  TerminalText::PrintDebug("Unable to open image file \"%s\" for writing: %s\n", outputPath.c_str(), strerror(errno));
          return errno;
     }
     
     fputs("#ifndef", outputFP);
     fputs(timeStampHdrDefine, outputFP);
     fputs("#define", outputFP);
     fputs(timeStampHdrDefine, outputFP);
     fputs("\n", outputFP);

     fputs("#ifndef __PIXEL_DATA_STRUCT_TYPE_H__\n", outputFP);
     fputs("typedef struct {\n     unsigned int width;\n     unsigned int height;\n     "
	   "unsigned int bytes_per_pixel;\n     const unsigned char *pixel_data;\n} PixelDataStruct_t;", outputFP);
     fputs("\n#endif\n\n", outputFP);

     char tempBuf[MAX_BUFFER_SIZE];
     snprintf(tempBuf, MAX_BUFFER_SIZE, "     %d, %d, 4, (const unsigned char *)\n", imgWidth, imgHeight);
     strftime(tempBuf, MAX_BUFFER_SIZE, "static PixelDataStruct_t RNAStructVizImage_%Y%m%d%H%M%S = {\n", tmCurrentTime);
     fputs("", outputFP);
     fputs(tempBuf, outputFP);
     for(int i = 0; i < 2 * imgWidth * imgHeight; i += 2) {
          if(i % (2 * imgWidth) == 0) {
               fputs("     \"", outputFP);
	  }
	  short int pixelData = (short int) imgPixelBuf[i];
	  sprintf(tempBuf, "\\0%02x", pixelData);
	  fputs(tempBuf, outputFP);
	  if((i + 2) % (2 * imgWidth) == 0) {
	       fputs("\"\n", outputFP);
	  }
     }
     fputs("};\n\n#endif", outputFP);

     fclose(outputFP);
     return 0;

}
