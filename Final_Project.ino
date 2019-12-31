#include <Servo.h>
#include <LiquidCrystal.h>
#include <MenuBackend.h>
#include <Keypad.h>
#define ACCOUNT_SIZE 64

/*
 * RS -> 50
 * Enable -> 48
 * D4 -> 28
 * D5 -> 26
 * D6 -> 24
 * D7 -> 22
 * 
 * keypad
 * R1 -> 13
 * R2 -> 12
 * R3 -> 11
 * R4 -> 10
 * C3 -> 9
 * C2 -> 8
 * C1 -> 7
 */

struct account{
   String username;
   String password;
};
typedef struct account account;


const byte ROWS = 5; // Four rows
const byte COLS = 4; // Three columns

account accounts[ACCOUNT_SIZE];
account admin;

// Define the Keymap
char keys[ROWS][COLS] = {
  {'f','g','#','*'},
  {'1','2','3','u'},
  {'4','5','6','d'},
  {'7','8','9','x'},
  {'l','0','r','e'}
};
// Connect keypad ROW0, ROW1, ROW2 and ROW3 to these Arduino pins.

byte rowPins[ROWS] = { 31,33,35,37,39 };
// Connect keypad COL0, COL1 and COL2 to these Arduino pins.
byte colPins[COLS] = { 47,45,43,41 }; 

// Create the KeypadKeypad 
Keypad kpd = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
LiquidCrystal lcd(50,48,28,26,24,22);


// Some variable to control the code
boolean return_to_root = false;
String password = "";
String username = "";
boolean login = false;
int state = 0;
int current = 1;
int logged_in_user_index = 0;
int setter_temperature = 100000;
int hatcher_temperature = 100000;
int temperature = 1000;
int servo_motor_status = false;
int system_temp = 0;
int servo_degree = 0;
int offset = 0;
int OFFSET = 5;
int LIMIT = 23;

unsigned long CURRENT = 0;
unsigned long DURATION = 10000;

boolean servo_active_hatcher = false;
boolean servo_active_setter = true;
boolean enable = false;

int BUZZERPORT = 10;
int SERVOPORT = 3;
Servo servomotor; 


int update_temperature(){
     // Calculating temperature
    float rawVoltage = analogRead(A0);
    float millivolts = (rawVoltage / 1024.0) * 3300;
    float celsius = millivolts / 10;
    return celsius;
}

void begin_hatcher(){
   temperature = hatcher_temperature;
   servo_motor_status = servo_active_hatcher;
}

void set_servomotor_status(){
  Serial.println("before if rotate");
  Serial.print("Servo degree");
  Serial.println(servo_degree);
  if(servo_motor_status){
    Serial.println("Should rotate");
    servo_degree += OFFSET;
    if(servo_degree >= 180){
      OFFSET = -OFFSET;
    }
    if(servo_degree <= 0){
      OFFSET = -OFFSET;
    }
      servomotor.write(servo_degree);
  }
  delay(200);
  return;
}

void update_status(){
  if(enable){
         system_temp = update_temperature();
         if (system_temp > temperature)
           digitalWrite(BUZZERPORT, HIGH);
         else
           digitalWrite(BUZZERPORT, LOW);
           
         if (system_temp < LIMIT)
           digitalWrite(LED_BUILTIN, HIGH);
         else
           digitalWrite(LED_BUILTIN, LOW);
           
         set_servomotor_status();
         if(millis() - CURRENT >= DURATION){
          CURRENT = millis();
          begin_hatcher();
          
        }
      }
}

void menuChanged(MenuChangeEvent changed){
    MenuItem newMenuItem=changed.to; //get the destination menu
    lcd.clear();
    lcd.setCursor(0,1); //set the start position for lcd printing to the second row
    lcd.print(newMenuItem.getName());
}

//Menu variables
MenuBackend menu = MenuBackend(menuUsed,menuChanged);
//initialize menuitems
MenuItem menu0Item1 = MenuItem("Display");
MenuItem menu0Item2 = MenuItem("Account");
MenuItem menu0Item3 = MenuItem("Exit");
MenuItem menu0Item4 = MenuItem("Mode Setting");
MenuItem enable_item = MenuItem("Enable");

