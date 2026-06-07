#include "fsm.hpp"
FSM motor;
void work();
void stop();
void error();
bool check();
int main(){
    motor.stateCreat("START",work);
    motor.stateCreat("STOP",stop);
    motor.stateSetTrans("START","STOP",check);
    while(true){
        motor.start("START");
        motor.stateUpdate();
    }
}