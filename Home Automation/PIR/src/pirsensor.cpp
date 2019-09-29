/**
 * This program monitors a PIR sensor and sends the status of the PIR sensor 
 * over the painless mesh so that others in the IoT network can check if 
 * there is motion or NOT and do something useful.
 * 
 * It sends a message about the motion status to other nodes in the mesh 
 * every 1 second or whenever there is a change in the motion status. We
 * send the data every second even if it does not change so that new nodes
 * can get the data and also to account for the fact that messages may get 
 * lots in the mesh.
 */
#include <painlessMesh.h>
#include "servo.h"

#define   LED_OUTPUT_PIN  5
#define   PIR_SENSOR_PIN  2
#define   SERVO_CONTROL_PIN 4

#define   MESH_PREFIX     "IOT2019"
#define   MESH_PASSWORD   "csueastbay"
#define   MESH_PORT       5555

void sendMessage(); // forward declaration.

Scheduler userScheduler;
painlessMesh  mesh;
Task taskSendMessage( 100 /* ms */ , TASK_FOREVER, &sendMessage );

int lastMessageSentTimeMs = 0;
bool lastMotionStatus = false;
int msgCount = 0;

Servo servo; 

void sendMessage() {
  bool motion = digitalRead(PIR_SENSOR_PIN); // Read the PIR

  // Turns ON/OFF the onboard blue LED to reflect the PIR sensor status for diganostics purpose.
  if (lastMotionStatus != motion) {
    if (motion) {
      digitalWrite(LED_OUTPUT_PIN, HIGH); 
    }
    else {
      digitalWrite(LED_OUTPUT_PIN, LOW);
    }
  }

  int time = millis();
  
  // Send the motion status to other mesh nodes.
  if ((lastMotionStatus != motion) || (abs(lastMessageSentTimeMs - time) >= 1000)) {
    lastMessageSentTimeMs = time;
    lastMotionStatus = motion;
    String msg = "Motion = ";
    msg += motion;
    Serial.printf("Time: %d Msg:: %s\n", time, msg.c_str());
    mesh.sendBroadcast( msg );
  }
}

// Needed for painless library
void receivedCallback( uint32_t from, String &msg ) {
  static bool toggle = false;
  Serial.printf("startHere: Received from %u msg=%s\n", from, msg.c_str());

  // Handle the temparature message then check if the temprature is high.
  // If yes, then turn on the FAN/Servo.
  if (msg.startsWith("Temp = ")) {
    int temparature = msg.substring(7).toInt();

    Serial.printf("Temp is = %d\n", temparature);
    if (temparature >= 28) {
      toggle = !toggle;
      if (toggle) {
        servo.write(135);
      } else {
        servo.write(45);
      }
    }
  }
}

void newConnectionCallback(uint32_t nodeId) {
    Serial.printf("--> startHere: New Connection, nodeId = %u\n", nodeId);
}

void changedConnectionCallback() {
    Serial.printf("Changed connections %s\n",mesh.subConnectionJson().c_str());
}

void nodeTimeAdjustedCallback(int32_t offset) {
    Serial.printf("Adjusted time %u. Offset = %d\n", mesh.getNodeTime(),offset);
}

void setup() {
  Serial.begin(115200);

  servo.attach(SERVO_CONTROL_PIN);

  // Setup the GPIO PINs
  pinMode(LED_OUTPUT_PIN, OUTPUT);
  pinMode(PIR_SENSOR_PIN, INPUT);

  // Create the Mesh Network.
  //mesh.setDebugMsgTypes( ERROR | MESH_STATUS | CONNECTION | SYNC | COMMUNICATION | GENERAL | MSG_TYPES | REMOTE ); // all types on
  mesh.setDebugMsgTypes( ERROR | MESH_STATUS | STARTUP);

  mesh.init(MESH_PREFIX, MESH_PASSWORD, &userScheduler, MESH_PORT);
  mesh.onReceive(&receivedCallback);
  mesh.onNewConnection(&newConnectionCallback);
  mesh.onChangedConnections(&changedConnectionCallback);
  mesh.onNodeTimeAdjusted(&nodeTimeAdjustedCallback);

  userScheduler.addTask( taskSendMessage );
  taskSendMessage.enable();
}

void loop() {
  userScheduler.execute(); // it will run mesh scheduler as well
  mesh.update();
}
