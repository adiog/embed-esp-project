#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>

const char* ssid = "Kopernika32";
const char* password = "zosienka";

ESP8266WebServer server(80);

const int led = 13;

void handleRoot()
{
    digitalWrite(led, 1);
    server.send(200, "text/plain", "hello from esp8266!");
    digitalWrite(led, 0);
}

void rpcResponse(char * response)
{
    Serial.println(response);
    server.send(200, "text/plain", response);
}

void rpcPinMode(int pin, int value)
{
    char response[128];
    pinMode(pin, value);
    sprintf(response, "pinMode(%d, %s)\n", pin, (value == INPUT) ? "INPUT" : "OUTPUT");
    rpcResponse(response);
}

void rpcDigitalRead(int pin)
{
    char response[128];
    int value = digitalRead(pin);
    sprintf(response, "digitalRead(%d): %s\n", pin, (value == LOW) ? "LOW" : "HIGH");
    rpcResponse(response);
}

void rpcDigitalWrite(int pin, int value)
{
    char response[128];
    digitalWrite(pin, value);
    sprintf(response, "digitalWrite(%d, %s)\n", pin, (value == LOW) ? "LOW" : "HIGH");
    rpcResponse(response);
}

void handleNotFound()
{
    digitalWrite(led, 1);
    String message = "File Not Found\n\n";
    message += "URI: ";
    message += server.uri();
    message += "\nMethod: ";
    message += (server.method() == HTTP_GET) ? "GET" : "POST";
    message += "\nArguments: ";
    message += server.args();
    message += "\n";
    for (uint8_t i = 0; i < server.args(); i++)
    {
        message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
    }
    server.send(404, "text/plain", message);
    digitalWrite(led, 0);
}

void setup(void)
{
    pinMode(led, OUTPUT);
    digitalWrite(led, 0);
    Serial.begin(115200);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    Serial.println("");

    // Wait for connection
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.print("Connected to ");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    if (MDNS.begin("esp8266"))
    {
        Serial.println("MDNS responder started");
    }

    server.on("/", handleRoot);

    char url[128];
    for(int pin : {1, 2, 3, 4})
    {
        sprintf(url, "/api/pinMode/%d/INPUT", pin);
        server.on(url, [pin](){
            rpcPinMode(pin, INPUT);
        });
        sprintf(url, "/api/pinMode/%d/OUTPUT", pin);
        server.on(url, [pin](){
            rpcPinMode(pin, OUTPUT);
        });
        sprintf(url, "/api/digitalRead/%d", pin);
        server.on(url, [pin](){
            rpcDigitalRead(pin);
        });
        sprintf(url, "/api/digitalWrite/%d/LOW", pin);
        server.on(url, [pin](){
            rpcDigitalWrite(pin, LOW);
        });
        sprintf(url, "/api/digitalWrite/%d/HIGH", pin);
        server.on(url, [pin](){
            rpcDigitalWrite(pin, HIGH);
        });
    }

    server.onNotFound(handleNotFound);

    server.begin();
    Serial.println("HTTP server started");
    digitalWrite(led, HIGH);
}

void loop(void)
{
    server.handleClient();
}
