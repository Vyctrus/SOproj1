#include <iostream>
#include <vector>
#include <thread>

#include <ctime>
#include <chrono>
#include <cmath>
#include <string>
#include <mutex>

#include <condition_variable>

#include <ncurses.h>

#include <condition_variable> // std::condition_variable

#include "Philosopher.cpp"
#include "Fork.cpp"
#include <chrono>


#define THINKING 1
#define EATING 2
#define HUNGRY 3
//bool turnOf=true;

//**wyswietlanie dane***********************************
//Lokalizacje
    int forkPlaceX[5];
    int philPlaceX[5];
    int forkPlaceY[5];
    int philPlaceY[5];

bool programRunning=true;


std::vector<Philosopher*> philosophers(5);

//ZALOZENIE: KAZDY NOWY FILOZOF PRZYNOSI WIDELEC I ZABIERA JAK WYCHODZI- stosunek zasobow do watkow 1:1
//ZALOZENIE UPDATE: AKTUALNIE JESTEM SPOZNIONY I NIE BEDZIE DODAWANIA KOLEJNYCH WATKOW--
std::vector<Fork*> forks(5) ;//shared
std::vector<std::thread*> threads(5);

std::mutex *forkLock;
int philNumb = 5;  //liczba filozofow

std::mutex myMutex1;
std::condition_variable myCondVar1;

std::mutex* myMutex2;
std::condition_variable* myCondVar2;

void takeForks(int index);
void putForks(int index);
void test(int index);
void think(int index);
void eat(int index);
void takeRightFork(int index);
void takeLeftFork(int index);
void putDownForks(int index);

void beginPhers(){
    philosophers.clear();
    for(int i=0;i <philNumb;i++){
            philosophers.push_back(new Philosopher(i));
            philosophers[i]->setState(1);//starts with thinking
    }
    philosophers[2]->setState(3);//starts with HUNGRY
    philosophers[3]->setState(3);//starts with HUNGRY
    forks.clear();
    for(int i=0;i <philNumb;i++){
            forks.push_back(new Fork(i));
    }
    
    threads.clear(); 
    forkLock= new std::mutex[philNumb];
    
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    myMutex2= new std::mutex[philNumb] ;
    myCondVar2=new std::condition_variable[philNumb];
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
}


int GetRandom(int max){
    srand(time(NULL));
    return rand() % max;
}

//LIFE CYCLE
void philosopherLife(int index){
    while(programRunning && philosophers[index]->getIsActive()){
        int myState=philosophers[index]->getState();
        if(myState==1){//Jak jest (zaczyna)THINKING, EATING stan zabroniony w tym miejscu, ja hungry skipuje do nastepnej sekcji
            think(index);//Point 1.
        }
        takeForks(index);//obecnie tutaj hungry lub eating idzi dalej tu musze zmienic stan THIKNING->HUNGRY ->(EATING/w8->EATING)
        if(programRunning){
            if(philosophers[index]->getState()!=2){
                std::cout<<"Probuje jesc ale nie je!\n";
            }
            eat(index);
            
            putForks(index);//stan juz jest thinking //EATING_>THINKING
            
            if(philosophers[index]->getState()!=1){
                std::cout<<"State powinien byc THINKING!!!\n";
            }
        }
    }
}

void think(int index){
    //filozof mysli
    auto t2 = std::chrono::high_resolution_clock::now();
    auto thinkingTime= std::chrono::milliseconds(2500+GetRandom(1000));//2.5- 3.5 sec
    philosophers[index]->setStartThink(t2);
    philosophers[index]->setThinkPeriod(thinkingTime);
    std::this_thread::sleep_for(thinkingTime);//2.5- 3.5 sec
}

std::mutex takeFroksMutex;
void takeForks(int index){//PRZEROBIONE
    takeFroksMutex.lock();
    philosophers[index]->setState(HUNGRY);
    takeFroksMutex.unlock();
    test(index);//Zacznij jesc jesli sasiedzi nie jedza
    //tutaj wychodzisz jedynie z EATING
}

std::mutex testMx;
void test(int index){// PRZEROBIONE
    powrot:
    if(!programRunning){
        return;
    }
    testMx.lock();
    //Moze jesc jesli zaden z jego sasiadow nie je I JEST GLODNY
    int myState=philosophers[index]->getState();
    int myState2=philosophers[(index-1+philNumb)%philNumb]->getState();
    int myState3=philosophers[(index+1)%philNumb]->getState();
    testMx.unlock();
    if(myState==HUNGRY// HUNGRY
        && (myState2!=EATING)//EATING
        && (myState3!=EATING)//EATING
    ){
        philosophers[index]->setState(EATING);//HUNGRY->EATING 
        takeRightFork(index);
        takeLeftFork(index);
        
    }else{
        std::unique_lock<std::mutex> lck2(myMutex2[index]);
        myCondVar2[index].wait(lck2);//a kiedy zostaniesz powiadomiony sprawdz jeszcze raz
        //test(index);//Tutaj bylo to blokowane- dowiedziec sie czemu, goto jest niezalecane, ale dziala
        goto powrot;
    }
    
}

