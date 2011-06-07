/*
 * uxselect.h
 * (c) 2011 Bernd Wachter <bwachter@lart.info>
 */

#ifndef _UXSELECT_H
#define _UXSELECT_H

#define LOWEST_ID 500

#include <QtGui>
#include "ui_uxselect.h"

#ifdef USE_UXLAUNCH
#include <uxlaunch-ipc.h>
#else
#include "uxlaunch-ipc.h"
#endif

#ifdef USE_PAMHELPER
#else
#include <security/pam_appl.h>
#endif

class UxSelect: public QMainWindow, Ui::UxSelect {
  Q_OBJECT

  public:
  UxSelect();
  enum UxConfig {
    UxDisplayPassword=1,
    UxDisplayUser=2,
    UxDisplaySession=4
  };

  private:
  UxConfig uxConfig;
  QSettings settings;
  int ret;
  QString shmId, defaultUser, defaultSession, defaultSessionPath,
    defaultUserIcon, defaultSessionIcon;
  uxlaunch_chooser_shm *shm;
#ifndef USE_PAMHELPER
  struct pam_conv pamc;
  pam_handle_t *pamh;
#endif
  static UxSelect *UxSelectInstance;

  void dumpData();
  void createUserList();
  void createUxList();
  void displayUserWidgets(bool state);
  void displayPasswordWidgets(bool state);
  void displaySessionWidgets(bool state);
  bool isPasswordWidgetActive();
  bool isUserWidgetActive();
  bool isSessionWidgetActive();
#ifndef USE_PAMHELPER
  static int pamConversation(int num_msg, const struct pam_message **msg,
                             struct pam_response **resp, void *appdata_ptr);
#endif

  public slots:
  void tryLogin();
  void selectUser(QListWidgetItem *item);
  void selectUx(QListWidgetItem *item);
};

#ifdef USE_PAMHELPER
extern int pfd[2];
#endif

#endif
