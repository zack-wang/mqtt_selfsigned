# Board Remote - ESP32 Config (開發版遙控器 - ESP32配置)
![Terminal Snapshot](/assets/images/serial2.png)

# Terminal Command
- cfg
> Enter config mode (enter main in 5 minutes), 進入設定模式
- rst
> Commit EEPROM and RESET , 燒錄EEPROM然後重啟
- ssid=wifi1
> Set wifi ssid to "wifi1" , 設定連上的wifi名稱
- pass=wifipass
> Set wifi password to "wifipass" , 設定連上wifi的密碼
- mqun=mqttusername
> Set MQTT broker username to "mqttusername" , 設定 mqtt 連線帳號
- mqpw=mqttpassword
> Set MQTT broker password to "mqttpassword" , 設定 mqtt 連線密碼 