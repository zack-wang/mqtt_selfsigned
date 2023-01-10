#include "Arduino.h"
#include <WiFi.h>
#include <EEPROM.h>
#include "esp_log.h"
#include "esp_system.h"
#include "esp_event.h"
#include "mqtt_client.h"
#include "esp_tls.h"
#include "time.h"
#define TOPIC_PREFIX "TW/CTRL/ZackLtd/"
#define BROKER_HOST "34.81.243.242"
#define BROKER_PORT 8883
#define PROM_ADDR_SSID 0
#define PROM_ADDR_PASS 32
#define PROM_ADDR_MQTT_USERNAME 64
#define PROM_ADDR_MQTT_PASSWORD 96
#define PROM_ADDR_MQTT_TOPIC 128

String saved_ssid = "";
String saved_pass = "";
String inputString = "";
String saved_mqtt_user ="";
String saved_mqtt_pass ="";
String saved_topic_prefix ="";
String sub_topic="";
String pub_topic="";
String topic="";
int waitTick = 30;

// NTP server to request epoch  time
const char* ntpServer = "time.stdtime.gov.tw";

// Variable to save current epoch time
unsigned long epochTime; 

// Function that gets current epoch time
unsigned long getTime() {
  time_t now;
  struct tm timeinfo;
  if (!getLocalTime(&timeinfo)) {
    //Serial.println("Failed to obtain time");
    return(0);
  }
  time(&now);
  return now;
}

static const unsigned char CA_cert[] PROGMEM= R"EOF( 
-----BEGIN CERTIFICATE-----
MIIFszCCA5ugAwIBAgIUXzl9LfjYrNkiaRZuT3HNt5cyms4wDQYJKoZIhvcNAQEL
BQAwaTELMAkGA1UEBhMCVFcxDzANBgNVBAgMBlRhaXdhbjERMA8GA1UEBwwIVGFp
Y2h1bmcxEjAQBgNVBAoMCVphY2sgTHRkLjELMAkGA1UECwwCUkQxFTATBgNVBAMM
DHphY2sucm9vdC5jYTAeFw0yMjEyMDQwMTM0MjdaFw0zMjA5MDIwMTM0MjdaMGkx
CzAJBgNVBAYTAlRXMQ8wDQYDVQQIDAZUYWl3YW4xETAPBgNVBAcMCFRhaWNodW5n
MRIwEAYDVQQKDAlaYWNrIEx0ZC4xCzAJBgNVBAsMAlJEMRUwEwYDVQQDDAx6YWNr
LnJvb3QuY2EwggIiMA0GCSqGSIb3DQEBAQUAA4ICDwAwggIKAoICAQDJBRbuGZH5
uJjZUsALV/CRypM4mgfZu9wnWgfB6e6WmX/ouHuvjtzP/Mt/pDg3GjCzwwJ2Utya
3N0DeHbWhPqvQa1bsD84OfUpybptjXIF64SSCrNXtLdG+nHxgJhxsjQHUza9nVj8
knhL5R2VKb8hq2fqQl6RaRVxsYruKevyehiaPi5/ctBLN3a34euqb4HXKeuQ49y2
7/YmDfHeaXfZLs+8tekOQQNr7KfSWwVU1f70wyTJdietQPovprm+tt5VkZDazLaJ
GvMgLOOZ3W3WVqGsm3TRRN8Gz/BmwTlZwMmEi6krCJiHnrduJxSjNeuJMYIArV+O
Kz5AnUWBE9H9kfiGlVOOJ/ms7lneAkXWwrdQwfH1PpBgjZQ+ZOggXlhb6R91+sfF
abJlce14mdX7jcUxJEC7DXofKAg2eItKJwI0XUkWqDMg3IhD21SEfwpzJkspmrDh
RxDpzuoMqvROpErSkTrZPNmv0py2sGAVG6bQeQNI3WP/HYizDTZ2RYrqbQHIqvSd
MKHpm6i9/3Q5sBFPHbk2AdV5dJqZmEUP0fsNe0to02Bs/sSXmZgjM732fWNUYoCA
BvvFE7owAqrAL74XReUKnysKvvoqxH2M4r5dX+hMd81AemYa00WdB1Kqa3iO4He6
2hcPw8f5Lmrv0d3Ywt6hMB76kOYinnP0ZQIDAQABo1MwUTAdBgNVHQ4EFgQUQRgX
IogNg/F5ZT624fQbNi7LCOAwHwYDVR0jBBgwFoAUQRgXIogNg/F5ZT624fQbNi7L
COAwDwYDVR0TAQH/BAUwAwEB/zANBgkqhkiG9w0BAQsFAAOCAgEAvkoHTmcsOPcs
cwQr81/N37fHeT0By1aTQFjkb8ixND+lW4urj0ytjhcWfxk5jqfbnw3OeDMb5DGv
aKSRvRd+OiwDlxlRBR1SAdbLgacImSa+oxnUFshmn1d5cTVYHLL3IW27Ii6HOqT+
yNXzaXmkImdDxcWAxnmTqe+KY6mw89ZCJgW6lxvngz8IaQai8m56UBM1NMkfvwhb
mxFUOCseswYAFFqtCo+lwUb1onUkHDiPXbGQrwctD9zIUiu3DTLNGPGxdr2zAIp4
3rsTWSffSaqwSppnuU34LefQCPe0x3EwqU42Jax+2JHygblddEw2JtKI+ECKiBDI
Xzplzq/MXY2ouMoF6VOcHoH+qyCTk99sfBwCYM338oOs/F5uhhzbTJoLNDxYFMs+
hMojwgJkhv+sFg9yUrYTgUP2taQXodmyU12/azNhyo5Sl3uDxCy0hA5sTpcFDkBw
26wxik1+u/CfRstS4CzhrqaYmR9Q8IVnetaTX0poqrdwHlv/cjVIieerRl0cIUD3
/FwsyktOkh3P97CT6KmTdlbnc4QjDKUYGGGhRMJ61fZVFMOQiyfsvOoSUIYH9BAR
Bc3zeHRK+EwK6Sduv1D7KUloKs8tmlKfpRGRUfbTxzZdD4+Ichi3s4hOFb30ZHRp
VQV3Mb04UfbipLDoQiX7YUm7jHo7XdE=
-----END CERTIFICATE-----
)EOF";

