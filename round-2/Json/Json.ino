#include <WiFi.h>
#include <WebServer.h>
#include <DHT.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>

const char* ssid = "Stars_iPhone";
const char* password = "ivfusbivm";

const int LED_PIN = 5;
const int DHT_PIN = 4;

const char* write_api_key = "XHOK42TTLBLV3F3W";
const char* thingspeak_server = "api.thingspeak.com";

const unsigned long sensor_read_interval = 2000;
const unsigned long data_print_interval = 5000;
const unsigned long thingspeak_interval = 30000;

float temperature = 0.0;
float humidity = 0.0;
bool ledState = false;
unsigned long previous_sensor_read_time = 0;
unsigned long previous_data_print_time = 0;
unsigned long previous_thingspeak_time = 0;

DHT dht(DHT_PIN, DHT11);
WebServer server(80);

void read_sensor_data() {
  float temp = dht.readTemperature();
  float hum = dht.readHumidity();
  
  if (!isnan(temp)) {  //isnan确保输入有效的数据(not a number)
    temperature = temp;
  }
  if (!isnan(hum)) {
    humidity = hum;
  }
}

void print_sensor_data() {
  Serial.print("温度: ");
  Serial.print(temperature);
  Serial.print("°C, 湿度: ");
  Serial.print(humidity);
  Serial.println("%");
}

void data_to_thingspeak(){
  if (WiFi.status() == WL_CONNECTED){ 
  
    HTTPClient http;//发送HTTP请求
    
    // 构建ThingSpeak API URL
    String url = "http://";
    url += thingspeak_server;
    url += "/update?api_key=";
    url += write_api_key;
    url += "&field1=";
    url += String(temperature);
    url += "&field2=";
    url += String(humidity);
    
    Serial.println("正在上传数据到ThingSpeak...");
    Serial.print("URL: ");
    Serial.println(url);
    
    // 发送HTTP GET请求
    http.begin(url);
    int httpResponseCode = http.GET();
    
    //$$
    if (httpResponseCode > 0) {
      Serial.print("ThingSpeak上传成功,响应码: ");
      Serial.println(httpResponseCode);
      
      // 读取服务器响应
      String response = http.getString();
      Serial.print("服务器响应: ");
      Serial.println(response);
    } else {
      Serial.print("ThingSpeak上传失败,错误码: ");
      Serial.println(httpResponseCode);
    }
    //$$这个if-else的反馈部分是上网查的
    
    http.end();
  
  }
}

//挑战一下使用ArduinoJson库
void Json(){
  // 创建动态JSON文档,灵活地添加各种数据类型
  DynamicJsonDocument doc(1024);//可以存储1024字节
  
  // 添加数据到JSON对象
  doc["temperature"] = temperature;
  doc["humidity"] = humidity;
  doc["led_status"] = ledState ? "ON" : "OFF";//三元运算符,True为ON，False为OFF
  doc["timestamp"] = millis();
  doc["device"] = "ESP32";//设备标识字段
  doc["ip_address"] = WiFi.localIP().toString();//获取ESP32的IP地址,将IP地址转换为字符串格式
  
  // 序列化为字符串
  String json;//创建空字符串用于存储序列化结果
  serializeJson(doc, json);//将JSON文档转换为字符串格式
  
  // 发送JSON响应
  server.send(200, "application/json", json);
  Serial.println("JSON API被访问,数据已发送");

}

void handleRoot() {
  String html = "<!DOCTYPE html><html><head><meta charset='UTF-8'><title>ESP32监控</title>";
  html += "<style>";
  html += "body{font-family:Arial; text-align:center; margin:50px; background:#f0f0f5;}";
  html += ".container{background:white; padding:20px; margin:20px auto; border-radius:8px; max-width:500px; box-shadow:0 2px 5px rgba(0,0,0,0.1);}";
  html += "button, .api-btn{padding:12px 25px; font-size:16px; border:none; border-radius:5px; color:white; margin:10px; text-decoration:none; display:inline-block;}";
  html += ".on-btn{background:#4CAF50;} .off-btn{background:#f44336;} .api-btn{background:#2196F3;}";
  html += ".sensor-value{font-size:22px; font-weight:bold; color:#2c3e50;}";
  html += ".api-section{background:#e3f2fd; padding:15px; margin:15px 0; border-radius:5px;}";
  html += "</style></head><body>";
  
  html += "<h1>ESP32智能监控系统</h1>";
  
  // 传感器数据显示
  html += "<div class='container'>";
  html += "<h2>📊 实时传感器数据</h2>";
  html += "<p>温度: <span class='sensor-value'>" + String(temperature, 1) + "°C</span></p>";
  html += "<p>湿度: <span class='sensor-value'>" + String(humidity, 1) + "%</span></p>";
  html += "</div>";
  
  // LED控制
  html += "<div class='container'>";
  html += "<h2>💡 LED远程控制</h2>";
  html += "<a href='/on'><button class='on-btn'>开灯</button></a>";
  html += "<a href='/off'><button class='off-btn'>关灯</button></a>";
  html += "<p>状态: <strong>" + String(ledState ? "🔵 开启" : "⚪ 关闭") + "</strong></p>";
  html += "</div>";
  
  // API接口说明区域              
  html += "<div class='container'>";
  html += "<h2>🔌 REST API接口</h2>";
  html += "<div class='api-section'>";
  html += "<p>获取JSON格式数据:</p>";
  html += "<a href='/json' class='api-btn'>访问 /json API</a>";
  html += "<p><small>返回格式: {\"temperature\":25.5, \"humidity\":60.2, \"led_status\":\"ON\"}</small></p>";
  html += "</div>";
  html += "<p><small>API地址: http://" + WiFi.localIP().toString() + "/json</small></p>";
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
  server.send(404, "text/plain", "404: Not found - 可用端点: /, /on, /off, /json");
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("ESP32设备启动中...");

  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);
  
  dht.begin();

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

  
  server.on("/", handleRoot);
  server.on("/on", handleOn);
  server.on("/off", handleOff);
  server.on("/json", Json);  // JSON API端点
  server.onNotFound(handleNotFound);

  server.begin();
  Serial.println("Web服务器已启动");
  Serial.println("API接口: http://" + WiFi.localIP().toString() + "/json");
  
  read_sensor_data();
  Serial.println("系统初始化完成");

}

void loop() {
  // put your main code here, to run repeatedly:
   unsigned long current_time = millis();
  
  server.handleClient();
  
  if (current_time - previous_sensor_read_time >= sensor_read_interval) {
    previous_sensor_read_time = current_time;
    read_sensor_data();
  }
  
  if (current_time - previous_data_print_time >= data_print_interval) {
    previous_data_print_time = current_time;
    print_sensor_data();
  }

  if (current_time - previous_thingspeak_time >= thingspeak_interval){
    previous_thingspeak_time = current_time;
    data_to_thingspeak();
  }
  //这里本来想简化一下这3个if语句的，但是由于变量名比较长，感觉没简化多少

}
