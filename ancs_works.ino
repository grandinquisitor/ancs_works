// Import libraries (BLEPeripheral depends on SPI)
#define BLE_SERIAL_DEBUG

#include <SPI.h>
#include <BLEPeripheral.h>

#include <BLEUtil.h>

#include "pack_lib.h"
#include "common.h"
#include "utringbuffer.h"

// define pins (varies per shield/board)
static const uint8_t REQ_PIN = 9;
static const uint8_t RDY_PIN = 2;
static const uint8_t RST_PIN = 10;

// create peripheral instance, see pinouts above
BLEPeripheral                    blePeripheral                            = BLEPeripheral(REQ_PIN, RDY_PIN, RST_PIN);
BLEBondStore                     bleBondStore;

// remote services
BLERemoteService                 ancsService                              = BLERemoteService("7905f431b5ce4e99a40f4b1e122d00d0");

// remote characteristics
BLERemoteCharacteristic          ancsNotificationSourceCharacteristic     = BLERemoteCharacteristic("9fbf120d630142d98c5825e699a21dbd", BLENotify);
BLERemoteCharacteristic          ancsControlPointCharacteristic           = BLERemoteCharacteristic("69d1d8f345e149a898219bbdfdaad9d9", BLEWrite | BLEWriteWithoutResponse | BLENotify);
BLERemoteCharacteristic          ancsDataSourceCharacteristic             = BLERemoteCharacteristic("22eac6e924d64bb5be44b36ace7c7bfb", BLENotify);


BLERemoteService                 remoteGenericAttributeService            = BLERemoteService("1800");

BLERemoteCharacteristic          remoteDeviceNameCharacteristic           = BLERemoteCharacteristic("2a00", BLERead);
BLERemoteCharacteristic          remoteTimeCharacteristic           = BLERemoteCharacteristic("2a08", BLERead);


UT_icd our_icd = {sizeof(AncsNotification), NULL, NULL, NULL };
UT_ringbuffer *history;

AncsNotification* findNotification(unsigned long notificationUid) {
  for (AncsNotification* p = (AncsNotification*)utringbuffer_front(history);
       p != NULL;
       p = (AncsNotification*)utringbuffer_next(history, p)) {
    if (p->notificationUid == notificationUid) {
      return p;
    }
  }
  return NULL;
}

volatile boolean isNew = false;
unsigned char* currentTitle = NULL;
unsigned long currentUid = 0;
unsigned char* currentMessage = NULL;
unsigned char* currentApp = NULL;


void setup() {
  Serial.begin(115200);
#if defined (__AVR_ATmega32U4__) || defined(__MK20DX256__)|| defined(__MK20DX128__)
  while (!Serial);
#endif

//pinMode(RDY_PIN, INPUT);
//pinMode(REQ_PIN, OUTPUT);
//  pinMode(RST_PIN, OUTPUT);

  
  pinMode(20, OUTPUT);
  analogWrite(20, 40);

  utringbuffer_new(history, 10, &our_icd);

  // clears bond data on every boot
  bleBondStore.clearData();

  blePeripheral.setBondStore(bleBondStore);
  

  blePeripheral.setServiceSolicitationUuid(ancsService.uuid());
  blePeripheral.setLocalName("ANCS2");

  // set device name and appearance
  blePeripheral.setDeviceName("Arduino ANCS2");
  blePeripheral.setAppearance(0x0080);

  blePeripheral.addRemoteAttribute(ancsService);
  blePeripheral.addRemoteAttribute(ancsNotificationSourceCharacteristic);
  blePeripheral.addRemoteAttribute(ancsControlPointCharacteristic);
  blePeripheral.addRemoteAttribute(ancsDataSourceCharacteristic);

  blePeripheral.addRemoteAttribute(remoteGenericAttributeService);
  blePeripheral.addRemoteAttribute(remoteDeviceNameCharacteristic);
  blePeripheral.addRemoteAttribute(remoteTimeCharacteristic);

  // assign event handlers for connected, disconnected to peripheral
  blePeripheral.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  blePeripheral.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);
  blePeripheral.setEventHandler(BLEBonded, blePeripheralBondedHandler);
  blePeripheral.setEventHandler(BLERemoteServicesDiscovered, blePeripheralRemoteServicesDiscoveredHandler);


  // assign event handlers for characteristic
  ancsNotificationSourceCharacteristic.setEventHandler(BLEValueUpdated, ancsNotificationSourceCharacteristicValueUpdated);
  ancsDataSourceCharacteristic.setEventHandler(BLEValueUpdated, ancsDataSourceCharacteristicValueUpdated);
  ancsControlPointCharacteristic.setEventHandler(BLEValueUpdated, ancsControlPointCharacteristicValueUpdated);

  //
  // blePeripheral.setEventHandler(BLEValueUpdated, bleRemoteDeviceNameCharacteristicValueUpdatedHandle);
  remoteDeviceNameCharacteristic.setEventHandler(BLEValueUpdated, ancsControlPointCharacteristicValueUpdated);
  remoteTimeCharacteristic.setEventHandler(BLEValueUpdated, ancsControlPointCharacteristicValueUpdated);


  // begin initialization
  blePeripheral.begin();

  Serial.println(F("BLE Peripheral - ANCS"));
}

