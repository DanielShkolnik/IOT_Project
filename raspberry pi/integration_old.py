import RPi.GPIO as GPIO 
GPIO.setwarnings(False) 
#GPIO.setmode(GPIO.BOARD) 
GPIO.setmode(GPIO.BCM)
GPIO.setup(15, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(25, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
#from datetime import datetime
#from picamera import PiCamera
from time import sleep
import os
import time

import pyrebase

import face_recognition
import picamera
import numpy as np

import requests
import json

import drivers

url_empty_machine = "https://us-central1-iotrain-49a8d.cloudfunctions.net/api/pop_user/1/"

url_add_queue = "https://us-central1-iotrain-49a8d.cloudfunctions.net/api/add_user"

firebaseConfig = {
    'apiKey': "AIzaSyDoTudKsvt-FjL1QJhOVxp4FeT_npH7Ds0",
    'authDomain': "iotrain-49a8d.firebaseapp.com",
    'databaseURL': "https://iotrain-49a8d-default-rtdb.europe-west1.firebasedatabase.app",
    'projectId': "iotrain-49a8d",
    'storageBucket': "iotrain-49a8d.appspot.com",
    'messagingSenderId': "850558766559",
    'appId': "1:850558766559:web:f803ee642de6a483e9886a",
    'measurementId': "G-RZGMT1JSD3",
    'serviceAccount': "/home/pi/Desktop/Integration/service_account.json"
}

#Ultrasonic distance function

#set GPIO Pins
GPIO_TRIGGER = 18
GPIO_ECHO = 24
 
#set GPIO direction (IN / OUT)
GPIO.setup(GPIO_TRIGGER, GPIO.OUT)
GPIO.setup(GPIO_ECHO, GPIO.IN)
 
def distance():
  # set Trigger to HIGH
  GPIO.output(GPIO_TRIGGER, True)

  # set Trigger after 0.01ms to LOW
  time.sleep(0.00001)
  GPIO.output(GPIO_TRIGGER, False)

  StartTime = time.time()
  StopTime = time.time()

  # save StartTime
  while GPIO.input(GPIO_ECHO) == 0:
    StartTime = time.time()

  # save time of arrival
  while GPIO.input(GPIO_ECHO) == 1:
    StopTime = time.time()

  # time difference between start and arrival
  TimeElapsed = StopTime - StartTime
  # multiply with the sonic speed (34300 cm/s)
  # and divide by 2, because there and back
  distance = (TimeElapsed * 34300) / 2

  return distance



firebase = pyrebase.initialize_app(firebaseConfig)

storage = firebase.storage()

path_on_firebase = "users"

path_on_raspberrypi = "/home/pi/Desktop/Integration/users/"


camera = picamera.PiCamera()
camera.resolution = (320, 240)
output = np.empty((240, 320, 3), dtype=np.uint8)

#Delete old users
filelist = [ f for f in os.listdir("users") if f.endswith(".jpg") ]
for f in filelist:
  os.remove(os.path.join("users",f))

#Update users folder
all_files = storage.bucket.list_blobs(prefix="users/")
for file in all_files:
  print(file.name)
  try:
    file.download_to_filename(file.name)
  except:
    print('Download Failed')
print("Update users from firebase")

#Load faces encodings
filelist = [ f for f in os.listdir("users") if f.endswith(".jpg") ]
face_images = []
images_face_encoding = []
known_face_names = []
known_face_uids = []
for f in filelist:
  face_images.append(face_recognition.load_image_file("users/" + f))
  images_face_encoding.append(face_recognition.face_encodings(face_images[-1])[0])
  known_face_names.append(f.split("_")[0])
  known_face_uids.append(f.split("_")[1].split(".")[0])
print("finished loading faces")

empty_counter = 0

counter = 0

display = drivers.Lcd()

r = requests.get(url_empty_machine + "easy") #FIX me add dificaulty
r = json.loads(r.text)
next_in_queue = r["user"]

while True: 
  if GPIO.input(15) == GPIO.HIGH:
    print("Button pushed -", counter)
    if counter >= 0:
      print("Capturing image.")
      camera.capture(output, format="rgb")
      face_location = face_recognition.face_locations(output)
      print("Found {} faces in image.".format(len(face_location)))
      if len(face_location) > 1:
        print("There are more than 1 face in the pic, try again")
      elif len(face_location) == 0:
        print("Not Recognized any face, please try again")
        print("Writing to display")
        display.lcd_display_string("No match..", 1)  
        display.lcd_display_string("Please try again", 2)  
        sleep(5)                                                                                     
        display.lcd_clear()
      else:
        recognized = False
        face_encoding = face_recognition.face_encodings(output, face_location)
        for i in range(len(images_face_encoding)):
          match = face_recognition.compare_faces(images_face_encoding[i], face_encoding)
          if match[0]:
            print("Recognized name = {0} with uid = {1}".format(known_face_names[i],known_face_uids[i]))
            recognized = True
            print("Writing to display")
            # known_face_uids[i] == next_in_queue.split("_")[1] :
            display.lcd_display_string("Greetings {}!".format(known_face_names[i]), 1)  
            display.lcd_display_string("No pain no gain", 2)  
            sleep(5)                                                                                     
            display.lcd_clear()                                    
        if recognized == False:
          print("There is no match, please try again")
          print("Writing to display")
          display.lcd_display_string("No match..", 1)  
          display.lcd_display_string("Please try again", 2)  
          sleep(5)                                                                                     
          display.lcd_clear() 
    counter += 1
    sleep(2)
  elif GPIO.input(25) == GPIO.HIGH:
    print("Add queue button pushed")
    print("Capturing image.")
    camera.capture(output, format="rgb")
    face_location = face_recognition.face_locations(output)
    print("Found {} faces in image.".format(len(face_location)))
    if len(face_location) > 1:
      print("There are more than 1 face in the pic, try again")
    elif len(face_location) == 0:
      print("Not Recognized any face, please try again")
      print("Writing to display")
      display.lcd_display_string("No match..", 1)  
      display.lcd_display_string("Please try again", 2)  
      sleep(5)                                                                                     
      display.lcd_clear()
    else:
      recognized = False
      face_encoding = face_recognition.face_encodings(output, face_location)
      for i in range(len(images_face_encoding)):
        match = face_recognition.compare_faces(images_face_encoding[i], face_encoding)
        if match[0]:
          print("Recognized name = {0} with uid = {1} and added to queue".format(known_face_names[i],known_face_uids[i]))
          recognized = True
          pload = {"device_id":1,"user":known_face_names[i] + "_" + known_face_uids[i], "fcm_token":"from machine"}
          print(pload)
          print(url_add_queue)
          r = requests.post(url_add_queue, data = pload)
          r = json.loads(r.text)
          print(r)
          if r["result"] == "already in queue":
            print("Already in queue")
            print("Writing to display")
            display.lcd_display_string("Greetings {}!".format(known_face_names[i]), 1)  
            display.lcd_display_string("Already in queue", 2)  
            sleep(5)                                                                                     
            display.lcd_clear()
          else:
            print("Writing to display")
            display.lcd_display_string("Greetings {}!".format(known_face_names[i]), 1)  
            display.lcd_display_string("Added to queue", 2)  
            sleep(5)                                                                                     
            display.lcd_clear()                                                  

      if recognized == False:
        print("There is no match, please try again")
        print("Writing to display")
        display.lcd_display_string("No match..", 1)  
        display.lcd_display_string("Please try again", 2)  
        sleep(5)                                                                                     
        display.lcd_clear() 
    sleep(2)
  else:
    try:
      dist = distance()
      print ("Measured Distance = %.1f cm" % dist)
      if dist > 10:
        empty_counter += 1
        if (empty_counter >= 6):
          if (empty_counter == 6):
            print("Machine is empty!")
            r = requests.get(url_empty_machine + "easy") #FIX me add dificaulty
            r = json.loads(r.text)
            if r != {}:
              next_in_queue = r["user"]
              print(r["user"])
              print("Writing to display")
              display.lcd_display_string("The next up is", 1)  
              display.lcd_display_string("{}".format(r["user"].split("_")[0]), 2) 
            else:
              next_in_queue = ""
              print("Empty")
              print("Writing to display")
              display.lcd_display_string("The queue is", 1)  
              display.lcd_display_string("Empty", 2) 
              
      else:
        empty_counter = 0
      time.sleep(1)

    # Reset by pressing CTRL + C
    except KeyboardInterrupt:
      print("Measurement stopped by User")
      GPIO.cleanup()

