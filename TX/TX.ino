
// PINOUT DECLARATION
  // Interface
    #define accel_pin           A3
    #define human_precence_pin  2
  //Internal
    #define self_batt_pin       A1
    #define buzz_pin            3


// BLUETOOTH CONFIGURATION
#define bt_BAUD             38400
#define data_trans_timeout  50
#define comm_timeout        500
#define serial_startup_time 300

#define bt_throttle_min 1
#define bt_throttle_max 255

// COMUNICATION WORDS
#define OK                      1994
#define BEGIN_CONNECTION        2001
#define LONGBOARD_BATTERY_BASE  6000
#define NO_MEANING              3000
#define LONGBOARD_BATTERY_INFO  LONGBOARD_BATTERY_BASE/10 // Equivale a 600
  // COMMANDS
  #define COMMAND_LIGHTS_INVERT   8000
  #define COMMAND_CHECK_BATTERY   8100
  #define RESEND                  10000

// OTHER
#define accel_mem_depth       3
#define double_click_interval 250
#define update_slow_data_time 1*1000


//#define DEBUG

#ifndef DEBUG
boolean connection_established = false;
#else
boolean connection_established = true;
#endif
// DEV VARIABLES
unsigned long no_meaning_count = 0;


// COMMUNICATION VARIABLES
  
  // INPUT
    boolean data_received   = true;
    int in_value            = 0;
    unsigned long last_recv = 0;
    
  //OUTPUT
    boolean resend          = false;
    int to_be_sent          = 0;
    unsigned long last_sent = 0;


// BUTTON VARIABLES
boolean button_pressed          = false;
boolean first_click_done        = false;
unsigned long first_click_time  = 0;

// STARTUP VARIABLES
boolean cold_start      = true;
uint8_t starter         = 0;
unsigned long accel_mem = 0;


// ACCELERATION DATA
uint8_t new_accel = 0;
uint8_t accel     = 0;


// SYSTEM MANAGEMENT
int error = 0;
unsigned long last_slow_data_update_time = 0;


// BATTERY DATA
uint8_t long_batt_level = 0;
uint8_t self_batt_level = 0;

void setup() {
  // Start bluetooth
  Serial.begin(bt_BAUD);  
  Serial.setTimeout(comm_timeout);
  
  delay(serial_startup_time);
  
  while(Serial.available())
    Serial.read();

  pinMode(buzz_pin, OUTPUT);
  pinMode(accel_pin, INPUT);
  pinMode(human_precence_pin, INPUT_PULLUP);
  pinMode(self_batt_pin, INPUT);
}

void loop(){
  if(!connection_established){
    Serial.println(BEGIN_CONNECTION); 
    delay(50);
    check_bt();
    return; 
  }
  check_bt();
  check_button();
  
  read_accel();
  send_accel();

  if((millis() - last_slow_data_update_time) > update_slow_data_time){
    //self_check_battery();
    //check_long_battery();
    display_data();
    last_slow_data_update_time = millis();
  }
  
  #ifndef DEBUG
    if((millis() - last_recv) > comm_timeout){
      connection_established = false; 
      cold_start = true;
    }
  #endif

}

void display_data(){
  uint8_t data;
  if(button_pressed)
    data = long_batt_level;
  else
    data = self_batt_level;

  #ifdef DEBUG
    Serial.println(button_pressed);
    Serial.println(accel);
    Serial.println(first_click_done);
    Serial.println(no_meaning_count);
    Serial.println("");
  #endif
  // Display data on the LEDs
}

void check_button(){
  // If the button in pressed...
  if(!digitalRead(human_precence_pin)){
    // ...it's marked as pressed
    button_pressed = true;  

    // If the first click has been done...
    if(first_click_done){
      // ...the state is changed, so an uneven number of clicks doesn't count
      first_click_done = false;
      // ...the handling function is called
      double_click();  
    // ...otherwise...
    }else
      // ...the time in which the click happened is recorder (the beginning of the click is recorded)
      first_click_time = millis(); 

  // If the button is not pressed, but it was pressed until the last iteration (which means button_pressed is still true)...
  }else if(button_pressed){
    // ...it's marked as unpressed
    button_pressed = false;
    // ...the release of the button counts as a complete first click
    first_click_done = true;

  // If the time elapsed since the firs click is greater than set... 
  }else if((millis() - first_click_time) > double_click_interval)
    // ...it doesn't count as a click anymore
    first_click_done = false;
}

void send_accel(){
  if(!cold_start)
    send_data(accel);
  else if(accel == bt_throttle_min)
    cold_start = false;
  else
    send_data(bt_throttle_min);
}

void read_accel(){
  // If the button is not pressed, the minimum value is set
  if(!button_pressed){
    accel = bt_throttle_min;
    return;
  }
  
  // Se ci si trova nella fase di avvio... (valori registrati minori di quelli su cui si vuole mediare)
  new_accel = map(analogRead(accel_pin), 0, 1023, bt_throttle_min, bt_throttle_max);
  if(starter < accel_mem_depth){
    accel_mem += new_accel;
    accel = new_accel;
    starter++;  
    return;
  }

  // Nella fase a regime (numero di valori misurati maggiore o uguale alla profondità della memoria impostata)
  accel_mem += (new_accel-accel);
  accel = accel_mem / accel_mem_depth;
}

void check_bt(){
  if (Serial.available() > 0) {
    in_value = Serial.parseInt();
    
    if(in_value != 0){
      
      switch(in_value){
        case  OK: data_received = true;
                  break;

        case  NO_MEANING: data_received = true;
                          no_meaning_count++;
                          break;
        
        case  BEGIN_CONNECTION:  //if(!connection_established){
                                    connection_established = true;
                                    if(!data_received)
                                      send_data(RESEND);
                                 //}
                                 break;

        case  LONGBOARD_BATTERY_INFO:  long_batt_level = (uint8_t)(in_value - LONGBOARD_BATTERY_BASE);
                                       data_received = true;
                                       break;
                          
        default:  return;
      }

      last_recv = millis();
    }
  }
}

void send_data(int _data){
  if(data_received && _data != RESEND){
    to_be_sent = _data;
    Serial.println(to_be_sent);
    #ifndef DEBUG
      data_received = false; 
    #endif 
  }else if(_data == RESEND)
    Serial.println(to_be_sent);
}

void double_click(){
  send_data(COMMAND_LIGHTS_INVERT);
}

void self_check_battery(){
  self_batt_level = map(analogRead(self_batt_pin), 0, 1023, 0, 9);
}

void check_long_battery(){
  send_data(COMMAND_CHECK_BATTERY);  
}
