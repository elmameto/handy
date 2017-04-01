#include <Servo.h>

// PINOUT DECLARATION
#define ESC_pin               10
#define throttle_program_pin  6
#define lights_pin            4
#define batt_level_pin        A1


// BLUETOOTH CONFIGURATION
#define bt_BAUD             38400
#define comm_timeout        800
#define serial_startup_time 300

#define bt_throttle_min 1
#define bt_throttle_max 255


//  ESC CONFIGURATION
#define esc_MIN 1000
#define esc_MAX 2000

#define program_throttle_max_time 7000
#define program_throttle_min_time 4000

#define esc_startup_time 1500


// COMUNICATION WORDS
#define OK                      1994
#define BEGIN_CONNECTION        2001
#define LONGBOARD_BATTERY_BASE  6000
#define NO_MEANING              3000
// COMMANDS
#define COMMAND_LIGHTS_INVERT   8000
#define COMMAND_CHECK_BATTERY   8100

//#define DEBUG

// Connection variables
#ifdef DEBUG
  boolean connection_established = true;
#else
  boolean connection_established = false;
#endif

// Timing variables
    // Instant in which was received the last valid command
      unsigned long last_recv = 0;
    // Instant in which the value sent to the ESC was updated for the last time
      unsigned long esc_mem = 0;

// ESC variables
  // Servo object
    Servo esc;
  // The 
    int esc_go_pulse = esc_MIN;
    int esc_pulse = esc_MIN;

// Acceleration variables
  // Accel
    int esc_increment = 4;
  // Brake
    int esc_decrement = -35;
  // Update period
    int increase_period = 40;

// Others
  // Value just received from the bluetooth
    int in_value = 0;
  // Last value received that was translated and set to the ESC
    int last_set = 0;
  // Battery level
    int batt_level = 0;

void setup() {
  // Start bluetooth
  Serial.begin(bt_BAUD);  
  Serial.setTimeout(comm_timeout);
  
  delay(serial_startup_time);
  
  while(Serial.available())
    Serial.read();

  // Configure pinout
  pinMode(throttle_program_pin, INPUT_PULLUP);
  pinMode(lights_pin, OUTPUT);
  pinMode(batt_level_pin, INPUT);
  esc.attach(ESC_pin);
  
  // Check what start sequence to use
  if(!digitalRead(throttle_program_pin))
    program_throttle();
  else
    startup_ESC();
}

void loop(){
  // Get new data from bluetooth
  check_bt();

  // Check if communication is still active
  #ifndef DEBUG
    if((millis() - last_recv) > comm_timeout || !connection_established){
      esc_go_pulse = esc_MIN;
      connection_established = false;
    }
  #endif
  
  // Write new values
  update_esc();
}

void check_bt(){
  if (Serial.available() > 0){
    // The value is sent as an int, so it has to be an int
    in_value = Serial.parseInt();

    // The Serial.parseInt() answers 0 if there's a timeout or if something else goes wrong,
    // so I have to check if the value is not 0
    if(in_value != 0){

      // Interpretates the received data
      switch(in_value){
        
                  // In confirmation of a sent value, do nothing
        case  OK: break;
                                 // Received when powering up or when restarting a new connection. 
                                 // It answers BEGIN_CONNECTION in order to let the transmitter know that
                                 // the pairing went well.
                                 // If the connection was already established, the message is ignored, for
                                 // security reasons.
        case  BEGIN_CONNECTION:  //if(!connection_established){
                                   Serial.println(BEGIN_CONNECTION);
                                   connection_established = true;
                                 //}
                                 break;

        case  COMMAND_LIGHTS_INVERT:  digitalWrite(lights_pin, digitalRead(lights_pin)^1);
                                      Serial.println(OK);
                                      break;

        case  COMMAND_CHECK_BATTERY:  Serial.println((int)(LONGBOARD_BATTERY_BASE + map(analogRead(batt_level_pin), 0, 1023, 0, 9)));
                                      break;

                  // If it's not a known word, it's most probably an acceleration setting
                  // If the new value is equal to the old one, save time and do nothing. Just
                  // remember that something was received        
        default:  if(in_value != last_set)
        
                    // Check if the value received makes any sense
                    if(in_value >= bt_throttle_min && in_value <= bt_throttle_max){
                      
                      // If so, rescale it to something sendable to the ESC
                      esc_go_pulse = map(in_value, bt_throttle_min, bt_throttle_max, esc_MIN, esc_MAX);
                      last_set = in_value;
                      #ifdef DEBUG
                        Serial.println(in_value);
                        Serial.println(esc_go_pulse);
                      #endif
                      
                    }else{
                      Serial.println(NO_MEANING);
                      return;
                    }
                      
                  // Answer an OK to let know the transmitter that, you know, it's OK
                  Serial.println(OK);
                  break;
      }

      // This is used to check how much time elapsed since the last value received
      last_recv = millis();
    }
  }
}

// The startup function lets know the ESC that you are ready to go
void startup_ESC(){
  esc.writeMicroseconds(esc_MIN);
    delay(esc_startup_time);
}

// This function programs the miminum and maximum value for acceleration to
// the ESC
void program_throttle(){
  esc.writeMicroseconds(esc_MAX);
    delay(program_throttle_max_time);
  esc.writeMicroseconds(esc_MIN);
    delay(program_throttle_min_time);
}

void update_esc(){
  // If the update time has elapsed...
  if(millis() - esc_mem >= increase_period){
    
    // If you are further away that the correct value than the change step...
    if(abs(esc_pulse - esc_go_pulse) > esc_increment){
      // ...select the direction in which to move...
      int delta = esc_go_pulse > esc_pulse ? esc_increment : esc_decrement;
      // ...and move.
      esc_pulse += delta;

    // Otherwise, just go for it
    }else if(esc_pulse != esc_go_pulse)
      esc_pulse = esc_go_pulse;

    // If they are the same, do nothing. Jump also the update phase (below)
     else
       return;

    // Write the new setting to the ESC
    esc.writeMicroseconds(esc_pulse);
    // Useful to check the update time
    esc_mem = millis();
    
    #ifdef DEBUG
      Serial.println(esc_pulse);
    #endif
  }
}
