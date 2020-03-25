//kopia zapasowa
#include <iostream>
#include <vector>
#include <thread>

#include <ctime>
#include <chrono>
#include <cmath>
#include <string>
#include <mutex>

#include <condition_variable> // std::condition_variable

#include "Philosopher.cpp"
#include "Fork.cpp"

bool programRunning=true;

std::condition_variable cv;

std::vector<Philosopher*> philosophers(5);

//ZALOZENIE: KAZDY NOWY FILOZOF PRZYNOSI WIDELEC I ZABIERA JAK WYCHODZI- stosunek zasobow do watkow 1:1
std::vector<Fork*> forks(5) ;//shared
std::vector<std::thread*> threads(5);
std::mutex *forkLock;
int philNumb = 5;  //liczba filozofow


void takeForks(int index);
void putForks(int index);
void test(int index);
void think();
void eat();
void takeRightFork(int index);
void takeLeftFork(int index);
void putDownForks(int index);

void beginPhers(){
    philosophers.clear();
    for(int i=0;i <philNumb;i++){
            philosophers.push_back(new Philosopher(i));
            philosophers[i]->setState(1);//starts with thinking
    }
    
    forks.clear();
    for(int i=0;i <philNumb;i++){
            forks.push_back(new Fork(i));
    }

    threads.clear();
    //Pamietac ze trzeba na nowo stworzyc tablice wartosci boolowskich po update philNumb
    
    forkLock= new std::mutex[philNumb];
}


int GetRandom(int max){
    srand(time(NULL));
    return rand() % max;
}

//LIFE CYCLE
void philosopherLife(int index){
    while(programRunning && philosophers[index]->getIsActive()){
        think();//Point 1.
        takeForks(index);//Point 2.
        eat();// Point 3.
        putForks(index);//Point 4.
    }
}
// .1
void think(){
    //filozof mysli
     std::this_thread::sleep_for(std::chrono::milliseconds(2500+GetRandom(1000)));//2.5- 3.5 sec
}

// .2
std::mutex tfMutex;
void takeForks(int index){
    //lock(mutex);
    tfMutex.lock();
    philosophers[index]->setState(3);//THINKING->HUNGRY
    test(index);//2.2
    //unlock(mutex);
    tfMutex.unlock();
    //down(s[i]);
}

// 2.2 ; 4.2-dowolny stan wejsciowy  ; 4.3- dowolny stan wejsciowy
void test(int index){// 2.2 jest w locku tfMUtex
    //Moze jesc jesli zaden z jego sasiadow nie je I JEST GLODNY
    if(philosophers[index]->getState()==3// HUNGRY
        && (philosophers[(index-1+philNumb)%philNumb]->getState()!=2)//EATING
        && (philosophers[(index+1)%philNumb]->getState()!=2)//EATING
    ){
        philosophers[index]->setState(2);//HUNGRY->EATING //4.2.2 4.3.2
        takeRightFork(index);//2.2.3
        takeLeftFork(index);//2.2.4
        //up(s[index]);
    }else{
        //POZOSTAJE W OBECNYM STANIE
        //2.2 HUNGRY->HUNGRY
        //If !HUNGRY --> THINKING->THINKING, EATING->EATING, HUNGRY->HUNGRY if cannot eat
        //Jak stan eating to ok niech sobie robi co robil
        //Jak stan THINKING to niech dalej mysli
        //Nie ma przyrostu wykladniczego bo naraz nie moze ich jesc az tyle 4.2.1 4.3.1
    }
}
//2.2.3
void takeRightFork(int index){//2.2.3 tez jest w locku tfMutex
    forkLock[index].lock();
    if(forks[index]->getAvailable()){
        forks[index]->setAvailable(false);
    }else{
        //jest juz zabezpieczone nie powinno tutaj juz do niczego dojsc
        std::cout<<"Bledna sytuacja!!!\n";
    }
    forkLock[index].unlock();
}
//2.2.4
void takeLeftFork(int ind){
    int index= (ind+1)%philNumb;
    forkLock[index].lock();
    if(forks[index]->getAvailable()){
        forks[index]->setAvailable(false);
    }else{
        //jest juz zabezpieczone nie powinno tutaj juz do niczego dojsc
        std::cout<<"Bledna sytuacja!!!\n";
    }
    forkLock[index].unlock();
}


void eat(){
    //filozof je
    std::this_thread::sleep_for(std::chrono::milliseconds(2500+GetRandom(1000)));//2.5- 3.5 sec
}


// .4
std::mutex pfMutex;
void putForks(int index){//tutaj upewnic sie ze mutex ma sens
    //lock(mutex);
    pfMutex.lock();
    philosophers[index]->setState(1);//EATING->THINKING
    putDownForks(index);//4.1
    
    test((index-1+philNumb)%philNumb);//should left phil start eating? 4.2 wracam tu z 4.2.1
    test((index+1)%philNumb);//should right phil start eating? 4.3 wracam tu z 4.3.1
    //unlock(mutex);
    pfMutex.unlock();
}
//4.1
void putDownForks(int index){
    //tu blokada chyba niekonieczna, ale jesli tylko na posiadanych widelcach nie powinno byc problemu
    forkLock[index].lock();
    forkLock[(index+1)%philNumb].lock();
    forks[index]->setAvailable(true);
    forks[(index+1)%philNumb]->setAvailable(true);
    forkLock[index].unlock();
    forkLock[(index+1)%philNumb].unlock();
}




void show();
int main(int argc, char **argv)
{
	std::cout<<("Hell world\n");
    beginPhers();
    std::thread printing(show);
    
    for(int j=0;j<philNumb;j++){
        threads.push_back( new std::thread(philosopherLife,j)); 
    }
    for(int j=0;j<philNumb;j++){
        threads[j]->join();
    }
    printing.join();

    char x;
    std::cin>>x;
	return 0;
}


std::string printState(int index){
    switch(index){
        case 1:
            return "THINKING";
            break;
        case 2: 
            return "EATING";
            break;
        case 3:
            return "HUNGRY";
            break;
    }
    return "ERROR";
}

void show(){
    for(int j=0;j<10;j++){
        std::this_thread::sleep_for(std::chrono::seconds(1));
         //wyswietl
         for(int i=0;i<philNumb;i++){
            std::cout<<"Filozof ["<<i<<"] -- "<<printState(philosophers[i]->getState())<<"\n";
         }
         std::cout<<std::endl;
    }
}