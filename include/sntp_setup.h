/* LwIP SNTP example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR
   CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include <time.h>
#include <sys/time.h>
//#include "esp_system.h"
//#include "esp_event.h"
#include "esp_log.h"
//#include "esp_attr.h"
//#include "protocol_examples_common.h"
//#include "esp_netif_sntp.h"
//#include "lwip/ip_addr.h"
#include "esp_sntp.h"

#ifndef INET6_ADDRSTRLEN
#define INET6_ADDRSTRLEN 48
#endif

void sntp_setup_init(void);
