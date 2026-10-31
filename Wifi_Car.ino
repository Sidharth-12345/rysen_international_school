/*
  WiFi Kids Car - ESP8266 (NodeMCU) + L298N Motor Driver
  -------------------------------------------------------
  ESP8266 khud ka WiFi (Access Point) banata hai.
  Phone se WiFi connect karo -> browser me 192.168.4.1 kholo.

  WIRING (NodeMCU -> L298N):
    D1 (GPIO5)  -> ENA   (left motor speed)
    D5 (GPIO14) -> IN1
    D6 (GPIO12) -> IN2
    D2 (GPIO4)  -> ENB   (right motor speed)
    D7 (GPIO13) -> IN3
    D4 (GPIO2)  -> IN4   (pehle D0 tha, ab D4 - yahi problem thi)
    GND (NodeMCU) -> GND (L298N)   <-- common ground zaroori hai!
    L298N 12V pin -> Battery +  (6-12V)
    L298N ENA/ENB ke jumper hata do (warna speed control nahi hoga)
*/

#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>

// ---------- WiFi settings ----------
const char* AP_SSID = "KidsCar";
const char* AP_PASS = "12345678";      // kam se kam 8 characters

// ---------- Pins ----------
const int ENA = 5;    // D1
const int IN1 = 14;   // D5
const int IN2 = 12;   // D6
const int ENB = 4;    // D2
const int IN3 = 13;   // D7
const int IN4 = 2;    // D4 (GPIO2)

// ---------- Safety (bachon ke liye) ----------
const int  MAX_SPEED_PERCENT = 80;     // car is se tez nahi chalegi
const int  MIN_PWM = 450;              // isse kam par motor start nahi hoti
const unsigned long FAILSAFE_MS = 400; // itni der command na aaye to car ruk jaye

ESP8266WebServer server(80);
unsigned long lastCmd = 0;
bool moving = false;

// ---------- Motor control ----------
void setMotor(int in_a, int in_b, int en, int dir, int pwm) {
  digitalWrite(in_a, dir > 0 ? HIGH : LOW);
  digitalWrite(in_b, dir < 0 ? HIGH : LOW);
  analogWrite(en, dir == 0 ? 0 : pwm);
}

void driveCar(char d, int speedPercent) {
  speedPercent = constrain(speedPercent, 0, 100);
  int maxPwm = 1023 * MAX_SPEED_PERCENT / 100;
  int pwm = map(speedPercent, 0, 100, 0, maxPwm);
  pwm = constrain(pwm, MIN_PWM, maxPwm);   // bahut kam PWM par motor ruk jati hai
  int turnPwm = pwm;                       // turn me bhi poori power (right/left same)

  switch (d) {
    case 'F': setMotor(IN1, IN2, ENA, 1, pwm);      setMotor(IN3, IN4, ENB, 1, pwm);      break;
    case 'B': setMotor(IN1, IN2, ENA, -1, pwm);     setMotor(IN3, IN4, ENB, -1, pwm);     break;
    case 'L': setMotor(IN1, IN2, ENA, -1, turnPwm); setMotor(IN3, IN4, ENB, 1, turnPwm);  break;
    case 'R': setMotor(IN1, IN2, ENA, 1, turnPwm);  setMotor(IN3, IN4, ENB, -1, turnPwm); break;
    default:  setMotor(IN1, IN2, ENA, 0, 0);        setMotor(IN3, IN4, ENB, 0, 0);        break;
  }
  moving = (d == 'F' || d == 'B' || d == 'L' || d == 'R');
}

