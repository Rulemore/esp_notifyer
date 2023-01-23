#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FastBot.h>

#define sensorPin D4

String ssid = "";
String pass = "";
String token = "";
String whiteList[10];
int whiteListed = 0;
bool isFull = false;

FastBot bot(token);

void newMsg(FB_msg& msg);
bool inWiteList(String id);
void addToWhiteList(String id);
void delFromWhiteList(String id);
void sendNotification();
void checkStatus(String id);

void setup() {
  Serial.begin(115200);
  pinMode(sensorPin, INPUT);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
  bot.notify(true);
  bot.attach(newMsg);
}

void newMsg(FB_msg& msg) {
  Serial.println(msg.toString());
  if (msg.text == "Подключить колодец") {
    addToWhiteList(msg.chatID);
  } else if (msg.text == "Отключить колодец") {
    delFromWhiteList(msg.chatID);
  } else if (msg.text == "Колодец") {
    checkStatus(msg.chatID);
  }
}

bool inWiteList(String id) {
  for (int i = 0; i < 10; i++) {
    if (whiteList[i] == id) {
      return true;
    }
  }
  return false;
}

void addToWhiteList(String id) {
  if (inWiteList(id)) {
    bot.setChatID(id);
    bot.sendMessage("Вы уже подключены к уведомлениям");
    return;
  }
  whiteList[whiteListed] = id;
  whiteListed++;
  bot.setChatID(id);
  bot.sendMessage("Вы подключены к уведомлениям");
}

void delFromWhiteList(String id) {
  if (!inWiteList(id)) {
    bot.setChatID(id);
    bot.sendMessage("Вы не подключены к уведомлениям");
    return;
  }
  for (int i = 0; i < whiteListed; i++) {
    if (whiteList[i] == id) {
      whiteList[i] = "";
      whiteListed--;
      bot.setChatID(id);
      bot.sendMessage("Вы отключены от уведомлений");
    }
  }
  String temp[10];
  for (int i = 0; i < 10; i++) {
    if (whiteList[i] != "") {
      temp[i] = whiteList[i];
    }
  }
  for (int i = 0; i < 10; i++) {
    whiteList[i] = temp[i];
  }
}

void sendNotification() {
  for (int i = 0; i < whiteListed; i++) {
    bot.setChatID(whiteList[i]);
    bot.sendMessage("Колодец переполнен");
  }
}

void checkStatus(String id) {
  bot.setChatID(id);
  if (isFull) {
    bot.sendMessage("Колодец переполнен");
  } else {
    bot.sendMessage("Колодец не переполнен");
  }
}

void smartdelay(unsigned long ms) {
  unsigned long start = millis();
  while (millis() - start < ms) {
    bot.tick();
  };
}

void loop() {
  bot.tick();
  if (digitalRead(sensorPin) == 0 && !isFull) {
    sendNotification();
    isFull = true;
    smartdelay(500);
  }
  if (digitalRead(sensorPin) == 1 && isFull) {
    isFull = false;
  }
}