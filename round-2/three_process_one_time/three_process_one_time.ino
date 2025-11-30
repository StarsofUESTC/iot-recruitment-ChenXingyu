#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>

const char* ssid = "Stars_iPhone";
const char* password = "ivfusbivm";

const int LED_PIN = 5;
const int DHT_PIN = 4;

const unsigned long sensor_read_interval = 2000;
const unsigned long data_print_interval = 5000;

float temperature = 0.0;
float humidity = 0.0;
bool ledState = false;//使用false表示LED熄灭，后面True表示LED点亮
unsigned long previous_sensor_read_time = 0;
unsigned long previous_data_print_time = 0;

DHT dht(DHT_PIN, DHT11);//创建对象
WebServer server(80);

// 读取DHT11传感器数据的函数
void read_sensor_data() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  
  if (!isnan(temp)) {
    temperature = temp;
  }
  if (!isnan(hum)) {
    humidity = hum;
  }
}

//打印传感器数据到串口的函数
void print_sensor_data() {
  Serial.print("温度: ");
  Serial.print(temperature);
  Serial.print("°C, 湿度: ");
  Serial.print(humidity);
  Serial.println("%");
}
//这里网页部分的代码先沿用第一期的，然后再补充显示温湿度的部分
void handleRoot() {
  // 使用String字符串拼接构建HTML页面
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32监控</title>";
  
  // 前端
  html += "<style>";
  html += "body{font-family:Arial; text-align:center; margin:50px; background:#f0f0f0;}";
  html += ".container{background:white; padding:20px; margin:20px auto; border-radius:8px; max-width:500px;}";
  html += "button{padding:12px 25px; font-size:16px; border:none; border-radius:5px; color:white; margin:10px;}";
  html += ".on-btn{background:#4CAF50;} .off-btn{background:#f44336;}";
  html += ".sensor-value{font-size:22px; font-weight:bold; color:#2c3e50;}";
  html += "</style></head><body>";
  
  html += "<h1>ESP32监控面板</h1>";
  
  // 温湿度数据显示区域
  html += "<div class='container'>";
  html += "<h2>传感器数据</h2>";
  html += "<p>温度: <span class='sensor-value'>" + String(temperature) + "°C</span></p>";//
  html += "<p>湿度: <span class='sensor-value'>" + String(humidity) + "%</span></p>";//
  html += "</div>";
  
  // LED控制区域
  html += "<div class='container'>";
  html += "<h2>LED控制</h2>";
  html += "<a href='/on'><button class='on-btn'>开灯</button></a>";
  html += "<a href='/off'><button class='off-btn'>关灯</button></a>";
  html += "<p>状态: " + String(ledState ? "开启" : "关闭") + "</p>";//
  html += "</div>";
  
  html += "</body></html>";
  
  server.send(200, "text/html", html);
}


void handleOn() {
  digitalWrite(LED_PIN, HIGH);
  ledState = true;
  Serial.println("收到开灯指令");
  server.send(200, "text/html", "<html><head><meta http-equiv='refresh' content='2;url=/'></head><body><p>LED已打开,2秒后返回...</p></body></html>");
}

void handleOff() {
  digitalWrite(LED_PIN, LOW);
  ledState = false;
  Serial.println("收到关灯指令");
  server.send(200, "text/html", "<html><head><meta http-equiv='refresh' content='2;url=/'></head><body><p>LED已关闭,2秒后返回...</p></body></html>");
}

void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

void setup() {
  // put your setup code here, to run once:
  // 初始化串口通信
  Serial.begin(115200);
  Serial.println("ESP32设备启动中...");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  Serial.println("LED引脚初始化完成");

  dht.begin();
  Serial.println("DHT11传感器初始化完成");

  WiFi.begin(ssid, password);
  Serial.print("正在连接WiFi");

  int attempts = 0;
while (WiFi.status() != WL_CONNECTED && attempts < 20) {
  delay(500);
  Serial.print(".");
  attempts++;
}

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\nWiFi连接成功!");
    Serial.print("IP地址: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\nWiFi连接失败");
  }

// 设置服务器路由
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.onNotFound(handleNotFound);

  // 启动Web服务器
  server.begin();
  Serial.println("Web服务器已启动,可通过浏览器访问设备IP");
  
  // 初始读取传感器数据
  read_sensor_data();
  Serial.println("系统初始化完成,开始运行主循环");
}

void loop() {
  // put your main code here, to run repeatedly:
  unsigned long current_time = millis();//这里使用millis函数,记录系统运行时间
  
  // 1. 即时处理网页请求
  server.handleClient();
  
  // 2. 每隔2s读取传感器数据
  if (current_time - previous_sensor_read_time >= sensor_read_interval) {
    previous_sensor_read_time = current_time;
    read_sensor_data();
  }
  
  // 3. 每隔5s打印数据到串口
  if (current_time - previous_data_print_time >= data_print_interval) {
    previous_data_print_time = current_time;
    print_sensor_data();
  }

}

