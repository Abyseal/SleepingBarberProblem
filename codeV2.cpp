#include <thread>
#include <semaphore>
#include <iostream>
#include <queue>
#include <chrono>
#include <mutex>

int CUSTOMERS_NUMBER = 5;
int SEATS_NUMBER = 2;

std::binary_semaphore barberRoomStatus(0);
std::counting_semaphore barberStatus(0);
std::mutex barberRoomDoor; //mutex.lock() = door unlocked, mutex.unlock() = door locked
std::mutex barbershopDoor;

std::queue<int> barbershopQueue = {};

void barber();
void customer(int number);

int main() {
	std::thread barb(barber);
	std::thread cust1(customer,5);
	std::thread cust2(customer, 6);
	std::thread cust3(customer, 7);
	std::thread cust4(customer, 8);
	std::thread cust5(customer, 9);

	barb.join();
	cust1.join();
	cust2.join();
	cust3.join();
	cust4.join();
	cust5.join();
	return 0;
}
void barber() {
	while (true) {
		if (barbershopQueue.size() == 0) {
			std::cout << "Barber is sleeping" << std::endl;
		}
		barberStatus.acquire();
		if (barbershopQueue.size() > 0) {
			int human = barbershopQueue.front();
			barbershopQueue.pop();
			std::cout << "Barber is cutting hair for " << human << " customer" << std::endl;
			
			std::this_thread::sleep_for(std::chrono::seconds(3));
			std::cout << "Barber finished " << human << " customer" << std::endl;
			barberRoomStatus.release();
		}
	}
}

void customer(int number) {
	std::this_thread::sleep_for(std::chrono::seconds(number));
	barbershopDoor.lock();
	if (barbershopQueue.size() < SEATS_NUMBER) {
		barbershopQueue.push(number);
		std::cout << "Customer " << number << " is waiting in the queue" << std::endl;
		barbershopDoor.unlock();

		barberRoomDoor.lock();
		barberStatus.release();
		barberRoomStatus.acquire();
		
		std::cout << "Customer " << number << " has finished" << std::endl;
		barberRoomDoor.unlock();
	}
	else {
		std::cout << "Customer " << number << " is leaving" << std::endl;
		barbershopDoor.unlock();
	}
}