MenuItem menu1Item1 = MenuItem("Hatcher");
MenuItem menu1Item2 = MenuItem("Setter");

MenuItem menu1Item1SI1 = MenuItem("Servomotor");
MenuItem menu1Item1SI2 = MenuItem("Hatcher Temp");
MenuItem menu1Item2SI1 = MenuItem("Servomotor");
MenuItem menu1Item2SI2 = MenuItem("Setter Temp");

MenuItem menu1Item4 = MenuItem("Servomotor State");
MenuItem menu1Item5 = MenuItem("Set Temperature");

MenuItem account_createAccount = MenuItem("Create Account");
MenuItem account_passwordChange = MenuItem("Password Change");

MenuItem servomotor_passive_hatcher = MenuItem("Deactive");
MenuItem servomotor_active_hatcher = MenuItem("Active");

MenuItem servomotor_passive_setter = MenuItem("Deactivate");
MenuItem servomotor_active_setter = MenuItem("Activate");

void menuUsed(MenuUseEvent used){
    lcd.setCursor(0,1); 
    
    return_to_root = false;

    if ((used.item.getName()) == "Display"){

        lcd.setCursor(0,0);
        lcd.print("Temp Humd Deg");
        lcd.setCursor(0,1);
        lcd.print("               ");
        while(true) {
          char key = kpd.getKey();
            if ( key == 'x'){
                menu.moveUp();
                break;
            }
            lcd.setCursor(0,1);
            lcd.print("               ");
            lcd.setCursor(0,1);        
            int celsius = update_temperature();
            lcd.print(celsius);
            lcd.print("   ");
            lcd.print("MLD");
            lcd.print("  ");
            lcd.print(servo_degree);
            set_servomotor_status();
            update_status();
          }
    }
    if ((used.item.getName()) == "Exit"){
        login = false;
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("Smart Incubation");
        lcd.setCursor(0,1);
        lcd.print("v0.0.-1 ");
    }

    if((used.item.getName()) == "Create Account"){
      Serial.println("in Create Account");
      int cr_state = 0;
      String cr_username = "";
      String cr_password = "";
      boolean finish = false;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("Enter username:");
      lcd.setCursor(0,1);
      while(!finish){
        char key = kpd.getKey();
        if(key){
          if(cr_state == 0){
            if(key == 'e'){
              boolean exist = false;
              for(int k = 0; k < ACCOUNT_SIZE; k++){
                if(accounts[k].username == cr_username){
                  lcd.clear();
                  lcd.setCursor(0,0);
                  lcd.print("Username Exists");
                  exist = true;
                  cr_state = 3;
                }
              }
              if(!exist){
                Serial.println("in password");
                cr_state++;
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Enter password:");
                lcd.setCursor(0,1);
                Serial.println("Username is" + cr_username);
              }
            }
            else{
              cr_username += key;
              lcd.print(key);
              Serial.println("Username is" + cr_username);
            }
          }
          else if(cr_state == 1){
              if(key == 'e'){
                cr_state++;
                Serial.println("Password is " + cr_password);
              }
              else{
                lcd.setCursor(0,1);
                for(int k = 0; k < cr_password.length() ; k++){
                  lcd.print("*");
                }
                cr_password += key;
                lcd.print(key);
              }
          }
          if(cr_state == 2){
            accounts[current].username = cr_username;
            accounts[current].password = cr_password;
            current++;
            finish = true;
          }
          if(cr_state == 3){
            if(key == 'e'){
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Enter username:");
              lcd.setCursor(0,1);
              cr_state = 0;
            }
          }
        }
        update_status();
      }
    if(finish){
      Serial.println("in to root");
      menu.toRoot();
    }
  }
  
    if((used.item.getName()) == "Password Change"){
        String chp_password = "";
        int chp_state = 0;
        boolean finish = false;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Enter New Password:");
        lcd.setCursor(0,1);
        while(!finish){
           char key = kpd.getKey();
            if(key){
                if(chp_state == 0){
                    if(key == 'e'){
                        accounts[logged_in_user_index].password = chp_password;
                        Serial.println("User " + accounts[logged_in_user_index].username);
                        Serial.println("Password set to " + accounts[logged_in_user_index].password);
                        finish = true;
                    }
                    else{
                        lcd.setCursor(0,1);
                        for(int k = 0; k < chp_password.length() ; k++)
                            lcd.print("*");
                        chp_password += key;
                        lcd.print(key);
                    }
                }
            }
            update_status();
        }
        if(finish)
            menu.toRoot();
    }
    
     if((used.item.getName()) == "Active"){
        servo_active_hatcher = true;
        menu.toRoot();
     }

     if((used.item.getName()) == "Deactive"){
        servo_active_hatcher = false;
        menu.toRoot();
     }

     if((used.item.getName()) == "Activate"){
       servo_active_setter = true;
       menu.toRoot();
     }

     if((used.item.getName()) == "Deactivate"){
       servo_active_setter = false;
       menu.toRoot();
     }

     if((used.item.getName()) == "Setter Temp"){
        String tmp = "";
        boolean finish = false;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Setter Temp");
        while(!finish){
           char key = kpd.getKey();
            if(key){
              lcd.setCursor(0,1);
                if(key == 'e'){
                  if(tmp.length() != 0){
                    setter_temperature = tmp.toInt();
                    finish = true;
                    Serial.print("setter_temperature was set to ");
                    Serial.println(setter_temperature);
                  }
                }else{
                  tmp += key;
                  lcd.print(tmp);
                }
              }
              update_status();
        }
        if(finish)
            menu.toRoot();
     }

     if((used.item.getName()) == "Hatcher Temp"){
        String tmp = "";
        boolean finish = false;
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Hatcher Temp");
        while(!finish){
           char key = kpd.getKey();
            if(key){
                lcd.setCursor(0,1);
                if(key == 'e'){
                  if (tmp.length() !=0){
                    hatcher_temperature = tmp.toInt();
                    finish = true;
                    Serial.print("hatcher_temperature was set to ");
                    Serial.println(hatcher_temperature);
                  }
                }else{
                  tmp += key;
                  lcd.print(tmp);
                }
              }
              update_status();
            }
        if(finish)
            menu.toRoot();
     }

     if((used.item.getName()) == "Enable"){
        CURRENT = millis();
        temperature = setter_temperature;
        servo_motor_status = servo_active_setter;
        enable = true;
        menu.toRoot();
     }
}

