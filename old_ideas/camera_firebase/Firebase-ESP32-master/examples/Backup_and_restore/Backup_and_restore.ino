/**
 * Created by K. Suwatchai (Mobizt)
 * 
 * Email: k_suwatchai@hotmail.com
 * 
 * Github: https://github.com/mobizt
 * 
 * Copyright (c) 2021 mobizt
 *
*/

//This example shows how to backup and restore database data

#include <WiFi.h>
#include <FirebaseESP32.h>

#define WIFI_SSID "WIFI_AP"
#define WIFI_PASSWORD "WIFI_PASSWORD"

#define FIREBASE_HOST "PROJECT_ID.firebaseio.com"

/** The database secret is obsoleted, please use other authentication methods, 
 * see examples in the Authentications folder. 
*/
#define FIREBASE_AUTH "DATABASE_SECRET"



//Define Firebase Data object
FirebaseData fbdo;

void setup()
{

  Serial.begin(115200);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  Firebase.reconnectWiFi(true);

  //Print to see stack size and free memory
  UBaseType_t uxHighWaterMark = uxTaskGetStackHighWaterMark(NULL);
  Serial.print("Stack: ");
  Serial.println(uxHighWaterMark);
  Serial.print("Heap: ");
  Serial.println(esp_get_free_heap_size());

  Serial.println("------------------------------------");
  Serial.println("Backup test...");
  
  //Provide specific SD card interface
  //Firebase.sdBegin(14, 2, 15, 13); //SCK, MISO, MOSI,SS for TTGO T8 v1.7 or 1.8

  //Download and save data at defined database path to SD card.
  //{TARGET_NODE_PATH} is the full path of database to backup and restore.

  if (!Firebase.backup(fbdo, StorageType::SD, "/{TARGET_NODE_PATH}", "/{PATH_IN_SD_CARD}"))
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.fileTransferError());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("PASSED");
    Serial.println("BACKUP FILE: " + fbdo.getBackupFilename());
    Serial.println("FILE SIZE: " + String(fbdo.getBackupFileSize()));
    Serial.println("------------------------------------");
    Serial.println();
  }

  Serial.println("------------------------------------");
  Serial.println("Restore test...");

  //Restore data to defined database path using backup file on SD card.
  //{TARGET_NODE_PATH} is the full path of database to restore

  if (!Firebase.restore(fbdo, StorageType::SD, "/{TARGET_NODE_PATH}", "/{PATH_IN_SD_CARD}"))
  {
    Serial.println("FAILED");
    Serial.println("REASON: " + fbdo.fileTransferError());
    Serial.println("------------------------------------");
    Serial.println();
  }
  else
  {
    Serial.println("PASSED");
    Serial.println("BACKUP FILE: " + fbdo.getBackupFilename());
    Serial.println("------------------------------------");
    Serial.println();
  }

  //Quit Firebase and release all resources
  Firebase.end(fbdo);
  
}

void loop()
{
}
