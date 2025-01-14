#include "../include/drivers/driver.h"


namespace uqaabOS{
    namespace driver{

        Driver::Driver(){

        }

        Driver::~Driver(){

        }

        void Driver::activate(){

        }
        void Driver::deactivate(){}

        int Driver::reset(){}

        DriverManager::DriverManager(){
            this -> num_drivers = 0;
        }

        void DriverManager::add_driver(Driver* driver){

            drivers[num_drivers] = driver;
            num_drivers++;

        }

        void DriverManager::activate_all(){

            for(int i = 0 ; i < num_drivers ; i++){
                drivers[i] -> activate();
            }
        }

    }
}
