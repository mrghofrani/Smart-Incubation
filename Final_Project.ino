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

MenuItem menu1Item1 = MenuItem("Hatcher");
MenuItem menu1Item2 = MenuItem("Setter");

MenuItem menu1Item1SI1 = MenuItem("Humidity");
MenuItem menu1Item1SI2 = MenuItem("Temperature");
MenuItem menu1Item2SI1 = MenuItem("Humidity");
MenuItem menu1Item2SI2 = MenuItem("Temperature");

MenuItem menu1Item3 = MenuItem("Set Humidity");
MenuItem menu1Item4 = MenuItem("Set Accemulator");
MenuItem menu1Item5 = MenuItem("Set Temperature");

void menuUsed(MenuUseEvent used){
    lcd.setCursor(0,1); 
    
    return_to_root = false;

    if ((used.item.getName()) == "Display"){

        lcd.setCursor(0,0);
          lcd.print("Temp Humd Deg");
          lcd.setCursor(0,1);
          // TODO: humidity temperature degree
        while(true) {
          char key = kpd.getKey();
          Serial.print("here in display");
            if ( key == 'x'){
                menu.moveUp();
                break;
            }
          }
        delay(1000);
    }
    if ((used.item.getName()) == "Exit"){
        login = false;
        lcd.clear();
        lcd.setCursor(0,0); 
        lcd.print("Smart Incubation");
        lcd.setCursor(0,1);
        lcd.print("v0.0.-1 ");
    }
    
}

void setup(){
    admin.username = "1";
    admin.password = "2";
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
    menu0Item1.addRight(menu0Item2);
    menu0Item2.addRight(menu0Item3);
    menu0Item3.addRight(menu0Item4);
    menu0Item4.addRight(menu0Item1);
    menu0Item1.addLeft(menu0Item4);

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
   menu1Item2.add(menu1Item1SI2);
   menu1Item2SI1.addRight(menu1Item2SI2);
   menu1Item2SI2.addRight(menu1Item2SI1);
   menu1Item2SI1.addLeft(menu1Item2SI2);
   menu1Item2SI2.addLeft(menu1Item2SI1);
    
    Serial.begin(9600);
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
              Serial.println("state 1 etner pressed");
              state++;
              lcd.clear();
              lcd.setCursor(0,0);
              lcd.print("Enter password:");
              lcd.setCursor(0,1);
            }
            else{
              lcd.print(key);
              username += key;
              Serial.println("username " + username);
            }
        }
        else if (state == 2){
            if (key == 'e'){
                state++;
            }else{
              lcd.print("*");
              password += key;
              Serial.println("Password " + password);
            }
        }
        if(state == 3){
            for (int i = 0; i < ACCOUNT_SIZE; i++){
                if(accounts[i].username == username){
                    if(accounts[i].password == password){
                        login = true;
                        menu.toRoot();
                        Serial.println("Logged in");
                        Serial.println("Matched user " + accounts[i].username);
                        Serial.println("Matched pass " + accounts[i].password);
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
    }
} 
