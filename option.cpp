#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "general.h"
#include "option.h"
#include "widget/widget.h"

extern Display* dpy;
extern Window gameWindow;

int Option::_speedup = 100;
bool Option::_queryWindow = false;
bool Option::_roundCard = true;
bool Option::_animation = true;
bool Option::_msSeed = false;
bool Option::_autoPlay = true;
bool Option::_load = false;
int Option::_doubleClick = 500;
std::string Option::_dir;

Option::Option()
  : mainCon(350, 250),
    speedCon(350, 50), speedLabel("Speed (ms)"), speedTF(80, this),
    togglesCon(350, 50), anim("Anim"), query("Query"), ms("MS Seed"), play("Auto"),
    clickCon(350, 50), clickLabel("Double click (ms)"), clickTF(30, this),
    replyCon(350, 100), okButton("  OK  ", this), cancelButton("Cancel", this)
{
  speedCon.add(&speedLabel); speedCon.add(&speedTF);
  speedCon.reallocate();

  togglesCon.add(&anim); togglesCon.add(&query); togglesCon.add(&ms);
  togglesCon.add (&play);
  togglesCon.reallocate();

  clickCon.add(&clickLabel); clickCon.add(&clickTF);
  clickCon.reallocate();

  replyCon.add(&okButton); replyCon.add(&cancelButton);
  replyCon.reallocate();

  mainCon.add(&speedCon); mainCon.add(&togglesCon); mainCon.add(&clickCon);
  mainCon.add(&replyCon);
  mainCon.reallocate();

  container(&mainCon);

  char* home = getenv("HOME");

  if (home == NULL) {
    fprintf(stderr, "Cannot get $HOME. Assuming I am at home directory now.\n");
    _dir = ".xfreecell";
  } else {
    _dir = home;
    _dir += "/.xfreecell";
  }

  DIR* dir = opendir(_dir.c_str());

  if (dir == NULL) {
    switch (errno) {
    case ENOENT:
      fprintf(stderr, "Directory %s not found. Creating.\n", _dir.c_str());
      mkdir(_dir.c_str(), 0755);
      break;
    case ENOTDIR:
      fprintf(stderr, "%s must be directory.\n", _dir.c_str());
      exit(1);
      break;
    default:
      perror("ScoreWindow::ScoreWindow()");
      exit(1);
    }
  } else {
    closedir(dir);
  }
  saveFile = _dir + "/prefs";

  readPrefs();
}

void Option::buttonAction(const XEvent& ev, void*)
{
  exitPressed = true;
  if (ev.xany.window == okButton.window()) okPressed = true;
}

void Option::textAction(bool ok, void *)
{
  exitPressed = true;
  okPressed = ok;
}

void Option::waitForEvent()
{
  char val[255];

  exitPressed = false;
  okPressed = false;

  sprintf (val, "%d", _speedup);
  speedTF.init(val);
  anim.toggled(_animation);
  query.toggled(_queryWindow);
  ms.toggled(_msSeed);
  sprintf (val, "%d", _doubleClick);
  clickTF.init(val);
  play.toggled(_autoPlay);

  map();
  XRaiseWindow(dpy, window());

  XEvent ev;
  while (mapped() && !exitPressed) {
    NSNextEvent(&ev);
    NSDispatchEvent(ev);
  }

  this->hide();

  if (okPressed) {
    int val = atoi(speedTF.text());
    if (val < 0) val = 0;
    _speedup = val;
    _animation = anim.toggled();
    _queryWindow = query.toggled();
    _msSeed = ms.toggled();
    _autoPlay = play.toggled();
    val = atoi(clickTF.text());
    if (val < 100) val = 100;
    if (val > 900) val = 900;
    _doubleClick = val;
  }

  writePrefs();
}

void Option::hide()
{
  unmap();
}


void Option::readPrefs()
{
  FILE* fp = fopen(saveFile.c_str(), "r");
  const int lineLength = 30;
  char line[lineLength];

  if (fp == NULL) {
    if (errno == ENOENT)
      fprintf(stderr, "%s doesn't exist. Creating.\n", saveFile.c_str());
    writePrefs();
    return;
  }

  if (fgets(line, lineLength, fp) == NULL) goto ERROR;
  if (sscanf(line, "speedup = %d", &_speedup) != 1) goto ERROR;

  int tmp;
  if (fgets(line, lineLength, fp) == NULL) goto ERROR;
  if (sscanf(line, "animation = %d", &tmp) != 1) goto ERROR;
  _animation = (tmp == 0 ? false : true);

  if (fgets(line, lineLength, fp) == NULL) goto ERROR;
  if (sscanf(line, "query = %d", &tmp) != 1) goto ERROR;
  _queryWindow = (tmp == 0 ? false : true);

  if (fgets(line, lineLength, fp) == NULL) goto ERROR;
  if (sscanf(line, "ms seed = %d", &tmp) != 1) goto ERROR;
  _msSeed = (tmp == 0 ? false : true);

  if (fgets(line, lineLength, fp) == NULL) goto ERROR;
  if (sscanf(line, "auto play = %d", &tmp) != 1) goto ERROR;
  _autoPlay = (tmp == 0 ? false : true);

  if (fgets(line, lineLength, fp) == NULL) goto ERROR;
  if (sscanf(line, "double click = %d", &_doubleClick) != 1) goto ERROR;

  if ( (_speedup < 0) || (_doubleClick < 100) || (_doubleClick > 900) ) {
    goto ERROR;
  }

  fclose(fp);
  return;

 ERROR:
  fprintf(stderr, "Error encountered while reading %s\n", saveFile.c_str());
  fclose(fp);
}

