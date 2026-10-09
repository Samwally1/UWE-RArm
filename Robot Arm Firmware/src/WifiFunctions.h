#include <Arduino.h>
#include <WiFi.h>
#include <ESP32Ping.h>
#include <HTTPClient.h>
#include <WiFiClientSecure.h>
#include <mbedtls/ssl_ciphersuites.h>

#include "Secrets.h"

static const char CONTROLLER_CA_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIICnzCCAiWgAwIBAgIQf/MZd5csIkp2FV0TttaF4zAKBggqhkjOPQQDAzBHMQsw
CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU
MBIGA1UEAxMLR1RTIFJvb3QgUjQwHhcNMjMxMjEzMDkwMDAwWhcNMjkwMjIwMTQw
MDAwWjA7MQswCQYDVQQGEwJVUzEeMBwGA1UEChMVR29vZ2xlIFRydXN0IFNlcnZp
Y2VzMQwwCgYDVQQDEwNXRTEwWTATBgcqhkjOPQIBBggqhkjOPQMBBwNCAARvzTr+
Z1dHTCEDhUDCR127WEcPQMFcF4XGGTfn1XzthkubgdnXGhOlCgP4mMTG6J7/EFmP
LCaY9eYmJbsPAvpWo4H+MIH7MA4GA1UdDwEB/wQEAwIBhjAdBgNVHSUEFjAUBggr
BgEFBQcDAQYIKwYBBQUHAwIwEgYDVR0TAQH/BAgwBgEB/wIBADAdBgNVHQ4EFgQU
kHeSNWfE/6jMqeZ72YB5e8yT+TgwHwYDVR0jBBgwFoAUgEzW63T/STaj1dj8tT7F
avCUHYwwNAYIKwYBBQUHAQEEKDAmMCQGCCsGAQUFBzAChhhodHRwOi8vaS5wa2ku
Z29vZy9yNC5jcnQwKwYDVR0fBCQwIjAgoB6gHIYaaHR0cDovL2MucGtpLmdvb2cv
ci9yNC5jcmwwEwYDVR0gBAwwCjAIBgZngQwBAgEwCgYIKwYBBAHWeQIEAgSB9QSB
8gDwAHUAlE5Dh/rswe+B8xkkJqgYZQHH0184AgE/cmd9VTcuGdgAAAGgROwrSwAA
BAMARjBEAiB+oOGCfXpbrUwp+82zIXw6IolL/vFlf+K5l9pAEBUDaQIgIn9utkV6
L/e7Aus3PSoqSTKxoSWxaIM0THP2xS3bexoAdwDYCVU7lE96/8gWGW+UT4WrsPj8
XodVJg8V0S5yu0VLFAAAAaBE7Ct6AAAEAwBIMEYCIQDAa+WtSud+09AOlye/77Kf
lxiOaDdw87f5H9X3xgHdegIhAKbZapaEHD0X0g78U3qBuonxc8WeJUMQgri6XDl3
USE8MAoGCCqGSM49BAMCA0gAMEUCIFistNUpLe4huh6gUggSzLU5APrccV4yCkCd
QX5msi6hAiEA8FyICGvvzgWNqEYQMx3YdkDp/hpx/hZEFvdHtmJRjd4=
-----END CERTIFICATE-----
)EOF";

