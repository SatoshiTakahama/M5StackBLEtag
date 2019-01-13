#include <M5Stack.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEServer.h>

#define BLE_DEVICE_NAME     "M5StackTag"
//LINK LOSS SERVICE
#define SERVICE_UUID        "1803"
//Alert Level
#define CHARACTERISTIC_UUID "2a06"

#define NOTE_DH2 661

BLEServer *pServer = NULL;
bool deviceConnected = false;
bool oldDeviceConnected = false;
uint8_t alertLevel = 0;

class MyServerCallbacks: public BLEServerCallbacks {
    void onConnect(BLEServer* pServer) {
      deviceConnected = true;
    };

    void onDisconnect(BLEServer* pServer) {
      deviceConnected = false;
    }
};

class MyCallbacks: public BLECharacteristicCallbacks {
    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string value = pCharacteristic->getValue();

      if (value[0]==1 || value[0]==2) {
        alertLevel = value[0];
      }
      else {
        alertLevel = 0;
      }
      M5.Lcd.printf("Set alertLevel=%d\n", value[0]);
    }
};

void setupBLE() {
  BLEDevice::init(BLE_DEVICE_NAME);
  pServer = BLEDevice::createServer();
  pServer->setCallbacks(new MyServerCallbacks());

  BLEService *pService = pServer->createService(SERVICE_UUID);

  BLECharacteristic *pCharacteristic = pService->createCharacteristic(
                                         CHARACTERISTIC_UUID,
                                         BLECharacteristic::PROPERTY_READ |
                                         BLECharacteristic::PROPERTY_WRITE
                                       );

  pCharacteristic->setCallbacks(new MyCallbacks());

  pCharacteristic->setValue((uint8_t*)&alertLevel, 1);

  // Start the service
  pService->start();

  // Start advertising
  pServer->getAdvertising()->start();
}

void setup()
{
  M5.begin();
  setupBLE();
  M5.Lcd.setBrightness(30);
  M5.Lcd.clear();
  M5.Lcd.println("waiting connect");
}

void loop()
{
  M5.update();
  if (M5.BtnA.wasPressed())
  {
    M5.Lcd.println("BtnA Pressed");
    M5.Speaker.mute();
  }
  if (M5.BtnB.wasPressed())
  {
    M5.Lcd.println("BtnB Pressed");
    M5.Speaker.tone(NOTE_DH2);
  }
  if (M5.BtnC.wasPressed())
  {
    M5.Lcd.println("BtnC Pressed");
    M5.Speaker.mute();
  }

  // disconnecting
  if (!deviceConnected && oldDeviceConnected)
  {
    delay(500); // give the bluetooth stack the chance to get things ready
    pServer->startAdvertising(); // restart advertising
    oldDeviceConnected = deviceConnected;
    if (alertLevel)
    {
      //start alert
      M5.Lcd.println("start alert");
      M5.Speaker.tone(NOTE_DH2);
    }
    else
    {
      // waiting connect
      M5.Lcd.println("waiting connect");
    }
  }
  // connecting
  if (deviceConnected && !oldDeviceConnected)
  {
  // do stuff here on connecting
    oldDeviceConnected = deviceConnected;
    if (alertLevel)
    {
      //stop alert
      M5.Lcd.println("stop alert");
      M5.Speaker.mute();
    }
    // waiting disconnect
    M5.Lcd.println("waiting disconnect");
  }
}