void Option::writePrefs()
{
  FILE* fp = fopen(saveFile.c_str(), "w+");

  if (fp == NULL) {
    fprintf(stderr, "Cannot open %s\n", saveFile.c_str());
    perror("Option::writePrefs()");
    return;
  }

  fprintf(fp, "speedup = %d\n", _speedup);
  fprintf(fp, "animation = %d\n", _animation);
  fprintf(fp, "query = %d\n", _queryWindow);
  fprintf(fp, "ms seed = %d\n", _msSeed);
  fprintf(fp, "auto play = %d\n", _autoPlay);
  fprintf(fp, "double click = %d\n", _doubleClick);

  fclose(fp);
  return;
}

static unsigned int numArg;
static bool parseBool (int argc, char* argv[])  {
  char* arg = argv[numArg+1];
  unsigned int i;
  int c;

  if ( (numArg+1 == (unsigned int)argc) || (arg[0] == '-') ) {
    // No more arg or another option => Default is true
    return true;
  }

  // Lowercase
  for (i = 0; i < strlen(arg); i++) {
    c = (int)arg[i];
    arg[i] = tolower (c);
  }

  numArg++;
  if ( (strcmp (arg, "yes") == 0) || (strcmp (arg, "y") == 0) ) return true;
  if ( (strcmp (arg, "no") == 0)  || (strcmp (arg, "n") == 0) ) return false;
  fprintf(stderr, "Yes or no must follow option\n");
  exit (1);
}

void Option::parse(int argc, char* argv[])
{
  
  if (argc == 1)
    return;

  numArg = 1;

  while (numArg < (unsigned int)argc) {
    char* arg = argv[numArg];
    if (strcmp(arg, "-v") == 0 || strcmp(arg, "--version") == 0) {
      printf("%s\n", VersionStr);
      printf("Written by Nakayama Shintaro\n");
      printf("Maintained by Pascal Malaise\n");
      exit(0);
    } else if (strcmp(arg, "-h") == 0 || strcmp(arg, "--help") == 0) {
      printf("Usage %s { [ <option> ] }\n", argv[0]);
      printf("<option> ::=\n");
      printf("  -s <positive> | --speed <positive>\n");
#ifdef SHAPE
      printf("  -r <boolean>  | --round-card <boolean>\n");
#endif
      printf("  -a <boolean>  | --animation <boolean>\n");
      printf("  -m <boolean>  | --ms-seed <boolean>\n");
      printf("  -p <boolean>  | --auto-play <boolean>\n");
      printf("  -d <100..900> | --double-click <100..900>\n");
      printf("<boolean> ::= y | Y | yes | YES | n | N | no | NO\n");
      exit(0);
    } else if (strcmp(arg, "-s") == 0 || strcmp(arg, "--speed") == 0) {
      numArg++;
      arg = argv[numArg];
      if (numArg == (unsigned int)argc) goto ERRORS;
      _speedup = atoi(arg);
      if (0 > _speedup) goto ERRORS;
    } else if (strcmp(arg, "-q") == 0 || strcmp(arg, "--query-window") == 0) {
      _queryWindow = parseBool(argc, argv);
#ifdef SHAPE
    } else if (strcmp(arg, "-r") == 0 || strcmp(arg, "--round-card") == 0) {
      _roundCard = parseBool(argc, argv);
#endif
    } else if (strcmp(arg, "-a") == 0 || strcmp(arg, "--animation") == 0) {
      _animation = parseBool(argc, argv);
    } else if (strcmp(arg, "-m") == 0 || strcmp(arg, "--ms-seed") == 0) {
      _msSeed = parseBool(argc, argv);
    } else if (strcmp(arg, "-p") == 0 || strcmp(arg, "--auto-play") == 0) {
      _autoPlay = parseBool(argc, argv);
    } else if (strcmp(arg, "-l") == 0 || strcmp(arg, "--load") == 0) {
      _load = parseBool(argc, argv);
    } else if (strcmp(arg, "-d") == 0 || strcmp(arg, "--double-click") == 0) {
      numArg++;
      arg = argv[numArg];
      if (numArg == (unsigned int)argc) goto ERRORD;
      _doubleClick = atoi(arg);
      if  ( (_doubleClick < 100 ) || (_doubleClick > 900 ) ) goto ERRORD;
    } else {
      fprintf(stderr, "Unknown option: %s\n", arg);
      exit (1);
    }
    numArg++;
  }
  return;
  ERRORS:
    fprintf(stderr, "Positive integer must follow -s\n");
    exit (1);
  ERRORD:
    fprintf(stderr, "Value between 100 and 900 must follow -d\n");
    exit (1);
}

/*
int main()
{
  NSInitialize();

  dpy = NSdisplay();
  gameWindow = RootWindow(dpy, 0);

  Option o;

  o.waitForEvent();

  return 0;
}
*/

