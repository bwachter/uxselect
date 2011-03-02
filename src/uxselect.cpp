#include "uxselect.h"

UxSelect *UxSelect::UxSelectInstance;

UxSelect::UxSelect(): QMainWindow(){
  setupUi(this);
  UxSelectInstance=this;

  pamh=NULL;
  pamc.conv=&pamConversation;
  pamc.appdata_ptr=NULL;

  ret=pam_start("login", "bwachter", &pamc, &pamh);
  if (ret!=PAM_SUCCESS)
    qDebug() << "pam_start failed: " << pam_strerror(NULL, ret);
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

void UxSelect::tryLogin(){
  qDebug() << "Trying PAM auth";
  ret=pam_authenticate(pamh, 0);
  if (ret!=PAM_SUCCESS){
    qDebug() << "We're doomed: " << pam_strerror(pamh, ret);
  } else
    qDebug() << "Success!";
}
