#include <SoftwareSerial.h>

SoftwareSerial gsmSerial(3, 2); // RX, TX

String  PIN      = "6219",
        APN      = "pinternet.interkom.de",
        /*connHost = "arduino.cc",
        connIP   = "54.92.254.3",
        connReq  = "GET /asciilogo.txt",*/
        connHost = "www.google.de",
        connReq = "GET /",
        //connReq = "GET /Geschlossene_Nervenheilanstalt.txt", //26kB
        //connReq  = "GET /",
        connPort = "80";
        
bool  pinEntryRequired = true,
      isBooted = false,
      hasSignal = false,
      establishedLink = false,
      tcpConnected = false,
      tcpSendRdy = false,
      dnsDone = false;

char rcvBuf[1024] = {0};


String resolvedIP="xxx.xxx.xxx.xxx"; //placeholder for RAM purposes

String dnsReq(String host)
{
  sendAT("AT+DNS=\"" + host + "\"");
  dnsDone = false;
  resolvedIP = "";
  while(!dnsDone) {
    parseSerial();
  }
  return resolvedIP;
}

void tcpConnect(String ip, String port)
{
  String tcpSetup = "AT+TCPSETUP=1," + ip + "," + port + "\r\n";
  tcpConnected = false;
  sendAT(tcpSetup);
  while (!tcpConnected)
  {
    parseSerial();
  }
}

void doTCP(String host, String req) //request with DNS lookup
{
  doTCP(dnsReq(host), "80", req); 
}

void doTCP(String ip, String port, String req)
{
  tcpConnect(ip, port);
  //sendAT(tcpSetup);
  String reqString = req + " HTTP/1.1\n";
  reqString += "host: " + connHost + "\n\r\n";
  String reqLength = String(reqString.length());
  //delayWithSerial(1000);
  sendAT("AT+TCPSEND=1," + reqLength + "\r\n", false);
  while(!tcpSendRdy)
  {
    parseSerial();
  }
  tcpSendRdy = false;
  Serial.print(reqString);
  gsmSerial.print(reqString);
  gsmSerial.print("\r");
}



void handleReturn(String retStr)
{
  //Serial.print("<R>");
  uint8_t colonIdx = retStr.indexOf(':');
  String cmd = retStr.substring(1, colonIdx);
  String param[5];
  uint8_t paramIdx = 0;
  
  String params = retStr.substring(colonIdx+1);
  //Serial.println("\n[" + params + "]"); 
  while(params.indexOf(',') > -1)
  {
    uint8_t commaIdx = params.indexOf(',');
    param[paramIdx] = params.substring(0, commaIdx);
    params = params.substring(commaIdx+1);
    ++paramIdx;
  }
  param[paramIdx] = params;

  if(cmd == "CPAS")
  {
    if(param[0] == " 0")
      pinEntryRequired = false;
  }
  else if(cmd == "PBREADY") { //after PIN entry
    isBooted = true;
  }
  else if(cmd == "CREG") { //network registered
    if(param[1] == "1") {
      hasSignal = true;
    }
  }
  else if(cmd == "XIIC")  { //GPRS link
    if(param[0] == "    1")
      establishedLink = true;
  }
  else if(cmd == "TCPSETUP") { //TCP connected
    if(param[0] == "1") {
      tcpConnected = true;
    }
    else if(param[0] == "Error n") {
      Serial.println("  TCP connection failed. Please retry...");
    }
  }
  else if(cmd == "TCPSEND") { //tcp connected
    if(param[0] == "1") {
      /*delayWithSerial(5000);
      sendAT("AT+TCPCLOSE=1");*/
    }
  }
  else if(cmd == "TCPCLOSE") { //tcp connected
    //handle TCP connection closing
  }
  else if(cmd == "DNS") { //DNS request result
    if(param[0] == "OK")
      dnsDone = true;
    else if(param[0] == "Error") {
      Serial.println("  DNS request failed!");
      dnsDone = true;
    }
    else if(resolvedIP = "") //only capture first DNS result
      resolvedIP = param[0];
  }
  else if(cmd == "TCPRECV")
  {
    
    if(param[0] == "1") {
      int len = param[1].toInt();
      Serial.println(param[2]);
    }
      
  }
  
}

