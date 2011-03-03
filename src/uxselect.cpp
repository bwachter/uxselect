#include <sys/shm.h>
#include <sys/types.h>
#include <pwd.h>
#include "uxselect.h"


UxSelect *UxSelect::UxSelectInstance;

UxSelect::UxSelect(): QMainWindow(){
  setupUi(this);
  UxSelectInstance=this;

  bool initialized=settings.value("initialized").toBool();
  if (!initialized){
    qDebug() << "Setting initial settings...";
    settings.setValue("initialized", true);
    settings.setValue("uxconfig", UxDisplaySession|UxDisplayUser);
  }

  shmId=getenv("SHM_ID");
  if (shmId.isNull()){
    qDebug() << "No SHM available!";
    messageLabel->setText("No SHM available.");
  } else {
    qDebug() << "SHM_ID: " << shmId;
    shm=(uxlaunch_chooser_shm *) shmat(shmId.toInt(), 0, 0);
    if (shm==(void*)-1){
      messageLabel->setText("Unable to attach SHM");
    } else {
      userInput->setText(shm->user);
      shmdt(shm);
    }
  }

  uxConfig=static_cast<UxConfig>(settings.value("uxconfig").toInt());
  qDebug() << uxConfig << (uxConfig & UxDisplayPassword) << (uxConfig & UxDisplaySession);
  if (!isPasswordWidgetActive())
    displayPasswordWidgets(false);

  if (!isUserWidgetActive())
    displayUserWidgets(false);
  else createUserList();

  if (!isSessionWidgetActive())
    displaySessionWidgets(false);
  else createUxList();
}

int UxSelect::pamConversation(int num_msg, const struct pam_message **msg,
                           struct pam_response **resp, void *appdata_ptr){
  struct pam_response *pamr;

  pamr=(struct pam_response *)calloc(num_msg, sizeof(struct pam_response));

  if (pamr==NULL) return PAM_BUF_ERR;

  qDebug() << "PAM conversation with " << num_msg << "messages";
  for (int i=0; i<num_msg; i++){
    pamr[i].resp_retcode=0;
    pamr[i].resp=NULL;
    switch(msg[i]->msg_style){
      case PAM_PROMPT_ECHO_ON:
        // asking for user
        qDebug() << "PAM prompting for user";
        pamr[i].resp=strdup(UxSelectInstance->userInput->text().toLatin1());
        break;
      case PAM_PROMPT_ECHO_OFF:
        //asking for password
        qDebug() << "PAM prompting for password";
        pamr[i].resp=strdup(UxSelectInstance->passwordInput->text().toLatin1());
        break;
      case PAM_TEXT_INFO:
        qDebug() << "PAM_TEXT_INFO: " << msg[i]->msg;
        break;
      case PAM_ERROR_MSG:
        pamr[i].resp_retcode=PAM_SUCCESS;
        pamr[i].resp=NULL;
      default:
        qDebug() << "PAM wants to chat" << msg[i]->msg_style;
        return PAM_CONV_ERR;
        break;
    }
  }

  *resp=pamr;
  return PAM_SUCCESS;
}

void UxSelect::createUserList(){
  struct passwd *pwent;
  setpwent();
  for (pwent=getpwent();pwent!=NULL;pwent=getpwent()){
    if (pwent->pw_uid<LOWEST_ID) continue;
    if (!strcmp(pwent->pw_shell, "/bin/false")) continue;
    QListWidgetItem *item=new QListWidgetItem;
    //item->setText(pwent->pw_gecos);
    //TODO: GECOS parsing
    item->setText(pwent->pw_name);
    item->setData(Qt::UserRole, pwent->pw_name);
    item->setIcon(QIcon(":/images/aardvark_icon.png"));
    userSelectionList->insertItem(0, item);
    qDebug() << "New user: " << pwent->pw_name << ", " << pwent->pw_uid
             << ", " << pwent->pw_shell;
    pwent = getpwent();
  }
  endpwent();
}

