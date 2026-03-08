#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <Preferences.h>
#include <SinricPro.h>
#include <SinricProSwitch.h>

// ==========================================
// 1. YOUR NETWORK & CLOUD CREDENTIALS
// ==========================================
const char* ssid = "NOKIA3310";       
const char* password = "asmaasma4";

#define APP_KEY           "1b5a2484-60d7-4299-889b-46aeec5620ba"
#define APP_SECRET        "f7ff3565-7f09-469d-a362-62a53372fa6e-3cc2cd1f-ee7f-418c-a3ba-ce658abc7da4"
#define SWITCH_ID_1       "69ac6dcdc2dbd7108b388e17" 
#define SWITCH_ID_2       "69ac6d0d17b32c0941ceb01d" 
#define SWITCH_ID_3       "69ac6dfa17b32c0941ceb11c" 

// ==========================================

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81); 
Preferences preferences; 

String masterPassword;

int relayPins[16] = {16, 17, 21, 22, 23, 25, 26, 27, 32, 33, 18, 19, 5, 4, 15, 2};
bool relayStates[16] = {false};
String relayNames[16]; 
bool relayVisible[16] = {true}; 

enum SeqMode { NONE, SEQ_ON, SEQ_OFF };
SeqMode currentMode = NONE;
int seqIndex = 0;
unsigned long lastSeqTime = 0;
const int seqDelay = 150; 

