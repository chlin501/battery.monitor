/**
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
#include <fcntl.h>
#include <glib.h>
#include <libnotify/notify.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>

#define true 1
#define AC_ONLINE 1
#define LOW_CAPACITY 10
#define SUMMARY "Battery Critically Low" "Plug in to AC or Suspend immediately"
#define BODY "Plug in to AC or Suspend immediately"
#define TIME_OUT 3000
#define ICON_PATH NULL

int main(int argc, char** argv)
{
  char ac_v[3] = "0";
  char cap_v[4] = "5";
  NotifyNotification *n;
  notify_init("Battery Monitor");
  //daemon(1, 1);
  openlog("batlog", LOG_PID | LOG_CONS, LOG_USER);
  while (true) {
    // 1. read /sys/class/power_supply  
    int ac_fd = open("/sys/class/power_supply/AC/online", O_RDONLY);
    if (0 > ac_fd) {
      syslog(LOG_ERR, "Can't open ac online property!"); 
      return -1;
    }
    if (0 > lseek(ac_fd, 0, SEEK_SET)) {
      syslog(LOG_ERR, "Can't reposition ac online property!"); 
      return -1;
    }

    int cap_fd = open("/sys/class/power_supply/BAT0/capacity", O_RDONLY);
    if (0 > cap_fd) {
      syslog(LOG_ERR, "Can't open battery capacity property!");
      return -1;
    }

    if (0 > lseek(cap_fd, 0, SEEK_SET)) {
      syslog(LOG_ERR, "Can't reposition ac online property!"); 
      return -1;
    }

    if(0 > read(ac_fd, &ac_v, sizeof(&ac_v))) {
      syslog(LOG_ERR, "Can't read ac online value!");
      return -1;
    }
    
    if(0 > read(cap_fd, &cap_v, sizeof(&cap_v))) {
      syslog(LOG_ERR, "Can't read battery capacity value!");
      return -1;
    }

    // 2. compare to threshold
    int acv = atoi(ac_v);
    int capv = atoi(cap_v);
    if (AC_ONLINE != acv && LOW_CAPACITY > capv) {
      n = notify_notification_new(SUMMARY, BODY, ICON_PATH);
      notify_notification_set_timeout(n, TIME_OUT); 
      if(!notify_notification_show(n, NULL)) {
        syslog(LOG_ERR,"Fail to send low battery notification!");
        return -1;
      }
      g_object_unref(G_OBJECT(n));
    } 
    sleep(5);
  }
  
  closelog();
  return 0;

}