void parseTCPData()
{
  bool isDone = false;
  byte commaCnt = 0;
  String tmpBuf;
  int dataLen;
  int rcvIdx = 0;
  while(!isDone)
  {
    if(gsmSerial.available())
    {
      char c = gsmSerial.read();
      if(commaCnt == 2) //when incoming chars represent data
      {
        if(rcvIdx < dataLen)
        {
          rcvBuf[rcvIdx] = c;
          ++rcvIdx;
        }
        else
          isDone = true;
      }
      else if(c == ',')
      {
        if (commaCnt == 0)
        {
          if(tmpBuf != "1")
          {
            Serial.println("Error during parsing.");
            break;
          }
          tmpBuf = "";
        }
        else if(commaCnt == 1) //parse length
        {
          dataLen = tmpBuf.toInt();
        }
        ++commaCnt;
      }
      else
        tmpBuf += c;
    }
  }
  Serial.println();
  Serial.println(rcvBuf);
}


String serBuf = "";
String retBuf = "";
bool isReturn = false;
void parseReturn(char c)
{
  if(c == '>')
    tcpSendRdy = true;
  else if(c == '+')
  {
    retBuf = "";
    isReturn = true;
  }
    
  if(isReturn)
  {
    if(c == ':')
    {
      if(retBuf == "+TCPRECV")
      {
        //parseTCPData();
        isReturn = false;
      }
    }
    if(c == '\r' || c == '\n')
    {
      isReturn = false;
      handleReturn(retBuf);
    }
    retBuf += char(c);
  }
}

void parseSerial()
{
  if(gsmSerial.available()) {
    char c = gsmSerial.read();
    Serial.write(c);
    parseReturn(c);
  }

  if(Serial.available()) {
    //gsmSerial.write(Serial.read());
    serBuf += char(Serial.read());
    if(serBuf.indexOf("\n") > -1)
    {
      sendAT(serBuf);
      serBuf = "";
    }
  }
}

void sendAT(String cmd) //with only one parameter, automatically set second parameter
{
  sendAT(cmd, true);
}

void sendAT(String cmd, bool waitForOK) //send AT command and wait for "OK/ERROR"
{
  Serial.println();
  Serial.print("< ");
  Serial.println(cmd);

  unsigned long sendATMillis = millis();
  String buf = "";

  gsmSerial.println(cmd);

  bool atReturnHandled = false;
  while (millis() - sendATMillis < 5000 && waitForOK) //timeout of 5s
  {
    if(gsmSerial.available()) {
      buf += char(gsmSerial.read());
    }

    //parse return commands (e.g. "+CPAS: 0")
    int retIdx = buf.indexOf("\r\n+");
    if(retIdx > -1 && !atReturnHandled)
    {
      String retStr = buf.substring(retIdx+2);
      if(retStr.indexOf("\r\n") > -1)
      {
        retStr = retStr.substring(0, retStr.length()-2);
        //Serial.println("\n[" + retStr + "]");
        atReturnHandled = true;
        handleReturn(retStr);
      }
    }
    if(buf.indexOf("OK") > -1 || buf.indexOf("ERROR") > -1)
    {
      //Serial.print(buf); //for the moment, only break loop
      break;
    }

  }
  Serial.print(buf);
  if(cmd.startsWith("AT+CPIN") && buf.indexOf("ERROR") > -1)
  {
    Serial.println("Wrong PIN entered");
  }
}


void initGSM()
{
  sendAT("AT+CPAS");
  if(pinEntryRequired)
  {
    sendAT("AT+CPIN=\"" + PIN + "\""); //enter PIN
    while(!isBooted) {
      parseSerial();
    }
  }
  //sendAT("AT+CREG?"); //check Network registration
  while(!hasSignal) {
    sendAT("AT+CREG?", false);
    delayWithSerial(500);
  }
  sendAT("AT+CMGF=1"); //set SMS text mode
  sendAT("AT+CSCS=\"HEX\""); //set HEX character set
  //gsmSerial.println("AT+CMGR=2"); Read SMS #2
}

void initGPRS()
{
  sendAT("AT+XISP=0"); //select internal TCP/IP protocol stack
  sendAT("AT+CGDCONT=1,\"IP\",\"" + APN + "\""); //set APN
  
  sendAT("AT+XIIC=1"); //establish link
  while (!establishedLink)
  {
    sendAT("AT+XIIC?");
    delayWithSerial(500);
  }
  
  //doTCP(connIP, connPort, connReq);
  doTCP(connHost, connReq);
}

void delayWithSerial(int ms)
{
  unsigned long delayTime = millis();
  while (millis() - delayTime <= ms)
  {
    parseSerial();
  }
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println(">> Elektronik/GSM/GSM_status <<");
  gsmSerial.begin(9600);
  sendAT("AT");
  initGSM();
  initGPRS();
}

void loop() {
  // put your main code here, to run repeatedly:
  parseSerial();

}