unsigned char plain_txt[256];

 

esp_mqtt_client_config_t mqtt_cfg;
esp_mqtt_client_handle_t client;

static esp_err_t mqtt_event_handler (esp_mqtt_event_handle_t event) {
  if (event->event_id == MQTT_EVENT_CONNECTED) {
    String top="";
    top=saved_topic_prefix+"cmd";
    ESP_LOGI ("TEST", "MQTT msgid= %d event: %d. MQTT_EVENT_CONNECTED", event->msg_id, event->event_id);
    // 接收命令的 topic = TOPIC_PREFIX "cmd" 
    esp_mqtt_client_subscribe (client, (const char*)sub_topic.c_str() , 0);
    // 發布狀態的 topic = TOPIC_PREFIX "sts"
    // esp_mqtt_client_publish (client, TOPIC_PREFIX "sts", "online", 6, 0, false);
    Serial.println("MQTT connected.");
  } 
  else if (event->event_id == MQTT_EVENT_DISCONNECTED) {
    ESP_LOGI ("TEST", "MQTT event: %d. MQTT_EVENT_DISCONNECTED", event->event_id);
    //esp_mqtt_client_reconnect (event->client); //not needed if autoconnect is enabled
  } else  if (event->event_id == MQTT_EVENT_SUBSCRIBED) {
    ESP_LOGI ("TEST", "MQTT msgid= %d event: %d. MQTT_EVENT_SUBSCRIBED", event->msg_id, event->event_id);
  } else  if (event->event_id == MQTT_EVENT_UNSUBSCRIBED) {
    ESP_LOGI ("TEST", "MQTT msgid= %d event: %d. MQTT_EVENT_UNSUBSCRIBED", event->msg_id, event->event_id);
  } else  if (event->event_id == MQTT_EVENT_PUBLISHED) {
    ESP_LOGI ("TEST", "MQTT event: %d. MQTT_EVENT_PUBLISHED", event->event_id);
  } else  if (event->event_id == MQTT_EVENT_DATA) {
    ESP_LOGI ("TEST", "MQTT msgid= %d event: %d. MQTT_EVENT_DATA", event->msg_id, event->event_id);
    ESP_LOGI ("TEST", "Topic length %d. Data length %d", event->topic_len, event->data_len);
    Serial.printf("Incoming data: %.*s %.*s\n", event->topic_len,event->topic, event->data_len,event->data);
    if (event->data_len > 1){
      if (event->data[0]=='S'){
        if(event->data[0]=='U'){
          digitalWrite(12,LOW);
          delay(500);
          digitalWrite(12,HIGH);
        }
        if(event->data[0]=='S'){
          digitalWrite(13,LOW);
          delay(500);
          digitalWrite(13,HIGH);
        }
        if(event->data[0]=='D'){
          digitalWrite(14,LOW);
          delay(500);
          digitalWrite(14,HIGH);
        }
      }
      if (event->data[0]=='2'){
        if (event->data[1]=='H'){
          digitalWrite(2,HIGH);
        }else if (event->data[1]=='L'){
          digitalWrite(2,LOW);
        }else if (event->data[1]=='P'){
          digitalWrite(2,HIGH);
          delay(300);
          digitalWrite(2,LOW);
        }else if (event->data[1]=='N'){
          digitalWrite(2,LOW);
          delay(300);
          digitalWrite(2,HIGH);
        }else{
          Serial.println("wrong command");
        }
      }
    }

  } else  if (event->event_id == MQTT_EVENT_BEFORE_CONNECT) {
    ESP_LOGI ("TEST", "MQTT event: %d. MQTT_EVENT_BEFORE_CONNECT", event->event_id);
  }
  return ESP_OK;
}

