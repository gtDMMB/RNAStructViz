/* CairoDrawingUtils.h : Wrapper around some nicer and more common drawing operations we 
 *                       can perform on cairo surfaces;
 * Author: Maxie D. Schmidt (maxieds@gmail.com)
 * Created: 2019.03.11
 */

#ifndef __CAIRO_DRAWING_UTILS_H__
#define __CAIRO_DRAWING_UTILS_H__

#include <ctype.h>
#include <inttypes.h>
#include <cairo.h>

#include <FL/fl_draw.H>
#include <FL/Enumerations.H>

namespace CairoDraw {

class CairoContext_t { 

     public:
          class CairoColor_t {

	       public:
		    typedef unsigned short CairoRGBA_t;
                    static const CairoRGBA_t CAIRO_COLOR_DEFAULT_ALPHA = 0x99;
		    static const CairoRGBA_t CAIRO_COLOR_TRANSPARENT = 0x00;
		    static const CairoRGBA_t CAIRO_COLOR_OPAQUE = 0xff;
	            static const CairoRGBA_t CAIRO_COLOR_RGBA_MAXVAL = 0xff;

                    typedef enum {
                         CR_UNDEFINED   = -1, 
			 CR_BLACK       = 0, 
                         CR_RED         = 1, 
                         CR_GREEN       = 2, 
                         CR_BLUE        = 3, 
                         CR_YELLOW      = 4, 
                         CR_MAGENTA     = 5, 
                         CR_CYAN        = 6, 
                         CR_BRANCH1     = 7, 
                         CR_BRANCH2     = 8, 
                         CR_BRANCH3     = 9, 
                         CR_BRANCH4     = 10, 
                         CR_WHITE       = 11,
                         CR_TRANSPARENT = 12,
                         CR_SOLID_BLACK = 14,
                         CR_SOLID_WHITE = 15, 
                         CR_LIGHT_GRAY  = 16, 
                    } CairoColorSpec_t;

		    CairoColorSpec_t ConvertFromFLColor(const Fl_Color &flColor) const;
                    Fl_Color ConvertToFLColor(CairoColorSpec_t &namedCairoColor) const;

	       private:
                    CairoRGBA_t R, G, B, A;
	       
	       public:
		    CairoColor_t(CairoRGBA_t r = 0, CairoRGBA_t g = 0, CairoRGBA_t b = 0, 
				 CairoRGBA_t alpha = CAIRO_COLOR_DEFAULT_ALPHA);

		    CairoColor_t & SetRGB(CairoRGBA_t r, CairoRGBA_t g, CairoRGBA_t b);
		    CairoColor_t & SetRGBA(CairoRGBA_t r, CairoRGBA_t g, CairoRGBA_t b, 
				 CairoRGBA_t alpha = CAIRO_COLOR_DEFAULT_ALPHA);
		    CairoColor_t & SetRed(CairoRGBA_t red);
		    CairoColor_t & SetRedRatio(float redPct);
		    CairoColor_t & SetGreen(CairoRGBA_t green);
		    CairoColor_t & SetGreenRatio(float greenPct);
		    CairoColor_t & SetBlue(CairoRGBA_t blue);
		    CairoColor_t & SetBlueRatio(float bluePct);
		    CairoColor_t & SetAlpha(CairoRGBA_t alpha);
		    CairoColor_t & SetAlphaRatio(float alphaPct);
		    CairoColor_t & ScaleAlpha(float ratio);

                    CairoRGBA_t Red() const;
		    double GetRedRatio() const;
		    CairoRGBA_t Green() const;
		    double GetGreenRatio() const;
		    CairoRGBA_t Blue() const;
		    double GetBlueRatio() const;
		    CairoRGBA_t Alpha() const;
		    double GetAlphaRatio() const;

		    Fl_Color ToFLColorType() const;
		    static CairoColor_t FromFLColorType(Fl_Color flColor);
		    unsigned int ToHexInteger() const;
		    CairoColor_t & FromHexInteger(unsigned int hexColor);
                    CairoColor_t & FromNamedConstant(const CairoColorSpec_t &namedConstant);

		    static CairoColor_t GetCairoColor(const CairoColorSpec_t &namedConstant);
		    static CairoColor_t GetCairoColor(CairoRGBA_t red, CairoRGBA_t green, 
				                      CairoRGBA_t blue, 
						      CairoRGBA_t alpha = CAIRO_COLOR_DEFAULT_ALPHA);
		    static CairoColor_t GetCairoColorFromRatio(float redPct, float greenPct, 
				                               float bluePct, float alphaPct);

		    bool ApplyRGBAColor(cairo_t *crContext) const;
		    bool ApplyRGBAColor(CairoContext_t &crContext) const;

		    CairoColor_t & operator+=(const CairoColor_t &rhsColor);
		    CairoColor_t & operator-=(const CairoColor_t &rhsColor);
		    
		    bool operator==(const CairoColor_t &rhsColor) const;

		    /* Transforms of the color: */
		    CairoColor_t Lighten(float pct);
		    CairoColor_t Darken(float pct);
		    CairoColor_t ToGrayscale();
		    CairoColor_t ToTransparent();
		    CairoColor_t ToOpaque();
		    CairoColor_t Tint(const CairoColor_t &tintColor, float pct);

	       public:
		    class ColorUtil {
                         
		         public:
                              static const uint32_t RGBHEX_RED_RSHIFT   = 24;
			      static const uint32_t RGBHEX_RED_MASK     = 0xff000000;
			      static const uint32_t RGBHEX_GREEN_RSHIFT = 16;
			      static const uint32_t RGBHEX_GREEN_MASK   = 0x00ff0000;
			      static const uint32_t RGBHEX_BLUE_RSHIFT  = 8;
			      static const uint32_t RGBHEX_BLUE_MASK    = 0x0000ff00;

