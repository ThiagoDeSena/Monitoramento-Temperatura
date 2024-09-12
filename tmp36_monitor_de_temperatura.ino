#ifdef __cplusplus
extern "C" {
#endif
uint8_t temprature_sens_read();
#ifdef __cplusplus
}
#endif
uint8_t temprature_sens_read();

const int TMP36 = 36; //Pino D36
float temperatura=0.0,ultimaTemperatura=0.0;

int intervalSensor = 2000;
long prevMillisThingSpeak = 0;
int intervalThingSpeak = 30000; // Intervalo minímo para escrever no ThingSpeak write é de 15 segundos

#include <WiFi.h>
const char ssid[] = "ifce-espacoMaker";
const char password[] = "CR1AT1V1UM";
WiFiClient client;

#include <ThingSpeak.h>
const long CHANNEL = 2596090;
const char *WRITE_API = "ZCFA5K4I5U1XJ286";
const int relePin = 25; // pino ao qual o Módulo Relé está conectado

void conectarWiFi();
float temperaturaMedia();
void escreverNoThingSpeak(float temperaturaMedia);

void setup()
{
  pinMode(relePin, OUTPUT); // seta o pino como saída
  digitalWrite(relePin, LOW); // seta o pino com nivel logico ALTO
  Serial.begin(115200);
  Serial.println();
  Serial.println("Envia os dados do sensor para o ThingSpeak usando o ESP32");
  Serial.println();

  WiFi.mode(WIFI_STA); //Modo Station
  ThingSpeak.begin(client);  // Inicializa o ThingSpeak
}

void loop(){

  conectarWiFi();

  float temperaturaMedia = mediaDaTemperatura();

  compararTemperatura(temperaturaMedia);

}

void conectarWiFi(){
  // Conecta ou reconecta o WiFi
  if (WiFi.status() != WL_CONNECTED) {
    Serial.print("Atenção para conectar o SSID: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED) {
      WiFi.begin(ssid, password);
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConectado");
  }
}

float mediaDaTemperatura(){
  int numeroLeituras = 10;
  float somaTemperatura=0;

  for(int i=0;i<numeroLeituras;i++){
    int ValorLido = analogRead(TMP36);
    temperatura = (ValorLido / 1023.0);
    temperatura = (temperatura-0.5) * 100; // temperatura em Celsius
    somaTemperatura += temperatura;
    delay(100);
  }

  float temperaturaMedia = somaTemperatura/numeroLeituras;
  
  if(temperaturaMedia>25){
    escreverNoThingSpeak(temperaturaMedia);
    digitalWrite(relePin, LOW);
  }else{
    digitalWrite(relePin, HIGH);
  }

  return temperaturaMedia;
}

void compararTemperatura(float temperaturaMedia){

  Serial.println("Temperatura");
  Serial.println(temperaturaMedia);
  Serial.println("°C");
  delay(2000);

  if(ultimaTemperatura!=temperaturaMedia && (temperaturaMedia<ultimaTemperatura*0.97 || temperaturaMedia>ultimaTemperatura*1.03)){
    if(ultimaTemperatura==0.0 || (temperaturaMedia<ultimaTemperatura*1.1 && temperaturaMedia>ultimaTemperatura*0.9)){
      escreverNoThingSpeak(temperaturaMedia);
    }
    
    ultimaTemperatura = temperaturaMedia;

  }else if(millis() - prevMillisThingSpeak > intervalThingSpeak){
    if(ultimaTemperatura==0.0 || (temperaturaMedia<ultimaTemperatura*1.1 && temperaturaMedia>ultimaTemperatura*0.9)){
      escreverNoThingSpeak(temperaturaMedia);
    }
  }
}

void escreverNoThingSpeak(float temperaturaMedia){
  //if (millis() - prevMillisThingSpeak > intervalThingSpeak) {

    char temperaturaFormatada[5];
    sprintf(temperaturaFormatada, "%.2f", temperaturaMedia);

    // Configura os campos com os valores
    ThingSpeak.setField(1,temperaturaFormatada);
    


    // Escreve no canal do ThingSpeak 
    int x = ThingSpeak.writeFields(CHANNEL, WRITE_API);
    if (x == 200) {
      Serial.println("Update realizado com sucesso");
    }
    else {
      Serial.println("Problema no canal - erro HTTP " + String(x));
    }

    prevMillisThingSpeak = millis();
  //}
}
