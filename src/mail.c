#include <stdio.h>
#include <stdlib.h>
#include "mail.h"

static char *SUBJECT = "Aquarium notification";

int send_mail(char *body) {
  char command[250];
  sprintf(command, "echo \"%s\" | mail -s \"%s\" promat85@gmail.com -r admin@iof", body, SUBJECT);
  return system(command);
}
