#include <Arduino.h>
#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <FastBot.h>

#define sensorPin D4

struct Users {
  int whiteListCount = 10;
  int whiteListed = 0;
  int whiteList[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
};

String ssid = "";
String pass = "";
String token = "";
bool isFull = false;
int adress = 0;

Users users;
FastBot bot(token);

void newMsg(FB_msg& msg);
bool inWiteList(String id);
void addToWhiteList(String id);
void delFromWhiteList(String id);
void sendNotification(String text);
void checkStatus(String id);

void setup() {
  EEPROM.begin(sizeof(users));
  EEPROM.get(adress, users);
  EEPROM.commit();
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
  sendNotification("Колодец перезагружен");
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
    if (users.whiteList[i] == id.toInt()) {
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
  if (users.whiteListed == users.whiteListCount) {
    bot.setChatID(id);
    bot.sendMessage("Список подключенных пользователей переполнен");
    return;
  }
  users.whiteList[users.whiteListed] = id.toInt();
  users.whiteListed++;
  bot.setChatID(id);
  bot.sendMessage("Вы подключены к уведомлениям");
  EEPROM.begin(sizeof(users));
  EEPROM.put(adress, users);
  EEPROM.commit();
}

void delFromWhiteList(String id) {
  if (!inWiteList(id)) {
    bot.setChatID(id);
    bot.sendMessage("Вы не подключены к уведомлениям");
    return;
  }
  for (int i = 0; i < users.whiteListed; i++) {
    if (users.whiteList[i] == id.toInt()) {
      users.whiteList[i] = 0;
      users.whiteListed--;
      bot.setChatID(id);
      bot.sendMessage("Вы отключены от уведомлений");
    }
  }
  int temp[10];
  for (int i = 0; i < 10; i++) {
    if (users.whiteList[i] != 0) {
      temp[i] = users.whiteList[i];
    }
  }
  for (int i = 0; i < 10; i++) {
    users.whiteList[i] = temp[i];
  }
  EEPROM.begin(sizeof(users));
  EEPROM.put(adress, users);
  EEPROM.commit();
}

void sendNotification(String text) {
  for (int i = 0; i < users.whiteListed; i++) {
    bot.setChatID(users.whiteList[i]);
    bot.sendMessage(text);
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
    sendNotification("Колодец переполнен");
    isFull = true;
    smartdelay(500);
  }
  if (digitalRead(sensorPin) == 1 && isFull) {
    isFull = false;
  }
}