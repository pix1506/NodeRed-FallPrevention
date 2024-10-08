#include <Flag_MPU6050.h>
#include <PubSubClient.h>
#include <WiFi.h> // 使用ESP32需要的WiFi库
#include <ArduinoJson.h>

// 网络设置
const char* ssid = "蜂窩性祖師爺"; // 替换成你的WiFi SSID
const char* password = "20202020"; // 替换成你的WiFi密码

// MQTT服务器设置
const char* mqtt_server = "test.mosquitto.org";
const int mqtt_port = 1883; // MQTT端口，默认通常是1883


WiFiClient espClient;
PubSubClient client(espClient);

Flag_MPU6050 mpu6050;

void setup_wifi() {
  delay(10);
  // 连接WiFi
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // 循环直到连接上MQTT
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // 尝试连接
    if (client.connect("arduinoClient")) { // "arduinoClient"是客户端ID，MQTT要求每个客户端ID唯一
      Serial.println("connected");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // 等5秒后重试
      delay(5000);
    }
  }
}

void setup(){
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);

  mpu6050.init();
  while(!mpu6050.isReady());
}

void loop(){
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  mpu6050.update();

  // 创建一个足够大的JSON文档
  StaticJsonDocument<256> jsonDoc;

  // 将数据添加到JSON文档
  jsonDoc["ACC_X"] = mpu6050.data.accX;
  jsonDoc["ACC_Y"] = mpu6050.data.accY;
  jsonDoc["ACC_Z"] = mpu6050.data.accZ;
  jsonDoc["GYR_X"] = mpu6050.data.gyrX;
  jsonDoc["GYR_Y"] = mpu6050.data.gyrY;
  jsonDoc["GYR_Z"] = mpu6050.data.gyrZ;
  jsonDoc["Temperature"] = mpu6050.data.temperature;

  // 序列化JSON文档为字符串
  char msg[256];
  serializeJson(jsonDoc, msg);

  // 发送JSON字符串到MQTT主题
  client.publish("sensor/mpu6050", msg);

  delay(1000);  

  mpu6050.update();

  // 顯示 MPU6050 資訊
  Serial.print("ACC_X水平左右加速度: "); 
  Serial.println(mpu6050.data.accX);
  Serial.print("ACC_Y水平前進加速度: "); 
  Serial.println(mpu6050.data.accY);
  Serial.print("ACC_Z蹲下跳躍加速度: "); 
  Serial.println(mpu6050.data.accZ);
  Serial.print("GYR_X正面旋轉速率: "); 
  Serial.println(mpu6050.data.gyrX);
  Serial.print("GYR_Y側面旋轉速率: "); 
  Serial.println(mpu6050.data.gyrY);
  Serial.print("GYR_Z原地旋轉速率: "); 
  Serial.println(mpu6050.data.gyrZ);
  Serial.print("Temperature: "); 
  Serial.println(mpu6050.data.temperature);
  Serial.println();

  delay(1000); // 根据需要调整发送间隔
}
