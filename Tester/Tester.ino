#include <ArduinoJson.h>
#include <SoftwareSerial.h>

#define BT_SERIAL_TX 18
#define BT_SERIAL_RX 19

SoftwareSerial BluetoothSerial(BT_SERIAL_TX, BT_SERIAL_RX);

int pinArray[13] = { 2, 3, 4, 5, 6, 7, 8, 8, 9, 10, 11, 12, 13 };

String data;
String response;
int counter;
int maxValues;
int currentInput;

const int capacity = 6 * JSON_ARRAY_SIZE(2) + JSON_ARRAY_SIZE(6) + JSON_OBJECT_SIZE(3);

void setup() {

  Serial.begin(9600);
  Serial.println("Arduino is listening");
  BluetoothSerial.begin(9600);

}

void loop() {

  if (BluetoothSerial.available()) {
    String data = BluetoothSerial.readStringUntil("\n");
    Serial.println(data);
    DynamicJsonBuffer jsonBuffer(capacity);
    JsonObject& obj = jsonBuffer.parseObject(data);

    if (obj.success()) {
      response = "{\"status\":1,\"result\": [";
      obj.printTo(Serial);

      const int pinNo = obj["pinNo"];
      const char* pinConfig = obj["config"];

      if (pinNo == 14) {
        int testSize = obj["test"].size();

        for (int i = 0; i < testSize; i++) {
          int testCase = obj["test"][i].size();
          int values[testCase];

          for (int j = 0; j < testCase; j++) {
            values[j] = obj["test"][i][j].as<int>();
          }

          response += "[";
          executeTest(pinNo, pinConfig, pinArray, values, testCase);
          if (i < testSize - 1) {
            response += "], ";
          } else {
            response += "]";
          }
        }

        delay(1000);
        setBaseState(pinArray);
        response += "],\"message\":\"Prueba realizada con éxito.\"}";
        emit(response);
      }
    } else {
      response = "{\"status\":2,\"message\":\"Error en JSON.\"}";
      emit(response);
    }
  }
}

/**
   Función que ejecuta una prueba
*/
void executeTest(int pinNo, char* pinConfig, int pinArray[], int testValues[], int valuesSize) {

  counter = 0;
  currentInput = 0;
  maxValues = 0;

  for (int i = 0; i < pinNo; i++) {
    if (pinConfig[i] == 'I') {
      pinMode(pinArray[i], INPUT);
      Serial.print("Pin ["); Serial.print(pinArray[i]); Serial.println("] configurado como ENTRADA.");
      maxValues++;
      
    } else if (pinConfig[i] == 'O') {

      pinMode(pinArray[i], OUTPUT);

      if (counter < valuesSize) {
        Serial.print("Pin ["); Serial.print(pinArray[i]); Serial.print("] configurado como SALIDA. ");
        Serial.print("Valor de prueba: "); Serial.println(testValues[counter]);
        digitalWrite(pinArray[i], getLowOrHigh(testValues[counter]));
        counter++;
      } else {
        counter = 0;
        digitalWrite(pinArray[i], getLowOrHigh(testValues[counter]));
        Serial.print("Pin ["); Serial.print(pinArray[i]); Serial.print("] configurado como SALIDA. ");
        Serial.print("Valor de prueba: "); Serial.println(testValues[counter]);
        counter++;
      }
    }
  }

  for (int i = 0; i < pinNo; i++) {
    if (pinConfig[i] == 'I') {
      currentInput++;
      Serial.print("Valor leído en Pin ["); Serial.print(pinArray[i]); Serial.print("] es: ");
      Serial.println(digitalRead(pinArray[i]));
      response += digitalRead(pinArray[i]);
      if (currentInput < maxValues) {
        response += ",";
      }
      // Serial.println("Así vá la respuesta: " + response); // testing
    }
  }
}

/**
   Función que ejecuta una prueba de tipo simple
**/
void simpleTest(int pinNo, char* pinConfig, int pinArray[], uint8_t testValue) {
  for (int i = 0; i < pinNo; i++) {
    if (pinConfig[i] == 'I') {
      pinMode(pinArray[i], INPUT);
      Serial.print("Pin ["); Serial.print(pinArray[i]); Serial.println("] configurado como ENTRADA.");
    } else if (pinConfig[i] == 'O') {
      pinMode(pinArray[i], OUTPUT);
      digitalWrite(pinArray[i], testValue);
      Serial.print("Pin ["); Serial.print(pinArray[i]); Serial.print("] configurado como SALIDA. ");
      Serial.print("Valor de prueba: "); Serial.println(testValue);
    }
  }

  for (int i = 0; i < pinNo; i++) {
    if (pinConfig[i] == 'I') {
      Serial.print("Valor leído en Pin ["); Serial.print(pinArray[i]); Serial.print("] es: ");
      //delay(1000);
      Serial.println(digitalRead(pinArray[i]));
    }
  }
}

/*
  Función que retorna un entero sin signo de 1 byte, LOW o HIGH
*/
uint8_t getLowOrHigh(int value) {
  if (value == 1) {
    return HIGH;
  } else if (value == 0) {
    return LOW;
  }
}

/*
   Función para setear el estado de las entradas/salidas a default una vez terminado el batch de pruebas
*/
void setBaseState(int pinArray[]) {
  for (int i = 0; i < sizeof(pinArray); i++) {
    pinMode(pinArray[i], INPUT);
  }
}

/*
   Función para enviar un mensaje a través del Bluetooth
*/
void emit(String message) {
  Serial.println(message);
  BluetoothSerial.print(message + "\n");
}

/*
  Función para limpiar la variable global de respuesta al finalizar el ciclo
*/
void clearMessage(String response) {
  response = "";
}
