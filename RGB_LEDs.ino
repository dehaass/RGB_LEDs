// Filename: RGB_LEDs.ino
// Created: Feb 15, 2016
// Author: Stuart de Haas
//
// Feb 26, 2016
//  Added a 'dimmer'
//
// RGB LED driver for LED strip lighting

#include <elapsedMillis.h>
#include <avr/sleep.h>
#include <avr/power.h>

elapsedMillis timeElapsed;
elapsedMillis timeStep;

// Global Vars
    // **************** State FLags
    // Used to recieve instructions from serial
        const int HEY             = 5;
        const int SHARE_SETTINGS  = 6;
        const int CHANGE_SETTINGS = 7;

    // Internal flags
        const int SLEEP      = 8;
        const int CHILL      = 9;
        const int SET_COL    = 10;
        const int DEFINE_COL = 11;
        const int PLAY_SEQ   = 12;
    // **************** 

    //************** Global Varialbles
        // Current device mode. Volatile for interupt
        volatile int STATE_FLAG = PLAY_SEQ;
        // fade time for QFADE_SEQ
        int FADETIME = 300;
        // time 'resolution' of fade effects
        int STEPTIME = 5;
        // Override for time between colours
        int WAIT = 2500;
        // the colour being displayed from the sequence
        int NUMCOL = 0;
        // The current sequence being displayed
        int SEQNUM = 1;
        // The current sequence type
        int SEQSTYLE = 2;
        // Time multiplier to slow down sequences
        int TIMEMULTI = 1;
        // The current colour being displayed
        int currCol[] = {0, 0, 0};
        // The percent brightness
        int DIMMER = 100;
        // Choose a colour from the list
        int SETCOL = 0;
        const int null = 0;

    // Output pins
        const int INT_PIN   = 2;
        const int LED_PIN   = 13;
        const int RED_PIN   = 9;
        const int GREEN_PIN = 10;
        const int BLUE_PIN  = 11;

    // Colours
        // Primary colours
        const int red[]   = {255, 0, 0 };
        const int green[] = {0, 255, 0 };
        const int blue[]  = {0, 0, 255 };

        // User defined colours
        const int white[]     = {200, 200, 255 };
        const int turquoise[] = {0, 255, 255 };
        const int purple[]    = {200, 0, 255 };
        const int yellow[] = {255, 255, 0 };
        const int orange[] = {255, 100, 50 };
        const int pink[] = {255, 0, 255 };
        const int lime[] = {200, 255, 200 };
        const int off[]       = {0, 0, 0 };

    // Status colours
        const int FUCKUP[] = {0, 0, 255};
        const int READING[] = {0, 255, 0};
        const int SHIT[] = {255, 0, 0};
        const int CHECK[] = {255, 255, 255};

    // Lists
        // list of colours
        const int *allCol[] = {red, green, blue, white, turquoise, purple, yellow, orange, pink, lime, off, nullptr};
        const unsigned int allCol_T[] = { 1000, 1000, 1000, 1000, 1000, 1000,
                            1000, 1000, 1000, 1000, 1000, null}; 

        // User defined sequences
        // Arrays of pointers to arrays
            const int *RGB[] ={red, blue, green, nullptr};
            const unsigned int RGB_T[] = {1000, 500, 1000, null};

            const int *seaSide[] = {blue, turquoise, 
                                    green, purple, white, nullptr};
            const unsigned int seaSide_T[] = {2000, 2000, 2000, 2000, 
                                              2000, 2000, null};

            const int *sunSet[] = {white, yellow, orange, pink, 
                                   red, purple, nullptr};
            const unsigned int sunSet_T[] = {1000, 2000, 2000, 2000,
                                             2000, 1000, null};

            const int *forrest[] = {green, lime, turquoise, blue, nullptr};
            const unsigned int forrest_T[] = {1000, 800, 800, 100, null};

            const int *rainbow[] = {red, orange, yellow, green, blue,
                                    purple, pink, nullptr};
            const unsigned int rainbow_T[] = {1000, 1000, 1000, 1000, 1000,
                                            1000, 1000, null};


            const int *wstrobe[] = {white, off, nullptr};
            const int *rstrobe[] = {red, off, nullptr};
            const int *bstrobe[] = {blue, off, nullptr};
            const int *gstrobe[] = {green, off, nullptr};
            const unsigned int strobe_T[] = {50, 50, null};

        // list of sequences
        // Array of pointers to arrays of pointers to arrays of int
        const int **seqList[] = {RGB, seaSide, sunSet, forrest, rainbow,
                                allCol, wstrobe, rstrobe, gstrobe, bstrobe};
        const unsigned int *timeList[] = {RGB_T, seaSide_T, sunSet_T,
                        forrest_T, rainbow_T, allCol_T,  strobe_T, strobe_T,
                        strobe_T, strobe_T};

