#include <sys/shm.h>
#include <sys/types.h>
#include <pwd.h>
#include "uxselect.h"


UxSelect *UxSelect::UxSelectInstance;

UxSelect::UxSelect(): QMainWindow(){
  setupUi(this);
  UxSelectInstance=this;

  shmId=getenv("SHM_ID");

  if (shmId.isNull())
    qDebug() << "No SHM available!";
  else
    qDebug() << "SHM_ID: " << shmId;

  if (getenv("UX_USER"))
    userInput->setText(getenv("UX_USER"));

  // insert some items for testing
  QListWidgetItem *item=new QListWidgetItem;
  item->setText("ion3");
  item->setIcon(QIcon(":/images/aardvark_icon.png"));
  uxSelectionList->insertItem(0, item);
  item=new QListWidgetItem;
  item->setText("fvwm2");
  item->setIcon(QIcon(":/images/aardvark_icon.png"));
  uxSelectionList->insertItem(0, item);

  createUserList();
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

void UxSelect::selectUser(QListWidgetItem *item){
  userInput->setText(item->data(Qt::UserRole).toString());
}

void UxSelect::tryLogin(){
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
    qDebug() << "We're doomed: " << pam_strerror(pamh, ret);
  } else
    qDebug() << "Success!";

  pam_end(pamh, ret);

  // throw the collected data into SHM, or just to stdout if SHM is not available
  if (shmId.isNull()){
    qDebug() << "Username: " << userInput->text();
    qDebug() << "Session: ";
  } else {
    shm=(shm_exchange *) shmat(shmId.toInt(), 0, 0);
    strncpy(shm->user, userInput->text().toLatin1(), 255);
    strncpy(shm->session, "/usr/bin/ion3", PATH_MAX);
    shmdt(shm);
  }
}
