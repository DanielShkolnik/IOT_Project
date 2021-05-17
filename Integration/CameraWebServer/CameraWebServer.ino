
//
// WARNING!!! PSRAM IC required for UXGA resolution and high JPEG quality
//            Ensure ESP32 Wrover Module or other board with PSRAM is selected
//            Partial images will be transmitted if image exceeds buffer size
//

// Select camera model
//#define CAMERA_MODEL_WROVER_KIT // Has PSRAM
//#define CAMERA_MODEL_ESP_EYE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_PSRAM // Has PSRAM
//#define CAMERA_MODEL_M5STACK_V2_PSRAM // M5Camera version B Has PSRAM
//#define CAMERA_MODEL_M5STACK_WIDE // Has PSRAM
//#define CAMERA_MODEL_M5STACK_ESP32CAM // No PSRAM
#define CAMERA_MODEL_AI_THINKER // Has PSRAM
//#define CAMERA_MODEL_M5STACK_NO_PSRAM
//#define CAMERA_MODEL_TTGO_T_JOURNAL // No PSRAM

#if defined(ESP32)
#include <WiFi.h>
#include <FirebaseESP32.h>
#elif defined(ESP8266)
#include <ESP8266WiFi.h>
#include <FirebaseESP8266.h>
#endif

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"


// xin zaho

#include "soc/soc.h"
#include "soc/rtc_cntl_reg.h"
#include "Base64.h"

#include "esp_camera.h"
#include "camera_pins.h"
#include "fd_forward.h"

// Remember change Tools->Partition Scheme->Huge App ***************************************

//const char* ssid = "TP-LINK_RoEm2.4";
//const char* ssid = "Xiaomi_2.4G";
//const char* password = "Rmalal92M";

const char* ssid = "DS OnePlus 6";
const char* password = "b4b79794f2da";

//const char* ssid = "Mi Phone";
//const char* password = "Oo123456";

int buttonState = 0;
int counter_global = 0;
dl_matrix3du_t *image_matrix_global_arr[5] = {NULL};
//dl_matrix3du_t *curr_image_matrix_global = NULL;


/* 2. Define the API Key */
#define API_KEY "AIzaSyDoTudKsvt-FjL1QJhOVxp4FeT_npH7Ds0"

/* 3. Define the RTDB URL */
#define DATABASE_URL "iotrain-49a8d-default-rtdb.europe-west1.firebasedatabase.app" //<databaseName>.firebaseio.com or <databaseName>.<region>.firebasedatabase.app

/* 4. Define the user Email and password that alreadey registerd or added in your project */
#define USER_EMAIL "iotrain.project@gmail.com"
#define USER_PASSWORD "Mm123456@#"

void startCameraServer();
esp_err_t capture_detect_save(dl_matrix3du_t **image_matrix_return);
esp_err_t enroll_face_to_db(dl_matrix3du_t *aligned_face);
esp_err_t recognize_face_from_db(dl_matrix3du_t *aligned_face);
void init_camera();
void print_camera_setting();
void send_photo(dl_matrix3du_t* image_matrix);
String FBPhoto2Base64(camera_fb_t* fb);
camera_fb_t* capture_detect();


//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

bool taskCompleted = false;

FirebaseJson json;



void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  pinMode(13, INPUT);
  pinMode(4, OUTPUT);
  
  camera_config_t camera_config;
  camera_config.ledc_channel = LEDC_CHANNEL_0;
  camera_config.ledc_timer = LEDC_TIMER_0;
  camera_config.pin_d0 = Y2_GPIO_NUM;
  camera_config.pin_d1 = Y3_GPIO_NUM;
  camera_config.pin_d2 = Y4_GPIO_NUM;
  camera_config.pin_d3 = Y5_GPIO_NUM;
  camera_config.pin_d4 = Y6_GPIO_NUM;
  camera_config.pin_d5 = Y7_GPIO_NUM;
  camera_config.pin_d6 = Y8_GPIO_NUM;
  camera_config.pin_d7 = Y9_GPIO_NUM;
  camera_config.pin_xclk = XCLK_GPIO_NUM;
  camera_config.pin_pclk = PCLK_GPIO_NUM;
  camera_config.pin_vsync = VSYNC_GPIO_NUM;
  camera_config.pin_href = HREF_GPIO_NUM;
  camera_config.pin_sscb_sda = SIOD_GPIO_NUM;
  camera_config.pin_sscb_scl = SIOC_GPIO_NUM;
  camera_config.pin_pwdn = PWDN_GPIO_NUM;
  camera_config.pin_reset = RESET_GPIO_NUM;
  camera_config.xclk_freq_hz = 20000000;
  camera_config.pixel_format = PIXFORMAT_JPEG;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    camera_config.frame_size = FRAMESIZE_UXGA;
    camera_config.jpeg_quality = 10;
    camera_config.fb_count = 2;
  } else {
    camera_config.frame_size = FRAMESIZE_SVGA;
    camera_config.jpeg_quality = 12;
    camera_config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&camera_config);
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }

  sensor_t * s = esp_camera_sensor_get();
  // initial sensors are flipped vertically and colors are a bit saturated
  if (s->id.PID == OV3660_PID) {
    s->set_vflip(s, 1); // flip it back
    s->set_brightness(s, 1); // up the brightness just a bit
    s->set_saturation(s, -2); // lower the saturation
  }
  // drop down frame size for higher initial frame rate
  s->set_framesize(s, FRAMESIZE_QVGA);