			      static uint32_t RGBRedComponent(uint32_t rgbHexColor);
                              static uint32_t RGBGreenComponent(uint32_t rgbHexColor);
                              static uint32_t RGBBlueComponent(uint32_t rgbHexColor);
			      
			      static uint32_t GetRGBColor(uint8_t r, uint8_t g, uint8_t b);
			      static uint32_t Lighter(uint32_t rgbHexColor, float alpha);
			      static uint32_t Darker(uint32_t rgbHexColor, float alpha);
			      static Fl_Color Inactive(Fl_Color flColor);
			      static Fl_Color Contrast(Fl_Color flColor);
			      static uint32_t RGBGetRed(uint32_t rgbHexColor);
                              static uint32_t RGBGetGreen(uint32_t rgbHexColor);
                              static uint32_t RGBGetBlue(uint32_t rgbHexColor);
			      static uint32_t RGBHexTupleFromFLColor(Fl_Color flColor);

		    }; // class ColorUtil

	  }; // class CairoColor_t

     public:
	  typedef enum {
	       CENTER, 
	       LEFT_JUSTIFY, 
	  } CairoTextDrawParams_t;

	  typedef enum {
	       SERIF     = 1, 
	       MONOSPACE = 2,
	       FUNKY     = 4
	  } CairoFontFace_t;
          static const uint32_t CAIRO_FONT_FACE_MASK = 0x000000ff;

	  typedef enum {
	       BOLD      = 1 << 8, 
	       ITALIC    = 2 << 8, 
	       NORMAL    = 4 << 8,
	       OBLIQUE   = 8 << 8
	  } CairoFontStyle_t;
	  static const uint32_t CAIRO_FONT_STYLE_MASK = 0x0000ff00;

	  typedef enum {
	       CIRCLE_NODE, 
	       SQUARE_NODE
	  } NodeStyle_t;

	  typedef enum {
	       FONT_SIZE_TINY      = 6, 
	       FONT_SIZE_SMALL     = 9, 
	       FONT_SIZE_NORMAL    = 11,
	       FONT_SIZE_LARGE     = 15, 
	       FONT_SIZE_HUGE      = 18, 
	       FONT_SIZE_HEADER    = 17, 
	       FONT_SIZE_SUBHEADER = 12
	  } FontSize_t;

     private:
	  cairo_surface_t *cairoSurface;
          cairo_t *cairoContext;
          size_t width, height;

	  void FreeCairoStructures();
	  bool InitCairoStructures(size_t width, size_t height);
	  bool InitCairoStructures(cairo_t *crContext);
	  bool CopyContextData(const CairoContext_t &crContext);

	  cairo_font_slant_t ExtractFontSlantFromStyle(uint16_t fontStyle);
	  cairo_font_weight_t ExtractFontWeightFromStyle(uint16_t fontStyle);

     public:
	  CairoContext_t();
          CairoContext_t(size_t width, size_t height);
	  CairoContext_t(cairo_t *crContext);
	  CairoContext_t(const CairoContext_t &cctxt);
	  CairoContext_t & operator=(const CairoContext_t &rhsContext);
	  ~CairoContext_t();

	  bool operator==(const uintptr_t &ptrAddr) const; /* Can Check: Context == NULL; */
          cairo_t * operator->() const;              /* Can Call: crContext->cairo_op(); */

	  size_t GetWidth() const;
	  size_t GetHeight() const;
	  bool Initialized() const;
	  cairo_t * GetCairoContext() const;
	  cairo_surface_t * GetCairoSurface() const;

	  bool LoadFromImage(const char *imageFilePath);
	  bool SaveToImage(const char *imageOutPath);
          CairoContext_t & Resize(size_t newWidth, size_t newHeight, bool scalePrevPixels = false);

	  void SaveSettings();
	  void RestoreSettings();

	  bool SetFontFace(const char *fontFaceName, uint16_t fontStyle);
	  bool SetFontFace(uint16_t fontStyleProps);
	  bool SetFontSize(int fontSize);
	  bool SetStrokeSize(int strokeSize);

	  /* Convenience color transformations: */
          bool CairoToGrayscale();
	  bool SetColor(const CairoColor_t &cairoColor);

	  /* Custom drawing items: */
	  bool BlankFillCanvas(CairoColor_t cairoFillColor);
	  bool Scale(double sxy);
	  bool Scale(double sx, double sy);
	  bool Translate(int xoffset, int yoffset);
          bool ResetToIdentityTransform();

	  bool OverlayGraphics(const CairoContext_t &overlayContext, int startX, int startY);
	  bool DrawBaseNode(int centerX, int centerY, const char *nodeLabel, 
			    size_t nodeSize, CairoColor_t cairoBaseColor, 
			    NodeStyle_t nodeStyle = CIRCLE_NODE);
	  bool DrawLine(size_t baseX, size_t baseY, size_t nextX, size_t nextY);
	  bool DrawText(size_t baseX, size_t baseY, const char *text, 
			CairoTextDrawParams_t = LEFT_JUSTIFY);

}; // class CairoContext_t 

} // namespace CairoDraw

/* Typedefs for convenience (keep from having to over-scope everything): */
using namespace CairoDraw;
typedef CairoContext_t::CairoColor_t::CairoColorSpec_t CairoColorSpec_t;
typedef CairoContext_t::CairoColor_t CairoColor_t;
typedef CairoColor_t::CairoRGBA_t CairoRGBA_t;
typedef CairoContext_t::CairoColor_t::ColorUtil ColorUtil;

#endif