// 'Firmware'
void setup() {
    // Initialize serial communications
    Serial.begin(9600);

    // Configure Pins
    pinMode(RED_PIN, OUTPUT);
    pinMode(GREEN_PIN, OUTPUT);
    pinMode(BLUE_PIN, OUTPUT);

    // Tell the Serial line we are alive
    Serial.write(HEY);
}// setup

void enterSleep(void){
    // This is how the arduino goes to sleep

    int colour[] = {0, 0, 0};
    displayColour(colour);
    //piWrite(SLEEP);
    //delay(100);
    set_sleep_mode(SLEEP_MODE_PWR_DOWN);
    sleep_enable();
    attachInterrupt(0, serialInterrupt, LOW);
    /* Now enter sleep mode. */
    sleep_mode();
  
    // Program will wake up here
    sleep_disable(); /* First thing to do is disable sleep. */
    detachInterrupt(0);
    /* Re-enable the peripherals. */
    power_all_enable();
    // Set colour to blue
    STATE_FLAG = CHILL;
    timeElapsed = 0;
    timeStep = 0;
}//enterSleep

void serialInterrupt(){
// Interupt Function
}// ISR

int piRead(){
    // Used to read from serial

    digitalWrite(LED_PIN, HIGH);
    for(int i=0; i<1000; i++){
        delay(10);
        if(Serial.available() > 0){
            digitalWrite(LED_PIN, LOW);
            return int(Serial.read());
        }
    }
    digitalWrite(LED_PIN, LOW);

    // Timeout
    return -1;
}//piRead

void piWrite(int num){
    // Writes to serial
    Serial.write(num); 
}

void displayColour(const int colour[]){
    // Display colours

    analogWrite(RED_PIN, colour[0]);
    analogWrite(GREEN_PIN, colour[1]);
    analogWrite(BLUE_PIN, colour[2]);

    // update the current colour (used in fade calculations)
    for(int i=0; i<3; i++){
        currCol[i] = colour[i];
    }
}//setCol

void displayColour(int colour[]){
    // Display colours

    analogWrite(RED_PIN, int(colour[0]*(DIMMER/100.0)) );
    analogWrite(GREEN_PIN, int(colour[1]*(DIMMER/100.0)) );
    analogWrite(BLUE_PIN, int(colour[2]*(DIMMER/100.0)) );

    // update the current colour (used in fade calculations)
    for(int i=0; i<3; i++){
        currCol[i] = colour[i];
    }
}//setCol

bool confirm(const int cmd){
    int val = piRead();
    if( val != cmd){
        displayColour(FUCKUP);
        STATE_FLAG = CHILL;
        return false;
    }else{
        return true;
    }
}//confirm

// Setting modifiers
void changeSettings(){
    if( !confirm(CHANGE_SETTINGS) ){
        delay(1000);
        displayColour(SHIT);
        return;
    }

    digitalWrite(LED_PIN, HIGH);
    for(int i=0; i<10000; i++){
        delay(10);
        if(Serial.available() > 0) break;
    }
    digitalWrite(LED_PIN, LOW);

    if(Serial.available() > 0){
        digitalWrite(LED_PIN, HIGH);
        int tempState;
        FADETIME   = 10*piRead();
        WAIT       = 10*piRead();
        TIMEMULTI  = piRead();
        SEQNUM     = piRead();
        SEQSTYLE   = piRead();
        SETCOL     = piRead();
        currCol[0] = piRead();
        currCol[1] = piRead();
        currCol[2] = piRead();
        DIMMER     = piRead();
        STATE_FLAG = piRead();

        NUMCOL = 0;
        timeElapsed = 0;
        timeStep = 0;
        digitalWrite(LED_PIN, LOW);
    }else{
        displayColour(FUCKUP);
        STATE_FLAG = CHILL;
    }

    piWrite(CHANGE_SETTINGS);
    confirm(CHANGE_SETTINGS);
}//changeSettings

void shareSettings(int prevState){
    if( !confirm(SHARE_SETTINGS) ){
        return;
    }else{
        piWrite(FADETIME/10);
        piWrite(WAIT/10);
        piWrite(TIMEMULTI);
        piWrite(SEQNUM);
        piWrite(SEQSTYLE);
        piWrite(SETCOL);
        piWrite(currCol[0]);
        piWrite(currCol[1]);
        piWrite(currCol[2]);
        piWrite(DIMMER);
        piWrite(prevState);
    }

    piWrite(SHARE_SETTINGS);
    confirm(SHARE_SETTINGS);
}// shareSettings