#if defined(CAMERA_MODEL_M5STACK_WIDE) || defined(CAMERA_MODEL_M5STACK_ESP32CAM)
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);
#endif

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");

  //startCameraServer();
  init_camera();

  Serial.print("Camera Ready! Use 'http://");
  Serial.print(WiFi.localIP());
  Serial.println("' to connect");


  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the user sign in credentials */
  auth.user.email = USER_EMAIL;
  auth.user.password = USER_PASSWORD;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h

  Firebase.begin(&config, &auth);

  //Or use legacy authenticate method
  //Firebase.begin(DATABASE_URL, DATABASE_SECRET);

  Firebase.reconnectWiFi(true);

#if defined(ESP8266)
  //Set the size of WiFi rx/tx buffers in the case where we want to work with large data.
  fbdo.setBSSLBufferSize(4096, 4096);
#endif

  //Set the size of HTTP response buffers in the case where we want to work with large data.
  fbdo.setResponseSize(4096);

  //Set database read timeout to 1 minute (max 15 minutes)
  Firebase.setReadTimeout(fbdo, 1000 * 60 * 3);
  //tiny, small, medium, large and unlimited.
  //Size and its write timeout e.g. tiny (1s), small (10s), medium (30s) and large (60s).
  Firebase.setwriteSizeLimit(fbdo, "large");

  //optional, set the decimal places for float and double data to be stored in database
  Firebase.setFloatDigits(2);
  Firebase.setDoubleDigits(6);

  /**
   * This option allows get and delete functions (PUT and DELETE HTTP requests) works for device connected behind the
   * Firewall that allows only GET and POST requests.
   * 
   * Firebase.enableClassicRequest(&fbdo, true);
  */
}


void loop() {
  // put your main code here, to run repeatedly:
   buttonState = digitalRead(13);
  if (buttonState == HIGH) {
    /*
    if (counter_global == 0) {
        for(int i=0; i<5; i++){
          esp_err_t res = capture_detect_save(&image_matrix_global_arr[i]);
          if (res == ESP_OK) {
            Serial.printf("HI HI - capture_detect_save %d\n",i);
          }
          else {
            Serial.printf("BYE BYE - capture_detect_save %d\n",i);
          }
        }
    }
    */
    if(counter_global >= 0){
      if (Firebase.ready()){
        camera_fb_t* fb = capture_detect();
        if (fb != NULL) {
          Serial.printf("HI HI - capture_detect\n");
          taskCompleted = true;
          Serial.printf("Test camera ready number %d\n",counter_global);
          String path = "/Users";
          FirebaseJson jsonData;
          FirebaseJsonData resp;
          String title = "User-";
          title += counter_global;
          String title_photo = title + "/Photo";
          String title_name = title + "/Name";
          jsonData.set(title_photo, FBPhoto2Base64(fb));
          jsonData.set(title_name, "Daniel");
         
          if (Firebase.set(fbdo, path.c_str(), jsonData)) Serial.println("PASSED - Firebase.set");
          else Serial.println("FAILED - Firebase.set");
          if (Firebase.get(fbdo, path.c_str())){
            Serial.println("PASSED");
            Serial.println("PATH: " + fbdo.dataPath());
            Serial.println("TYPE: " + fbdo.dataType());
            Serial.print("VALUE: ");
            if (fbdo.dataType() == "json") {
              printResult(fbdo); //see addons/RTDBHelper.h
            }
            Serial.println("------------------------------------");
            Serial.println();
            }
          else {
            Serial.println("FAILED");
            Serial.println("REASON: " + fbdo.errorReason());
            Serial.println("------------------------------------");
            Serial.println();
          }
        jsonData = fbdo.jsonObject();

        jsonData.get(resp, title_photo);
        const char* resp_photo = resp.stringValue.c_str();
        String parse_resp_photo = resp_photo + strlen("data:image/jpeg;base64,");
        //Serial.println(title_photo + " = " + resp_photo);
        Serial.println(title_photo + " = " + parse_resp_photo);
        
        jsonData.get(resp, title_name);
        String resp_name = resp.stringValue;
        Serial.println(title_name + " = " + resp_name);

        }
        else{
          Serial.println("Firebase.ready() - Not ready!");
        }
        esp_camera_fb_return(fb);
        fb = NULL;
        }
        else {
          Serial.printf("BYE BYE - capture_detect\n");
        }
    }

    else if(counter_global == 2){
      for(int i=0; i<5; i++){
        esp_err_t res = enroll_face_to_db(image_matrix_global_arr[i]);
        if (res == ESP_OK) {
          Serial.printf("HI HI - enroll_face_to_db %d\n",i);
        }
        else {
          Serial.printf("BYE BYE - enroll_face_to_db %d\n",i);
        }
      } 
    }
    else if(counter_global >= 3){
      esp_err_t res = capture_detect_save(&image_matrix_global_arr[0]);
      if (res == ESP_OK) {
        Serial.printf("HI HI - capture_detect_save\n");
      }
      else {
        Serial.printf("BYE BYE - capture_detect_save\n");
      }

      while(recognize_face_from_db(image_matrix_global_arr[0]) != ESP_OK){
        dl_matrix3du_free(image_matrix_global_arr[0]);
        image_matrix_global_arr[0] = NULL;
        res = capture_detect_save(&image_matrix_global_arr[0]);
      }
      res = recognize_face_from_db(image_matrix_global_arr[0]);
      if (res == ESP_OK) {
        Serial.println("HI HI - recognize_face_from_db");
      }
      else {
        Serial.println("BYE BYE - recognize_face_from_db");
      }
    }
    // turn LED on
    digitalWrite(4, HIGH);
    counter_global++;
  } else {
    // turn LED off
    digitalWrite(4, LOW);
    
  }
  delay(100);
}



