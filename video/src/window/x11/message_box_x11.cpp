/*******************************************************************************
Author  :     Romain Vinders
License :     MIT
--------------------------------------------------------------------------------
Description : Message box - Xorg implementation (Linux/Unix/BSD)
*******************************************************************************/
#if !defined(_WINDOWS) && !defined(__APPLE__) && !defined(__ANDROID__) && !defined(_P_ENABLE_LINUX_WAYLAND) && (defined(__linux__) || defined(__linux) || defined(__unix__) || defined(__unix))
# include <cassert>
# include <cstdint>
# include <cstring>
# include <stdexcept>
# include <locale.h>
# include "hardware/_private/_libraries_x11.h"
# include "video/_private/_message_box_common.h"
# include "video/message_box.h"

  using namespace pandora::video;
  using pandora::hardware::LibrariesX11;
  
  
  namespace __XMessageBox {
    // Size info container
    struct Size {
      uint32_t width;
      uint32_t height;
    };
    
    // Button description (for X11 event loop)
    struct ButtonParams {
      XRectangle area;
      const char* label;
      GC* sharedGC;
      GC* sharedHoverGC;
      GC* sharedClickGC;
      GC* activeGC;
    };
    
    // RAII message container (exception-safe)
    class MessageLines {
    public:
      MessageLines() {
        parentLocale = setlocale(LC_ALL, nullptr);
        setlocale(LC_ALL, "");
      }
      ~MessageLines() {
        if (lines) free(lines);
        if (lengths) free(lengths);
        if (buffer) free(buffer);
        if (parentLocale) setlocale(LC_ALL, parentLocale);
      }

      inline char** getLines() noexcept { return lines; }
      inline uint32_t* getLengths() noexcept { return lengths; }
      inline uint32_t size() noexcept { return linesCount; }
      
      // Split multi-line message into list of lines
      void splitLines(const char* message); // throws

    private:
      char* parentLocale = nullptr;
      char* buffer = nullptr;          // string: copy of all message lines, separated by '0' characters
      char** lines = nullptr;          // array: pointers to the beginning of each line in 'buffer'
      uint32_t* lengths = nullptr; // array: length of each line in 'buffer'
      uint32_t linesCount = 0;         // size of arrays
    };
  }


// -- message split helpers -- -------------------------------------------------

# define __P_MAX_LINE_NUMBER 48 // max 48 lines
# define __P_MAX_LABEL_LENGTH 48 // max 48 characters

  // Split multi-line message into list of lines
  void __XMessageBox::MessageLines::splitLines(const char* message) {
    size_t messageLength = (message != nullptr) ? strnlen(message, 1600) : 0;
    buffer = (messageLength > 0) ? strdup(message) : (char*)calloc(1, sizeof(char));
    if (buffer == nullptr)
      throw std::runtime_error("splitMessageLines: strdup: memory allocation failure");

    // count lines + store line positions + replace new-lines by end of substrings
    linesCount = 1;   
    uint32_t beginPositions[__P_MAX_LINE_NUMBER];
    beginPositions[0] = 0;
    uint32_t currentPos = 0;
    for (char* it = buffer; *it; ++it, ++currentPos) {
      if (*it == '\n') {
        *it = '\0'; // replace end-of-line by end-of-string for each line
        beginPositions[linesCount] = currentPos + 1; // store next line position
        ++linesCount;
        if (linesCount >= __P_MAX_LINE_NUMBER)
          break;
      }
    }
    if (linesCount > 1 && buffer[beginPositions[linesCount - 1] ] == 0) // empty last line: remove it (unless only 1 line)
      --linesCount;
    
    // store pointer to beginning of each line + length of each line
    lines = (char**)malloc(linesCount*sizeof(char*));
    lengths = (uint32_t*)malloc(linesCount*sizeof(uint32_t));
    if (lines == nullptr || lengths == nullptr)
      throw std::runtime_error("splitMessageLines: malloc: memory allocation failure");
    
    for (uint32_t i = 0; i < linesCount; ++i) {
      lines[i] = &(buffer[beginPositions[i] ]);
      
      if (i + 1 < linesCount)
        lengths[i] = beginPositions[i + 1] - beginPositions[i] - 1;
      else
        lengths[i] = strlen(lines[i]);
    }
  }
  

// -- size/position helpers ----------------------------------------------------

