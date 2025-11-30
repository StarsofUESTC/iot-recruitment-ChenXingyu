#include <WiFi.h>       // ESP32专用的WiFi库
#include <WebServer.h>  // ESP32专用的WebServer库

// Wi-Fi信息
const char* ssid = "Stars_iPhone";
const char* password = "ivfusbivm";

// 定义LED引脚
#define LED_PIN 5

// 在端口80上创建一个服务器对象
WebServer server(80);

// 处理根路径“/”的函数
void handleRoot() {
  // 构建一个简单的HTML网页，包含两个按钮
  String html = "<!DOCTYPE html> <html> <head> <meta charset='UTF-8'> <title>ESP32遥控器</title> <style>body{font-family:Arial; text-align:center; margin-top:50px;} button{padding:10px 20px; font-size:16px; margin:10px;}</style> </head> <body>";
  html += "<h1>ESP32 LED遥控器</h1>";
  html += "<p><a href='/on'><button style='background-color: #4CAF50; color: white;'>开灯</button></a></p>";
  html += "<p><a href='/off'><button style='background-color: #f44336; color: white;'>关灯</button></a></p>";
  html += "</body> </html>";
  server.send(200, "text/html", html);
}

// 处理“/on”路径的函数
void handleOn() {
  digitalWrite(LED_PIN, HIGH); // 点亮LED
  Serial.println("收到开灯指令");
  // 返回一个提示信息，并自动跳转回首页
  server.send(200, "text/html", "<html><head><meta http-equiv='refresh' content='2;url=/'></head><body><p>LED已打开,2秒后返回...</p></body></html>");
}

// 处理“/off”路径的函数
void handleOff() {
  digitalWrite(LED_PIN, LOW); // 熄灭LED
  Serial.println("收到关灯指令");
  server.send(200, "text/html", "<html><head><meta http-equiv='refresh' content='2;url=/'></head><body><p>LED已关闭,2秒后返回...</p></body></html>");
}

// 处理未找到的路径（404错误）
void handleNotFound() {
  server.send(404, "text/plain", "404: Not found");
}

void setup() {
  Serial.begin(115200);
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW); // 初始化时确保LED熄灭

  // 连接Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("正在连接WiFi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi连接成功!");

  // 打印IP地址到串口监视器
  Serial.print("ESP32的IP地址是: ");
  Serial.println(WiFi.localIP());

  // 设置服务器路由（将URL路径绑定到处理函数）
  server.on("/", handleRoot);   // 当访问根目录时
  server.on("/on", handleOn);   // 当访问 “/on” 时
  server.on("/off", handleOff); // 当访问 “/off” 时
  server.onNotFound(handleNotFound); // 当访问不存在的路径时

  // 启动服务器
  server.begin();
  Serial.println("Web服务器已启动!");
  Serial.println("请用浏览器访问以上IP地址");
}

void loop() {
  server.handleClient(); // 持续监听客户端的请求
}