void setup() {
  // 板子 ESP32DOIT DevKit V1, 繼電器是反向時作動, 即低電位動作, 高電位不動作
  pinMode(2, OUTPUT); // 藍色LED
  pinMode(12, OUTPUT);
  digitalWrite(12,HIGH); 
  pinMode(13, OUTPUT);
  digitalWrite(13,HIGH);
  pinMode(14, OUTPUT);
  digitalWrite(14,HIGH);

  // 設定序列埠 (USB)
  Serial.begin(115200);
  Serial.println("Start - type cfg or wait 15 seconds");
  EEPROM.begin(192); //32*6 , topic is length 39 

  // Read Serial
  for (int i=0;i<waitTick;i++){
    if (Serial.available() > 0) {
      // read the incoming string:
      inputString = Serial.readString();
      int len=0;
      len=inputString.length();
      Serial.println("->"+inputString.substring(0,len-1));

      // Enter Config mode
      if ( strncmp(inputString.c_str(),"cfg",3)==0 ){
        waitTick=600;
        Serial.println("!!!Config Mode!!! - 5 minutes left");
      }

      // Enter Reset mode
      if ( strncmp(inputString.c_str(),"rst",3)==0 ){
        EEPROM.commit(); // important
        Serial.println("Resetting... in 3 seconds");
        delay(3000);
        ESP.restart();
      }

      // change SSID
      if ( strncmp(inputString.c_str(),"ssid=",5)==0 ) {
        EEPROM.writeString(PROM_ADDR_SSID,inputString.substring(5,len-1));
        Serial.println("wifi ssid written");
      }

      // change wifi password
      if ( strncmp(inputString.c_str(),"pass=",5)==0 ) {
        EEPROM.writeString(PROM_ADDR_PASS,inputString.substring(5,len-1));
        Serial.println("wifi password written");
      }
      
      // change mqtt username
      if ( strncmp(inputString.c_str(),"mqun=",5)==0 ) {
        EEPROM.writeString(PROM_ADDR_MQTT_USERNAME,inputString.substring(5,len-1));
        Serial.println("mqtt username written");
        topic=topic+TOPIC_PREFIX;
        topic=topic+inputString.substring(5,len-1);
        topic=topic+"/";
        EEPROM.writeString(PROM_ADDR_MQTT_TOPIC,topic);
        Serial.println("mqtt topic prefix "+topic+" written");
      }
      
      // change mqtt password
      if ( strncmp(inputString.c_str(),"mqpw=",5)==0 ) {
        EEPROM.writeString(PROM_ADDR_MQTT_PASSWORD,inputString.substring(5,len-1));
        Serial.println("mqtt password written");
      }

    }
    delay(500);
  }
  // EEPROM
  saved_ssid=EEPROM.readString(PROM_ADDR_SSID);
  saved_pass=EEPROM.readString(PROM_ADDR_PASS);
  saved_mqtt_user=EEPROM.readString(PROM_ADDR_MQTT_USERNAME);
  saved_mqtt_pass=EEPROM.readString(PROM_ADDR_MQTT_PASSWORD);
  saved_topic_prefix=EEPROM.readString(PROM_ADDR_MQTT_TOPIC);
  //Serial.println(EEPROM.readString(PROM_ADDR_MQTT_TOPIC));
  Serial.println("EEPROM ...");
  Serial.println(saved_ssid);
  Serial.println(saved_pass);
  Serial.println(saved_mqtt_user);
  Serial.println(saved_mqtt_pass);
  Serial.println(saved_topic_prefix);
  pub_topic=saved_topic_prefix+"sts";
  sub_topic=saved_topic_prefix+"cmd";
  Serial.println("Topics ...");
  Serial.println(pub_topic);
  Serial.println(sub_topic);
    
  int ret;
  unsigned long ts;
  size_t outputLength,i;
  
  WiFi.begin(saved_ssid.c_str(), saved_pass.c_str());

  // attempt to connect to Wifi network:
  Serial.println("Connecting WiFi\n");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    // wait 1 second for re-trying
    delay(1000);
  }
  Serial.println("Connected.");
  configTime(0, 0, ntpServer);
  delay(5000);
  ts=getTime();

  /*
  sprintf((char*)plain_txt,"usr=abcd1234,iat=%d",ts);
  Serial.printf("payload=%s\n",plain_txt);
  Serial.printf("payload text length=%d\n",strlen((const char*)plain_txt));
  */
    
  mqtt_cfg.host = BROKER_HOST;
  mqtt_cfg.port = BROKER_PORT;
  
  mqtt_cfg.transport = MQTT_TRANSPORT_OVER_SSL;
  /*
  mqtt_cfg.crt_bundle_attach = esp_crt_bundle_attach;
  mqtt_cfg.lwt_topic = "lwt/ESP32";
  mqtt_cfg.lwt_msg = "offline";
  mqtt_cfg.lwt_msg_len = 7;
  char *uri_line=(char *)malloc(100);
  sprintf(uri_line,"mqtt://%s:%u",MQTT_HOST,MQTT_PORT);
  */
  mqtt_cfg.username = saved_mqtt_user.c_str();
  mqtt_cfg.password = saved_mqtt_pass.c_str();
  
  mqtt_cfg.client_id = saved_mqtt_user.c_str();
  mqtt_cfg.keepalive = 60;
  
  mqtt_cfg.event_handle = mqtt_event_handler;
  // 服務器使用ip地址, 所以CN 不檢查
  mqtt_cfg.skip_cert_common_name_check = true;
  mqtt_cfg.cert_pem = (const char *)CA_cert;

  
  esp_err_t err;
  /* 正式的CA
  esp_tls_init_global_ca_store ();
  esp_err_t err = esp_tls_set_global_ca_store (CA_cert, sizeof (CA_cert));
  ESP_LOGI ("TEST","CA store set. Error = %d %s", err, esp_err_to_name(err));
  */
  client = esp_mqtt_client_init (&mqtt_cfg);
  
  //esp_mqtt_client_register_event (client, ESP_EVENT_ANY_ID, mqtt_event_handler, client); // not implemented in current Arduino core
  err = esp_mqtt_client_start (client);
  ESP_LOGI ("TEST", "Client connect. Error = %d %s", err, esp_err_to_name (err));
  esp_mqtt_client_publish (client, (const char*)pub_topic.c_str(), "online", 6, 0, false);
  delay(1000);
  Serial.printf("All set\n");
}

void loop() {
  //esp_mqtt_client_connect(client);
  // 每一分鐘送出在線訊息, 非必要
  esp_mqtt_client_publish (client, (const char*)pub_topic.c_str(), "online", 6, 0, false);
  delay (60000);
}