// Sequences and colours
void sequence(const int *seq[], const unsigned int seqT[]){
    // Displays a sequence of colours with no fade

    int time;
    // determine whether to use defined sequence delays or override delay
    WAIT == 0 ? time = seqT[NUMCOL] : time = WAIT;
    // Has enough time passed?
    if(timeElapsed < (TIMEMULTI * time )) return;

    int colour[3];
    // fetch the new colour and read it into a variable
    for(int i=0; i<3; i++){
        colour[i] = *(seq[NUMCOL] + i);
    }

    displayColour(colour);
    // Increment and wrap colour sequence
    if(seq[++NUMCOL] == nullptr) NUMCOL = 0;
    // Reset some clocks
    timeElapsed = 0;
    timeStep = 0;
    return;
}//sequence

void fadeSeq(const int *seq[], const unsigned int seqT[]){
    // displays a sequence that has a partial fade between colours.

    // Is the fade complete or still in progress?
    // If complete then just let the normal 'sequence' function finish it
    if(timeElapsed > FADETIME) sequence(seq, seqT);
    if(timeStep < STEPTIME) return;

    int newColour[3];
    // determine the amount to fade each step
    int stepsize = (FADETIME - timeElapsed) / STEPTIME;

    // Protect against deadly zeros
    if( stepsize <= 0 ) return;

    // Math
    for(int i=0; i<3; i++){
        newColour[i] = currCol[i] + int(( *(seq[NUMCOL] + i) - currCol[i] ) / stepsize );
    }

    displayColour(newColour);
    timeStep = 0;
}//fadeSeq

void smoothFadeSeq(const int *seq[], const unsigned int seqT[]){
    // continuous fade between colours in sequence

    int time;
    // delay override enabled or use defined time?
    WAIT == 0 ? time = seqT[NUMCOL] : time = WAIT;

    //when fade is complete just let 'sequence' finish it
    if(timeElapsed > ( TIMEMULTI * time )) sequence(seq, seqT);
    if(timeStep < STEPTIME) return;

    int newColour[3];
    
    // determine amount to fade each step
    int stepsize = ((TIMEMULTI * time ) - timeElapsed) / STEPTIME;

    if( stepsize <= 0 ) return;

    // Math
    for(int i=0; i<3; i++){
        newColour[i] = currCol[i] + int(( *(seq[NUMCOL] + i) - currCol[i] ) / stepsize );
    }

    displayColour(newColour);
    timeStep = 0;
}//fadeSeq

void setColour(){
    // Let the user select a colour from the list

    // timeout 
    for(int i=0; i<10000; i++){
        delay(10);
        if(Serial.available() > 0){
            break;
        }
    }

    if(Serial.available() > 0){
        digitalWrite(LED_PIN, HIGH);

        // read in users desired colour
        int colourNum = piRead();
        int colour[] = {0, 0, 0};
        // fetch the colour
        for(int i=0; i<3; i++){
            colour[i] = *(allCol[colourNum] + i);
        }
        displayColour(colour);
        NUMCOL = 0;
        timeElapsed = 0;
        timeStep = 0;
        Serial.flush();
        digitalWrite(LED_PIN, LOW);
        STATE_FLAG = CHILL;
        return;
    }else{
        displayColour(FUCKUP);
        STATE_FLAG = CHILL;
    }
}//manualColour

//Main loop function
void loop() {
    int prevState;

    // check for any new data 
    if(Serial.available() > 0){
        digitalWrite(LED_PIN, HIGH);
        Serial.flush();
        prevState = STATE_FLAG;
        STATE_FLAG = piRead();
        if(STATE_FLAG != HEY) piWrite(STATE_FLAG);
        NUMCOL = 0;
        timeElapsed = 0;
        timeStep = 0;
        Serial.flush();
        digitalWrite(LED_PIN, LOW);
    }//if

    // go to the correct state
    switch (STATE_FLAG){
        case HEY:
            piWrite(HEY);
            Serial.flush();
            STATE_FLAG = prevState;
            break;
        case SET_COL:
            displayColour(allCol[SETCOL]);
            STATE_FLAG = CHILL;
            break;
        case DEFINE_COL:
            displayColour(currCol);
            STATE_FLAG = CHILL;
            break;
        case PLAY_SEQ:
            if(SEQSTYLE == 0) 
                sequence(seqList[SEQNUM], timeList[SEQNUM]);
            if(SEQSTYLE == 1)
                fadeSeq(seqList[SEQNUM], timeList[SEQNUM]);
            if(SEQSTYLE == 2)
                smoothFadeSeq(seqList[SEQNUM], timeList[SEQNUM]);
            break;
        case SHARE_SETTINGS:
            shareSettings(prevState);
            STATE_FLAG = prevState;
            break;
        case CHANGE_SETTINGS:
            changeSettings();
            break;
        case CHILL:
            delay(5);
            displayColour(currCol);
            break;
        case SLEEP:
            enterSleep();
            STATE_FLAG = prevState;
            break;
        default:
            displayColour(SHIT);
            break;
    }
}//loop