# define __P_MIN_BOX_WIDTH  200
# define __P_MIN_BOX_HEIGHT 100
# define __P_BOX_PADDING    28

# define __P_LINE_SPACING        5
# define __P_DEFAULT_CHAR_WIDTH  10
# define __P_DEFAULT_CHAR_HEIGHT 16
# define __P_MAX_MESSAGE_WIDTH   1300
# define __P_MSG_BUTTON_SPACING  10

# define __P_MIN_BUTTON_WIDTH     74
# define __P_BUTTON_HEIGHT        24
# define __P_BUTTON_SPACING       10
# define __P_MIN_BUTTON_LABEL_PAD 12
# define __P_BUTTON_PADDING_Y     12
# define __P_BUTTON_BAR_HEIGHT    (__P_BUTTON_HEIGHT + __P_BUTTON_PADDING_Y*2)

  // estimate display size of full message
  static void __computeMessageSize(LibrariesX11& libs, XFontSet& fontSet, __XMessageBox::MessageLines& message, 
                                   __XMessageBox::Size& outSize, uint32_t& outLineHeight) noexcept {
    outSize.width = outSize.height = 0;
    outLineHeight = 0;
    
    if (libs.xlib.utf8TextExtents_) { // X11 calculation
      XRectangle area = { 0, 0, 0, 0 };
      for (uint32_t i = 0; i < message.size(); ++i) {
        libs.xlib.utf8TextExtents_(fontSet, message.getLines()[i], (int)message.getLengths()[i], &area, nullptr);
        
        if (area.width > outSize.width) {
          outSize.width = area.width;
          outLineHeight = area.height;
        }
        outSize.height += area.height + __P_LINE_SPACING;
      }
    }
    else { // if not available, make rough estimation
      for (uint32_t i = 0; i < message.size(); ++i) { // find max length
        if (message.getLengths()[i] > outSize.width)
          outSize.width = message.getLengths()[i];
      }
      outSize.width = outSize.width*__P_DEFAULT_CHAR_WIDTH; // length to pixels
      outSize.height = (__P_DEFAULT_CHAR_HEIGHT + __P_LINE_SPACING)*message.size();
      outLineHeight = __P_DEFAULT_CHAR_HEIGHT;
    }
    
    if (outSize.width > __P_MAX_MESSAGE_WIDTH)
      outSize.width = __P_MAX_MESSAGE_WIDTH;
    if (outLineHeight == 0)
      outLineHeight = __P_DEFAULT_CHAR_HEIGHT;
  }
  
  // estimate display size of all buttons (outSize must be pre-allocated ButtonParams[nbButtons])
  static void __computeButtonsSize(LibrariesX11& libs, XFontSet& fontSet,
                                   const char** buttonLabels, uint32_t nbButtons,
                                   __XMessageBox::ButtonParams* outSizes) noexcept {
    if (libs.xlib.utf8TextExtents_) { // X11 calculation
      for (uint32_t i = 0; i < nbButtons; ++i) {
        __XMessageBox::ButtonParams& cur = outSizes[i];
        libs.xlib.utf8TextExtents_(fontSet, const_cast<char*>(buttonLabels[i]), (int)strlen(buttonLabels[i]), &(cur.area), nullptr);
        cur.area.width += __P_MIN_BUTTON_LABEL_PAD*2;
        
        cur.area.x = (i > 0) ? (outSizes[i - 1].area.x + (int)outSizes[i - 1].area.width + __P_BUTTON_SPACING) : 0;
        cur.area.y = __P_BUTTON_PADDING_Y;
        if (cur.area.width < __P_MIN_BUTTON_WIDTH) 
          cur.area.width = __P_MIN_BUTTON_WIDTH;
        cur.area.height = __P_BUTTON_HEIGHT;
      }
    }
    else { // if not available, make rough estimation
      for (uint32_t i = 0; i < nbButtons; ++i) {
        __XMessageBox::ButtonParams& cur = outSizes[i];
        cur.area.width = (uint32_t)strlen(buttonLabels[i])*__P_DEFAULT_CHAR_WIDTH + __P_MIN_BUTTON_LABEL_PAD*2;
        
        cur.area.x = (i > 0) ? (outSizes[i - 1].area.x + (int)outSizes[i - 1].area.width + __P_BUTTON_SPACING) : 0;
        cur.area.y = __P_BUTTON_PADDING_Y;
        if (cur.area.width < __P_MIN_BUTTON_WIDTH) 
          cur.area.width = __P_MIN_BUTTON_WIDTH;
        cur.area.height = __P_BUTTON_HEIGHT;
      }
    }
  }
  
  // estimate display size for dialog (outButtonSize must have 3 slots)
  static void __computeDialogArea(LibrariesX11& libs, Window parent, XFontSet& fontSet, __XMessageBox::MessageLines& message, 
                                  __XMessageBox::Size& messageSize, const __XMessageBox::ButtonParams* buttonSizes, uint32_t nbButtons,
                                  XRectangle& outDialogSize) noexcept {
    // dimensions
    outDialogSize.width = messageSize.width + __P_BOX_PADDING*2;
    uint32_t buttonBarWidth = (uint32_t)buttonSizes[nbButtons - 1].area.x + buttonSizes[nbButtons - 1].area.width + __P_BOX_PADDING*2;
    if (outDialogSize.width < buttonBarWidth)
      outDialogSize.width = buttonBarWidth;
    if (outDialogSize.width < __P_MIN_BOX_WIDTH)
      outDialogSize.width = __P_MIN_BOX_WIDTH;
    
    outDialogSize.height = __P_BOX_PADDING + messageSize.height + __P_MSG_BUTTON_SPACING + __P_BUTTON_PADDING_Y*2 + __P_BUTTON_HEIGHT;
    if (outDialogSize.height < __P_MIN_BOX_HEIGHT)
      outDialogSize.height = __P_MIN_BOX_HEIGHT;
    
    // position
    outDialogSize.x = outDialogSize.y = 0; // relative to parent window
    
    XWindowAttributes parentAttr;
    if (libs.xlib.GetWindowAttributes_(libs.displayServer, parent, &parentAttr)) {
      if (parentAttr.width > outDialogSize.width)
        outDialogSize.x = (parentAttr.width - outDialogSize.width) >> 1;
      if (parentAttr.height > outDialogSize.height)
        outDialogSize.y = (parentAttr.height - outDialogSize.height) >> 1;
    }
  }