void UxSelect::createUxList(){
  //FIXME, ignore empty/bogus entries
  QStringList sessionList=settings.value("uxlist").toStringList();

  for (int i=0;i<sessionList.size();i++){
    QListWidgetItem *item=new QListWidgetItem;
    settings.beginGroup(sessionList.at(i));
    if (!QFile::exists(settings.value("path").toString()))
      continue;
    item->setText(settings.value("name").toString());
    item->setData(Qt::UserRole, settings.value("path").toString());
    item->setData(Qt::UserRole+1, settings.value("description").toString());
    item->setIcon(QIcon(":/images/aardvark_icon.png"));
    settings.endGroup();
    uxSelectionList->insertItem(0, item);
  }
}

void UxSelect::dumpData(){
  QListWidgetItem *selectedUx=NULL;

  if (isSessionWidgetActive())
    selectedUx = uxSelectionList->currentItem();

  // throw the collected data into SHM, or just to stdout if SHM is not available
  if (shmId.isNull()){
    if (isUserWidgetActive())
      qDebug() << "Username: " << userInput->text();

    if (isSessionWidgetActive())
      qDebug() << "Session: " << selectedUx->data(Qt::UserRole).toString();
  } else {
    shm=(uxlaunch_chooser_shm *) shmat(shmId.toInt(), 0, 0);

    if (isUserWidgetActive())
      strncpy(shm->user, userInput->text().toLatin1(), 255);

    if (isSessionWidgetActive()){
      strncpy(shm->session_path,
              selectedUx->data(Qt::UserRole).toString().toLatin1(), PATH_MAX);
      strncpy(shm->session_name,
              selectedUx->text().toLatin1(), UXLAUNCH_NAME_LIMIT);
    }

    shmdt(shm);
  }

  QCoreApplication::exit(0);
}

void UxSelect::displayUserWidgets(bool state){
  userSelectionList->setVisible(state);
  userSelectionLabel->setVisible(state);
  userDescriptionLabel->setVisible(state);
  userLabel->setVisible(state);
  userInput->setVisible(state);
}

void UxSelect::displayPasswordWidgets(bool state){
  passwordLabel->setVisible(state);
  passwordInput->setVisible(state);
}

void UxSelect::displaySessionWidgets(bool state){
  uxSelectionList->setVisible(state);
  uxSelectionLabel->setVisible(state);
}

bool UxSelect::isPasswordWidgetActive(){
  return (uxConfig & UxDisplayPassword) ? true : false;
}

bool UxSelect::isUserWidgetActive(){
  return (uxConfig & UxDisplayUser) ? true : false;
}

bool UxSelect::isSessionWidgetActive(){
  return (uxConfig & UxDisplaySession) ? true : false;
}

void UxSelect::selectUser(QListWidgetItem *item){
  userInput->setText(item->data(Qt::UserRole).toString());
  if (uxConfig == UxDisplayUser)
    dumpData();
}

void UxSelect::selectUx(QListWidgetItem *item){
  uxDescriptionLabel->setText(item->data(Qt::UserRole).toString());

  if (uxConfig == UxDisplaySession)
    dumpData();
}

void UxSelect::tryLogin(){
  if (isPasswordWidgetActive()){
    // try to do pam-foo, if required
    pamh=NULL;
    pamc.conv=&pamConversation;
    pamc.appdata_ptr=NULL;

    ret=pam_start("login",
                  userInput->text().toLatin1()
                  , &pamc, &pamh);
    if (ret!=PAM_SUCCESS)
      qDebug() << "pam_start failed: " << pam_strerror(NULL, ret);

    qDebug() << "Trying PAM auth";
    ret=pam_authenticate(pamh, 0);
    if (ret!=PAM_SUCCESS){
      //FIXME, make some nice UI foo about login failed
      messageLabel->setText("Login failed.");
      qDebug() << "We're doomed: " << pam_strerror(pamh, ret);
    } else
      dumpData();

    pam_end(pamh, ret);
  } else {
    dumpData();
  }
}
