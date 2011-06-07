#include <sys/shm.h>
#include <sys/types.h>
#include <pwd.h>
#include "uxselect.h"

#ifdef USE_PAMHELPER
#include <unistd.h>
#include <sys/wait.h>
#endif

UxSelect *UxSelect::UxSelectInstance;

UxSelect::UxSelect(): QMainWindow(){
  setupUi(this);
  UxSelectInstance=this;

  bool initialized=settings.value("initialized").toBool();
  if (!initialized){
    qDebug() << "Setting initial settings...";
    settings.setValue("initialized", true);
    settings.setValue("uxconfig", UxDisplaySession|UxDisplayUser);
    settings.setValue("uxIconSize", QSize(64,64));
    settings.setValue("userIconSize", QSize(64,64));
    settings.setValue("sessionIcon", "");
    settings.setValue("userIcon", "");
    settings.setValue("bannerPixmap", "");
  }

  QSize iconSize=settings.value("uxIconSize").toSize();
  if (iconSize.isValid())
    uxSelectionList->setIconSize(iconSize);
  iconSize=settings.value("userIconSize").toSize();
  if (iconSize.isValid())
    userSelectionList->setIconSize(iconSize);

  defaultSessionIcon=settings.value("sessionIcon").toString();
  if (defaultSessionIcon.isEmpty() || !QFile::exists(defaultSessionIcon))
    defaultSessionIcon=":/images/aardvark_icon.png";

  defaultUserIcon=settings.value("userIcon").toString();
  if (defaultUserIcon.isEmpty() || !QFile::exists(defaultUserIcon))
    defaultUserIcon=":/images/aardvark_icon.png";

  bannerLabel->setPixmap(QPixmap(settings.value("bannerPixmap").toString()));

  shmId=getenv("SHM_ID");
  if (shmId.isNull()){
    qDebug() << "No SHM available!";
    messageLabel->setText(tr("No SHM available."));
  } else {
    qDebug() << "SHM_ID: " << shmId;
    shm=(uxlaunch_chooser_shm *) shmat(shmId.toInt(), 0, 0);
    if (shm==(void*)-1){
      messageLabel->setText(tr("Unable to attach SHM"));
    } else {
      userInput->setText(shm->user);
      defaultUser=shm->user;
      defaultSession=shm->session_name;
      defaultSessionPath=shm->session_path;
      shmdt(shm);
    }
  }

  uxConfig=static_cast<UxConfig>(settings.value("uxconfig").toInt());
  if (!isPasswordWidgetActive())
    displayPasswordWidgets(false);

  if (!isUserWidgetActive())
    displayUserWidgets(false);
  else createUserList();

  if (!isSessionWidgetActive())
    displaySessionWidgets(false);
  else createUxList();
}

#ifndef USE_PAMHELPER
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
#endif

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
    item->setIcon(QIcon(defaultUserIcon));

    item->setTextAlignment(Qt::AlignHCenter|Qt::AlignBottom);

    userSelectionList->insertItem(0, item);

    if (defaultUser==item->data(Qt::UserRole).toString())
      userSelectionList->setCurrentItem(item);

    pwent = getpwent();
  }
  endpwent();

  if (userSelectionList->count()!=0){
    if (userSelectionList->selectedItems().isEmpty())
      userSelectionList->setCurrentItem(userSelectionList->item(0));

    userInput->setText(
      userSelectionList->currentItem()->data(Qt::UserRole).toString());
  } else
    displayUserWidgets(false);
}

void UxSelect::createUxList(){
  QStringList sessionList=settings.value("uxlist").toStringList();

  for (int i=0;i<sessionList.size();i++){
    QListWidgetItem *item=new QListWidgetItem;
    QString iconPath;
    settings.beginGroup(sessionList.at(i));
    item->setText(settings.value("name").toString());
    item->setData(Qt::UserRole, settings.value("path").toString());
    item->setData(Qt::UserRole+1, settings.value("description").toString());

    iconPath=settings.value("icon").toString();
    if (!iconPath.isEmpty() && QFile::exists(iconPath))
      item->setIcon(QIcon(iconPath));
    else
      item->setIcon(QIcon(defaultSessionIcon));

    item->setTextAlignment(Qt::AlignHCenter|Qt::AlignBottom);

    settings.endGroup();
    if (QFile::exists(item->data(Qt::UserRole).toString())){
      uxSelectionList->insertItem(0, item);

      // session path may be ambigous. Only select entry based on
      // path if there's nothing yet selected
      if (defaultSession==item->text())
        uxSelectionList->setCurrentItem(item);
      else if (defaultSessionPath==item->data(Qt::UserRole).toString() &&
               uxSelectionList->selectedItems().isEmpty())
        uxSelectionList->setCurrentItem(item);
    }
  }

  if (uxSelectionList->count()!=0){
    if (uxSelectionList->selectedItems().isEmpty())
      uxSelectionList->setCurrentItem(uxSelectionList->item(0));
  } else
    displaySessionWidgets(false);
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

  if (state)
    uxConfig=static_cast<UxConfig>(uxConfig|UxDisplayUser);
  else if (uxConfig & UxDisplayUser)
    uxConfig=static_cast<UxConfig>(uxConfig^UxDisplayUser);
}

void UxSelect::displayPasswordWidgets(bool state){
  passwordLabel->setVisible(state);
  passwordInput->setVisible(state);

  if (state)
    uxConfig=static_cast<UxConfig>(uxConfig|UxDisplayPassword);
  else if (uxConfig & UxDisplayPassword)
    uxConfig=static_cast<UxConfig>(uxConfig^UxDisplayPassword);
}

void UxSelect::displaySessionWidgets(bool state){
  uxSelectionList->setVisible(state);
  uxSelectionLabel->setVisible(state);

  if (state)
    uxConfig=static_cast<UxConfig>(uxConfig|UxDisplaySession);
  else if (uxConfig & UxDisplaySession)
    uxConfig=static_cast<UxConfig>(uxConfig^UxDisplaySession);
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
#ifdef USE_PAMHELPER
    int status;
    pid_t pid=fork();
    if (pid==-1){
      qDebug() << "fork() failed";
      QCoreApplication::exit(-1);
    } else if (pid==0){
      ::close(4);
      execlp("uxselect.pamhelper", "uxselect.pamhelper",
             "/bin/true", (char *)NULL);
    } else {
      ::close(3);
      char buf[512];
      int len;

      strncpy(buf, userInput->text().toLatin1(), 512);
      len=userInput->text().length()+1;
      strncpy(buf+len, passwordInput->text().toLatin1(), 512-len);
      len+=passwordInput->text().length()+1;

      write(4,buf,len+1);
      ::close(4);

      waitpid(pid, &status, 0);
      if (status!=0){
        messageLabel->setText(tr("Login failed: %1").arg(status));
        // since we just closed fd 3 and 4 we'll most likely get them
        // again when creating a new pipe now
        if (pipe(pfd)){
          qDebug() << "pipe() failed";
          QCoreApplication::exit(-1);
        }
        if (pfd[0]!=3){
          qDebug() << "pfd != 3: " << pfd[0];
          QCoreApplication::exit(-1);
        }
      } else
        dumpData();
    }

#else
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
      messageLabel->setText(tr("Login failed: %1").arg(pam_strerror(pamh, ret)));
    } else
      dumpData();

    pam_end(pamh, ret);
#endif
  } else {
    dumpData();
  }
}