// -- components helpers -- ----------------------------------------------------
  
  // set dialog size limits
  static inline void __setDialogLimits(LibrariesX11& libs, Window handle, uint32_t width, uint32_t height) noexcept {
    XSizeHints hints;
    hints.flags      = (PSize | PMinSize | PMaxSize);
    hints.min_width  = hints.max_width  = hints.base_width  = width;
    hints.min_height = hints.max_height = hints.base_height = height;
    libs.xlib.SetWMNormalHints_(libs.displayServer, handle, &hints);
  }

  // configure dialog caption (text + type)
  static inline void __setDialogCaption(LibrariesX11& libs, Window handle, const char* caption) noexcept {
    if (libs.atoms.NET_WM_NAME && libs.atoms.UTF8_STRING) {
      const char* title = (caption) ? caption : "Error";
      int length = (caption) ? (int)strnlen(caption, 128) : 5;
      libs.xlib.ChangeProperty_(libs.displayServer, handle, libs.atoms.NET_WM_NAME, libs.atoms.UTF8_STRING, 8,
                                PropModeReplace, (const unsigned char*)title, length);
    }
    if (libs.atoms.NET_WM_WINDOW_TYPE && libs.atoms.NET_WM_WINDOW_TYPE_DIALOG) {
      libs.xlib.ChangeProperty_(libs.displayServer, handle, libs.atoms.NET_WM_WINDOW_TYPE, XA_ATOM, 32,
                                PropModeReplace, (unsigned char *)&(libs.atoms.NET_WM_WINDOW_TYPE_DIALOG), 1);
    }
  }
  
  // ---
  