void takeRightFork(int index){
    forkLock[index].lock();
    if(forks[index]->getAvailable()){
        forks[index]->setAvailable(false);
    }else{
        //jest juz zabezpieczone nie powinno tutaj juz do niczego dojsc
        std::cout<<"Bledna sytuacja!!!\n";
    }
    forkLock[index].unlock();
}

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


void eat(int index){
    //filozof je
    auto t2 = std::chrono::high_resolution_clock::now();
    auto eatingTime= std::chrono::milliseconds(2500+GetRandom(1000));//2.5- 3.5 sec
    philosophers[index]->setStartEat(t2);
    philosophers[index]->setEatPeriod(eatingTime);
    std::this_thread::sleep_for(eatingTime);//2.5- 3.5 sec
}

//ZALOZENIE: NARAZ STAN ZMIENIA TYLKO 1
std::mutex putForksMx;
void putForks(int index){
    putDownForks(index);
    
    putForksMx.lock();
    philosophers[index]->setState(THINKING);//EATING->THINKING
    putForksMx.unlock();
    //test((index-1+philNumb)%philNumb);//Spytaj sasiadow czy moze moga zaczac
    //test((index+1)%philNumb);
    myCondVar2[(index-1+philNumb)%philNumb].notify_all();//POWAIADOM sasiadow ze moze nastal czas kiedy oni winni ucztowac
    myCondVar2[(index+1)%philNumb].notify_all();

}

void putDownForks(int index){
    forkLock[index].lock();
    forkLock[(index+1)%philNumb].lock();
    forks[index]->setAvailable(true);
    forks[(index+1)%philNumb]->setAvailable(true);
    forkLock[index].unlock();
    forkLock[(index+1)%philNumb].unlock();
}

void show();
void ncWindow();
void controlWindow();
int main(int argc, char **argv)
{
	std::cout<<("Hell world console\n");
    beginPhers();
    //std::thread printing(show);
    std::thread printing2(ncWindow);
    std::thread control2(controlWindow);
    for(int j=0;j<philNumb;j++){
        threads.push_back( new std::thread(philosopherLife,j)); 
    }
    
    for(int j=0;j<philNumb;j++){
        threads[j]->join();
    }
    //printing.join();
    printing2.join();
    control2.join();
    
    char x;
    std::cout<<"Program skonczyl dzialanie\n";
    //std::cin>>x;
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
        default:
            return "ERROR";
            break;
    }
    return "ERROR";
}

void show(){
    for(int j=0;j<5*60*5;j++){
        std::this_thread::sleep_for(std::chrono::milliseconds(200));//std::chrono::seconds(1)
         //wyswietl
         for(int i=0;i<philNumb;i++){
            int x;
            x=philosophers[i]->getState();
            std::cout<<"Filozof ["<<i<<"] -- "<<printState(x)<<"\n";
         }
         std::cout<<std::endl;
    }
    std::cout<<"Wszystko przeszlo, sprawdz czy sie zmienialo\n";
}

void printingDeluxe(WINDOW * win);

