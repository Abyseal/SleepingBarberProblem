#include <thread>
#include <semaphore>
#include <iostream>
#include <queue>
#include <chrono>
#include <mutex>

int CUSTOMERS_NUMBER = 5; //число посетителей
int SEATS_NUMBER = 2; // число стульев в комнате ожидания

std::binary_semaphore barberRoomStatus(0); //эмитация кресла
std::counting_semaphore barberStatus(0); //<0 свободен,=0 занят
std::mutex barberRoomDoor; //mutex.lock() = door unlocked, mutex.unlock() = door locked
std::mutex barbershopDoor;//дверь в барбершоп (исключение проблемы, когда 2 клиента могут сесть на одно место)

std::queue<int> barbershopQueue = {}; //эмитация зала ожидания

void barber();
void customer(int number);

int main() {
	setlocale(LC_ALL, "rus");
	
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
			std::cout << "Barber is sleeping" << std::endl; //Если в очереди/зале ожидания никого нет, то барбер спит
		}
		barberStatus.acquire(); //пока барбер спит, он свободен,т.е его поток заблокирован
		if (barbershopQueue.size() > 0) { // поток продолжается когда барбера разбудили, т.е пришел клиент
			int human = barbershopQueue.front(); //приглашается человек, который дольше всего ждет (исключение проблемы starvation)
			barbershopQueue.pop();
			std::cout << "Barber is cutting hair for customer №" << human << std::endl;
			std::this_thread::sleep_for(std::chrono::seconds(3)); //процесс стрижки
			std::cout << "Barber has finished cutting hair for customer №" << human << std::endl;
			std::cout << "Barber-chair is now available" << std::endl;
			barberRoomStatus.release(); //Клиент подстригся, комната освобождается
		}
	}
}

void customer(int number) {
	std::this_thread::sleep_for(std::chrono::seconds(number)); // клиент заходит через некоторое время. Представим, что время на самом деле случайное.
	barbershopDoor.lock(); // клиент вошел в зал ожидания, открыл входную дверь. В это время никто больше не может войти, исключение проблемы, когда 2 клиента могут сесть на одно место
	if (barbershopQueue.size() < SEATS_NUMBER) { // Если есть свободное место, то клиент становится в очередь/садится на свободный стул
		barbershopQueue.push(number); 
		std::cout << "Customer №" << number << " is waiting in the queue" << std::endl;
		barbershopDoor.unlock(); // когда клиент сел входная дверь "освободилась" теперь следующий клиент сможет войти

		barberRoomDoor.lock(); // Клиент пытается войти в комнату барбера.Если комната уже занята, то поток блокируется (посетитель ждет).
		std::cout << "Customer №" << number << " entered barber room" << std::endl;
		barberStatus.release(); // Когда клиент смог попасть в комнату, он "будит" барбера, если тот спал или просто садится в кресло
		std::cout << "Barber chair is now occupied" << std::endl;
		barberRoomStatus.acquire(); // Клиент занимает кресло
		
		std::cout << "Customer №" << number << " has finished getting a haircut and left the barbershop " << std::endl;
		barberRoomDoor.unlock(); //Клиент выходит из комнаты барбера
	}
	else {
		std::cout << "Customer №" << number << " is leaving, because waiting room is full" << std::endl; // Если мест в зале ожидания нет, то клиент уходит
		barbershopDoor.unlock();
	}
}
