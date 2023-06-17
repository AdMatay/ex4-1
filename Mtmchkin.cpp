#include "Mtmchkin.h"
#include "Cards/BattleCards/Gremlin.h"
#include "Cards/BattleCards/Witch.h"
#include "Cards/BattleCards/Dragon.h"
#include "Cards/ChildCards/BarFight.h"
#include "Cards/ChildCards/Mana.h"
#include "Cards/ChildCards/Treasure.h"
#include "Cards/ChildCards/Well.h"
#include "Cards/ChildCards/MerchantCards/Merchant.h"
#include "Players/Healer.h"
#include "Players/Ninja.h"
#include "Players/Warrior.h"
#include <algorithm>


 /**
    *static const variables for use in the mtmchkin class for better readability
    */
    static const std::string NINJA = "Ninja";
    static const std::string HEALER = "Healer";
    static const std::string WARRIOR = "Warrior";
    static const int START_OF_FILE = 0; //
    static const int MIN_FILE_SIZE = 5;
    static const int MIN_NUM_OF_PLAYERS = 2;
    static const int MAX_NUM_OF_PLAYERS = 6;
    static const int MAX_PLAYER_NAME_SIZE = 15;
    static const int MIN_PLAYER_NAME_SIZE = 0;
    static const int ALLOWED_SPACES_IN_LINE = 1;

Mtmchkin::Mtmchkin(const std::string &fileName):m_roundNumCounter(0){
    std::ifstream deckFile(fileName);
    if(!deckFile){
      throw DeckFileNotFound();
    }
    pushCards(deckFile);
    inputPlayers();
}

  std::shared_ptr<Card> Mtmchkin::TypeOfCard(const std::string& name, int line) {
    if (name == "Gremlin"){
        return std::shared_ptr<Card>(new Gremlin());
    }
    else if (name == "Witch"){
        return std::shared_ptr<Card>(new Witch());
    }
    else if (name == "Dragon"){
        return std::shared_ptr<Card>(new Dragon());
    }
    else if (name == "Mana"){
        return std::shared_ptr<Card>(new Mana());
    }
    else if (name == "Barfight"){
        return std::shared_ptr<Card>(new BarFight());
    }
    else if (name == "Treasure"){
        return std::shared_ptr<Card>(new Treasure());
    }
    else if (name == "Merchant"){
        return std::shared_ptr<Card>(new MerchantCard());
    }
    else if (name == "Well"){
        return std::shared_ptr<Card>(new Well());
    }
    else{
        throw DeckFileFormatError(line);
    }
}

void Mtmchkin::pushCards(std::ifstream& deckFile){
    int lineNumber = START_OF_FILE;
    std::string cardName;
    while(std::getline(deckFile,cardName)){ 
        lineNumber++;
        try{
        this->m_cardDeck.push_back(TypeOfCard(cardName,lineNumber));
        } catch(std::bad_alloc& e){
            m_cardDeck.clear();
            deckFile.close();
            throw e;
            }
        }
        if(lineNumber<MIN_FILE_SIZE){
            m_cardDeck.clear();
            deckFile.close();
            throw DeckFileInvalidSize();
        }
}

void Mtmchkin::inputPlayers(){
    printStartGameMessage();
    printEnterTeamSizeMessage();
   int numOfPlayers = readNumOfPlayers();
   printInsertPlayerMessage();
   std::string line = readPlayerLine();
    int spacePosition = spaceBetweenTypeAndName(line);
    std::string name = line.substr(0,spacePosition);
    std::string type = line.substr(spacePosition+1);
    try{
        for (int i = 0;i<numOfPlayers;i++){
            m_activePlayers.push_back(TypeOfPlayer(name,type));
         }
    } catch(std::bad_alloc& e){
        m_activePlayers.clear();
        throw e;
    }

}

