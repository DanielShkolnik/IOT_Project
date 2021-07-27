#!/user/bin/python3

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

############################################################################################################################################
# SETUP
############################################################################################################################################

url_pop_and_get_next = "https://us-central1-iotrain-49a8d.cloudfunctions.net/api/pop_user/1/"
url_add_queue = "https://us-central1-iotrain-49a8d.cloudfunctions.net/api/add_user"
url_get_next = "https://us-central1-iotrain-49a8d.cloudfunctions.net/api/get_next/1"
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

# Set GPIO Pins
GPIO_TRIGGER = 18
GPIO_ECHO = 24
 
# Set GPIO direction (IN / OUT)
GPIO.setup(GPIO_TRIGGER, GPIO.OUT)
GPIO.setup(GPIO_ECHO, GPIO.IN)

# Firebase config
try:
  firebase = pyrebase.initialize_app(firebaseConfig)
  storage = firebase.storage()
except:
  print("SETUP: Network failure in Firebase config - Exiting...")
  exit()
path_on_firebase = "users"
path_on_raspberrypi = "/home/pi/Desktop/Integration/users/"

# Camera config
camera = picamera.PiCamera()
camera.resolution = (320, 240)
output = np.empty((240, 320, 3), dtype=np.uint8)

# Global varibles
images_face_encoding = []
known_face_names = []
known_face_uids = []

# Init LCD display
display = drivers.Lcd()

# Threshold representes occupied machine for ultrasonic distance sensor
OCCUPIED_THRESHOLD = 40

############################################################################################################################################
# FUNCTIONS
############################################################################################################################################

# Ultrasonic distance function
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

# print on LCD display function
def lcd_print(line1, line2):
  display.lcd_clear()
  print("Writing to display")
  display.lcd_display_string(line1, 1)  
  display.lcd_display_string(line2, 2)  
  sleep(5)                                                                                     
  display.lcd_clear()

# Delete old local users, download from DB updated users and load faces encoding for face-detection
def sync_images_db():
  #Delete old users
  filelist = [ f for f in os.listdir("users") if f.endswith(".jpg") ]
  for f in filelist:
    os.remove(os.path.join("users",f))

  # Update users folder
  try:
    all_files = storage.bucket.list_blobs(prefix="users/")
    for file in all_files:
      print(file.name)
      try:
        file.download_to_filename(file.name)
      except:
        print('Download Failed')
    print("Update users from firebase")
  except:
    print("sync_images_db: Network error")
    return -1

  #Load faces encodings
  filelist = [ f for f in os.listdir("users") if f.endswith(".jpg") ]
  face_images = []
  global images_face_encoding
  global known_face_names
  global known_face_uids  
  for f in filelist:
    image = face_recognition.load_image_file("users/" + f)
    face_location = face_recognition.face_locations(image)
    print("Found {} faces in image of {}".format(len(face_location),f))
    if len(face_location) > 1:
      print("There are more than 1 face in the pic, please change picture")
    elif len(face_location) == 0:
      print("Not Recognized any face, please change picture")
    else:
      face_images.append(face_recognition.load_image_file("users/" + f))
      images_face_encoding.append(face_recognition.face_encodings(face_images[-1])[0])
      known_face_names.append(f.split("_")[0])
      known_face_uids.append(f.split("_")[1].split(".")[0])
  print("finished loading faces")
  return 0

# Capture image, compares faces to DB and return the face index or -1 if error accured
def capture_and_detect_face_image():
  print("Capturing image.")
  camera.capture(output, format="rgb")
  face_location = face_recognition.face_locations(output)
  print("Found {} faces in image.".format(len(face_location)))
  if len(face_location) > 1:
    print("There are more than 1 face in the pic, try again")
    lcd_print("Multiple faces,", "Please try again")
  elif len(face_location) == 0:
    print("Not Recognized any face, please try again")
    lcd_print("No match..", "Please try again")
  else:
    recognized = False
    face_encoding = face_recognition.face_encodings(output, face_location)
    for i in range(len(images_face_encoding)):
      match = face_recognition.compare_faces(images_face_encoding[i], face_encoding)
      if match[0]:
        print("Recognized name = {0} with uid = {1}".format(known_face_names[i],known_face_uids[i]))
        recognized = True
        return i                      
    if recognized == False:
      print("There is no match, please try again")
      lcd_print("No match..","Please try again")
  return -1

