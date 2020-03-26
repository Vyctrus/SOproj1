#include <ctime>
#include <chrono>
#include <cmath>
#include <mutex>
#include <thread>

class Philosopher
{
    public:
        int id;
        int philNumb;
        bool isPhilActive=true;
        
        bool getIsActive(){
            return isPhilActive;
        }
        void turnOf(){
            isPhilActive=false;
        }
        int getId(){
            return id;
        }
        
        int getState(){
            return state;
        }
        
        void setState(int number){
            std::lock_guard<std::mutex> lock(m1);
            state=number;
        }
        
        Philosopher(int index)
        :id(index){
            //id= index;
            philNumb=5;
        }
        
        
        void setStartThink(std::time_t timeVal){
            thinkingStarted = timeVal;
        }
        void setThinkPeriod(std::chrono::milliseconds timeVal){
            thinkingPeriod=timeVal;
        }
        std::time_t getStartThink(){
            return thinkingStarted;
        }
        std::chrono::milliseconds getThinkPeriod(){
            return thinkingPeriod;
        }
        
        void setStartEat(std::time_t timeVal){
            eatingStarted=timeVal;
        }
        void setEatPeriod(std::chrono::milliseconds timeVal){
            eatingPeriod=timeVal;
        }
        std::time_t getStartEat(){
            return eatingStarted;
        }
        std::chrono::milliseconds getEatPeriod(){
            return eatingPeriod;
        }
private:
    int state=1; //1-thinking 2-eating 3-hungry
    std::time_t thinkingStarted;
    std::chrono::milliseconds thinkingPeriod;
    std::time_t eatingStarted;
    std::chrono::milliseconds eatingPeriod;
    std::mutex m1;

};