std::string Mtmchkin::readPlayerLine(){
    std::string line;
   std::getline(std::cin,line);
   if(!checkPlayerLineValid(line)){
       printInvalidName();
       return readPlayerLine();
   }
    int spacePosition = spaceBetweenTypeAndName(line);
    std::string name = line.substr(0,spacePosition);
    std::string type = line.substr(spacePosition+1);

    if(!checkNameValid(name)){
        printInvalidName();
        return readPlayerLine();
    }
    if(!checkTypeValid(type)){
        printInvalidClass();
        return readPlayerLine();
    }
   return line;
}

int spaceBetweenTypeAndName(std::string& line){
     int i=0;
    while(i<line.length()&&line[i]!=' '){
        i++;
    } 
    return i;
}

int Mtmchkin::readNumOfPlayers(){
    std::string line;
    std::getline(std::cin,line);
    int numOfPlayers;
    try{
     numOfPlayers = stoi(line);
    } catch(std::invalid_argument& e){
        printInvalidTeamSize();
        return readNumOfPlayers();
    }
    if(numOfPlayers>MAX_NUM_OF_PLAYERS||numOfPlayers<MIN_NUM_OF_PLAYERS){
        printInvalidTeamSize();
        return readNumOfPlayers();
    }
    return numOfPlayers;
}

std::shared_ptr<Player> Mtmchkin::TypeOfPlayer(const std::string& name, const std::string& type){
     if(type==NINJA){
         return std::shared_ptr<Player>(new Ninja(name));
     }
     else if(type==WARRIOR){
         return std::shared_ptr<Player>(new Warrior(name));
     }
     return std::shared_ptr<Player>(new Healer(name));
}

bool Mtmchkin::checkPlayerLineValid(std::string& line){
    int spaces = std::count(line.begin(),line.end(),' ');
    if(spaces!=ALLOWED_SPACES_IN_LINE||line.back()==' '||line.front()==' '){
        return false;
    }
    return true;
}

bool checkNameValid(std::string& name){
    if(name.length()==MIN_PLAYER_NAME_SIZE||name.length()>MAX_PLAYER_NAME_SIZE){
        return false;
    }
    for(char& c : name){
        if(!((c>='a'&&c<='z')||(c>='A'&&c<='Z'))){
            return false;
        }
    }
    return true;
} 

bool checkTypeValid(std::string& type){
    if(type==NINJA||type==WARRIOR||type==HEALER){
        return true;
    }
    return false;
}

void Mtmchkin::playRound(){
    m_roundNumCounter++;
    printRoundStartMessage(m_roundNumCounter);
    int numOfActivePlayers =m_activePlayers.size();
    for(int i=0; i<numOfActivePlayers;i++){
        printTurnStartMessage(m_activePlayers[i]->getName());
        m_cardDeck[0]->applyEncounter(*m_activePlayers[i]);
        if(m_activePlayers[i]->getLevel()==Player::MAX_LEVEL){
            m_playerWinners.push_back(m_activePlayers[i]);
            m_activePlayers.erase(m_activePlayers.begin()+i);
            i--;
        }
        else if(m_activePlayers[i]->isKnockedOut()){
            m_playerLosers.insert(m_playerLosers.begin(),m_activePlayers[i]);
            m_activePlayers.erase(m_activePlayers.begin()+i);
            i--;
        }
        m_cardDeck.push_back(m_cardDeck[0]);
        m_cardDeck.erase(m_cardDeck.begin());
    }
        if(m_activePlayers.empty()){
        printGameEndMessage();
        }
} 

int Mtmchkin::getNumberOfRounds()const{
    return m_roundNumCounter;
}

bool Mtmchkin::isGameOver()const{
    return m_activePlayers.empty();
}

void Mtmchkin::printLeaderBoard() const{
    printLeaderBoardStartMessage();
    int numOfWinners = m_playerWinners.size();
    int numOfLosers = m_playerLosers.size();
    for(int i=1;i<=numOfWinners;i++){
        printPlayerLeaderBoard(i,*m_playerWinners[i]);
    }
    for (int j=1;j<=numOfLosers;j++){
        printPlayerLeaderBoard(j+numOfWinners,*m_playerLosers[j]);
    }
}