void setup(){
    admin.username = "1";
    admin.password = "123";
    accounts[0] = admin;
    lcd.begin(16, 2);

    lcd.setCursor(0,0); 
    lcd.print("Smart Incubation");
    lcd.setCursor(0,1);
    lcd.print("v0.0.-1 ");

    //configure menu
    menu.getRoot().add(menu0Item1);
    menu.getRoot().add(menu0Item2);
    menu.getRoot().add(menu0Item3);
    menu.getRoot().add(menu0Item4);
    menu.getRoot().add(enable_item);
    menu0Item1.addRight(menu0Item2);
    menu0Item2.addRight(menu0Item3);
    menu0Item3.addRight(menu0Item4);
    menu0Item4.addRight(enable_item);
    enable_item.addRight(menu0Item1);
    menu0Item1.addLeft(enable_item);
    

    menu0Item4.add(menu1Item1);
    menu0Item4.add(menu1Item2);

    menu1Item1.addRight(menu1Item2);
    menu1Item2.addRight(menu1Item1);
    menu1Item2.addLeft(menu1Item1);
    menu1Item1.addLeft(menu1Item2);

   menu1Item1.add(menu1Item1SI1);
   menu1Item1.add(menu1Item1SI2);
   menu1Item1SI1.addRight(menu1Item1SI2);
   menu1Item1SI2.addRight(menu1Item1SI1);
   menu1Item1SI1.addLeft(menu1Item1SI2);
   menu1Item1SI2.addLeft(menu1Item1SI1);

   menu1Item2.add(menu1Item2SI1);
   menu1Item2.add(menu1Item2SI2);
   menu1Item2SI1.addRight(menu1Item2SI2);
   menu1Item2SI2.addRight(menu1Item2SI1);
   menu1Item2SI1.addLeft(menu1Item2SI2);
   menu1Item2SI2.addLeft(menu1Item2SI1);

   menu0Item2.add(account_createAccount);
   menu0Item2.add(account_passwordChange);
   account_createAccount.addRight(account_passwordChange);
   account_passwordChange.addRight(account_createAccount);
   account_createAccount.addLeft(account_passwordChange);
   account_passwordChange.addLeft(account_createAccount);

   menu1Item1SI1.add(servomotor_active_hatcher);
   menu1Item1SI1.add(servomotor_passive_hatcher);
   servomotor_passive_hatcher.addRight(servomotor_active_hatcher);
   servomotor_active_hatcher.addRight(servomotor_passive_hatcher);
   servomotor_passive_hatcher.addLeft(servomotor_active_hatcher);
   servomotor_active_hatcher.addLeft(servomotor_passive_hatcher);

   menu1Item2SI1.add(servomotor_active_setter);
   menu1Item2SI1.add(servomotor_passive_setter);
   servomotor_active_setter.addRight(servomotor_passive_setter);
   servomotor_passive_setter.addRight(servomotor_active_setter);
   servomotor_passive_setter.addLeft(servomotor_active_setter);
   servomotor_active_setter.addLeft(servomotor_passive_setter);

   Serial.begin(9600);
   
   pinMode(BUZZERPORT, OUTPUT); 
   pinMode(LED_BUILTIN, OUTPUT);
   temperature = 30;

   servomotor.attach(SERVOPORT);
}