# define __COLOR_16BIT(val8bit) (((uint32_t)val8bit * 65535u) / 255u)
  
  // configure graphics context (colors) 
  // remark: call COLOR_16BIT for color components
  static inline void __setGraphicsContext(LibrariesX11& libs, Window handle, const Colormap& colors,
                                          uint32_t red16b, uint32_t green16b, uint32_t blue16b, GC& outGC) noexcept {
    XColor color;
    memset((void*)&color, 0, sizeof(color));
    color.red   = (unsigned short)red16b;
    color.green = (unsigned short)green16b;
    color.blue  = (unsigned short)blue16b;
    color.flags = DoRed | DoGreen | DoBlue;
    
    outGC = libs.xlib.CreateGC_(libs.displayServer, handle, 0, nullptr);
    libs.xlib.AllocColor_(libs.displayServer, colors, &color);
    libs.xlib.SetForeground_(libs.displayServer, outGC, color.pixel);
  }
  
  // configure font set
  static inline void __setFontSet(LibrariesX11& libs, XFontSet& outFontSet) {
    char** missingCharset = nullptr;
    int missingCharsetLength = 0;
    const char* fontNameList = "-*-*-medium-r-*-*-*-140-60-60-*-*-*-*";
    outFontSet = libs.xlib.CreateFontSet_(libs.displayServer, const_cast<char*>(fontNameList),
                                          &missingCharset, &missingCharsetLength, nullptr);
    if (!outFontSet) {
      fontNameList = "*-18-*";
      outFontSet = libs.xlib.CreateFontSet_(libs.displayServer, const_cast<char*>(fontNameList),
                                          &missingCharset, &missingCharsetLength, nullptr);
      if (!outFontSet) {
        fontNameList = "*";
        outFontSet = libs.xlib.CreateFontSet_(libs.displayServer, const_cast<char*>(fontNameList),
                                            &missingCharset, &missingCharsetLength, nullptr);
        if (!outFontSet)
          throw std::runtime_error("X11: could not load base font");
      }
    }
    if (missingCharset)
      libs.xlib.FreeStringList_(missingCharset);
  }
  

// -- dialog events/drawing -- -------------------------------------------------

  // verify if a position is inside a rectangle
  static inline bool __isPositionInArea(int x, int y, const XRectangle& area){
    return (x >= area.x && y >= area.y && x < (area.x + area.width) && y < (area.y + area.height));
  }

  // process native dialog event loop (draw + wait for user action)
  // returns: user action (button index with base 1)
  static uint32_t __processDialogEvents(Window handle, XFontSet& fontSet, GC& textGC, GC& buttonBarGC, 
                                        const XRectangle& dialogSize, uint32_t textLineHeight, __XMessageBox::MessageLines& message, 
                                        __XMessageBox::ButtonParams* buttonParams, uint32_t nbButtons) {
    LibrariesX11& libs = LibrariesX11::instance();
    uint32_t userAction = 0;
    bool isRunning = true;
    
    while (isRunning) {
      XEvent e;
      libs.xlib.NextEvent_(libs.displayServer, &e);

      switch (e.type) {
        // -- user actions --
        case MotionNotify:
        case ButtonPress:
        case ButtonRelease:
          for (uint32_t i = 0; i < nbButtons; ++i) {
            GC* prevGC = buttonParams[i].activeGC;
            
            if (__isPositionInArea(e.xmotion.x, e.xmotion.y, buttonParams[i].area)) {
              if (e.type == ButtonPress && e.xbutton.button == Button1) { // if left click
                buttonParams[i].activeGC = buttonParams[i].sharedClickGC;
                userAction = i + 1;
              }
              else { // hover or click release
                buttonParams[i].activeGC = buttonParams[i].sharedHoverGC;
                if (e.type == ButtonRelease && userAction != 0)
                  isRunning = false;
              }
            }
            else
              buttonParams[i].activeGC = buttonParams[i].sharedGC;
            
            // repaint on status change
            if (isRunning && prevGC != buttonParams[i].activeGC) {
              XEvent redrawEvent;
              memset(&redrawEvent, 0, sizeof(redrawEvent));
              redrawEvent.type = Expose;
              redrawEvent.xexpose.window = handle;
              libs.xlib.SendEvent_(libs.displayServer, handle, False, ExposureMask, &redrawEvent);
              libs.xlib.Flush_(libs.displayServer);
            }
          }
        case ClientMessage:
          if((unsigned int)(e.xclient.data.l[0]) == (unsigned int)libs.atoms.WM_DELETE_WINDOW) { // close button
            userAction = nbButtons; // use last button (cancellation)
            isRunning = false;
          }
          break;
          
        // -- drawing --
        case Expose: {
          // message
          for (uint32_t i = 0; i < message.size(); ++i) {
            uint32_t posY = __P_BOX_PADDING + i*(__P_LINE_SPACING + textLineHeight) + (textLineHeight >> 1);
            libs.xlib.utf8DrawString_(libs.displayServer, handle, fontSet, textGC, __P_BOX_PADDING, posY, 
                                      message.getLines()[i], (int)message.getLengths()[i]);
          }
          
          // button bar
          libs.xlib.FillRectangle_(libs.displayServer, handle, buttonBarGC, 
                                   0, dialogSize.height - __P_BUTTON_BAR_HEIGHT, 
                                   dialogSize.width, __P_BUTTON_BAR_HEIGHT);
          // buttons
          for (uint32_t i = 0; i < nbButtons; ++i) {
            __XMessageBox::ButtonParams& params = buttonParams[i];
            libs.xlib.FillRectangle_(libs.displayServer, handle, *(params.activeGC), 
                                     params.area.x, params.area.y, params.area.width, params.area.height);
            
            XRectangle labelArea;
            int labelLength = (int)strlen(params.label);
            if (libs.xlib.utf8TextExtents_)
              libs.xlib.utf8TextExtents_(fontSet, const_cast<char*>(params.label), labelLength, &labelArea, nullptr);
            else {
              labelArea.width = labelLength * __P_DEFAULT_CHAR_WIDTH;
              labelArea.height = __P_DEFAULT_CHAR_HEIGHT;
            }
            libs.xlib.utf8DrawString_(libs.displayServer, handle, fontSet, textGC, 
                                      params.area.x + (int)((params.area.width - labelArea.width) >> 1),
                                      params.area.y + (int)((params.area.height - labelArea.height) >> 1) + (int)textLineHeight - (int)(textLineHeight>>3),
                                      const_cast<char*>(params.label), labelLength);
          }
          libs.xlib.Flush_(libs.displayServer);
          break;
        }
        default:
          break;
      }
    }
    return userAction;
  }


