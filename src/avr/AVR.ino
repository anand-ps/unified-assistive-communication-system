#include <SoftwareSerial.h>
#include "PMButton.h"

PMButton button(2);

bool developer_mode       = 1;    // make it 0 for normal mode
bool boot_feedback        = 0;    // boot success audible notification
bool audio_feedback       = 1;
bool haptic_feedback      = 1;
bool convenience_feedback = 1;

int character_length_delay    = 800; // delay between each character
int word_length_delay         = 2000; // delay between each word
int sentence_length_delay     = 3000; // delay between each sentence
int dot_audio_length_delay    = 100;
int dash_audio_length_delay   = 200;

unsigned long key_pressed_millis    = 0;
unsigned long dot_audio_millis      = 0;
unsigned long dash_audio_millis     = 0;
unsigned long char_completed_millis = 0;
unsigned long word_completed_millis = 0;

bool run_audio_feedback_dot  = 0;
bool run_audio_feedback_dash = 0;
bool run_char_feedback       = 0;
bool run_word_feedback       = 0;


bool char_joined         = 1;
bool word_joined         = 1;
bool sentence_completed  = 1;


int motor_pin     = A0;
int buzzer_pin    = 5;
int bt_state_pin  = 4;
int button_pin    = 2;
int led_pin       = 13;

int  received_data      = 0;
bool bluetooth_state    = 0;
bool bt_connected_flag  = 0;

String click_input      = "";
String input_word       = "";
String input_sentence   = ""; 

SoftwareSerial btSerial(12, 11); // RX, TX

void setup() 
{
  Serial.begin(9600);
  btSerial.begin(9600);
  
  pinMode(motor_pin,    OUTPUT);
  pinMode(buzzer_pin,   OUTPUT);
  pinMode(bt_state_pin, INPUT);
  pinMode(button_pin,   INPUT_PULLUP);
  pinMode(led_pin,      OUTPUT);

  button.begin();
  
  //You can set button timing values for each button to fine tune interaction.
  button.debounce     (10);   // Default is 10 milliseconds
  button.dcGap        (5);    // Time between clicks for Double click. Default is 200 milliseconds
  button.holdTime     (150);  // DASH length__Default is 2 seconds
  button.longHoldTime (1500); // BACKSPACE__Default is 5 seconds
  
//  while (!Serial) 
//  {
//    ; // wait for serial port to connect. Needed for native USB port only
//  }

  if(!digitalRead(button_pin))
  {
    digitalWrite(motor_pin,HIGH);
    delay(200);
    digitalWrite(motor_pin,LOW);
    delay(100);
    digitalWrite(motor_pin,HIGH);
    delay(500);
    digitalWrite(motor_pin,LOW);
    
    audio_feedback       = 0;
    Serial.println("Audio feedback disabled!");
   
    while(!digitalRead(button_pin))
      {
        Serial.println("Release the button to continue...");
        delay(1000);
      }
      Serial.println("");
  }

  if(boot_feedback)
  {
    for(int i=0;i<3;i++)  // Boot successful__Audible & Haptic feedback
    {
      if(audio_feedback) digitalWrite(buzzer_pin, HIGH);
      if(haptic_feedback)digitalWrite(motor_pin,  HIGH);
      delay(100);
      if(audio_feedback) digitalWrite(buzzer_pin, LOW);
      if(haptic_feedback)digitalWrite(motor_pin,  LOW);
      delay(80);
    }
  }
  Serial.println("__Device initialization successful__");
}

void buttonStatusUpdater()
{  
  if(button.clicked())  //short click DOT
  {
    key_pressed_millis  = millis();
    
    char_joined         = 0;
    word_joined         = 0;
    sentence_completed  = 0;

       
    click_input+=".";
    Serial.print(".");
 
    if(audio_feedback)  digitalWrite(buzzer_pin, HIGH);
    if(haptic_feedback) digitalWrite(motor_pin,  HIGH);

    dot_audio_millis       = millis();
    run_audio_feedback_dot = 1;
  
  }

  if(button.held())  //long click DASH
  {
    key_pressed_millis = millis();
    
    char_joined         = 0;
    word_joined         = 0;
    sentence_completed  = 0;
    
    
    click_input+="_";
    Serial.print("_");
    
    if(audio_feedback)  digitalWrite(buzzer_pin, HIGH);
    if(haptic_feedback) digitalWrite(motor_pin,  HIGH);

    dash_audio_millis       = millis();
    run_audio_feedback_dash = 1;
   
  }
   
  if(button.heldLong())
  {
    key_pressed_millis = millis();
    
    Serial.println("backspace");
    Send(input_sentence);
    sentence_completed = 1;  //to prevent continuous call of send() if time elapsed
  }
}

