/*
 * Modified from an IRrecv demo from Ken Shirrif
 * Reads light strike IR codes
 * Sends codes formatted as [TEAM] [COUNT] [WEAPON] on serial input
 */
#include <IRremote.h>

int RECV_PIN = PIN_D7;

IRrecv irrecv(RECV_PIN);
IRsend irsend;

decode_results results;

void setup()
{
  Serial.begin(9600);
  irrecv.enableIRIn(); // Start the receiver
}

// Dumps out the decode_results structure.
// Call this after IRrecv::decode()
// void * to work around compiler issue
//void dump(void *v) {
//  decode_results *results = (decode_results *)v
void dump(decode_results *results) {
  int count = results->rawlen;
  if (results->decode_type == UNKNOWN) {
    Serial.println("Could not decode message");
  } 
  else {
    if (results->decode_type == NEC) {
      Serial.print("Decoded NEC: ");
    } 
    else if (results->decode_type == SONY) {
      Serial.print("Decoded SONY: ");
    } 
    else if (results->decode_type == RC5) {
      Serial.print("Decoded RC5: ");
    } 
    else if (results->decode_type == RC6) {
      Serial.print("Decoded RC6: ");
    }
    Serial.print(results->value, HEX);
    Serial.print(" (");
    Serial.print(results->bits, DEC);
    Serial.println(" bits)");
  }
  Serial.print("Raw (");
  Serial.print(count, DEC);
  Serial.print("): ");

  for (int i = 0; i < count; i++) {
    if ((i % 2) == 1) {
      Serial.print(results->rawbuf[i]*USECPERTICK, DEC);
    } 
    else {
      Serial.print(-(int)results->rawbuf[i]*USECPERTICK, DEC);
    }
    Serial.print(" ");
  }
  Serial.println("");
}

unsigned long toLazer = 0;

void loop() {
  if (irrecv.decode(&results)) {  
    Serial.println(results.value, HEX);
    dump(&results);
    irrecv.resume(); // Receive the next value
  }
  
  if (Serial.available() > 0) {
    
    // read the incoming byte:
    String input;
    char array[16];
    while(Serial.available() > 0) {
      input += (char)Serial.read();
    }
    if(input == " " && toLazer != 0)
    {
      Serial.println("Re-firin' mah lazer");
      irsend.sendLS(toLazer, 32);
    } else {
      Serial.println("Firin' mah lazer");
      toLazer = stringToLazer(input);
      irsend.sendLS(toLazer, 32);
    }  
    irrecv.enableIRIn();
  }
}

//one hex digit in ascii to an int
int htoi (char c) {  //does not check that input is valid
    if (c<='9')
        return c-'0';
    if (c<='F')
        return c-'A'+10;
    if (c<='f')
        return c-'a'+10;
    return 0;
}

//Take string in format of "[hex team code] [hex count] [hex weapon]" and returns the int to send out
unsigned long stringToLazer(String input)
{

  String buf[3];
  int first = 0;
  for(int i = 0; i < 2; i++)
  {
    int last;
    last = input.indexOf(' ', first)+1;
    if(last != -1)
    {
      buf[i] = input.substring(first, last-1);
      first = last;
    } else {
      return 0;
    }
  }
  buf[2] = input.substring(first);

  unsigned int section[3] = {0, 0, 0};
  for(int i = 0; i < 3; i++)
  {
    for(int j = 0; j < buf[i].length(); j++)
    {
      char charbuf[6];
      buf[i].toCharArray(charbuf, 6);
      section[i] += htoi(charbuf[j]) << (4*(buf[i].length()-j-1));
    }
  }
  unsigned long output = ((unsigned long)section[0] << 24) | ((unsigned long)section[1] << 16) | section[2];
  Serial.println(output, HEX);
  return output;


}