// -- simple message box -- ----------------------------------------------------

  // create native dialog
  static uint32_t __showX11MessageBox(const char* caption, const char* message, 
                                      const char** actions, uint32_t actionsLength, WindowHandle parent) noexcept {
    uint32_t userAction = 0;
    try {
      LibrariesX11& libs = LibrariesX11::instance(); // throws
      if (!libs.xlib.CreateSimpleWindow_ || !libs.xlib.GetWindowAttributes_ || !libs.xlib.ChangeProperty_)
        throw std::runtime_error("X11: XCreateSimpleWindow not available on current system");

      // split message lines
      __XMessageBox::MessageLines messageLines;
      messageLines.splitLines(message); // throws
      
      // create window
      __XMessageBox::ButtonParams buttons[3];
      Window parentHandle = (parent) ? (Window)parent : libs.rootWindow;
      XFontSet fontSet;
      __setFontSet(libs, fontSet); // throws
      
      XRectangle dialogSize;
      uint32_t textLineHeight;
      __XMessageBox::Size textBlockSize;
      __computeMessageSize(libs, fontSet, messageLines, textBlockSize, textLineHeight);
      __computeButtonsSize(libs, fontSet, actions, actionsLength, &buttons[0]);
      __computeDialogArea(libs, parentHandle, fontSet, messageLines, textBlockSize, buttons, actionsLength, dialogSize);

      Window handle = libs.xlib.CreateSimpleWindow_(libs.displayServer, parentHandle, dialogSize.x, dialogSize.y, dialogSize.width, dialogSize.height, 
                                                    1, BlackPixel(libs.displayServer, libs.screenIndex), WhitePixel(libs.displayServer, libs.screenIndex));
      if (!handle) {
        XFreeFontSet(libs.displayServer, fontSet);
        throw std::runtime_error("X11: XCreateSimpleWindow failed to create dialog");
      }
      
      libs.xlib.SelectInput_(libs.displayServer, handle, (ExposureMask | PointerMotionMask | ButtonPressMask | ButtonReleaseMask));
      __setDialogLimits(libs, handle, dialogSize.width, dialogSize.height);
      if (libs.atoms.WM_DELETE_WINDOW) // set window close button
        libs.xlib.SetWMProtocols_(libs.displayServer, handle, &(libs.atoms.WM_DELETE_WINDOW), 1);
      
      // configure buttons
      Colormap colorMap = DefaultColormap(libs.displayServer, libs.screenIndex);
      GC buttonBarGC, buttonGC, buttonHoverGC, buttonClickGC;
      __setGraphicsContext(libs, handle, colorMap, __COLOR_16BIT(242),__COLOR_16BIT(242),__COLOR_16BIT(242), buttonBarGC);
      __setGraphicsContext(libs, handle, colorMap, __COLOR_16BIT(202),__COLOR_16BIT(202),__COLOR_16BIT(202), buttonGC);
      __setGraphicsContext(libs, handle, colorMap, __COLOR_16BIT(210),__COLOR_16BIT(210),__COLOR_16BIT(210), buttonHoverGC);
      __setGraphicsContext(libs, handle, colorMap, __COLOR_16BIT(182),__COLOR_16BIT(182),__COLOR_16BIT(182), buttonClickGC);
      
      uint32_t buttonOffsetX = dialogSize.width - ((uint32_t)buttons[actionsLength - 1].area.x + buttons[actionsLength - 1].area.width + __P_BOX_PADDING);
      uint32_t buttonOffsetY = dialogSize.height - __P_BUTTON_BAR_HEIGHT;
      for (uint32_t i = 0; i < actionsLength; ++i) {
        buttons[i].label = actions[i];
        buttons[i].sharedGC = &buttonGC;
        buttons[i].sharedHoverGC = &buttonHoverGC;
        buttons[i].sharedClickGC = &buttonClickGC;
        buttons[i].activeGC = buttons[i].sharedGC;
        buttons[i].area.x += buttonOffsetX;
        buttons[i].area.y += buttonOffsetY;
      }
      
      // map window + set "top most"
      libs.xlib.MapRaised_(libs.displayServer, handle);
    
      // create text drawing context
      XGCValues gcValues;
      gcValues.foreground = BlackPixel(libs.displayServer, 0);
      GC textGC = libs.xlib.CreateGC_(libs.displayServer, handle, GCForeground, &gcValues);

      // set caption + type
      __setDialogCaption(libs, handle, caption);
      libs.xlib.Flush_(libs.displayServer);

      // draw + wait for user action
      try {
        userAction = __processDialogEvents(handle, fontSet, textGC, buttonBarGC, dialogSize,
                                           textLineHeight, messageLines, buttons, actionsLength);
      }
      catch (const std::exception& exc) {
        __MessageBox::setLastError(exc.what());
        userAction = 0;
      }
      libs.xlib.UnmapWindow_(libs.displayServer, handle);
      libs.xlib.DestroyWindow_(libs.displayServer, handle);
      libs.xlib.Flush_(libs.displayServer);
      
      libs.xlib.FreeFontSet_(libs.displayServer, fontSet);
      libs.xlib.FreeGC_(libs.displayServer, textGC);
      libs.xlib.FreeGC_(libs.displayServer, buttonBarGC);
      libs.xlib.FreeGC_(libs.displayServer, buttonGC);
      libs.xlib.FreeGC_(libs.displayServer, buttonHoverGC);
      libs.xlib.FreeGC_(libs.displayServer, buttonClickGC);
      libs.xlib.FreeColormap_(libs.displayServer, colorMap);
    }
    catch (const std::exception& exc) {
      __MessageBox::setLastError(exc.what());
      userAction = 0;
    }
    return userAction;
  }

  // ---

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::ActionType actions, 
                                      MessageBox::IconType, bool, WindowHandle parent) noexcept {
    const char* buttons[3] = { nullptr };
    uint32_t length = __MessageBox::toActionLabels(actions, &buttons[0]);
    uint32_t result = __showX11MessageBox(caption, message, &buttons[0], length, parent);
    return __MessageBox::toDialogResult(result, length);
  }
  
  // ---
  
  // get last error message (in case of Result::failure)
  std::string MessageBox::getLastError() { 
    return __MessageBox::getLastError();
  }
  

// -- custom message box -- ----------------------------------------------------

  // show modal message box
  MessageBox::Result MessageBox::show(const char* caption, const char* message, MessageBox::IconType,
                                      const char* button1, const char* button2, const char* button3,
                                      bool, WindowHandle parent) noexcept {
    const char* buttons[3] = { nullptr };
    uint32_t length = __MessageBox::toActionLabels(button1, button2, button3, &buttons[0]);
    uint32_t result = __showX11MessageBox(caption, message, &buttons[0], length, parent);
    return __MessageBox::toDialogResult(result, length);
  }

#endif
