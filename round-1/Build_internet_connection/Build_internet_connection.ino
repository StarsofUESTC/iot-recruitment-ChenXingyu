#include <WiFi.h>
void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);

  Serial.print("正在连接到: ");
  Serial.println("Stars_iPhone");
  Serial.println("【测试】串口通信正常！程序已开始执行！");
  WiFi.begin("Stars_iPhone","ivfusbivm");

  int attempts = 0;

  while(WiFi.status() != WL_CONNECTED && attempts < 30){
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n✅ 连接成功！IP地址: " + WiFi.localIP().toString());
  } else {
    Serial.println("\n❌ 连接失败！请检查热点信息。");
  }

}

void loop() {
  // put your main code here, to run repeatedly:

}
