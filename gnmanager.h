#ifndef GNManager_H
#define GNManager_H

#include <string>

#ifdef __GXX_EXPERIMENTAL_CXX0X__
#  include <unordered_set>
   typedef std::unordered_set<int> Set;
#else
#  include <ext/hash_set>
   typedef __gnu_cxx::hash_set<int> Set;
#endif


class GameNumberManager {
public:
  GameNumberManager();

  bool alreadyPlayed(int num) { return alreadyWon(num) || alreadyLost(num); }

  void addWonGame(int);
  void addLostGame(int);
  void writeFiles();
  int randomLostGame();

private:
  bool alreadyWon(int);
  bool alreadyLost(int);

  void readFiles();
  void readFile(const std::string&, Set*);
  void writeFile(const std::string&, Set*);

  std::string lostGameFile, wonGameFile;
  
  Set wonGames;
  Set lostGames;

  std::string msLostGameFile, msWonGameFile;

  Set msWonGames;
  Set msLostGames;
};

#endif
