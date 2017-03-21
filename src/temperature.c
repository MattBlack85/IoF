#include <dirent.h>
#include <stdio.h>
#include <string.h>
#include <time.h>


int main(void) {
  DIR *main_dir;
  FILE *temp_file;
  struct dirent *read_dir;
  char devices_folder[20] = "/sys/bus/w1/devices";
  char dev_folder[16];
  char full_path[128];
  char buf[256];
  size_t nread;
  char tmpData[6];
  char lock_folder[15] = "/tmp/temp.lock";
  int err = 2;

  while (1) {
    // every process calling this will need to wait for an exclusive lock to read the temp
    err = mkdir(lock_folder, 0755);
    if (err == 0) {
      break;
    }
  }

  main_dir = opendir(devices_folder);

  if (main_dir == NULL) {
    printf("Folder %s not found\n", devices_folder);
    rmdir(lock_folder);
    return 1;
  }

  while ((read_dir = readdir(main_dir))) {
    // Look for device folders, start with 28-
    if (strstr(read_dir->d_name, "28-")) {
      strcpy(dev_folder, read_dir->d_name);
    }
  }
  closedir(main_dir);

  // Build the full path to the file where the read will be written
  sprintf(full_path, "%s/%s/w1_slave", devices_folder, dev_folder);

  temp_file = fopen(full_path, "r");
  if(temp_file == NULL) {
    printf("Couldn't open the w1 device.\n");
    rmdir(lock_folder);
    return 1;
  }
  while((nread = fread(buf, 1, 255, temp_file))) {
    strncpy(tmpData, strstr(buf, "t=") + 2, 5);
    int tempC = atoi(tmpData);
    printf("%.3f\n", (double) tempC / 1000);
  }
  close(temp_file);

  rmdir(lock_folder);

  return 0;
}
