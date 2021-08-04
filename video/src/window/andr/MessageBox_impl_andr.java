// MIT License
// Copyright (c) 2021 Romain Vinders

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO WARRANTIES OF MERCHANTABILITY, FITNESS
// FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS
// OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
// WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR
// IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

package pandora.video;
import android.app.Activity;
import android.content.Context;
import android.app.AlertDialog;
import android.app.AlertDialog.Builder;
import android.content.DialogInterface;
import java.util.concurrent.Semaphore;
import android.os.Handler;
import android.os.Message;
import android.os.Looper;

// Modal message box (message, warning, confirmation, question...)
public class MessageBox {
  static final int NO_USER_ACTION = 0;
  
  // icon types
  static final int ICON_NONE = 0;
  static final int ICON_INFO = 1;
  static final int ICON_QUESTION = 2;
  static final int ICON_WARNING = 3;
  static final int ICON_ERROR = 4;
  
  // initialize message box params
  MessageBox(Context context, String caption, String message, int icon, String[] actions) {
    _userAction = NO_USER_ACTION;
    _context = context;
    _caption = caption;
    _message = message;
    _icon = icon;
    _actions = actions;
  }
  
  // display message box + wait for user action
  public int show() {
    _isFromUiThread = Looper.getMainLooper().isCurrentThread();
    
    // UI-thread: run event loop + use exception as an event
    if (_isFromUiThread) {
      _messageHandler = new Handler(Looper.getMainLooper()) {
        @Override
        public void handleMessage(Message msg) {
          throw new RuntimeException();
        }
      };
      
      createDialog();
      try { 
        Looper.getMainLooper().loop(); 
      }
      catch (RuntimeException exc) {}
    }
    // non UI-thread: use semaphore to wait
    else {
      ((Activity)_context).runOnUiThread(_dialogRunner);
      try {
        _dialogSemaphore.acquire();
      }
      catch (InterruptedException exc2) {}
    }
    return _userAction;
  }
  
  // -- private --

  // create message box
  private void createDialog() {
    AlertDialog.Builder builder = new AlertDialog.Builder(_context);
    builder.setTitle(_caption);
    builder.setMessage(_message);
    
    switch (_icon) {
      case ICON_INFO: builder.setIcon(android.R.drawable.ic_dialog_info); break;
      case ICON_QUESTION: builder.setIcon(android.R.drawable.ic_menu_help); break;
      case ICON_WARNING: builder.setIcon(android.R.drawable.ic_dialog_alert); break;
      case ICON_ERROR: builder.setIcon(android.R.drawable.ic_dialog_alert); break;
      default: break;
    }
    
    builder.setPositiveButton(_actions[0], new DialogInterface.OnClickListener() {
      public void onClick(DialogInterface dialog, int which) {
        _userAction = 1;
        if (_isFromUiThread)
          _messageHandler.sendMessage(_messageHandler.obtainMessage());
        else
          _dialogSemaphore.release();
      }
    });
    
    if (_actions.length >= 2) {
      DialogInterface.OnClickListener actionListener = new DialogInterface.OnClickListener() {
        public void onClick(DialogInterface dialog, int which) {
          _userAction = 2;
          if (_isFromUiThread)
            _messageHandler.sendMessage(_messageHandler.obtainMessage());
          else
            _dialogSemaphore.release();
        }
      };
      
      if (_actions.length == 2) {
        builder.setNegativeButton(_actions[1], actionListener);
      }
      else {
        builder.setNeutralButton(_actions[1], actionListener);
        builder.setNegativeButton(_actions[2], new DialogInterface.OnClickListener() {
          public void onClick(DialogInterface dialog, int which) {
            _userAction = 3;
            if (_isFromUiThread)
              _messageHandler.sendMessage(_messageHandler.obtainMessage());
            else
              _dialogSemaphore.release();
          }
        });
      }
    }

    builder.setCancelable(false);
    builder.create().show();
  }
  
  // -- properties --
  
  private int _userAction = NO_USER_ACTION;
  private boolean _isFromUiThread = false;
  private Context _context;
  private String _caption;
  private String _message;
  private int _icon;
  private String[] _actions; 
  
  final Semaphore _dialogSemaphore = new Semaphore(0, true);
  private final Runnable _dialogRunner = new Runnable() {
    public void run() { createDialog(); }
  };
  Handler _messageHandler = null;
}
