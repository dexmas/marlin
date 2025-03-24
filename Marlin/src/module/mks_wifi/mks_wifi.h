#pragma once

#include "../../MarlinCore.h"
#include "../../inc/MarlinConfig.h"

#include "mks_wifi_settings.h"

#include "../../gcode/queue.h"

//Вызывается, когда пришёл текстовый ответ от выполнения GCODа
void mks_wifi_out_add(uint8_t *data, uint32_t size);

//#define ENABLE_MKS_WIFI_DEBUG

#ifdef ENABLE_MKS_WIFI_DEBUG
	void debug_to_uart(char *fmt,...);
	#define MKS_WIFI_DEBUG(fmt, args...) debug_to_uart((char*)"[DBG] " fmt "\n", ## args)
#else
	#define MKS_WIFI_DEBUG(...) NOOP
#endif

#ifdef MKS_WIFI

#define MKS_OUT_BUFF_SIZE (ESP_PACKET_DATA_MAX_SIZE)
#define MKS_IN_BUFF_SIZE (ESP_PACKET_DATA_MAX_SIZE + 30)

#define MKS_TOTAL_PACKET_SIZE (ESP_PACKET_DATA_MAX_SIZE+10)

#define WIFI_MODE_AP				(uint8_t)1
#define WIFI_MODE_CLIENT			(uint8_t)2

#define ESP_NET_WIFI_CONNECTED		(uint8_t)0x0A
#define ESP_NET_WIFI_FAIL			(uint8_t)0x05
#define ESP_NET_WIFI_EXCEPTION		(uint8_t)0x0E

#define ESP_WIFI_ACT_CONN			(uint8_t)0x01
#define ESP_WIFI_ACT_DISCONN		(uint8_t)0x02
#define ESP_WIFI_ACT_FORGET			(uint8_t)0x03

#define ESP_PROTOC_HEAD				(uint8_t)0xa5
#define ESP_PROTOC_TAIL				(uint8_t)0xfc

//от ESP к STM32
#define ESP_TYPE_NET				(uint8_t)0x0
#define ESP_TYPE_GCODE				(uint8_t)0x1
#define ESP_TYPE_FILE_FIRST			(uint8_t)0x2
#define ESP_TYPE_FILE_FRAGMENT		(uint8_t)0x3
#define ESP_TYPE_WIFI_LIST		    (uint8_t)0x4

//от STM32 к ESP
#define ESP_RTYPE_NETCFG			(uint8_t)0x0
#define ESP_RTYPE_GCODEREPLY		(uint8_t)0x2
#define ESP_RTYPE_EXCEPTION			(uint8_t)0x3
#define ESP_RTYPE_CLOUDCFG			(uint8_t)0x4
#define ESP_RTYPE_UNBIND			(uint8_t)0x5
#define ESP_RTYPE_NETSCAN			(uint8_t)0x7 //Значение из документации 0x06 - вранье
#define ESP_RTYPE_WIFICONN			(uint8_t)0x9

#define ESP_PACKET_DATA_MAX_SIZE	1024
#define ESP_SERIAL_OUT_MAX_SIZE		1024

#define NOP	__asm volatile ("nop")

typedef struct {
	uint8_t type;
	uint16_t dataLen;
	uint8_t *data;
} ESP_PROTOC_FRAME;

typedef struct {
	uint8_t ip[4];
	uint16_t port;
	uint8_t state;
	uint8_t mode;
	uint8_t wifi_name_len;
	char wifi_name[256];
	uint8_t wifi_key_len;
	char wifi_key[256];
	uint8_t cloud_state;
	uint8_t cloud_host_len;
	uint16_t cloud_port;
	uint8_t id_len;
	uint8_t ver_len;
	char ver[256];
} ESP_NET_STATUS;

typedef struct {
	uint8_t name_len;
	char name[32];
	uint8_t rssi;
} ESP_NET_DATA;

typedef struct {
	uint8_t net_cnt;
	ESP_NET_DATA networks[20];
} ESP_SCAN_RESULT;

extern ESP_NET_STATUS mks_global_net_status;
extern ESP_SCAN_RESULT mks_global_scan_result;

//инициализация MKS WIFI модуля
void mks_wifi_init(void);

//Перезагрузить модуль (зачем?)
void mks_wifi_reset();

//Вызывается, когда от ESPшки пришёл байт
uint8_t mks_wifi_input(uint8_t data);

//Обработать пакет, пришедший от ESPшки
void mks_wifi_parse_in_packet(ESP_PROTOC_FRAME *packet);

//Вызывается при каждом обновлении статуса сети
void mks_wifi_update_netstatus();

//Вызывается при каждом получении результатов сканирования сетей
void mks_wifi_update_scanresult();

//Отправить байты напрямую в ESPшку
void mks_wifi_send(uint8_t *packet, uint16_t size);

//Собрать пакет esp_frame в буфер packet, возвращает размер; нужен буфер размером MKS_TOTAL_PACKET_SIZE
uint16_t mks_wifi_build_packet(uint8_t *packet, ESP_PROTOC_FRAME *esp_frame);

//Настроить сеть; mode=WIFI_MODE_*; name_len - длина имени сети(<32 байт); key_len - длина пароля(<64 байт)
void mks_wifi_send_netconf(uint8_t mode, uint8_t name_len, char* name, uint8_t key_len, char* key);

//Ответ от выполнения GCODа
void mks_wifi_send_gcoderep(uint8_t len, char* str);

//В документации не описано, что это и когда отправлять, пусть будет
void mks_wifi_send_exception(uint8_t code);

//отключиться от сети
void mks_wifi_send_unbind();

//сканировать сети
void mks_wifi_send_scan();

//подключиться(ESP_WIFI_ACT_CONN) к сохраненной/отключиться(ESP_WIFI_ACT_DISCONN)/забыть пароль от сети(ESP_WIFI_ACT_FORGET)
void mks_wifi_send_wifi_connect(uint8_t mode);

uint8_t check_char_allowed(char data);


#endif