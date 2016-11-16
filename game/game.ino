#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <LiquidCrystal.h>

const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
DNSServer dnsServer;
ESP8266WebServer webServer(80);

LiquidCrystal lcd(0,2,4,14,12,13);

String main = ""
"<!DOCTYPE html>\n"
"<html>\n"
"<head>\n"
"<meta charset=\"utf-8\">\n"
"<style>\n"
"html, body {\n"
"  overflow: hidden;\n"
"  background:#aaa;\n"
"  height:100%;\n"
"  border:0px;\n"
"  padding:0px;\n"
"}\n"
"\n"
"canvas {\n"
"  display:block;\n"
"  background:#fff;\n"
"  border:0px;\n"
"  padding:0px;\n"
"}\n"
"\n"
"#sendscore {\n"
"  padding:50px;\n"
"  position: absolute;\n"
"  background: rgba(255, 255, 255, 0.3);\n"
"  display:none;\n"
"}\n"
"</style>\n"
"</head>\n"
"<body onload=\"init()\">\n"
"<div id=\"sendscore\">\n"
"  <h1>Game over</h1>\n"
"  <h2 id=\"scoretext\"></h2>\n"
"  <form action=\"newScore.php\" method=\"post\">\n"
"    Name:<br>\n"
"    <input type=\"text\" name=\"user\" id=\"user\" value=\"\"><br>\n"
"    <input type=\"hidden\" name=\"score\" id=\"score\" value=\"\">\n"
"    <input type=\"submit\" value=\"Submit\" onclick=\"document.cookie = 'user=' + document.getElementById('user').value\">\n"
"  </form>"
"</div>\n"
"<canvas id=\"paint\">\n"
"Sorry, your browser does not support canvas. Try an other browser.\n"
"</canvas>\n"
"\n"
"<script>\n"
"var canvas;\n"
"var ctx;\n"
"var intervl;\n"
"var W;\n"
"var H;\n"
"var lane; //-1 for left, 0 for middle, 1 for right\n"
"var speed;\n"
"var distance;\n"
"var obst;\n"
"var score;\n"
"var lifes;\n"
"var shield;\n"
"\n"
"var MAX_OBSTACLES = 1000;\n"
"var acc = 1.004;\n"
"var carColor = \"#0000ff\";\n"
"var shieldColor = \"#00ffff\";\n"
"var posObColor = \"#00ff00\";\n"
"var negObColor = \"#ff0000\";\n"
"var bgrColor = \"#ffffff\";\n"
"\n"
"function init() {\n"
"  //save global variables\n"
"  canvas = document.getElementById(\"paint\");\n"
"  ctx = canvas.getContext(\"2d\");\n"
"  lane = 0;\n"
"  speed = 0.01;\n"
"  distance=0;\n"
"  score = 0;\n"
"  lifes = 5;\n"
"  shield = 0;\n"
"  obst = generateObstacles();\n"
"  //make it fullscreen\n"
"  canvas.width = document.body.clientWidth;\n"
"  canvas.height = document.body.clientHeight;\n"
"  W = canvas.width;\n"
"  H = canvas.height;\n"
" \n"
"  canvas.addEventListener(\"touchstart\", touchStart, false);\n"
"  canvas.addEventListener(\"touchmove\", touchMove, false);\n"
"  canvas.addEventListener(\"touchend\", touchEnd, false);\n"
"\n"
"  if( canvas.getContext ) {\n"
"    setup();\n"
"    intervl = setInterval( run , 33 );\n"
"  }\n"
"}\n"
"\n"
"function generateObstacles() {\n"
"  var ob = [];\n"
"  for (var i=0; i<MAX_OBSTACLES; i++) {\n"
"    ob.push(Math.round(Math.random() * 30))\n"
"  }\n"
"  console.log(ob);\n"
"  return ob;\n"
"}\n"
"\n"
"function setup() {\n"
"  run();\n"
"}\n"
"\n"
"function run() {\n"
"  distance+=speed;\n"
"  if(shield >= 0) shield-=speed;\n"
"  speed *= acc;\n"
"  ctx.clearRect(0, 0, W, H);\n"
"  drawObstacles();\n"
"  if(shield < 0) calculateCrash();\n"
"  drawCar();\n"
"  writeText();\n"
"  if(lifes < 1) gameOver();\n"
"}\n"
"\n"
"function gameOver() {\n"
"  clearInterval(intervl);\n"
"  acc = 0;\n"
"  speed = 0;\n"
"  \n"
"  document.getElementById(\"scoretext\").innerHTML = \"Your score: \" + score;\n"
"  if(document.cookie) document.getElementById(\"user\").value = document.cookie.substring(5)  ;\n"
"  document.getElementById(\"score\").value = score;\n"
"  document.getElementById(\"sendscore\").style.display = \"block\";\n"
"}\n"
"\n"
"function writeText() {\n"
"  ctx.font = \"20px Georgia\";\n"
"  ctx.fillText(\"Score: \" + score + \", Life: \" + \"♥\".repeat(lifes), 10, 20);\n"
"}\n"
"\n"
"function calculateCrash() {\n"
"  //top 2 corner of the car should be white\n"
"  var topleftPx = GetPixel((0.35+0.35*lane)*W+10,0.9*H);\n"
"  var toprightPx = GetPixel((0.65+0.35*lane)*W-10,0.9*H);\n"
"  var btmleftPx = GetPixel((0.35+0.35*lane)*W+10,0.99*H);\n"
"  var btmrightPx = GetPixel((0.65+0.35*lane)*W-10,0.99*H);\n"
"  if(topleftPx == posObColor || toprightPx == posObColor || btmleftPx == posObColor || btmrightPx == posObColor) score += 100;\n"
"  if(topleftPx == negObColor || toprightPx == negObColor || btmleftPx == negObColor || btmrightPx == negObColor) {\n"
"    lifes -= 1;\n"
"    shield = 0.3;\n"
"  } \n"
"}\n"
"\n"
"function GetPixel(x, y)\n"
"{\n"
"    var p = ctx.getImageData(x, y, 1, 1).data; \n"
"    var hex = \"#\" + (\"000000\" + rgbToHex(p[0], p[1], p[2])).slice(-6);  \n"
"    return hex;\n"
"}\n"
"\n"
"function rgbToHex(r, g, b) {\n"
"    if (r > 255 || g > 255 || b > 255)\n"
"        throw \"Invalid color component\";\n"
"    return ((r << 16) | (g << 8) | b).toString(16);\n"
"}\n"
"\n"
"function drawCar() {\n"
"  if(shield < 0)\n"
"    ctx.fillStyle = carColor;\n"
"  else\n"
"    ctx.fillStyle = shieldColor;\n"
"  ctx.fillRect((0.35+0.35*lane)*W,0.9*H,0.3*W,0.1*H);\n"
"}\n"
"\n"
"function drawObstacles() {\n"
"  for(var i=0; i<6; i++) {\n"
"    var ob = obst[i+Math.floor(distance)*3];\n"
"    var type = getType(ob);\n"
"    if(type == 0) continue;\n"
"    var oblane = getLane(ob);\n"
"    if(type == 1) ctx.fillStyle = posObColor;\n"
"    else ctx.fillStyle = negObColor;\n"
"    ctx.fillRect((0.35+0.35*oblane)*W,(1-i/3 + distance%1)*H,0.3*W,0.2*H);\n"
"  }\n"
"}\n"
"\n"
"function touchStart(event) {\n"
"  var touchX = event.targetTouches[0].pageX;\n"
"  if(touchX < 0.3*W) lane = -1;\n"
"  else if(touchX > 0.7*W) lane = 1;\n"
"  else lane = 0; \n"
"}\n"
"\n"
"function touchEnd(event) {\n"
"  lane = 0;\n"
"}\n"
"\n"
"function touchMove(event) {\n"
"  touchEnd(event);\n"
"  touchStart(event);\n"
"}\n"
"\n"
"//for non-touch-screens\n"
"function checkKey(e) {\n"
" e = e || window.event;\n"
" if(e.keyCode == 37) //left\n"
"   lane = -1;\n"
" if(e.keyCode == 39) //right\n"
"   lane = 1;\n"
"}\n"
"document.onkeydown = checkKey;\n"
"\n"
"function keyup(e) {\n"
"  lane = 0;\n"
"}\n"
"document.onkeyup = keyup;\n"
"\n"
"function getLane(ob) {\n"
"  if(ob < 10) return -1;\n"
"  if(ob < 20) return 0;\n"
"  return 1;\n"
"}\n"
"\n"
"function getType(ob) {\n"
"  var m = ob % 10;\n"
"  if(m < 4) return 0;\n"
"  if(m < 7) return -1;\n"
"  return 1;\n"
"}\n"
"</script>\n"
"</body>\n"
"</html>";