# Check if the user recognized is next in queue
def verify_user_is_next(user_index):
  global next_in_queue
  if known_face_uids[user_index] == next_in_queue.split("_")[1]:
    lcd_print("Greetings {}!".format(known_face_names[user_index]),"No pain no gain")
    return True
  else:
    lcd_print("Greetings {}!".format(known_face_names[user_index]),"Not your turn")
    return False

# Add user to queue
def add_to_queue():
  user_index = capture_and_detect_face_image()
  if user_index == -1:
    print("Unable to add to queue")
    lcd_print("Unable to add","to queue")
    return ""
  else:
    pload = {"device_id":1, "user":known_face_names[user_index] + "_" + known_face_uids[user_index], "fcm_token":"from machine"}
    try:
      r = requests.post(url_add_queue, data = pload)
      r = json.loads(r.text)
    except:
      print("add_to_queue: Network Error")
      return ""
    if r["result"] == "already in queue":
      print("Already in queue")
      lcd_print("Greetings {}!".format(known_face_names[user_index]),"Already in queue")
    else:
      lcd_print("Greetings {}!".format(known_face_names[user_index]),"Added to queue")
  return known_face_names[user_index] + "_" + known_face_uids[user_index]

# Pop the user that just finished at the machine and get next user in queue
def pop_user_from_queue_and_get_next(selected_dificulty):
  try:
    r = requests.get(url_pop_and_get_next + selected_dificulty)
    r = json.loads(r.text)
  except:
    print("pop_user_from_queue_and_get_next: Network Error")
    return "error"
  if r != {} and r["user"] != "empty":
    print(r["user"])
    lcd_print("The next up is","{}".format(r["user"].split("_")[0]))
    return r["user"]
  else:
    print("Empty")
    print("Writing to display")
    lcd_print("The queue is","Empty")
    return ""

# Get the next user in queue
def get_next_user_from_queue():
  try:
    r = requests.get(url_get_next)
    r = json.loads(r.text)
  except:
    print("get_next_user_from_queue: Network Error")
    return "error"
  if r != {} and r["user"] != "empty":
    print(r["user"])
    lcd_print("The next up is","{}".format(r["user"].split("_")[0]))
    return r["user"]
  else:
    print("Empty")
    print("Writing to display")
    lcd_print("The queue is","Empty")
    return ""

# print on LCD display the dificulty menu and retun the selected dificulty
def select_dificulty():
  display.lcd_clear()
  selected_line = 0
  selection_changed = False
  display.lcd_display_string("--> Easy", 1)  
  display.lcd_display_string("    Hard", 2)
  while GPIO.input(25) != GPIO.HIGH:
    if selection_changed:
      selection_changed = False
      sleep(1)
      display.lcd_clear()
      if selected_line == 0:
        display.lcd_display_string("--> Easy", 1)  
        display.lcd_display_string("    Hard", 2)
      else:
        display.lcd_display_string("    Easy", 1)  
        display.lcd_display_string("--> Hard", 2)

    if GPIO.input(15) == GPIO.HIGH:
      selection_changed = True
      selected_line = (selected_line + 1)%2
  display.lcd_clear()
  return "easy" if (selected_line == 0) else "hard"

# Check if there is unauthorized entrence to the machine
def check_unauthorized_entrence():
  counter = 0
  for _ in range(20):
    time.sleep(1/40)
    if distance() < OCCUPIED_THRESHOLD:
      counter = counter + 1
  return counter > 10

# print on LCD display the menu options and retun the selected option
def select_menu():
  display.lcd_clear()
  selected_line = 0
  selection_changed = False
  display.lcd_display_string("--> Recognize", 1)  
  display.lcd_display_string("    Register", 2)
  while GPIO.input(25) != GPIO.HIGH:
    if selection_changed:
      selection_changed = False
      sleep(1)
      display.lcd_clear()
      if selected_line == 0:
        display.lcd_display_string("--> Recognize", 1)  
        display.lcd_display_string("    Register", 2)
      else:
        display.lcd_display_string("    Recognize", 1)  
        display.lcd_display_string("--> Register", 2)

    if GPIO.input(15) == GPIO.HIGH:
      selection_changed = True
      selected_line = (selected_line + 1)%2
  display.lcd_clear()
  return "recognize" if (selected_line == 0) else "register"
      