void controlWindow(){
    int userInp =getch();
    while(programRunning){
       std::this_thread::sleep_for(std::chrono::milliseconds(10));//2.5- 3.5 sec
        if(userInp=='x'){
            programRunning=false;
            for(int index=0;index<5;index++){
                myCondVar2[index].notify_all();//co by hungry skonczyl dzialanie
            }
        }else{
            userInp =getch();
        }
    }
}
void ncWindow(){
    initscr();
    
    //PHILOSOPHERS PLACES...........................   
    philPlaceX[0]=30;
    philPlaceY[0]=2;
    
    philPlaceX[1]=50;
    philPlaceY[1]=10;
    
    philPlaceX[2]=50;
    philPlaceY[2]=20;

    philPlaceX[3]=10;
    philPlaceY[3]=20;

    philPlaceX[4]=10;
    philPlaceY[4]=10;
    
//FORKS...............................    
    forkPlaceX[0]=10;
    forkPlaceY[0]=2;
    
    forkPlaceX[1]=50;
    forkPlaceY[1]=2;
    
    forkPlaceX[2]=50;
    forkPlaceY[2]=15;

    forkPlaceX[3]=30;
    forkPlaceY[3]=20;
    
    forkPlaceX[4]=10;
    forkPlaceY[4]=15;
//...................................
//******************************************************
    
    int y_max_size, x_max_size;
    getmaxyx(stdscr, y_max_size, x_max_size);
    
    int height,width, start_y, start_x;
    height= 24;
    width = 80;
    start_y= 0;
    start_x= 0;
    
    start_color();
    WINDOW * win = newwin(height,width,start_y,start_x);
    refresh();
    box(win,0,0);
    wrefresh(win);    
      
    //PHILOSOPHERS PRINT....................
    init_pair(1, COLOR_BLACK, COLOR_WHITE);//THINKING
    init_pair(3, COLOR_GREEN, COLOR_RED);//HUNGRY
    init_pair(2, COLOR_BLACK, COLOR_BLUE);//EATING
    init_pair(4, COLOR_BLACK, COLOR_GREEN);//available
    init_pair(5, COLOR_BLACK, COLOR_RED);//not available


    //int userInp =wgetch(win);
    while(programRunning){
       std::this_thread::sleep_for(std::chrono::milliseconds(50));//2.5- 3.5 sec
        printingDeluxe(win);
    }

    //getch();
    endwin();
    
}

void printingPercentEat(int y,int x, int index){
    move(y+1,x);
    printw("|");
    move(y+1,x+11);
    printw("|");
    auto t3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = t3-philosophers[index]->getStartEat();
    for(int j=0;j<10;j++){
        int upToTen= time_span *10/std::chrono::milliseconds(philosophers[index]->getEatPeriod());
        if(upToTen >=j){
            move(y+1,x+1+j);
            printw("O");
        }
    }
}

void printingPercentThink(int y,int x, int index){
    move(y+1,x);
    printw("|");
    move(y+1,x+11);
    printw("|");
    auto t3 = std::chrono::high_resolution_clock::now();
    std::chrono::duration<double> time_span = t3-philosophers[index]->getStartThink();
    for(int j=0;j<10;j++){
        int upToTen= time_span *10/std::chrono::milliseconds(philosophers[index]->getThinkPeriod());
        if(upToTen >=j){
            move(y+1,x+1+j);
            printw("O");
        }
    }
}


void printingDeluxe(WINDOW * win){
    clear();
    move(1,1);
    printw("Press X to close simulation(keyboard)\n");
    
    for(int i=0;i<5;i++){
        int x;
        x=philosophers[i]->getState();
        switch(x){
            case 1://THINKING
                attron(COLOR_PAIR(1));
                move(philPlaceY[i],philPlaceX[i]);
                printw("Phil.%d",i);
                move((philPlaceY[i]+1),philPlaceX[i]);
                printw("Status: THINKING");
                printingPercentThink(philPlaceY[i]+1,philPlaceX[i],i);
                attroff(COLOR_PAIR(1));
                break;
            case 2://EATING
                attron(COLOR_PAIR(2));
                move(philPlaceY[i],philPlaceX[i]);
                printw("Phil.%d",i);
                move((philPlaceY[i]+1),philPlaceX[i]);
                printw("Status: EATING");
                printingPercentEat(philPlaceY[i]+1,philPlaceX[i],i);
                attroff(COLOR_PAIR(2));
                break;
            case 3://HUNGRY
                attron(COLOR_PAIR(3));
                move(philPlaceY[i],philPlaceX[i]);
                printw("Phil.%d",i);
                move((philPlaceY[i]+1),philPlaceX[i]);
                printw("Status: HUNGRY");
                attroff(COLOR_PAIR(3));
                break;
        }

    }
    //FORKS PRINT............................
    for(int j=0;j<5;j++){
        bool x;
        //forkLock[j].lock();
        x=forks[j]->getAvailable();
        //forkLock[j].unlock();
        if(x){
            attron(COLOR_PAIR(4));
            move(forkPlaceY[j],forkPlaceX[j]);
            printw("Fork: %d",j);
            move((forkPlaceY[j]+1),forkPlaceX[j]);
            printw("Status: AVAILABLE!");
            attroff(COLOR_PAIR(4));           
        }else{
            attron(COLOR_PAIR(5));
            move(forkPlaceY[j],forkPlaceX[j]);
            printw("Fork: %d",j);
            move((forkPlaceY[j]+1),forkPlaceX[j]);
            printw("Status: TAKEN!");
            attroff(COLOR_PAIR(4)); 
        }

    }
    refresh();
    wrefresh(win);  
}