/*
void loop()
{
  if (Firebase.ready() && !taskCompleted)
  {
    
    taskCompleted = true;

    String path = "/Test";
    String node;
    
  
    FirebaseJson jsonData;
    jsonData.add("photo", Photo2Base64());
    String photoPath = "/esp32-cam";
    
    if (Firebase.pushJSONAsync(fbdo, photoPath, jsonData)) {
      Serial.println(fbdo.dataPath());
      Serial.println(fbdo.pushName());
      Serial.println(fbdo.dataPath() + "/"+ fbdo.pushName());
    } 
    else {
      Serial.println(fbdo.errorReason());
    }
  

  }
}
*/

String Photo2Base64(dl_matrix3du_t* image_matrix) {
    String imageFile = "data:image/jpeg;base64,"; 
    char *input = (char *)image_matrix->item;
    int len = image_matrix->h * image_matrix->w * image_matrix->c;
    Serial.printf("Photo2Base64 - LEN = %d\n", len);
    char output[base64_enc_len(3)];
    for (int i=0;i<len;i+=3, input+=3) {
      base64_encode(output, input, 3);
      imageFile += urlencode(String(output));
    }
    return imageFile;
}

String FBPhoto2Base64(camera_fb_t* fb) {
    //camera_fb_t * fb = NULL;
    //fb = esp_camera_fb_get();  
    if(!fb) {
      Serial.println("Camera capture failed");
      return "";
    }
  
    String imageFile = "data:image/jpeg;base64,";
    char *input = (char *)fb->buf;
    Serial.printf("OGPhoto2Base64 - LEN = %d\n", fb->len);
    //char output[base64_enc_len(fb->len)];
    //base64_encode(output, input, fb->len);
    //imageFile += urlencode(String(output));
    char output[base64_enc_len(3)];
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
    }
    esp_camera_fb_return(fb);
    
    return imageFile;
}



String OGPhoto2Base64() {
    camera_fb_t * fb = NULL;
    fb = esp_camera_fb_get();  
    if(!fb) {
      Serial.println("Camera capture failed");
      return "";
    }
  
    String imageFile = "data:image/jpeg;base64,";
    char *input = (char *)fb->buf;
    Serial.printf("OGPhoto2Base64 - LEN = %d\n", fb->len);
    //char output[base64_enc_len(fb->len)];
    //base64_encode(output, input, fb->len);
    //imageFile += urlencode(String(output));
    char output[base64_enc_len(3)];
    for (int i=0;i<fb->len;i++) {
      base64_encode(output, (input++), 3);
      if (i%3==0) imageFile += urlencode(String(output));
    }
    esp_camera_fb_return(fb);
    
    return imageFile;
}



//https://github.com/zenmanenergy/ESP8266-Arduino-Examples/
String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    char code2;
    for (int i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;
}



void send_photo(dl_matrix3du_t* image_matrix){
  String imageFile = "data:image/jpeg;base64,";
  char *input = (char *)image_matrix->item;
  int len = image_matrix->h * image_matrix->w * image_matrix->c;
  Serial.printf("Photo2Base64 - LEN = %d\n", len);
  char output[base64_enc_len(3)];
  for (int k=0; k<4; k++){
    for (int i=0;i<len/4;i+=4, input+=4) {
      base64_encode(output, input, 4);
      imageFile += urlencode(String(output));
    }
    FirebaseJson jsonData;
    jsonData.add("photo", imageFile);

    String photoPath = "/esp32-cam";
    
    Serial.printf("Test jsonData.add number %d\n",counter_global);

    if (Firebase.pushJSONAsync(fbdo, photoPath, jsonData)) {
      Serial.println(fbdo.dataPath());
      Serial.println(fbdo.pushName());
      Serial.println(fbdo.dataPath() + "/"+ fbdo.pushName());
    } 
    else {
      Serial.println(fbdo.errorReason());
    }
    imageFile = "";
  }
}