static const char CONTROLLER_RSA_CA_CERT[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFVzCCAz+gAwIBAgINAgPlk28xsBNJiGuiFzANBgkqhkiG9w0BAQwFADBHMQsw
CQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZpY2VzIExMQzEU
MBIGA1UEAxMLR1RTIFJvb3QgUjEwHhcNMTYwNjIyMDAwMDAwWhcNMzYwNjIyMDAw
MDAwWjBHMQswCQYDVQQGEwJVUzEiMCAGA1UEChMZR29vZ2xlIFRydXN0IFNlcnZp
Y2VzIExMQzEUMBIGA1UEAxMLR1RTIFJvb3QgUjEwggIiMA0GCSqGSIb3DQEBAQUA
A4ICDwAwggIKAoICAQC2EQKLHuOhd5s73L+UPreVp0A8of2C+X0yBoJx9vaMf/vo
27xqLpeXo4xL+Sv2sfnOhB2x+cWX3u+58qPpvBKJXqeqUqv4IyfLpLGcY9vXmX7w
Cl7raKb0xlpHDU0QM+NOsROjyBhsS+z8CZDfnWQpJSMHobTSPS5g4M/SCYe7zUjw
TcLCeoiKu7rPWRnWr4+wB7CeMfGCwcDfLqZtbBkOtdh+JhpFAz2weaSUKK0Pfybl
qAj+lug8aJRT7oM6iCsVlgmy4HqMLnXWnOunVmSPlk9orj2XwoSPwLxAwAtcvfaH
szVsrBhQf4TgTM2S0yDpM7xSma8ytSmzJSq0SPly4cpk9+aCEI3oncKKiPo4Zor8
Y/kB+Xj9e1x3+naH+uzfsQ55lVe0vSbv1gHR6xYKu44LtcXFilWr06zqkUspzBmk
MiVOKvFlRNACzqrOSbTqn3yDsEB750Orp2yjj32JgfpMpf/VjsPOS+C12LOORc92
wO1AK/1TD7Cn1TsNsYqiA94xrcx36m97PtbfkSIS5r762DL8EGMUUXLeXdYWk70p
aDPvOmbsB4om3xPXV2V4J95eSRQAogB/mqghtqmxlbCluQ0WEdrHbEg8QOB+DVrN
VjzRlwW5y0vtOUucxD/SVRNuJLDWcfr0wbrM7Rv1/oFB2ACYPTrIrnqYNxgFlQID
AQABo0IwQDAOBgNVHQ8BAf8EBAMCAYYwDwYDVR0TAQH/BAUwAwEB/zAdBgNVHQ4E
FgQU5K8rJnEaK0gnhS9SZizv8IkTcT4wDQYJKoZIhvcNAQEMBQADggIBAJ+qQibb
C5u+/x6Wki4+omVKapi6Ist9wTrYggoGxval3sBOh2Z5ofmmWJyq+bXmYOfg6LEe
QkEzCzc9zolwFcq1JKjPa7XSQCGYzyI0zzvFIoTgxQ6KfF2I5DUkzps+GlQebtuy
h6f88/qBVRRiClmpIgUxPoLW7ttXNLwzldMXG+gnoot7TiYaelpkttGsN/H9oPM4
7HLwEXWdyzRSjeZ2axfG34arJ45JK3VmgRAhpuo+9K4l/3wV3s6MJT/KYnAK9y8J
ZgfIPxz88NtFMN9iiMG1D53Dn0reWVlHxYciNuaCp+0KueIHoI17eko8cdLiA6Ef
MgfdG+RCzgwARWGAtQsgWSl4vflVy2PFPEz0tv/bal8xa5meLMFrUKTX5hgUvYU/
Z6tGn6D/Qqc6f1zLXbBwHSs09dR2CQzreExZBfMzQsNhFRAbd03OIozUhfJFfbdT
6u9AWpQKXCBfTkBdYiJ23//OYb2MI3jSNwLgjt7RETeJ9r/tSQdirpLsQBqvFAnZ
0E6yove+7u7Y/9waLd64NnHi/Hm3lCXRSHNboTXns5lndcEZOitHTtNCjv0xyBZm
2tIMPNuzjsmhDYAPexZ3FL//2wmUspO8IFgV6dtxQ/PeEMMA3KgqlbbC1j+Qa3bb
bP6MvPJwNQzcmRk13NfIRmPVNnGuV/u3gm3c
-----END CERTIFICATE-----
)EOF";

static const uint16_t CONTROLLER_CIPHER_SUITES[] = {
    MBEDTLS_TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256,
    0
};

bool ConnectToWiFi() {

    // Print a message to the Serial Monitor.
    Serial.println("Connecting to WiFi...");

    // Set WiFi to station mode so the board connects to a router.
    WiFi.mode(WIFI_STA);

    // Start connecting using the WiFi name and password.
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // Record the start time in milliseconds to check for a timeout.
    unsigned long StartTime = millis();

    // Wait until the WiFi connection is established.
    while (WiFi.status() != WL_CONNECTED) {

        // Stop trying after 15 seconds and report failure.
        if (millis() - StartTime >= 15000) {
            Serial.println("\nWiFi connection failed!");
            return false;
        }

        // Wait half a second between connection checks.
        delay(500);

        // Print a dot to show that connection is still in progress.
        Serial.print(".");
    }

    // Report a successful connection.
    Serial.println("\nWiFi connected!");

    // Print the local IP address assigned to the board.
    Serial.print("IP Address: ");
    Serial.println(WiFi.localIP());

    // Return true to indicate a successful connection.
    return true;
}

void TestLatency()
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return;
    }

    String pingHost = String(HOST);
    pingHost.replace("https://", "");
    pingHost.replace("http://", "");
    const int pathStart = pingHost.indexOf('/');
    if (pathStart >= 0) {
        pingHost.remove(pathStart);
    }

    // Ping only the host name; Ping cannot resolve a URL with a protocol or path.
    if (Ping.ping(pingHost.c_str(), 3)) {
        Serial.printf("Average latency: %.2f ms\n", Ping.averageTime());
    } else {
        Serial.println("Ping failed");
    }
}

bool CheckControllerMode()
{
    if (WiFi.status() != WL_CONNECTED) {
        Serial.println("WiFi not connected");
        return false;
    }

    WiFiClientSecure client;
    client.setCACert(CONTROLLER_RSA_CA_CERT);
    client.setCipherSuites(CONTROLLER_CIPHER_SUITES);

    HTTPClient http;
    const String controllerUrl = "https://" + String(HOST) + "/mode";
    if (!http.begin(client, controllerUrl)) {
        Serial.println("Failed to initialise request");
        return false;
    }

    const char* headerKeys[] = {"mode"};
    http.collectHeaders(headerKeys, 1);
    int statusCode = http.GET();
    bool isTrainingMode = false;

    if (statusCode == HTTP_CODE_OK) {
        const String mode = http.header("mode");
        Serial.printf("Controller mode: %s\n", mode.c_str());
        isTrainingMode = mode == "Training";
    } else if (statusCode > 0) {
        Serial.printf("HTTP error: %d\n", statusCode);
    } else {
        Serial.printf("GET failed: %s\n",
                      HTTPClient::errorToString(statusCode).c_str());
    }

    http.end();
    return isTrainingMode;
}