############################################################################################################################################
# State Machine
############################################################################################################################################
#states = ["Machine Vacant", "Machine Enternce", "Queue Registration", "Difficulty Selection", "Machine Occupied", "Error"]
print("Starting sync_images_db()...")
return_value = -1
while return_value != 0:
  return_value = sync_images_db()

current_state = "Machine Vacant"

# Init next in queue from DB
return_value = "error"
while return_value == "error":
  return_value = get_next_user_from_queue()
next_in_queue = return_value

# Selected dificulty to be used in 
selected_dificulty = ""

while True:
  print("Entering " + current_state)
  if current_state != "Machine Occupied" and check_unauthorized_entrence():
    current_state = "Error"
  print("Finished checking unauthorized entrence")

  if current_state == "Machine Vacant":
    selected_option = select_menu()
    if selected_option == "recognize":
      current_state = "Machine Enternce"
    else:
      current_state = "Queue Registration"
    # counter = 0
    # while counter < 10:
    #   if GPIO.input(15) == GPIO.HIGH:
    #     print("Machine Enternce Button Pushed")
    #     current_state = "Machine Enternce"
    #     break
    #   counter = counter + 1

  elif current_state == "Machine Enternce":
    return_value = "error"
    while return_value == "error":
      return_value = get_next_user_from_queue()
    next_in_queue = return_value
    if next_in_queue == "":
      next_in_queue = add_to_queue()
      if next_in_queue == "":
        print("Machine Enternce: Error in add_to_queue")
        continue
    user_index = capture_and_detect_face_image()
    if user_index == -1:
      print("Machine Enternce: Error in user user_index")
      current_state = "Machine Vacant"
    else:
      print("next_in_queue = " + next_in_queue)
      if verify_user_is_next(user_index) == True:
        current_state = "Difficulty Selection"
      else:
        print("Machine Enternce: Error in user verify_user_is_next")
        current_state = "Machine Vacant"

  elif current_state == "Queue Registration":
    added_user = add_to_queue()
    print("Queue Registration: Added user = " + added_user)
    current_state = "Machine Vacant"

  elif current_state == "Difficulty Selection":
    selected_dificulty = select_dificulty()
    lcd_print("Please enter", "the machine")
    while distance() > OCCUPIED_THRESHOLD:
      sleep(1)
      print("Difficulty Selection: Waiting for user to occupy machine")
    current_state = "Machine Occupied"


  elif current_state == "Machine Occupied":
    display.lcd_clear()
    display.lcd_display_string("--> Register", 1)
    counter = 0
    while counter < 3:
      if GPIO.input(25) == GPIO.HIGH:
        display.lcd_clear()
        display.lcd_display_string("Recognizing face,", 1)
        display.lcd_display_string("Stay still...", 2)
        added_user = add_to_queue()
        print("Machine Occupied: Added user to queue = " + added_user)
        display.lcd_clear()
        display.lcd_display_string("--> Register", 1)
      print("Machine Occupied: Waiting for user to vacate machine")
      if distance() > OCCUPIED_THRESHOLD:
        counter = counter + 1
      else:
        counter = 0
      sleep(1)
    print("Machine Occupied: User vacated the machine")
    return_value = "error"
    while return_value == "error":
      return_value = pop_user_from_queue_and_get_next(selected_dificulty)
    next_in_queue = return_value
    current_state = "Machine Vacant"

  # current_state == "Error"
  else:
    print("Error accured - exiting program")
    display.lcd_display_string("Its not your", 1)  
    display.lcd_display_string("turn, leave!", 2)
    counter = 0
    while counter < 5:
      print("Unauthorized user: Waiting for user to vacate machine")
      if distance() > OCCUPIED_THRESHOLD:
        counter = counter + 1
      else:
        counter = 0
      sleep(1)
    display.lcd_clear()
    print("Unauthorized user: User vacated the machine")
    current_state = "Machine Vacant"
    


