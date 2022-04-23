#include "LedControl.h"
#include <arduino-timer.h>
#include <WiFi.h>
#include <WebServer.h>
#define MAX 8

LedControl lc=LedControl(36,40,26,1);
auto timer = timer_create_default();
WebServer server(80);

int Map[MAX+1][MAX + 1] = { 0 };
int temp[MAX+1][MAX+ 1] = { 0 };
bool store[MAX+1][MAX+ 1][MAX*MAX] = { 0 };
int temp_ = 0;
int num = 0;
int point_coordinate[2] = { 0 };
bool _Status = false;
const char PAGE_INDEX[] PROGMEM= R"=====(
<!DOCTYPE html>
<html>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<title>走迷宫</title>
<body>
<script>
function up()
{select("u");}
function down()
{select("d");}
function right()
{select("r");}
function left()
{select("l");}
function select(message)
{
  xmlhttp=new XMLHttpRequest();
  var url =  "/?position=" + message;
  xmlhttp.open("GET",url,true);
  xmlhttp.send();
}
</script>
<div>
<center><button type="button" onclick="up()">up</button></center>
<center><button type="button" onclick="left()">left</button><button type="button" onclick="right()">right</button></center>
<center><button type="button" onclick="down()">down</button></center>
<br>
</div>
</body>
</html>
)=====";

void recursion(int x,int y,int dir);
void miss();
void appear();
void generate();
void point_move();
bool point_twinkle(void *);
void handleRoot();
void init();

void setup() {
  Serial.begin(115200);
  lc.shutdown(0,false);
  lc.setIntensity(0,5);
  lc.clearDisplay(0);
  WiFi.mode(WIFI_AP);
  WiFi.softAP("RC1", "");
  server.on("/", handleRoot);
  server.begin();
  timer.every(500, point_twinkle);
  miss();
  generate();
}

void loop() {
  server.handleClient();
  timer.tick();
}

void point_move(int dir,char axis)
{
  if (axis == 'x'&&(point_coordinate[1]+dir)>=0&&(point_coordinate[1]+dir)<= MAX-1)
  {
    if (!(Map[point_coordinate[0]][point_coordinate[1]+dir]))
      point_coordinate[1] = point_coordinate[1]+dir;
  }
  if (axis == 'y'&&(point_coordinate[0]+dir)>=0&&(point_coordinate[0]+dir)<= MAX-1)
  {
    if (!(Map[point_coordinate[0]+dir][point_coordinate[1]]))
      point_coordinate[0] = point_coordinate[0]+dir;
  }
  if (point_coordinate[0] == MAX-1 &&point_coordinate[1] == MAX-1 )
  {
    point_coordinate[0] = 0;
    point_coordinate[1] = 0;
    miss();
    generate();
  }
}

bool point_twinkle(void *)
{
  int temp_map[MAX][MAX];
    for (int i = 0; i < MAX; i ++)
      for (int j = 0; j < MAX; j ++)
        temp_map[i][j] = Map[i][j];
    if (_Status)
  {
    temp_map[point_coordinate[0]][point_coordinate[1]] = 1;
  }
    for (int j = 0; j <=7; j ++)
    {
    int temp = 0;
    for (int i = 0; i <= 7; i ++)
      {
         if (temp_map[j][i]) {temp += pow(2,7-i);}
         if (i == 7) {lc. setColumn(0,7-j,temp);}
      }
    }
  _Status = !_Status;
  return true;
}

void generate()
{
  randomSeed(analogRead(2));
  recursion(0,0,1);
  while(Map[MAX-1][MAX-1] == 1)
  {
    for (int j = 0; j <= MAX; j++)
      for (int i = 0; i <= MAX; i++)
        Map[i][j] = 1;
    for (int j = 0; j <= MAX; j++)
      for (int i = 0; i <= MAX; i++)
        temp[i][j] = 0;
    Map[0][0] = 0;
    temp_ = 0;
    recursion(0,0,1);
  }
  appear();
  for (int N = 1; N <= temp_; N ++)
  {
    for (int i = 0;i < MAX; i++)
    {
      for (int j = 0; j < MAX; j++)
      {
        Map[i][j] = store[i][j][N];
      }
    }
    Draw();
    delay(50);
  }
}

