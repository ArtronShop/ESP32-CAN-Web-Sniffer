#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "driver/gpio.h"
#include "driver/twai.h"

// Web Page
#include "web/index_html.h"

// WiFi AP Mode
const char* ssid = "ESP32-CAN";

// WebSocket server
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// ตั้งค่า CAN
void setupCAN() {
  twai_general_config_t g_config = {
    .mode = TWAI_MODE_NORMAL,
    .tx_io = GPIO_NUM_5,
    .rx_io = GPIO_NUM_4,
    .clkout_io = TWAI_IO_UNUSED,
    .bus_off_io = TWAI_IO_UNUSED,
    .tx_queue_len = 10,
    .rx_queue_len = 10,
    .alerts_enabled = TWAI_ALERT_NONE,
    .clkout_divider = 0
  };

  twai_timing_config_t t_config = TWAI_TIMING_CONFIG_500KBITS();
  twai_filter_config_t f_config = TWAI_FILTER_CONFIG_ACCEPT_ALL();

  if (twai_driver_install(&g_config, &t_config, &f_config) == ESP_OK) {
    Serial.println("CAN driver installed");
  }

  if (twai_start() == ESP_OK) {
    Serial.println("CAN started");
  }
}

// รับข้อความจาก WebSocket
void onWebSocketEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len) {
  if (type == WS_EVT_CONNECT) {
    Serial.println("WebSocket client connected");
    client->text("Connected to ESP32 CAN WebSocket");
  } else if (type == WS_EVT_DATA) {
    // ส่งข้อความผ่าน CAN จากข้อความที่ client ส่งมา
    String msg = "";
    for (size_t i = 0; i < len; i++) {
      msg += (char)data[i];
    }

    if (msg.startsWith("BAUD:")) {
        msg.replace("BAUD:", "");
        long new_baud = msg.toInt();
    }

    /*
    twai_message_t tx_msg;
    tx_msg.identifier = 0x123;
    tx_msg.flags.extended = 0;
    tx_msg.data_length_code = msg.length() > 8 ? 8 : msg.length();
    memcpy(tx_msg.data, msg.c_str(), tx_msg.data_length_code);

    if (twai_transmit(&tx_msg, pdMS_TO_TICKS(100)) == ESP_OK) {
      Serial.println("Sent CAN: " + msg);
    } else {
      Serial.println("CAN TX failed");
    }
    */
  }
}

void setup() {
  Serial.begin(115200);

  // WiFi Access Point
  WiFi.softAP(ssid);
  Serial.println("WiFi AP started");

  // Start WebSocket
  ws.onEvent(onWebSocketEvent);
  server.addHandler(&ws);
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request){
    request->send(200, "text/html", index_html);
  });
  server.begin();
  Serial.println("WebSocket server started");

  // Start CAN
  setupCAN();
}

void loop() {
  // รับ CAN frame แล้วส่งผ่าน WebSocket
  twai_message_t rx_msg;
  if (twai_receive(&rx_msg, pdMS_TO_TICKS(10)) == ESP_OK) {
    String msg = "CAN ID 0x" + String(rx_msg.identifier, HEX) + ": ";
    for (int i = 0; i < rx_msg.data_length_code; i++) {
      msg += String(rx_msg.data[i], HEX) + " ";
    }
    Serial.println(msg);
    ws.textAll(msg);
  }
}
