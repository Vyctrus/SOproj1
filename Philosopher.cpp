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
            philNumb=5;
        }
        
        
        void setStartThink(std::chrono::high_resolution_clock::time_point  timeVal){
            thinkingStarted = timeVal;
        }
        void setThinkPeriod(std::chrono::milliseconds timeVal){
            thinkingPeriod=timeVal;
        }
        std::chrono::high_resolution_clock::time_point  getStartThink(){
            return thinkingStarted;
        }
        std::chrono::milliseconds getThinkPeriod(){
            return thinkingPeriod;
        }
        
        void setStartEat(std::chrono::high_resolution_clock::time_point timeVal){
            eatingStarted=timeVal;
        }
        void setEatPeriod(std::chrono::milliseconds timeVal){
            eatingPeriod=timeVal;
        }
        std::chrono::high_resolution_clock::time_point getStartEat(){
            return eatingStarted;
        }
        std::chrono::milliseconds getEatPeriod(){
            return eatingPeriod;
        }
private:
    int state=1; //1-thinking 2-eating 3-hungry
    std::chrono::high_resolution_clock::time_point  thinkingStarted;
    std::chrono::milliseconds thinkingPeriod;
     std::chrono::high_resolution_clock::time_point eatingStarted;
    std::chrono::milliseconds eatingPeriod;
    std::mutex m1;

};