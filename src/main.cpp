#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <DNSServer.h>
#include "driver/gpio.h"
#include "driver/twai.h"

// Web Page
#include "web/index_html.h"

// WiFi AP Mode
const char* ssid = "ESP32-CAN";
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 4, 1);

// WebSocket server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");
DNSServer dns;

void changeTWAIBaudrate(long baud) {
  // หยุดและถอด driver เดิม
  twai_stop();
  twai_driver_uninstall();
  Serial.println("TWAI stopped and uninstalled");

  // กำหนด timing config ใหม่
  twai_timing_config_t t_config;

  if (baud == 1000000) {
    t_config = TWAI_TIMING_CONFIG_1MBITS();
  } else if (baud == 500000) {
    t_config = TWAI_TIMING_CONFIG_500KBITS();
  } else if (baud == 250000) {
    t_config = TWAI_TIMING_CONFIG_250KBITS();
  } else if (baud == 125000) {
    t_config = TWAI_TIMING_CONFIG_125KBITS();
  } else {
    Serial.println("Unsupported baudrate!");
    return;
  }

  // ติดตั้งใหม่
  twai_general_config_t g_config = TWAI_GENERAL_CONFIG_DEFAULT(GPIO_NUM_26, GPIO_NUM_27, TWAI_MODE_NORMAL);
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("TWAI driver re-installed");
  }

  if (twai_start() == ESP_OK) {
    Serial.printf("TWAI started at %d kbps\n", baud);
  }
}

// รับข้อความจาก WebSocket
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");
    // client->text("Connected to ESP32 CAN WebSocket");

    client->setCloseClientOnQueueFull(false);
  } else if (type == WS_EVT_DATA) {
    // ส่งข้อความผ่าน CAN จากข้อความที่ client ส่งมา
    String msg = "";
    for (size_t i = 0; i < len; i++) {
      msg += (char)data[i];
    }

    if (msg.startsWith("BAUD:")) {
        msg.replace("BAUD:", "");
        long new_baud = msg.toInt();
        Serial.println("Change CAN baud to " + String(new_baud));
        changeTWAIBaudrate(new_baud);
    } else {
      twai_message_t tx_msg;
      tx_msg.identifier = (data[0] << 8) | data[1];
      tx_msg.extd = 0;
      tx_msg.data_length_code = len - 2;
      if (tx_msg.data_length_code > 0) {
        memcpy(tx_msg.data, &data[2], tx_msg.data_length_code);
      }

      if (twai_transmit(&tx_msg, pdMS_TO_TICKS(100)) == ESP_OK) {
        String payload = "";
        for (int i=0;i<tx_msg.data_length_code;i++) {
          if (tx_msg.data[i] < 0x10) {
            payload += "0";
          }
          payload += String(tx_msg.data[i], HEX);
          if (i != (tx_msg.data_length_code - 1)) {
            payload += " ";
          }
        }
        Serial.println("Sent CAN: 0x" + String(tx_msg.identifier, 16) + " " + payload);
      } else {
        Serial.println("CAN TX failed");
      }
    }
  }
}

void setup() {
  Serial.begin(115200);

  Serial.println("Start...");

  delay(2000);

  // WiFi Access Point
  WiFi.disconnect();
  WiFi.mode(WIFI_AP_STA);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(ssid);
  Serial.println("WiFi AP started");

  // เริ่ม DNS: ทุกโดเมน → IP ESP32
  dns.start(DNS_PORT, "*", apIP);

  // Start WebSocket
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });
  server.begin();
  Serial.println("WebSocket server started");
}

void loop() {
  dns.processNextRequest();  // ให้ DNS server ทำงาน

  // รับ CAN frame แล้วส่งผ่าน WebSocket
  twai_message_t rx_msg;
  if (twai_receive(&rx_msg, pdMS_TO_TICKS(10)) == ESP_OK) {
    String msg = "CAN ID 0x" + String(rx_msg.identifier, HEX) + ": ";
    for (int i = 0; i < rx_msg.data_length_code; i++) {
      msg += String(rx_msg.data[i], HEX) + " ";
    }
    Serial.println(msg);

    size_t len = rx_msg.data_length_code + 2;
    uint8_t buff[len];
    memset(buff, 0, len);
    buff[0] = (rx_msg.identifier >> 8) & 0xFF;
    buff[1] = rx_msg.identifier & 0xFF;
    for (int i=0;i<rx_msg.data_length_code;i++) {
      buff[2 + i] = rx_msg.data[i];
    }
    ws.binaryAll(buff, len);
  }
}
