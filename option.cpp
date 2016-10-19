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

Option::Option()
  : mainCon(350, 200), speedCon(350, 50), speedLabel("Speed (ms)"), speedTF(80, this),
    togglesCon(350, 50), anim("Anim"), query("Query"), ms("MS Seed"),
    play("Auto"), replyCon(300, 100),
    okButton("  OK  ", this), cancelButton("Cancel", this)
{
  speedCon.add(&speedLabel); speedCon.add(&speedTF);
  speedCon.reallocate();

  togglesCon.add(&anim); togglesCon.add(&query); togglesCon.add(&ms);
  togglesCon.add (&play);
  togglesCon.reallocate();

  replyCon.add(&okButton); replyCon.add(&cancelButton);
  replyCon.reallocate();

  mainCon.add(&speedCon); mainCon.add(&togglesCon); mainCon.add(&replyCon);
  mainCon.reallocate();

  container(&mainCon);

  char* home = getenv("HOME");

  if (home == NULL) {
    fprintf(stderr, "Cannot get $HOME. Assuming I am at home directory now.\n");
    saveFile = ".xfreecell";
  } else {
    saveFile = home;
    saveFile += "/.xfreecell";
  }

  DIR* dir = opendir(saveFile.c_str());

  if (dir == NULL) {
    switch (errno) {
    case ENOENT:
      fprintf(stderr, "Directory %s not found. Creating.\n", saveFile.c_str());
      mkdir(saveFile.c_str(), 0755);
      break;
    case ENOTDIR:
      fprintf(stderr, "%s must be directory.\n", saveFile.c_str());
      exit(1);
      break;
    default:
      perror("ScoreWindow::ScoreWindow()");
      exit(1);
    }
  } else
    closedir(dir);
  saveFile += "/prefs";

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

  fclose(fp);
  return;
}

static bool parseBool (int i, int argc, char* argv[])  {
  char* arg = argv[i];

  if (i == argc) goto ERRORB;
  if (strcmp (arg, "true") == 0) return true;
  else if (strcmp (arg, "false") == 0) return false;
  ERRORB:
    fprintf(stderr, "Boolean (true or false) integer must follow option\n");
    exit (1);
}

void Option::parse(int argc, char* argv[])
{
  if (argc == 1)
    return;

  for (int i = 1; i < argc; i++) {
    char* arg = argv[i];
    i++;
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
      exit(0);
    } else if (strcmp(arg, "-s") == 0 || strcmp(arg, "--speed") == 0) {
      arg = argv[i];
      if (i == argc) goto ERRORI;
      _speedup = atoi(arg);
      if (0 > _speedup) goto ERRORI;
      continue;
    ERRORI:
      fprintf(stderr, "Positive integer must follow -s\n");
      exit (1);
    } else if (strcmp(arg, "-q") == 0 || strcmp(arg, "--query-window") == 0) {
      _queryWindow = parseBool(i, argc, argv);
#ifdef SHAPE
    } else if (strcmp(arg, "-r") == 0 || strcmp(arg, "--round-card") == 0) {
      _roundCard = parseBool(i, argc, argv);
#endif
    } else if (strcmp(arg, "-a") == 0 || strcmp(arg, "--animation") == 0) {
      _animation = parseBool(i, argc, argv);
    } else if (strcmp(arg, "-m") == 0 || strcmp(arg, "--ms-seed") == 0) {
      _msSeed = parseBool(i, argc, argv);
    } else if (strcmp(arg, "-p") == 0 || strcmp(arg, "--auto-play") == 0) {
      _autoPlay = parseBool(i, argc, argv);
    } else {
      fprintf(stderr, "Unknown option: %s\n", arg);
      exit (1);
    }
  }
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

