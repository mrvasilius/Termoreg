#define VERSION "1.0.0"

// mkspiffs -c data fs.bin
//Файл для загрузки должен иметь расширение *.bin
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPUpdateServer.h>
#define OTAUSER "admin"         // Логин для входа в OTA
#define OTAPASSWORD "987654123" // Пароль для входа в ОТА
#define OTAPATH "/firmware"     // Путь, который будем дописывать после ip адреса в браузере.
#define SERVERPORT 80           // Порт для входа, он стандартный 80 это порт http

#include <FS.h>

#include <PubSubClient.h>

#include <Wire.h>
#include <OneWire.h>

#define REQUIRESALARMS false
#include <DallasTemperature.h>

#include "GyverTimer.h"

#define RELAY_BUS D0
#include "RelayControl.h"

#define MIN_TEMP 0
#define MAX_TEMP 30
#define DISP_CLK D1
#define DISP_DIO D3
#define ENC_S1 D6
#define ENC_S2 D5
#define ENC_KEY D4
#include "ModeControl.h"

#define ONE_WIRE_BUS D2

const char ssid[] = "CoDeHW";   //  SSID (название) вашей сети
const char pass[] = "fgtkmcby"; // пароль к вашей сети

const char mqttUser[] = "home";
const char mqttPass[] = "987654123";

const char *envFileName = "/room.env";

// timers
GTimer_ms tempTimer(60000);
GTimer_ms sendTimer(1000);
GTimer_ms connectTimer(1);

OneWire sensDs(ONE_WIRE_BUS);
DallasTemperature ds(&sensDs);

// MQTT Client
WiFiClient wclient;
IPAddress server(192, 168, 18, 18);
int port = 1883;
void receivedCallback(char *topic, byte *payload, unsigned int length);
PubSubClient client(server, port, receivedCallback, wclient);

// HTTP Server
bool isHTTPServerStared = false;
ESP8266WebServer HttpServer(SERVERPORT);
ESP8266HTTPUpdateServer httpUpdater;

float temperature;           // фактическая температура
int theTemp = 20;            // установленная температура
int qty_sens;                // количество сенсоров
int actualSens = 0;          // выбранный сенсор
String room;                 // комната
bool workStandalone = false; // работать автономно

void setup()
{
    // Serial.begin(115200);

    SPIFFS.begin();
    File f = SPIFFS.open(envFileName, "r");
    if (!f)
    {
        workStandalone = true;
        room = "test";
    }

    if (!workStandalone)
    {
        for (int i = 0; i < f.size(); i++)
        {
            room += (char)f.read();
        }
        f.close();
    }
    WiFi.hostname(String("ESP-") + room);
    WiFi.begin(ssid, pass);

    ds.begin();
    qty_sens = ds.getDeviceCount();
    ds.requestTemperatures();
    delay(1000);
    readTemp();
    ds.requestTemperatures();
    MC.init(RC, &theTemp, &temperature, &qty_sens, &actualSens, sendDto);

    HttpServer.on("/", showVervion);
}
void loop()
{
    if (WiFi.status() == WL_CONNECTED)
    {
        if (!isHTTPServerStared)
        {
            httpUpdater.setup(&HttpServer, OTAPATH, OTAUSER, OTAPASSWORD);
            HttpServer.begin();
            isHTTPServerStared = true;
        }

        HttpServer.handleClient();
        if (!workStandalone)
        {
            if (client.connected())
            {
                client.loop();
            }
            if (!client.connected() && connectTimer.isReady())
            {
                char roomBuf[room.length() + 1];
                room.toCharArray(roomBuf, room.length() + 1);
                client.connect(roomBuf, mqttUser, mqttPass);
                subscribe(String("termoreg/") + room + "/cmd/#");
                connectTimer.setInterval(60000);
            }
        }
    }

    if (tempTimer.isReady())
    {
        readTemp();
        RC.setRelay(theTemp, temperature);
        ds.requestTemperatures();
    }

    MC.loop();
}

void readTemp()
{
    temperature = ds.getTempCByIndex(actualSens);
    sendData();
}

void sendData()
{
    if (client.connected())
    {
        String switcher = String("switcher,room=") + room + ",heater=1 status=" + (int)RC.getStatus() + ",controlMode=" + RC.getControlMode();
        publish(String("influx/") + room + "/switcher", switcher);

        String setTemp = String("temperature,room=") + room + ",sens=set temperature=" + theTemp;
        publish(String("influx/") + room + "/setTemp", setTemp);

        for (int i = 0; i < qty_sens; i++)
        {
            String temperature = String("temperature,room=") + room + ",sens=" + i + " temperature=" + ds.getTempCByIndex(i);
            publish(String("influx/") + room + "/temperature" + i, temperature);
        }

        sendDto();
    }
}

void sendDto()
{
    if (sendTimer.isReady())
    {
        publish(String("termoreg/") + room + "/dto/temp", String(temperature));
        publish(String("termoreg/") + room + "/dto/setTemp", String(theTemp));
        publish(String("termoreg/") + room + "/dto/mode", String(RC.getControlMode()));
    }
}

void receivedCallback(char *topic, byte *payload, unsigned int length)
{
    if (!workStandalone)
    {
        if (String(topic) == String("termoreg/") + room + "/cmd/setTemp")
        {
            MC.setMode(ModeControl::eMode::EDIT_TEMP_MODE);
            int temp = atoi((char *)payload);
            if (temp > MAX_TEMP)
                temp = MAX_TEMP;
            if (temp < MIN_TEMP)
                temp = MIN_TEMP;
            theTemp = temp;
            RC.setControlMode(RelayControl::cMode::C_AUTO);
        }
        if (String(topic) == String("termoreg/") + room + "/cmd/mode")
        {
            MC.setMode(ModeControl::eMode::EDIT_CONTROL_MODE);
            RC.setControlMode((RelayControl::cMode)atoi((char *)payload));
        }
        sendDto();
    }
}

boolean subscribe(String topic)
{
    char topicBuf[topic.length() + 1];
    topic.toCharArray(topicBuf, topic.length() + 1);
    return client.subscribe(topicBuf);
}

boolean publish(String topic, String payload)
{
    if (client.connected())
    {
        char topicBuf[topic.length() + 1];
        char payloadBuf[payload.length() + 1];
        topic.toCharArray(topicBuf, topic.length() + 1);
        payload.toCharArray(payloadBuf, payload.length() + 1);
        return client.publish(topicBuf, payloadBuf, true);
    }
}

void showVervion()
{
    String ptr = "<!DOCTYPE html> <html>\n";
    ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
    ptr += "<meta charset=\"utf-8\">";
    ptr += "<title>ESP8266-";
    ptr += room;
    ptr += "</title>\n";
    ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
    ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
    ptr += "p {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
    ptr += "</style>\n";
    ptr += "</head>\n";
    ptr += "<body>\n";
    ptr += "<h1>Room: ";
    ptr += room;
    ptr += "</h1>\n";
    ptr += "<h3>Firmware version: ";
    ptr += VERSION;
    ptr += " <a href=\"/firmware\">обновить</a></h3>\n";
    HttpServer.send(200, "text/html", ptr);
}