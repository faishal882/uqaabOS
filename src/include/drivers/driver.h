#ifndef __DRIVER_H
#define __DRIVER_H

namespace uqaabOS
{

    namespace driver
    {
        class Driver
        {
        public:
            Driver();
            ~Driver();

            virtual void activate();
            virtual void deactivate();
            virtual int reset();
        };

        class DriverManager
        {

        private:
            Driver *drivers[365];
            int num_drivers;

        public:
            DriverManager();
            void add_driver(Driver*);
            void activate_all();
        };
    }
}

#endif