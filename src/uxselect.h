/*
 * uxselect.h
 * (c) 2011 Bernd Wachter <bwachter@lart.info>
 */

#ifndef _UXSELECT_H
#define _UXSELECT_H

#define LOWEST_ID 500

#include <QtGui>
#include <security/pam_appl.h>
#include "ui_uxselect.h"

#include "shmdata.h"

class UxSelect: public QMainWindow, Ui::UxSelect {
  Q_OBJECT

  public:
  UxSelect();

  private:
  int ret;
  pam_handle_t *pamh;
  QString shmId;
  shm_exchange *shm;
  struct pam_conv pamc;
  static UxSelect *UxSelectInstance;

  void createUserList();
  static int pamConversation(int num_msg, const struct pam_message **msg,
                             struct pam_response **resp, void *appdata_ptr);

  public slots:
  void tryLogin();
  void selectUser(QListWidgetItem *item);
};


#endif