// =========================================================================
// PROGMEM HTML BLOCKS - AUTO-ALIGN GOLD EDITION
// =========================================================================
const char HTML_HEAD[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1.0, maximum-scale=1.0, user-scalable=no">
  <title>Commander Pro</title>
  <style>
    @import url('https://fonts.googleapis.com/css2?family=Inter:wght@400;600;800&family=Orbitron:wght@500;900&display=swap');
    :root { --bg: #05070a; --surface: rgba(15, 20, 30, 0.4); --primary: #0ea5e9; --on-color: #10b981; --gold: #d4af37; }
    * { box-sizing: border-box; font-family: 'Inter', sans-serif; -webkit-tap-highlight-color: transparent; user-select: none; }
    body { background-color: var(--bg); color: #f8fafc; margin: 0; min-height: 100vh; display: flex; flex-direction: column; align-items: center; overflow-x: hidden; padding-bottom: 50px; }
    
    .topbar { width: 100%; padding: 18px 25px; display: flex; justify-content: space-between; align-items: center; background: rgba(5, 7, 10, 0.7); backdrop-filter: blur(15px); position: sticky; top: 0; z-index: 50; border-bottom: 1px solid rgba(255,255,255,0.03); }
    .logo { font-size: 20px; font-weight: 900; letter-spacing: 1px; background: linear-gradient(to right, #fff, #94a3b8); -webkit-background-clip: text; -webkit-text-fill-color: transparent;}
    .dot { width: 8px; height: 8px; border-radius: 50%; background: #ef4444; box-shadow: 0 0 8px rgba(239, 68, 68, 0.6);}
    .dot.connected { background: var(--on-color); box-shadow: 0 0 12px var(--on-color); }
    .user-avatar { width: 38px; height: 38px; border-radius: 50%; background: #1e293b; border: 2px solid rgba(14, 165, 233, 0.5); display: flex; align-items: center; justify-content: center; font-weight: 800; cursor: pointer; background-size: cover; }
    
    .master-controls { display: flex; gap: 12px; margin: 20px 0; width: 100%; max-width: 600px; padding: 0 20px; }
    .btn-master { flex: 1; padding: 15px; border-radius: 12px; border: none; font-weight: 800; font-size: 11px; cursor: pointer; backdrop-filter: blur(10px); transition: 0.3s; }
    .btn-master.on { background: rgba(14, 165, 233, 0.1); color: #0ea5e9; border: 1px solid rgba(14, 165, 233, 0.2); }
    .btn-master.off { background: rgba(30, 41, 59, 0.3); color: #94a3b8; border: 1px solid rgba(255, 255, 255, 0.05); }

    .section-label { width: 100%; max-width: 600px; padding: 0 20px; font-family: 'Orbitron', sans-serif; font-size: 9px; color: #64748b; font-weight: 600; letter-spacing: 2px; display: flex; align-items: center; gap: 8px; margin-top: 15px; margin-bottom: 10px; text-transform: uppercase; }
    .label-gold { color: var(--gold); }

    .premium-container { width: 100%; max-width: 600px; padding: 0 20px; display: flex; flex-direction: column; gap: 12px; }
    .premium-btn { width: 100%; min-height: 80px !important; flex-direction: row !important; justify-content: flex-start !important; padding: 0 25px !important; border: 1px solid rgba(212,175,55,0.2) !important; background: linear-gradient(90deg, rgba(212,175,55,0.05), transparent) !important;}
    .premium-btn .power-icon { margin-bottom: 0 !important; margin-right: 20px; width: 24px; height: 24px; }
    .premium-btn .relay-name { text-align: left !important; font-size: 14px !important; letter-spacing: 1px; }
    .premium-btn .mic-icon { position: static !important; margin-left: auto; opacity: 0.3; }

    .local-grid { 
      display: grid; 
      grid-template-columns: repeat(auto-fit, minmax(140px, 1fr)); 
      gap: 12px; 
      width: 100%; 
      max-width: 600px; 
      padding: 0 20px;
      justify-content: center;
    }

    .relay-btn { 
      background: var(--surface); border: 1px solid rgba(255,255,255,0.03); backdrop-filter: blur(12px); 
      border-radius: 14px; padding: 20px 10px; display: flex; flex-direction: column; align-items: center; 
      justify-content: center; cursor: pointer; transition: all 0.3s cubic-bezier(0.4, 0, 0.2, 1); min-height: 100px; 
      position: relative;
    }
    .relay-btn.on { transform: translateY(-2px); }
    .relay-btn.on.standard { background: rgba(16, 185, 129, 0.1); border-color: rgba(16,185,129,0.4); }
    .relay-btn.on.premium-btn { background: rgba(212,175,55,0.15) !important; border-color: rgba(212,175,55,0.6) !important; }
    
    .power-icon { width: 28px; height: 28px; fill: none; stroke: #475569; stroke-width: 2.5; transition: 0.3s; margin-bottom: 10px;}
    .relay-btn.on.standard .power-icon { stroke: var(--on-color); filter: drop-shadow(0 0 8px var(--on-color)); }
    .relay-btn.on.premium-btn .power-icon { stroke: #fbbf24; filter: drop-shadow(0 0 8px #fbbf24); }

    .relay-name { font-size: 11px; font-weight: 700; color: #94a3b8; text-transform: uppercase; }
    .relay-btn.on .relay-name { color: white; }

    .action-grp { position: absolute; top: 8px; right: 8px; display: none; gap: 4px; }
    body.admin-mode .action-grp { display: flex; }
    .action-btn { background: rgba(0,0,0,0.5); border: none; color: #94a3b8; padding: 4px; border-radius: 6px; font-size: 10px; }

    .hidden-card { display: none !important; }
    body.admin-mode .hidden-card { display: flex !important; opacity: 0.2; border: 1px dashed gray; }

    .modal-overlay { position: fixed; top: 0; left: 0; width: 100%; height: 100%; background: rgba(0,0,0,0.9); backdrop-filter: blur(10px); display: none; align-items: center; justify-content: center; z-index: 1000; }
    .modal-overlay.active { display: flex; }
    .modal { background: #0f172a; border-radius: 20px; padding: 30px; width: 85%; max-width: 320px; border: 1px solid rgba(255,255,255,0.05); }
    .modal h3 { margin: 0 0 20px 0; font-size: 16px; text-align: center; color: var(--gold); }
    .modal input { width: 100%; padding: 12px; border-radius: 10px; border: 1px solid #334155; background: #05070a; color: white; margin-bottom: 20px; outline: none; }
    .modal-btn { width: 100%; padding: 12px; border-radius: 10px; border: none; font-weight: 700; cursor: pointer; margin-bottom: 10px;}
    .btn-save { background: var(--primary); color: white; }
    .btn-cancel { background: transparent; color: #64748b; }
  </style>
  <script>
)rawliteral";

const char HTML_SCRIPT_END[] PROGMEM = R_rawliteral(
    var ws; var isAdmin = false; var currentEditId = -1;
    function connect() {
      ws = new WebSocket('ws://' + window.location.hostname + ':81/');
      ws.onopen = () => document.getElementById('status-dot').className = 'dot connected';
      ws.onclose = () => { document.getElementById('status-dot').className = 'dot'; setTimeout(connect, 2000); };
      ws.onmessage = (e) => {
        var d = e.data.split(':');
        if(d[0]=='STA'){
          var id=d[1], s=d[2], b=document.getElementById('c'+id);
          if(s=='1') b.classList.add('on'); else b.classList.remove('on');
        } else if(d[0]=='RNM') document.getElementById('t'+d[1]).innerText=d.slice(2).join(':');
        else if(d[0]=='VIS') { if(d[2]=='1') document.getElementById('c'+d[1]).classList.remove('hidden-card'); else document.getElementById('c'+d[1]).classList.add('hidden-card'); }
        else if(d[0]=='VLD') { if(d[1]=='1'){ isAdmin=true; document.body.classList.add('admin-mode'); document.getElementById('passOverlay').classList.remove('active'); } }
      };
    }
    window.onload=()=>{ connect(); };
    function tgl(id){ ws.send('TGL:'+id); }
    function mstr(c){ ws.send(c); }
    function openRename(id, e){ e.stopPropagation(); currentEditId=id; document.getElementById('editInput').value=document.getElementById('t'+id).innerText; document.getElementById('renameOverlay').classList.add('active'); }
    function saveName(){ ws.send('REN:'+currentEditId+':'+document.getElementById('editInput').value); document.getElementById('renameOverlay').classList.remove('active'); }
    function handleAvatar(){ if(!isAdmin) document.getElementById('passOverlay').classList.add('active'); else { isAdmin=false; document.body.classList.remove('admin-mode'); } }
    function verifyAdmin(){ ws.send('CHK:'+document.getElementById('passInput').value); }
  </script>
</head>
<body>
  <div class="topbar">
    <div class="logo">COMMANDER PRO</div>
    <div style="display:flex; align-items:center; gap:15px;">
      <div id="status-dot" class="dot"></div>
      <div class="user-avatar" id="avatar" onclick="handleAvatar()">A</div>
    </div>
  </div>

  <div class="master-controls">
    <button class="btn-master on" onclick="mstr('SEQ_ON')">MASTER ON</button>
    <button class="btn-master off" onclick="mstr('SEQ_OFF')">MASTER OFF</button>
  </div>

  <div class="section-label label-gold">
    <svg style="width:12px; height:12px;" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M12 2a3 3 0 0 0-3 3v7a3 3 0 0 0 6 0V5a3 3 0 0 0-3-3Z"></path><path d="M19 10v2a7 7 0 0 1-14 0v-2"></path></svg>
    Cloud Voice Link
  </div>
  
  <div class="premium-container">
)rawliteral";

const char HTML_FOOTER[] PROGMEM = R"rawliteral(
  <div id="renameOverlay" class="modal-overlay">
    <div class="modal">
      <h3>Rename Channel</h3>
      <input type="text" id="editInput" maxlength="15">
      <button class="modal-btn btn-save" onclick="saveName()">Apply Change</button>
      <button class="modal-btn btn-cancel" onclick="document.getElementById('renameOverlay').classList.remove('active')">Cancel</button>
    </div>
  </div>

  <div id="passOverlay" class="modal-overlay">
    <div class="modal">
      <h3>Admin Access</h3>
      <input type="password" id="passInput" placeholder="Password">
      <button class="modal-btn btn-save" onclick="verifyAdmin()">Unlock</button>
      <button class="modal-btn btn-cancel" onclick="document.getElementById('passOverlay').classList.remove('active')">Close</button>
    </div>
  </div>
</body>
</html>
)rawliteral";

// =========================================================================
// ESP32 CORE LOGIC
// =========================================================================

void broadcastState(int id) {
  String msg = "STA:" + String(id) + ":" + String(relayStates[id] ? "1" : "0");
  webSocket.broadcastTXT(msg);
}

// Fixed Callback logic for SinricPro 4.1.0+
bool onPowerState0(const String &deviceId, bool &state) { relayStates[0] = state; digitalWrite(relayPins[0], state ? HIGH : LOW); broadcastState(0); return true; }
bool onPowerState1(const String &deviceId, bool &state) { relayStates[1] = state; digitalWrite(relayPins[1], state ? HIGH : LOW); broadcastState(1); return true; }
bool onPowerState2(const String &deviceId, bool &state) { relayStates[2] = state; digitalWrite(relayPins[2], state ? HIGH : LOW); broadcastState(2); return true; }

void handleRoot() {
  server.setContentLength(CONTENT_LENGTH_UNKNOWN);
  server.send(200, "text/html", "");
  server.sendContent(HTML_HEAD);
  
  for(int i=0; i<3; i++) {
    String v = relayVisible[i]?"":" hidden-card";
    String s = relayStates[i]?" on":"";
    String card = "<div class='relay-btn premium-btn"+s+v+"' id='c"+String(i)+"' onclick='tgl("+String(i)+")'>";
    card += "<svg class='power-icon' viewBox='0 0 24 24'><path d='M18.36 6.64a9 9 0 1 1-12.73 0'></path><line x1='12' y1='2' x2='12' y2='12'></line></svg>";
    card += "<div class='relay-name' id='t"+String(i)+"'>"+relayNames[i]+"</div>";
    card += "<svg class='mic-icon' viewBox='0 0 24 24' fill='none' stroke='currentColor' stroke-width='2'><path d="M12 2a3 3 0 0 0-3 3v7a3 3 0 0 0 6 0V5a3 3 0 0 0-3-3Z"></path><path d="M19 10v2a7 7 0 0 1-14 0v-2"></path></svg>";
    card += "<div class='action-grp'><button class='action-btn' onclick='openRename("+String(i)+",event)'>EDIT</button></div></div>";
    server.sendContent(card);
  }
  
  server.sendContent("</div><div class='section-label'>Local Terminals</div><div class='local-grid'>");
  
  for(int i=3; i<16; i++) {
    String v = relayVisible[i]?"":" hidden-card";
    String s = relayStates[i]?" on":"";
    String card = "<div class='relay-btn standard"+s+v+"' id='c"+String(i)+"' onclick='tgl("+String(i)+")'>";
    card += "<div class='action-grp'><button class='action-btn' onclick='openRename("+String(i)+",event)'>EDIT</button></div>";
    card += "<svg class='power-icon' viewBox='0 0 24 24'><path d='M18.36 6.64a9 9 0 1 1-12.73 0'></path><line x1='12' y1='2' x2='12' y2='12'></line></svg>";
    card += "<div class='relay-name' id='t"+String(i)+"'>"+relayNames[i]+"</div></div>";
    server.sendContent(card);
  }
  
  server.sendContent("</div>");
  server.sendContent(HTML_FOOTER);
}

void processCommand(String msg) {
  if(msg.startsWith("TGL:")) {
    int id = msg.substring(4).toInt();
    relayStates[id] = !relayStates[id];
    digitalWrite(relayPins[id], relayStates[id]?HIGH:LOW);
    broadcastState(id);
    
    // Fixed Cloud Update for SinricPro 4.1.0+
    if(id==0) { SinricProSwitch &mySwitch = SinricPro[SWITCH_ID_1]; mySwitch.sendPowerStateEvent(relayStates[0]); }
    if(id==1) { SinricProSwitch &mySwitch = SinricPro[SWITCH_ID_2]; mySwitch.sendPowerStateEvent(relayStates[1]); }
    if(id==2) { SinricProSwitch &mySwitch = SinricPro[SWITCH_ID_3]; mySwitch.sendPowerStateEvent(relayStates[2]); }
  }
  else if(msg == "SEQ_ON") { currentMode = SEQ_ON; seqIndex = 0; lastSeqTime = millis(); }
  else if(msg == "SEQ_OFF") { currentMode = SEQ_OFF; seqIndex = 0; lastSeqTime = millis(); }
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if(type == WStype_TEXT) {
    String msg = (char*)payload;
    if(msg.startsWith("TGL:") || msg.startsWith("SEQ")) processCommand(msg);
    else if(msg.startsWith("REN:")) {
      int c1 = msg.indexOf(':'), c2 = msg.indexOf(':', c1+1);
      int id = msg.substring(c1+1, c2).toInt(); String n = msg.substring(c2+1);
      relayNames[id]=n; preferences.putString(String(id).c_str(), n);
      webSocket.broadcastTXT("RNM:"+String(id)+":"+n);
    }
    else if(msg.startsWith("CHK:")) { if(msg.substring(4)==masterPassword) webSocket.sendTXT(num, "VLD:1"); }
  }
}

void setup() {
  preferences.begin("relays", false);
  masterPassword = preferences.getString("pwd", "9893316346");
  for(int i=0; i<16; i++) {
    pinMode(relayPins[i], OUTPUT); digitalWrite(relayPins[i], LOW);
    relayNames[i] = preferences.getString(String(i).c_str(), "Channel " + String(i+1));
  }
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) delay(500);
  
  // Fixed setup for SinricPro 4.1.0+
  SinricProSwitch& mySwitch1 = SinricPro[SWITCH_ID_1];
  mySwitch1.onPowerState(onPowerState0);
  SinricProSwitch& mySwitch2 = SinricPro[SWITCH_ID_2];
  mySwitch2.onPowerState(onPowerState1);
  SinricProSwitch& mySwitch3 = SinricPro[SWITCH_ID_3];
  mySwitch3.onPowerState(onPowerState2);

  SinricPro.begin(APP_KEY, APP_SECRET);

  server.on("/", handleRoot);
  server.begin();
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  server.handleClient();
  webSocket.loop();
  SinricPro.handle();
  
  if (currentMode != NONE && millis() - lastSeqTime > seqDelay) {
    if (seqIndex < 16) {
      relayStates[seqIndex] = (currentMode == SEQ_ON);
      digitalWrite(relayPins[seqIndex], relayStates[seqIndex]?HIGH:LOW);
      broadcastState(seqIndex);
      seqIndex++; lastSeqTime = millis();
    } else currentMode = NONE;
  }
}
