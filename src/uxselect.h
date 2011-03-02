/*
 * uxselect.h
 * (c) 2011 Bernd Wachter <bwachter@lart.info>
 */

#ifndef _UXSELECT_H
#define _UXSELECT_H

#include <QtGui>
#include <security/pam_appl.h>
#include "ui_uxselect.h"

class UxSelect: public QMainWindow, Ui::UxSelect {
  Q_OBJECT

  public:
  UxSelect();

  private:
  int ret;
  pam_handle_t *pamh;
  struct pam_conv pamc;
  static UxSelect *UxSelectInstance;

  static int pamConversation(int num_msg, const struct pam_message **msg,
                             struct pam_response **resp, void *appdata_ptr);

  public slots:
  void tryLogin();
};


#endif
