/**
* Esse script mostra como usar o rtc ds3231
* e fazer sua atualização utilizando o NTP
*/

#include <Arduino.h>
#include <Wire.h>
#include <WiFi.h>
#include <NTPClient.h> //disponivel em "https://github.com/arduino-libraries/NTPClient"
#include <WiFiUdp.h>
#include <RTClib.h>  //https://github.com/adafruit/RTClib



//objetos do rtc
RTC_DS3231 relogio;
DateTime tempoAtual;

WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "0.br.pool.ntp.org", -3 * 3600, 60000); //pode-se alterar o servidor ntp

void atualizarRelogio();

bool eduroam = false; //true se a rede for eduroam
String wifiSsid = "";
String wifiSenha = "";
String eduroanLogin = "";
String eduroanSenha = "";

unsigned long intervaloRelogio = 0;//ajusta o intervalo entre as consultas do rtc

void setup() {
    Serial.begin(9600);
    Wire.begin(21,22);
    if(!relogio.begin()) Serial.print("RTC FALHOU!");

    aturalizarRelogio();
}

void loop() {
    //Cria uma varial que gerencia o controle de tempo no loop
    unsigned long currentMillis = millis();
    //Sincrociza a varialvel tempoAtual com o modulo rtc a cada 1 segundo
    if(currentMillis - intervaloRelogio >= 1000)
    {
        intervaloRelogio = currentMillis;
        tempoAtual = relogio.now();
        char texto[20] = "hh:mm:ss\nDD/MM/YYYY";
        //emprime na tela o horario e data
        Serial.print(tempoAtual.toString(texto));
    }
}
void aturalizarRelogio()
{
            WiFi.disconnect(true);  //disconnect form wifi to set new wifi connection
            WiFi.mode(WIFI_STA); //init wifi mode

            if(eduroam)
            {
                //Redes Eduroam
                esp_wifi_sta_wpa2_ent_set_identity((uint8_t *)configuracao.eduroanLogin, strlen(eduroanLogin)); //provide identity
                esp_wifi_sta_wpa2_ent_set_username((uint8_t *)configuracao.eduroanLogin, strlen(eduroanLogin)); //provide username --> identity and username is same
                esp_wifi_sta_wpa2_ent_set_password((uint8_t *)configuracao.eduroanSenha, strlen(eduroanSenha)); //provide password
                esp_wpa2_config_t wifiMode = WPA2_CONFIG_INIT_DEFAULT();
                esp_wifi_sta_wpa2_ent_enable(&wifiMode);
                WiFi.begin(wifiSsid);
                vTaskDelay(5000/ portTICK_PERIOD_MS);
            }else
            {   
                //Redes Normais
                WiFi.begin(wifiSsid, wifiSenha);
                vTaskDelay(5000/ portTICK_PERIOD_MS);
            }
            if (WiFi.status()==WL_CONNECTED)
            {
                Serial.println("Ok, conectado!");
	        Serial.print("Endereco IP: ");
                Serial.println(WiFi.localIP());
            }else
            {
                do
                {
                    WiFi.reconnect();
                    vTaskDelay(5000/ portTICK_PERIOD_MS);
                } while (WiFi.status()==WL_CONNECTED);   
            }

            if (WiFi.status()==WL_CONNECTED)
            {
                //Ajusta a hora do rtc pelo ntp
                timeClient.setTimeOffset(-3*3600);
                timeClient.begin();
                vTaskDelay(5000/ portTICK_PERIOD_MS);
                timeClient.update();

                int a, m, d, h, n, s;
                String FechaNTP = timeClient.getFormattedDate();                        
                a = FechaNTP.substring( 0, 4).toInt();
                m = FechaNTP.substring( 5, 7).toInt();
                d = FechaNTP.substring( 8, 10).toInt();
                h = FechaNTP.substring(11, 13).toInt();
                n = FechaNTP.substring(14, 16).toInt();
                s = FechaNTP.substring(17, 19).toInt();
                relogio.adjust(DateTime(a, m, d, h, n, s));
                   
                ESP.restart();
            }
}
