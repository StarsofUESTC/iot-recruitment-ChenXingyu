//定义LED引脚数组
int led_pin_list[5] = {13,12,14,27,26};
int num = sizeof(led_pin_list)/sizeof(led_pin_list[0]);

void setup() {
  // put your setup code here, to run once:
  //设置所有引脚为输出模式
  for(int i = 0;i < num;i++){
    pinMode(led_pin_list[i], OUTPUT);
    }
  
}

void loop() {
  // put your main code here, to run repeatedly:
  //设置所有引脚为高电平
  for (int j = 0;j < num ;j++){
    digitalWrite(led_pin_list[j],HIGH);
    delay(1000);
  }
  
  //设置所有引脚为低电平
  for (int j = 0;j < num ;j++){
    digitalWrite(led_pin_list[j],LOW);
    delay(1000);

  }
}