void loop() {
  static uint32_t screen_update_timer = 0;
    if ((millis() - screen_update_timer) > 10000) {
    Serial.println("still running");
    //digitalWrite(13, lastState = !lastState);
    //    update_lcd();
    screen_update_timer = millis();
    }

    
  blePeripheral.poll();

  if (isNew) {
    AncsNotification* notification = findNotification(currentUid);
    if (notification != NULL) {
      Serial.println(notification->catergoryId);
      Serial.println(((char*) currentApp));
      Serial.println(((char*) currentTitle));
      Serial.println(((char*) currentMessage));
    }
    isNew = false;
  }
}

void blePeripheralConnectHandler(BLECentral& central) {
  // central connected event handler
  Serial.print(F("Connected event, central: "));
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLECentral& central) {
  // central disconnected event handler
  Serial.print(F("Disconnected event, central: "));
  Serial.println(central.address());
}

void blePeripheralBondedHandler(BLECentral& central) {
  // central bonded event handler
  Serial.print(F("Remote bonded event, central: "));
  Serial.println(central.address());

  if (ancsNotificationSourceCharacteristic.canSubscribe()) {
    ancsNotificationSourceCharacteristic.subscribe();
  }
}

void blePeripheralRemoteServicesDiscoveredHandler(BLECentral& central) {
  // central remote services discovered event handler
  Serial.print(F("Remote services discovered event, central: "));
  Serial.println(central.address());

  // does this belong here? or in the bonded handler?? not sure what the difference is.
  if (ancsNotificationSourceCharacteristic.canSubscribe()) {
    Serial.println("notifications subscribed");
    ancsNotificationSourceCharacteristic.subscribe();
  }

  if (ancsDataSourceCharacteristic.canSubscribe()) {
    Serial.println("data subscribed");
    ancsDataSourceCharacteristic.subscribe();
  }

  if (ancsControlPointCharacteristic.canSubscribe()) {
    Serial.println("control subscribed");
    ancsControlPointCharacteristic.subscribe();
  }

  if (remoteDeviceNameCharacteristic.canRead()) {
    Serial.println("device name read");
    remoteDeviceNameCharacteristic.read();
  }

  if (remoteTimeCharacteristic.canRead()) {
    Serial.println("time  read");
    remoteTimeCharacteristic.read();
  }
}