void loop(){
   char key = kpd.getKey();
   
   if(!login){
      if(key){
        if(state == 0){
          if(key == 'e'){
            Serial.println("first state enter pressed");
            state++;
             lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Enter username:");
              lcd.setCursor(0,1);
          }
        }
        else if(state == 1){
            if (key == 'e'){
              if(username.length() != 0){
                state++;
                Serial.println("state 1 enter pressed");
                lcd.clear();
                lcd.setCursor(0,0);
                lcd.print("Enter password:");
                lcd.setCursor(0,1);
              }
            }
            else{
              lcd.print(key);
              username += key;
              Serial.println("username " + username);
            }
        }
        else if (state == 2){
            if (key == 'e'){
              if(password.length() != 0)
                  state++;
            }else{
              lcd.setCursor(0,1);
              for(int k = 0; k < password.length() ; k++)
                  lcd.print("*");
              password += key;
              lcd.print(key);
            }
        }
        if(state == 3){
            for (int i = 0; i < ACCOUNT_SIZE; i++){
              if(username.length()!=0){
                if(accounts[i].username == username){
                    if(accounts[i].password == password){
                        login = true;
                        logged_in_user_index = i;
                        menu.toRoot();
                        Serial.println("Logged in");
                        Serial.println("Matched user " + accounts[i].username);
                        Serial.println("Matched pass " + accounts[i].password);
                    }
                }
              }
            }
           if (!login){
                  lcd.clear();
                  lcd.setCursor(0,0);
                  lcd.print("Password Wrong");   
                  Serial.println("Loggin not successful"); 
              }
            password = "";
            username = "";
            state = 0; 
        }
      }  
    }

    else{
      
      if (key) {
          MenuItem currentMenu=menu.getCurrent();
          Serial.println(key);
          if (key == 'e'){
              if(currentMenu.moveDown()) //if the current menu has a child and has been pressed enter then menu navigate to item below
                  menu.moveDown();
              else //otherwise, if menu has no child and has been pressed enter the current menu is used
                  menu.use();
          }
          else if (key == 'r'){
              menu.moveLeft();
          }
          else if (key == 'l')
              menu.moveRight();
          else if (key == 'x'){
              menu.moveUp();
  //            lcd.setCursor(0,0);
  //            lcd.print("Smart Incubation v0.0.-1");
          }
      }
      update_status();
    }
} 