// ---------- Web page ----------
const char PAGE[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="hi">
<head>
<meta charset="UTF-8">
<meta name="viewport" content="width=device-width, initial-scale=1, maximum-scale=1, user-scalable=no">
<title>Kids Car</title>
<style>
  *{box-sizing:border-box;-webkit-tap-highlight-color:transparent;user-select:none;-webkit-user-select:none;touch-action:none}
  body{margin:0;min-height:100vh;font-family:system-ui,Arial,sans-serif;color:#fff;
       background:linear-gradient(160deg,#1e3c72,#2a5298 50%,#6a3093);
       display:flex;flex-direction:column;align-items:center;justify-content:center;gap:18px;padding:16px}
  h1{margin:0;font-size:1.6rem;text-shadow:0 2px 6px rgba(0,0,0,.35)}
  #status{font-size:.9rem;padding:4px 14px;border-radius:20px;background:rgba(0,0,0,.3)}
  #status.ok::before{content:"● ";color:#4ade80}
  #status.bad::before{content:"● ";color:#f87171}
  .pad{display:grid;grid-template-columns:repeat(3,1fr);gap:12px;width:min(88vw,340px)}
  .btn{aspect-ratio:1;border:none;border-radius:24px;font-size:2.6rem;color:#fff;
       box-shadow:0 8px 0 rgba(0,0,0,.3);transition:transform .05s,box-shadow .05s}
  .btn:active,.btn.on{transform:translateY(6px);box-shadow:0 2px 0 rgba(0,0,0,.3);filter:brightness(1.25)}
  .fwd{background:#22c55e;grid-column:2}
  .lft{background:#3b82f6;grid-column:1;grid-row:2}
  .stp{background:#ef4444;grid-column:2;grid-row:2;font-size:1.4rem;font-weight:700}
  .rgt{background:#f59e0b;grid-column:3;grid-row:2}
  .bck{background:#a855f7;grid-column:2;grid-row:3}
  .speed{width:min(88vw,340px);background:rgba(0,0,0,.25);padding:14px 18px;border-radius:20px;text-align:center}
  .speed label{display:block;margin-bottom:8px;font-weight:600}
  input[type=range]{width:100%;height:34px;accent-color:#fde047;touch-action:pan-x}
</style>
</head>
<body>
  <h1>🚗 Meri Car</h1>
  <div id="status" class="bad">Connecting...</div>

  <div class="pad">
    <button class="btn fwd" data-d="F">▲</button>
    <button class="btn lft" data-d="L">◀</button>
    <button class="btn stp" data-d="S">STOP</button>
    <button class="btn rgt" data-d="R">▶</button>
    <button class="btn bck" data-d="B">▼</button>
  </div>

  <div class="speed">
    <label>Speed: <span id="sv">50</span>%</label>
    <input id="spd" type="range" min="20" max="100" value="50">
  </div>

<script>
  let speed = 50, timer = null, active = null;
  const st = document.getElementById('status');
  const spd = document.getElementById('spd');
  const sv = document.getElementById('sv');

  spd.addEventListener('input', () => { speed = +spd.value; sv.textContent = speed; });

  function send(d){
    fetch('/cmd?d=' + d + '&s=' + speed, {cache:'no-store'})
      .then(() => { st.textContent = 'Connected'; st.className = 'ok'; })
      .catch(() => { st.textContent = 'Disconnected'; st.className = 'bad'; });
  }
  function start(btn){
    const d = btn.dataset.d;
    if (d === 'S') { stop(); return; }
    stop(true);
    active = btn; btn.classList.add('on');
    send(d);
    timer = setInterval(() => send(d), 150);   // jab tak button dabaya hai
  }
  function stop(silent){
    if (timer) { clearInterval(timer); timer = null; }
    if (active) { active.classList.remove('on'); active = null; }
    if (!silent) send('S');
  }

  document.querySelectorAll('.btn').forEach(b => {
    b.addEventListener('pointerdown', e => { e.preventDefault(); b.setPointerCapture(e.pointerId); start(b); });
    b.addEventListener('pointerup', () => { if (b.dataset.d !== 'S') stop(); });
    b.addEventListener('pointercancel', () => stop());
    b.addEventListener('contextmenu', e => e.preventDefault());
  });
  window.addEventListener('blur', () => stop());
  document.addEventListener('visibilitychange', () => { if (document.hidden) stop(); });
  setInterval(() => { if (!timer) send('P'); }, 2000);   // connection check
</script>
</body>
</html>
)rawliteral";

// ---------- Handlers ----------
void handleRoot() { server.send_P(200, "text/html", PAGE); }

void handleCmd() {
  String d = server.arg("d");
  int s = server.hasArg("s") ? server.arg("s").toInt() : 50;
  char c = d.length() ? d[0] : 'S';
  if (c == 'P') {                       // sirf ping, motors ko mat chhedo
    server.send(200, "text/plain", "OK");
    return;
  }
  driveCar(c, s);
  lastCmd = millis();
  server.send(200, "text/plain", "OK");
}

void setup() {
  pinMode(ENA, OUTPUT); pinMode(ENB, OUTPUT);
  pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
  pinMode(IN3, OUTPUT); pinMode(IN4, OUTPUT);
  analogWriteRange(1023);
  analogWriteFreq(1000);
  driveCar('S', 0);

  Serial.begin(115200);
  WiFi.mode(WIFI_AP);
  WiFi.softAP(AP_SSID, AP_PASS);
  Serial.print("WiFi: "); Serial.println(AP_SSID);
  Serial.print("Page : http://"); Serial.println(WiFi.softAPIP());

  server.on("/", handleRoot);
  server.on("/cmd", handleCmd);
  server.begin();
}

void loop() {
  server.handleClient();
  // Failsafe: WiFi toot jaye ya phone band ho jaye to car ruk jaye
  if (moving && millis() - lastCmd > FAILSAFE_MS) {
    driveCar('S', 0);
  }
}
