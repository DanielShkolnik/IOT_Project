#include "esp_camera.h"
#include <WiFi.h>

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

#include "camera_pins.h"
#include "fd_forward.h"


// Remember change Tools->Partition Scheme->Huge App ***************************************

//const char* ssid = "TP-LINK_RoEm2.4";
const char* ssid = "Xiaomi_2.4G";
const char* password = "Rmalal92M";

//const char* ssid = "Mi Phone";
//const char* password = "Oo123456";

int buttonState = 0;
int counter_global = 0;
dl_matrix3du_t *image_matrix_global_arr[5] = {NULL};
//dl_matrix3du_t *curr_image_matrix_global = NULL;

void startCameraServer();
esp_err_t capture_detect_save(dl_matrix3du_t **image_matrix_return);
esp_err_t enroll_face_to_db(dl_matrix3du_t *aligned_face);
esp_err_t recognize_face_from_db(dl_matrix3du_t *aligned_face);
void init_camera();
void print_camera_setting();

void setup() {
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  Serial.println();

  pinMode(13, INPUT);
  pinMode(4, OUTPUT);
  
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  
  // if PSRAM IC present, init with UXGA resolution and higher JPEG quality
  //                      for larger pre-allocated frame buffer.
  if(psramFound()){
    config.frame_size = FRAMESIZE_UXGA;
    config.jpeg_quality = 10;
    config.fb_count = 2;
  } else {
    config.frame_size = FRAMESIZE_SVGA;
    config.jpeg_quality = 12;
    config.fb_count = 1;
  }

#if defined(CAMERA_MODEL_ESP_EYE)
  pinMode(13, INPUT_PULLUP);
  pinMode(14, INPUT_PULLUP);
#endif

  // camera init
  esp_err_t err = esp_camera_init(&config);
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
}


void loop() {
  // put your main code here, to run repeatedly:
   buttonState = digitalRead(13);
  if (buttonState == HIGH) {
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
    else if(counter_global == 1){
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
    else if(counter_global >= 2){
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
  delay(10);
}