void ancsNotificationSourceCharacteristicValueUpdated(BLECentral& central, BLERemoteCharacteristic& characteristic) {
  Serial.println(F("ANCS Notification Source Value Updated:"));
  struct AncsNotification notification;

  memcpy(&notification, characteristic.value(), sizeof(notification));

  Serial.print("\tEvent ID: ");
  Serial.println(notification.eventId);
  Serial.print("\tEvent Flags: 0x");
  Serial.println(notification.eventFlags, HEX);
  Serial.print("\tCategory ID: ");
  Serial.println(notification.catergoryId);
  Serial.print("\tCategory Count: ");
  Serial.println(notification.catergoryCount);
  Serial.print("\tNotification UID: ");
  Serial.println(notification.notificationUid);

  BLEUtil::printBuffer(characteristic.value(), characteristic.valueLength());

  // I HAVE NO IDEA IF THIS IS "THREAD" SAFE.
  // expects (const unsigned char value[], unsigned char length)
  /*
    static const uint8_t BUFFER_LEN = 13;
    static const char* BUFFER_FMT = "BIBBHBBH";
    uint8_t buffer[BUFFER_LEN];

    //    format specifiers:
    //        b   = int8_t    B   = uint8_t
    //        h   = int16_t   H   = uint16_t
    //        i   = int32_t   I   = uint32_t
    //        l   = int64_t   L   = uint64_t
    //        _   = skip one byte (mainly useful for unpacking)

    pack(
    buffer,
    BUFFER_FMT,
    ANCS_COMMAND_GET_NOTIF_ATTRIBUTES,
    notification.notificationUid,
    ANCS_NOTIFICATION_ATTRIBUTE_APP_IDENTIFIER,
    ANCS_NOTIFICATION_ATTRIBUTE_TITLE, TITLE_LENGTH,
    ANCS_NOTIFICATION_ATTRIBUTE_DATE,
    ANCS_NOTIFICATION_ATTRIBUTE_MESSAGE, MESSAGE_LENGTH
    );
  */

  static const uint8_t BUFFER_LEN = 8;
  static const char* BUFFER_FMT = "BIBH";
  uint8_t buffer[BUFFER_LEN];

  //    format specifiers:
  //        b   = int8_t    B   = uint8_t
  //        h   = int16_t   H   = uint16_t
  //        i   = int32_t   I   = uint32_t
  //        l   = int64_t   L   = uint64_t
  //        _   = skip one byte (mainly useful for unpacking)

  pack(
    buffer,
    BUFFER_FMT,
    ANCS_COMMAND_GET_NOTIF_ATTRIBUTES,
    notification.notificationUid,
    ANCS_NOTIFICATION_ATTRIBUTE_TITLE, 8
  );

  utringbuffer_push_back(history, &notification);

  BLEUtil::printBuffer(buffer, BUFFER_LEN);
  if (ancsControlPointCharacteristic.canWrite()) {
    Serial.println("attempting to write");
    ancsControlPointCharacteristic.write(buffer, BUFFER_LEN);
  } else {
    Serial.println("can't write");
  }
}

void ancsDataSourceCharacteristicValueUpdated(BLECentral& central, BLERemoteCharacteristic& characteristic) {
  Serial.print(F("ANCS Data Source Value Updated: "));

  // see corebluetooth docs for response format.
  // todo: write unpack
  unpack(characteristic.value() + 1, "I", &currentUid);

  uint8_t attributeId;
  uint16_t attributeLen;

  uint16_t offset = 5;
  for (uint8_t i = 0; i < 4; ++i) {
    unpack(characteristic.value() + offset, "BI", &attributeId, &attributeLen);

    unsigned char* result = (unsigned char*) malloc(attributeLen);
    strncpy((char*) result, (char*) characteristic.value() + offset + 3, attributeLen);
    result[attributeLen] = '\0';

    switch (attributeId) {
      case ANCS_NOTIFICATION_ATTRIBUTE_APP_IDENTIFIER:
        // requires another cache for app attributes.
        // you need to make an app attributes call. the only actual attribute is display name.
        free(currentApp);
        currentApp = result;
        break;
      case ANCS_NOTIFICATION_ATTRIBUTE_DATE:
        // I might not actually want to do any of this.
        break;
      case ANCS_NOTIFICATION_ATTRIBUTE_TITLE:
        free(currentTitle);
        currentTitle = result;
        break;
      case ANCS_NOTIFICATION_ATTRIBUTE_MESSAGE:
        free(currentMessage);
        currentMessage = result;
        break;
    }
    offset += 3 + attributeLen;
  }

  BLEUtil::printBuffer(characteristic.value(), characteristic.valueLength());

  isNew = true;
}

void ancsControlPointCharacteristicValueUpdated(BLECentral& central, BLERemoteCharacteristic& characteristic) {
  Serial.print(F("ANCS Control Source Value Updated: "));

  BLEUtil::printBuffer(characteristic.value(), characteristic.valueLength());
}

// best resource
// https://developer.apple.com/library/content/documentation/CoreBluetooth/Reference/AppleNotificationCenterServiceSpecification/Specification/Specification.html


