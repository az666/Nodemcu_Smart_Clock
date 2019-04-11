#include <ArduinoJson.h>
#include <ESP8266WiFi.h>
#include <EEPROM.h>
#include "D:\arduinoprojects\arduino_projects-master\qingqing_baijian\hz.c"
#include <ESP8266HTTPClient.h>
#define LED 2
String payload = "";              //获取数据储存变量
String com, tem1, tem2, humi, temnow, sta, state1, state2, time0, time1, time2, english;
String webadd = "http://flash.weather.com.cn/wmaps/xml/zhejiang.xml";    //天气接口地址
String city_id = "101210101";//浙江杭州
String time_url = "http://quan.suning.com/getSysTime.do";   //时间接口地址
String english_url = "http://open.iciba.com/dsapi/";   //英语接口地址
String daytime = "";
struct CONFIG {  //结构体存放账号密码
  char ssid[32];
  char password[32];
};
void smartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig");
  WiFi.beginSmartConfig();
  while (1)
  {
    Serial.print(".");
    digitalWrite(LED, 0);
    sao_sao();
    delay(500);
    digitalWrite(LED, 1);
    delay(500);
    if (WiFi.smartConfigDone())
    {
       EEPROM.begin(512);
      CONFIG buf;
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      strcpy(buf.ssid,  WiFi.SSID().c_str());
      strcpy(buf.password, WiFi.psk().c_str());
      EEPROM.put<CONFIG>(0, buf);
      EEPROM.commit();
      Serial.println(buf.ssid);
      Serial.println(buf.password);
      break;
    }
  }
}
void setup()
{
  Serial.begin(115200);
  sao_sao();
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);
  setup_wifi();
  open_show();
}
void sao_sao()
{
  Serial.print("CLS(0);");
  Serial.print("BPIC(1,45,29,17);");
  Serial.print("DS16(36,8,'"); Serial.print(hz[2]); Serial.print("',13,0);");
  Serial.print("\r\n");
}
void setup_wifi() {
 EEPROM.begin(512);
  CONFIG buf;
  EEPROM.get<CONFIG>(0, buf);
  Serial.println(buf.ssid);
  Serial.println(buf.password);
  EEPROM.commit();
  WiFi.begin(buf.ssid, buf.password);
  long lastMsg = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    long now = millis();
    if (now - lastMsg > 10000) {
      smartConfig();  //微信智能配网
      break;
    }
  }
}
void open_show()
{
  Serial.print("CLS(0);");
  delay(200);
  Serial.print("CLS(0);"); //CLS(0);
  Serial.print("DS12(10,20,'"); Serial.print(hz[0]); Serial.print("',4,0);"); //DS12(10,20,'晴天',4,0);
  Serial.print("CBOX(3,3,90,120,10,5);");
  Serial.print("CBOX(2,125,217,173,5,5);");
  Serial.print("PL(3,53,90,53,5);");
  Serial.print("CIR(150,60,50,1);");
  Serial.print("PIC(125,40,13);");
  Serial.print("\r\n");
  delay(200);
}
/*****************************************http数据获取*******************************************/
void http() {
  HTTPClient http;
  http.begin(webadd);
  int httpCode = http.GET();
  if (httpCode > 0) {
    payload = http.getString();
    //Serial.println(payload);
    int a = payload.indexOf(city_id);//自己搜索链接可知指定城市的id郑州市的id为‘101180101’
    //indexof();在字符串中中查找字符位置，返回值为给定字符的第一个位置，查找失败返回-1
    com = payload.substring(a - 280, a + 12);
    //字符串解析函数：substring(?,...),返回值为指定位置的字符串值
    //Serial.println(com);
    //<city cityX="376" cityY="184" cityname="郑州市" centername="郑州市" fontColor="FFFF00" pyName="" state1="0" state2="1" stateDetailed="晴转多云" tem1="37" tem2="24" temNow="33" windState="南风3-4级" windDir="西南风" windPower="2级" humidity="39%" time="11:00" url="101180101"/>
  }
  http.end();
}
/*********************************获取时间**************************************/
void get_time()
{
  HTTPClient http;
  http.begin(time_url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    payload = http.getString();
    //Serial.println(payload);
    /************************************json 数据处理********************************************/
    DynamicJsonBuffer jsonBuffer;
    String  input =   payload;
    JsonObject& root = jsonBuffer.parseObject(input);
    String  daytime =  root[String("sysTime2")];
    time0 = daytime;
    time1 = daytime.substring(0, 10);//2018-06-17
    time2 =  daytime.substring(11, 19);//2018-06-17
  }
  http.end();
}
/*************************获取每日英语*******************/
void get_english()
{
  HTTPClient http;
  http.begin(english_url);
  int httpCode = http.GET();
  if (httpCode > 0) {
    payload = http.getString();
    //Serial.println(payload);
    /*******json 数据处理******/
    DynamicJsonBuffer jsonBuffer;
    String  input =   payload;
    JsonObject& root = jsonBuffer.parseObject(input);
    String  content =  root[String("content")];
    english = content;
    //Serial.println(english);
  }
  http.end();
}
/**************************************天气判断**************************************************/
void tianqi()
{
  switch (state1.toInt()) {
    case 0: Serial.print("PIC(43,11,1);"); Serial.print("DS12(10,20,'"); Serial.print(tq[0]); Serial.println("',4,0);"); break; //晴天
    case 1: Serial.print("PIC(43,11,2);"); Serial.print("DS12(10,20,'"); Serial.print(tq[1]); Serial.println("',4,0);"); break;//多云
    case 2: Serial.print("PIC(43,11,3);"); Serial.print("DS12(10,20,'"); Serial.print(tq[2]); Serial.println("',4,0);"); break;//阴天
    case 3: Serial.print("PIC(43,11,4);"); Serial.print("DS12(10,20,'"); Serial.print(tq[4]); Serial.println("',4,0);"); break;//阵雨
    case 4: Serial.print("PIC(43,11,4);"); Serial.print("DS12(10,20,'"); Serial.print(tq[5]); Serial.println("',4,0);"); break;//雷阵雨
    case 7: Serial.print("PIC(43,11,4);"); Serial.print("DS12(10,20,'"); Serial.print(tq[3]); Serial.println("',4,0);"); break;//小雨
    case 8: Serial.print("PIC(43,11,4);"); Serial.print("DS12(10,20,'"); Serial.print(tq[6]); Serial.println("',4,0);"); break;//中雨
    case 9: Serial.print("PIC(43,11,4);"); Serial.print("DS12(10,20,'"); Serial.print(tq[7]); Serial.println("',4,0);"); break;//大雨
    case 5: Serial.print("PIC(43,11,5);"); Serial.print("DS12(10,20,'"); Serial.print(tq[8]); Serial.println("',4,0);"); break;//雪天
    case 6: Serial.print("PIC(43,11,5);"); Serial.print("DS12(10,20,'"); Serial.print(tq[8]); Serial.println("',4,0);"); break;//雪天
  }
  delay(200);
}
/*************************************截取字符数据处理*******************************************/
void data() {
  int i = 0;
  i = com.indexOf("tem1");
  tem1 = com.substring(i + 6, i + 9);
  i = 0;
  i = com.indexOf("tem2");  //tem2="24"
  tem2 = com.substring(i + 6, i + 9);
  i = 0;
  i = com.indexOf("temNow");
  temnow = com.substring(i + 8, i + 11);
  i = 0;
  i = com.indexOf("humidity"); // humidity="88%" time="22:00"
  humi = com.substring(i + 10, i + 13); //humidity="27%"
  i = 0;
  i = com.indexOf("state1");
  state1 = com.substring(i + 8, i + 9);
  i = 0;
  i = com.indexOf("state2");
  state2 = com.substring(i + 7, i + 11);
  i = 0;
  com = "";
}
/*********************************************屏幕显示**************************************************/
void show()
{
  Serial.print("DS12(7,60,'"); Serial.print(hz[6]); Serial.print(humi); Serial.print("',13);");
  Serial.print("DS12(7,75,'"); Serial.print(hz[3]); Serial.print(tem1); Serial.print("c',3);"); //DS12(7,65,'最高温度:',1);
  Serial.print("DS12(7,90,'"); Serial.print(hz[4]); Serial.print(temnow); Serial.print("c',3);"); //DS12(7,85,'当前温度：',3);
  Serial.print("DS12(7,105,'"); Serial.print(hz[5]); Serial.print(tem2); Serial.print("c',3);"); //DS12(7,105,'最低温度：',13);
  Serial.print("DS12(100,109,'"); Serial.print(time0); Serial.println("',1);"); //BS12(8,140,216,4,'日期时间',1);
  delay(200);
  Serial.print("BS12(8,130,210,2,'"); Serial.print("  "); Serial.println("',13);");//局部刷新--原创000
  //经过我的研究发现英文中不能出现“'”这个符号，例如：that's就不行，所以将其替换为“.”
  english.replace("'", ".");
  Serial.print("BS12(8,130,210,1,'"); Serial.print(english); Serial.println("',13);");
  delay(300);
}
void loop()
{
  digitalWrite(LED, 1);
  Serial.println("PIC(141,53,10);");
  get_time();
  get_english();
  http();
  Serial.println("PIC(135,48,11);");
  data();
  Serial.println("PIC(132,45,12);");
  show();
  tianqi();
  digitalWrite(LED, 0);
  Serial.println("PIC(125,40,13);");
  delay(1000);
}
