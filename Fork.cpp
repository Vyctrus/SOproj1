class Fork{
    public:
        bool getAvailable(){
            return isAvailable;
        }
        void setAvailable(bool value){
            isAvailable=value;
        }
        Fork(int index){
            id=index;
            isAvailable=true;
        }
private:
    int id;
    bool isAvailable;
};