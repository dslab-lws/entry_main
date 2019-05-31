/****************************************************************************
*
* Copyright 2018 baruncorechips All Rights Reserved.
*
* Filename: A053BasicKit.h
* Author: sj.yang
* Release date: 2018/10/04
* Version: 1.3
*
****************************************************************************/

#ifndef A053BASICKIT_H_
#define A053BASICKIT_H_

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <artik_module.h>
#include <artik_wifi.h>
#include <artik_network.h>
#include <readline.h>
#include <netdb.h>
#include <apps/shell/tash.h>
#include <net/lwip/sockets.h>
#include <fcntl.h>
#include <tinyara/config.h>
#include <tinyara/gpio.h>	// for GPIO control
#include <tinyara/pwm.h>	// for PWM control
#include <tinyara/analog/adc.h>	// for ADC control
#include <tinyara/analog/ioctl.h>	// for ADC control
#include <errno.h>	// for ADC control
#include <net/if.h>
#include <apps/netutils/wifi/slsi_wifi_api.h>

#include <apps/netutils/dhcpc.h>
#include <apps/netutils/mqtt_api.h>
#include <apps/netutils/ntpclient.h>

// WIFi
#define STATE_DISCONNECTED      0
#define STATE_CONNECTED       1
#define SLSI_WIFI_SECURITY_OPEN   "open"
#define SLSI_WIFI_SECURITY_WPA2_AES "wpa2_aes"
#define SSID "D.S.Lab" //FIX
#define PSK  "dslab123" //FIX

// Cloud
#define RED_ON_BOARD_LED 45
#define ADC_MAX_SAMPLES 4
#define NET_DEVNAME "wl1"
#define DEFAULT_CLIENT_ID "123456789"
#define SERVER_ADDR "api.artik.cloud"//"52.200.124.224"
#define DEVICE_ID "aef35a6273f24264826dd396bd920eac" //FIX
#define DEVICE_TOKEN "9e911f582ea646e2bdc43424274278cf" // FIX
#define ACTION_TOPIC "/v1.1/actions/aef35a6273f24264826dd396bd920eac" // FIX
#define MESSAGE_TOPIC "/v1.1/messages/aef35a6273f24264826dd396bd920eac" //FIX

#define WIFI_SCAN_TIMEOUT       15
#define WIFI_CONNECT_TIMEOUT    30
#define WIFI_DISCONNECT_TIMEOUT 10

//Weather API
#define NTP_REFRESH_PERIOD  (60 * 60) /* seconds */
#define ARRAY_SIZE(a) (sizeof(a)/sizeof((a)[0]))

#define TEMP_LEN 7
#define HUMI_LEN 3
#define TMIN_LEN 7
#define TMAX_LEN 7
#define DATA_OFFSET 2
#define ABS_TEMP 273.15

#define BACKLOG 10
#define MAX_DATA_SIZE 1500

struct callback_result {
	sem_t sem;
	artik_wifi_connection_info info;
	artik_error error;
};

// Hexagon GPIO Pin number
#define PIN_D2 46
#define PIN_D4 47
#define PIN_D7 48
#define PIN_D8 50

// Hexagon ADC Pin number
#define A0 0
#define A1 1
#define A2 2
#define A3 3

// Hexagon PWM Pin number
#define PWM0 0
#define PWM1 1
#define PWM2 2
#define PWM4 4

// octave_chord period[us] // frequency[hz]
#define O5_DO 1911 // 523.251[hz]
#define O5_RE 1703 // 587.330[hz]
#define O5_MI 1517 // 659.254[hz]
#define O5_FA 1432 // 698.456[hz]
#define O5_SO 1276 // 783.990[hz]
#define O5_LA 1136 // 880.000[hz]
#define O5_TI 1012 // 987.766[hz]

#define O6_DO 956 // 1046.502[hz]
#define O6_RE 851 // 1174.659[hz]
#define O6_MI 758 // 1318.510[hz]
#define O6_FA 716 // 1396.913[hz]
#define O6_SO 638 // 1567.982[hz]
#define O6_LA 568 // 1760.000[hz]
#define O6_TI 506 // 1975.533[hz]

#define	HIGH	1
#define	LOW		0
#define ENABLE	1
#define DISABLE	0
#define FALSE	0
#define TRUE	1
#define false	0
#define true	1
#define S5J_ADC_MAX_CHANNELS	4


//GPIO function
void gpio_write(int port, int value);
void gpio_write_toggle(int port);
int gpio_read(int port);

//PWM function
int pwm_open (int port);
void pwm_write(int fd, int period, int duty_cycle);
void pwm_close (int fd);
void ServoAngle(int fd, int PERIOD, int angle);

//ADC function
int read_adc(int channel);

//Wifi function
void networkLinkUpHandler(slsi_reason_t* reason);
void networkLinkDownHandler(slsi_reason_t* reason);
int8_t start_wifi_interface(void);

static void ntp_link_error(void);
void onConnect(void* client, int result);
void onDisconnect(void* client, int result);
void onMessage(void* client, mqtt_msg_t *msg);
void initializeConfigUtil(void);
int cloud_main(int argc, char *argv[]);
void wifi_connect_callback(void *result, void *user_data);
int wifi_connect(const char* ssid, const char* passphrase);
int start_dhcp_client(void);
int weather_api_main(void);

//mqtt function
#endif /* A053BASICKIT_H_ */
