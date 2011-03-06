// http://cr.yp.to/checkpwd/interface.html

#include <stdlib.h>
#include <security/pam_appl.h>
#include <unistd.h>
#include <string.h>

#include <stdio.h>

char *password;
char *username;

int pamConversation(int num_msg, const struct pam_message **msg,
                           struct pam_response **resp, void *appdata_ptr){
  struct pam_response *pamr;

  pamr=(struct pam_response *)calloc(num_msg, sizeof(struct pam_response));

  if (pamr==NULL) return PAM_BUF_ERR;

  for (int i=0; i<num_msg; i++){
    pamr[i].resp_retcode=0;
    pamr[i].resp=NULL;
    switch(msg[i]->msg_style){
      case PAM_PROMPT_ECHO_ON:
        // asking for user
        pamr[i].resp=strdup(username);
        break;
      case PAM_PROMPT_ECHO_OFF:
        //asking for password
        pamr[i].resp=strdup(password);
        break;
      case PAM_TEXT_INFO:
        break;
      case PAM_ERROR_MSG:
        pamr[i].resp_retcode=PAM_SUCCESS;
        pamr[i].resp=NULL;
      default:
        return PAM_CONV_ERR;
        break;
    }
  }

  *resp=pamr;
  return PAM_SUCCESS;
}

int main(int argc, char **argv){
  pam_handle_t *pamh;
  struct pam_conv pamc;
  int ret, len;
  char buf[513];

  if (!argv[1]) return 2;

  for (len=0;len<512;){
    ssize_t r;
    r=read(3,buf+len,512-len);
    if (r==0) break;
    if (r==-1) return 111;
    len+=r;
  }

  buf[len]='\0';
  username=buf;
  password=buf+strlen(username)+1;

  pamh=NULL;
  pamc.conv=&pamConversation;
  pamc.appdata_ptr=NULL;

  ret=pam_start("login", username, &pamc, &pamh);
  if (ret!=PAM_SUCCESS)
    return -1;

  ret=pam_authenticate(pamh, 0);
  if (ret==PAM_SUCCESS)
    execv(argv[1],argv+1);

  return 111;
}