void recursion(int x,int y,int dir)
{
  int num = 0;
  temp_ ++;
  for (int i = 0; i < MAX; i++)
  {
    for (int j = 0; j < MAX; j++)
    {
      store[i][j][temp_] = Map[i][j];
      if (Map[i][j] == 0)
        num++;
    }
  }
  if (num>=(MAX*MAX)/2&&Map[MAX-1][MAX-1] == 0)
  {
    return;
  }
  int temp_rand[5] = {8,8,8,8,8};
  bool status = true;
  while (status)
  {
    status = false;
    temp_rand[0] = random() % 4;
    temp_rand[1] = random() % 4;
    temp_rand[2] = random() % 4;
    temp_rand[3] = random() % 4;
    for (int i = 0; i <= 3; i++)
    {
      for (int j = i + 1; j <= 4; j++)
      {
        if (temp_rand[i] == temp_rand[j])
        {
          status = true;
        }
      }
    }
  }
  for (int i = 0; i <= 3; i++)
  {
    if (temp_rand[i] == 0&&x + 1 < MAX && temp[y][x + 1] != 3 && Map[y][x + 1] != 0 && (Map[y + 1][x + 1] + Map[y][x + 2] + Map[y - 1][x + 1] + Map[y][x]) == 3)
    {
      Map[y][x + 1] = 0;
      recursion(x + 1, y, 1);
    }
    if (temp_rand[i] == 1&&x > 0 && temp[y][x - 1] != 3 && Map[y][x - 1] != 0 && (Map[y + 1][x - 1] + Map[y][x] + Map[y - 1][x - 1] + Map[y][x - 2]) == 3)
    {
      Map[y][x - 1] = 0;
      recursion(x - 1, y, 2);
    }
    if (temp_rand[i] == 2&&y + 1 < MAX && temp[y + 1][x] != 3 && Map[y + 1][x] != 0 && (Map[y + 1][x + 1] + Map[y + 1][x - 1] + Map[y][x] + Map[y + 2][x]) == 3)
    {
      Map[y + 1][x] = 0;
      recursion(x, y + 1, 3);
    }
    if (temp_rand[i] == 3&&y > 0)
    {
      if (y - 1==0&&temp[y - 1][x] != 3 && Map[y - 1][x] != 0 && (Map[y][x] + Map[y - 1][x - 1] + Map[y - 1][x + 1]) == 2)
      {
        Map[y - 1][x] = 0;
        recursion(x, y - 1, 4);
      }
      if (y-1>0&&temp[y - 1][x] != 3 && Map[y - 1][x] != 0 && (Map[y][x] + Map[y - 2][x] + Map[y - 1][x - 1] + Map[y - 1][x + 1]) == 3)
      {
        Map[y - 1][x] = 0;
        recursion(x, y - 1, 4);
      }
    }
  }
  if (dir == 1&&x>0)
  {
    temp[y][x] = 3;
    x--;
    return;
  }
  if (dir == 2&&x+1<MAX)
  {
    temp[y][x] = 3;
    x++;
    return;
  }
  if (dir == 3&&y>0)
  {
    temp[y][x] = 3;
    y--;
    return;
  }
  if (dir == 4&&y+1<MAX)
  {
    temp[y][x] = 3;
    y++;
    return;
  }
}

void miss()
{
  delay(500);
  for (int i = 0; i <= 7; i ++)
  {
    lc. setColumn(0,7-i,B00000000);
    delay(50);
  }
  delay(500);
  for (int N = 0; N <= temp_; N ++)
    for (int i = 0;i < MAX; i++)
      for (int j = 0; j < MAX; j++)
        store[i][j][N] = 0;
  temp_ = 0;
  for (int j = 0; j <= MAX; j++)
    for (int i = 0; i <= MAX; i++)
      Map[i][j] = 1;
  for (int j = 0; j <= MAX; j++)
    for (int i = 0; i <= MAX; i++)
      temp[i][j] = 0;
  Map[0][0] = 0;
}

void appear()
{
  for (int i = 7; i >=0; i --)
  {
    lc. setColumn(0,i,B11111111);
    delay(50);
  }
}

void Draw()
{
  for (int j = 0; j <=7; j ++)
  {
    int temp = 0;
    for (int i = 0; i <= 7; i ++)
    {
      if (Map[j][i]) {temp += pow(2,7-i);}
      if (i == 7) {lc. setColumn(0,7-j,temp);}
    }
  }
}

void handleRoot() 
{
  String web = PAGE_INDEX;
  server.send(200, "text/html", web);
  String message = server.arg("position");
  char temp[message.length()+1];
  strcpy(temp,message.c_str());
  if (temp[0] == 'u'){point_move(-1,'y');}
  if (temp[0] == 'd'){point_move(1,'y');}
  if (temp[0] == 'r'){point_move(1,'x');}
  if (temp[0] == 'l'){point_move(-1,'x');}
}
