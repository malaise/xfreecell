#include <cstdio>
#include <cstdlib>
#include <dirent.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <errno.h>

#include "gnmanager.h"
#include "option.h"

const int lineLength = 30;

GameNumberManager::GameNumberManager()
{
  char* home = getenv("HOME");
  std::string directory;

  if (home == NULL) {
    fprintf(stderr, "Cannot get $HOME. Assuming I am at home directory now.\n");
    directory = ".xfreecell";
  } else {
    directory = home;
    directory += "/.xfreecell";
  }

  DIR* dir = opendir(directory.c_str());

  if (dir == NULL) {
    switch (errno) {
    case ENOENT:
      fprintf(stderr, "Directory %s not found. Creating.\n", directory.c_str());
      mkdir(directory.c_str(), 0755);
      break;
    case ENOTDIR:
      fprintf(stderr, "%s must be directory.\n", directory.c_str());
      exit(1);
      break;
    default:
      perror("GameNumberManager::GameNumberManager()");
      exit(1);
    }
  } else
    closedir(dir);

  wonGameFile = lostGameFile = msWonGameFile = msLostGameFile = directory;

  wonGameFile += "/won";
  lostGameFile += "/lost";
  msWonGameFile += "/msWon";
  msLostGameFile += "/msLost";

  readFiles();
}

void GameNumberManager::addWonGame(int num)
{
  if (alreadyWon(num)) return;

  if (Option::msSeed()) {
    msWonGames.insert(num);
    msLostGames.erase(num);
  } else {
    wonGames.insert(num);
    lostGames.erase(num);
  }
}

void GameNumberManager::addLostGame(int num)
{
  if (alreadyLost(num)) return;

  if (Option::msSeed()) {
    msLostGames.insert(num);
    msWonGames.erase(num);
  } else {
    lostGames.insert(num);
    wonGames.erase(num);
  }
}

bool GameNumberManager::alreadyWon(int num)
{
  Set::iterator i;

  if (Option::msSeed()) {
    i = msWonGames.find(num);
    if (i != msWonGames.end()) return true;
  } else {
    i = wonGames.find(num);
    if (i != wonGames.end()) return true;
  }

  return false;
}

bool GameNumberManager::alreadyLost(int num)
{
  Set::iterator i;

  if (Option::msSeed()) {
    i = msLostGames.find(num);
    if (i != msLostGames.end()) return true;
  } else {
    i = lostGames.find(num);
    if (i != lostGames.end()) return true;
  }

  return false;
}

int GameNumberManager::randomLostGame()
{
  if (Option::msSeed()) {
    if (msLostGames.size() == 0) return 0;
  } else {
    if (lostGames.size() == 0) return 0;
  }

  int num;
  Set::iterator iter;

  if (Option::msSeed()) {
    num = time(0) % msLostGames.size();
    iter = msLostGames.begin();
  } else {
    num = time(0) % lostGames.size();
    iter = lostGames.begin();
  }

  std::advance(iter, num);

  return *iter;
}

void GameNumberManager::readFiles()
{
  readFile(wonGameFile, &wonGames);
  readFile(lostGameFile, &lostGames);
  readFile(msWonGameFile, &msWonGames);
  readFile(msLostGameFile, &msLostGames);
}

void GameNumberManager::readFile(const std::string& file, Set* map)
{
  FILE* fp = fopen(file.c_str(), "r");
  char line[lineLength];

  if (fp == NULL) {
    if (errno == ENOENT)
      fprintf(stderr, "%s doesn't exist. Creating.\n", file.c_str());
    writeFile(file, map);
    return;
  }

  while (fgets(line, lineLength, fp) != NULL)
    map->insert(atoi(line));

  fclose(fp);
}

void GameNumberManager::writeFiles()
{
  writeFile(wonGameFile, &wonGames);
  writeFile(lostGameFile, &lostGames);
  writeFile(msWonGameFile, &msWonGames);
  writeFile(msLostGameFile, &msLostGames);
}

void GameNumberManager::writeFile(const std::string& file, Set* map)
{
  FILE* fp = fopen(file.c_str(), "w+");

  if (fp == NULL) {
    fprintf(stderr, "Cannot open %s\n", file.c_str());
    perror("GameNumberManager::writeFile()");
    return;
  }

  for (Set::iterator i = map->begin(); i != map->end(); i++)
    fprintf(fp, "%d\n", *i);

  fclose(fp);
}