int maxscore = 0;

void setup() {
  Serial.begin(115200);
  lcd.begin(16, 2);
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP("Let's play a game");

  // if DNSServer is started with "*" for domain name, it will reply with
  // provided IP to all DNS request
  dnsServer.start(DNS_PORT, "*", apIP);

  // redirect all requests to the login page
  webServer.onNotFound([]() {
    webServer.sendHeader("Location", String("http://www.playhere.eit/"), true);
    webServer.send ( 302, "text/plain", "");
  });
  
  webServer.on("/", []() {
    if(webServer.hostHeader().indexOf("playhere.eit") == -1) {
      webServer.sendHeader("Location", String("http://www.playhere.eit/"), true);
      webServer.send ( 302, "text/plain", "");
    } else {
      webServer.send(200, "text/html", main);
    }
  });

  webServer.on("/newScore.php", []() {
    if(webServer.args() > 0) {
      String user;
      String score;
      if(webServer.argName(0) == "user") {
        user = webServer.arg(0);
        score = webServer.arg(1);
      } else {
        user = webServer.arg(1);
        score = webServer.arg(0);    
      }
      lcd.setCursor(0,0);
      lcd.print(user.substring(0,9));
      lcd.print("-");
      lcd.print(score);
      lcd.print("                  ");
      Serial.println(score);
      Serial.println(score.toInt());
      if(score.toInt() > maxscore) {
        maxscore = score.toInt();
        lcd.setCursor(0,1);
        lcd.print(user.substring(0,9));
        lcd.print("-");
        lcd.print(score);
        lcd.print("                  ");
      }
    }
    
    webServer.sendHeader("Location", String("http://www.playhere.eit/"), true);
    webServer.send ( 302, "text/plain", "");
  });
  
  webServer.begin();
}

void loop() {
  dnsServer.processNextRequest();
  webServer.handleClient();
}