void audio_haptic_feedback_handler()
{
  if((millis() - dot_audio_millis >= dot_audio_length_delay) && run_audio_feedback_dot)
    {
      run_audio_feedback_dot = 0;
      
      digitalWrite(buzzer_pin,LOW);
      digitalWrite(motor_pin, LOW);
    }

  if((millis() - dash_audio_millis >= dash_audio_length_delay) && run_audio_feedback_dash)
    {
      run_audio_feedback_dash = 0;
      
      digitalWrite(buzzer_pin,LOW);
      digitalWrite(motor_pin, LOW);
    }

  if((millis() - char_completed_millis >= 60) && run_char_feedback)
    {
      run_char_feedback = 0;
      
      digitalWrite(buzzer_pin,LOW);
      digitalWrite(motor_pin,LOW);
    }

  if((millis() - word_completed_millis >= 60) && run_word_feedback)
    {
      run_word_feedback = 0;
      
      digitalWrite(buzzer_pin,LOW);
      digitalWrite(motor_pin, LOW);
    }
}  

void timerHandler()
{
  // one English character
  if( ((millis() - key_pressed_millis >= character_length_delay) || click_input.length() >= 5) && char_joined==0)
    {
       click_input    += " ";
       input_word     += click_input;
       click_input     = "";
       char_joined     = 1; // one English character completed
       
       Serial.print(" ");

       // one English character
       if(convenience_feedback)
        {
          if(audio_feedback)  digitalWrite(buzzer_pin, HIGH);
          if(haptic_feedback) digitalWrite(motor_pin,  HIGH);

          char_completed_millis    = millis();
          run_char_feedback        = 1;
        }
    }

  else if(millis() - key_pressed_millis >= word_length_delay  &&  word_joined == 0)
    {
             
       input_word     += "  ";
       input_sentence += input_word;
       word_joined     = 1;
       
       Serial.print(" ");

       if(convenience_feedback)
        {
          if(audio_feedback)  digitalWrite(buzzer_pin, HIGH);
          if(haptic_feedback) digitalWrite(motor_pin,  HIGH);

          word_completed_millis    = millis();
          run_word_feedback        = 1;
        }
    }

  else if(millis() - key_pressed_millis >= sentence_length_delay && sentence_completed == 0)
    {
       Send(input_sentence);
       sentence_completed = 1;
    }
}

void Send(String input)
{
  btSerial.println(input);
  Serial.println("Data send");
  
  input_sentence = "";
  input_word     = "";

  for(int i=0;i<2;i++)
    {
      if(audio_feedback) digitalWrite(buzzer_pin, HIGH);
      if(haptic_feedback)digitalWrite(motor_pin,  HIGH);
      delay(60);
      if(audio_feedback) digitalWrite(buzzer_pin, LOW);
      if(haptic_feedback)digitalWrite(motor_pin,  LOW);
      delay(60);
    }
}



void loop() 
{
  button.checkSwitch();
  
  audio_haptic_feedback_handler();
  
  timerHandler();
  
  //used to see the state change
  buttonStatusUpdater();
  
  read_sensors();

  receive_data();

//  if(developer_mode)  serial_feedback();

  bt_connection_status_feedback();
}


void read_sensors()
{
  bluetooth_state = digitalRead(bt_state_pin);
}


void receive_data()
{
  if (btSerial.available()>0)
  {
    received_data = btSerial.read();

    if(received_data == '0')
    {
      digitalWrite(buzzer_pin,LOW);
      digitalWrite(motor_pin, LOW);
    }
    if(received_data == '1')
    {
      digitalWrite(motor_pin,HIGH);
    }
  
    if(received_data == '2')
    {
     digitalWrite(buzzer_pin,HIGH);
    }
  }
}


void bt_connection_status_feedback()
{
  if(bluetooth_state == 1 && bt_connected_flag == 0)
  {
    for(int i=0;i<2;i++)
    {
      if(audio_feedback)  digitalWrite(buzzer_pin, HIGH);
      if(haptic_feedback) digitalWrite(motor_pin,  HIGH);
      delay(100);
      if(audio_feedback)  digitalWrite(buzzer_pin, LOW);
      if(haptic_feedback) digitalWrite(motor_pin,  LOW);
      delay(80);
    }

    bt_connected_flag = 1;
  }

  if(bluetooth_state == 0 && bt_connected_flag == 1)
  {
      if(audio_feedback)  digitalWrite(buzzer_pin, HIGH);
      if(haptic_feedback) digitalWrite(motor_pin,  HIGH);
      delay(500);
      if(audio_feedback)  digitalWrite(buzzer_pin, LOW);
      if(haptic_feedback) digitalWrite(motor_pin,  LOW);
    

    bt_connected_flag = 0;
  }
}

void serial_feedback()
{
  Serial.print("BT state: ");
  Serial.print(bluetooth_state);

  Serial.print(" | Rx Data: ");
  Serial.print(received_data);

  Serial.print(" | Tx Data: ");
  //Serial.println(data_send);
}
