﻿#include <iostream>

using namespace std;

/**
* \brief Конкретный класс модели изменения температуры
*
* Предоставляет функцию изменения температуры по нелинейной модели 
*/
class NonLinearModel
{
	double A = 1.000005, B = 0.0000001, C = 0.00075, D = 0.1; ///< коэффициенты нелинейной модели 
 
public:
	/**
	* функция для расчёта температуры по нелинейной модели
	* \param T1 температура на прошлом моменте времени
	* \param T2 температура на позапрошлом моменте времени
	* \param W1 тепло на прошлом моменте времени
	* \param W2 тепло на позапрошлом моменте времени
	* \return параметр вещественного типа, определяющий температуру в настоящий момент времени
	*/
	double count_temperature(double T1, double T2, double W1, double W2) {
		 return A * T1 - B * pow(T2, 2) + C * W1	+ D * sin(W2);
	}
};

/**
* \brief Абстрактный класс объекта для ПИД-регулирования
*
* Предоставляет интерфейс для работы с объектом, подлежащего регулированию 
*/
class PID_Object 
{
public:
	/**
	* для расчёта температуры в настоящий момент времени
	*/
	virtual void count_y_t() = 0;
	/**
	* для установление управляющего воздействия регулятора
	* \param u_t определяет управляющее воздействие регулятора
	*/
	virtual void set_w_t(double u_t) = 0;
	/**
	* для получения температуры в настоящий момент времени
	* \return параметр вещественного типа, определяющий температуру в настоящий момент времени
	*/
	virtual double get_y_t() = 0;

	/**
	* виртуальный деструктор абстрактного класса объекта, подлежащего регулированию
	*/
	~PID_Object() {}
};

/**
* \brief Конкретный класс рокеты в качестве объекта ПИД-регулирования
*
* Релизует интерфейс от родительского класса для работы с рокетой, подлежащей регулированию
*/
class Rocket : public PID_Object {
	NonLinearModel NLM; ///< агрегированная переменная нелинейной модели изменения температуры

	double y_t[3]; ///< массив для хранения значений температуры в настоящий, прошлый и позапрошлый моменты времени
	double w_t[2]; ///< массив для хранения значений тепла в настоящий и прошлый моменты времени
 
public:
	/**
	* для расчёта температуры в настоящий момент времени
	*/
	virtual void count_y_t() override {
		y_t[0] = y_t[1]; ///< сохраняем температуру на позапрошлом моменте времени
		y_t[1] = y_t[2]; ///< сохраняем температуру на прошлом моменте времени
		y_t[2] = NLM.count_temperature(y_t[1], y_t[0], w_t[1], w_t[0]); ///< производим расчёт температуры в настоящий момент времени
	}

	/**
	* для установление управляющего воздействия регулятора
	* \param u_t определяет управляющее воздействие регулятора
	*/
	virtual void set_w_t(double u_t) override {
		w_t[0] = w_t[1]; ///< сохраняем температуру на прошлом моменте времени
		w_t[1] = u_t; ///< сохраняем управляющее воздествие как тепло в настоящий момент времени 
	}

	/**
	* для получения температуры в настоящий момент времени
	* \return параметр вещественного типа, определяющий температуру в настоящий момент времени
	*/
	virtual double get_y_t() override {
		return y_t[2]; 
	}
	
	/**
	* Конструктор объекта регулирования
	* \param t_0 начальная температура объекта
	* \param w_0 начальное тепло, поступающее объекту
	*/
	Rocket(double t_0, double w_0) : PID_Object() {
		y_t[0] = 0; ///< приравниваем к нулю, т.к. пзапрошлого момента ещё нет
		y_t[1] = 0; ///< приравниваем к нулю, т.к. прошлого момента ещё нет
		y_t[2] = t_0; ///< устанавливаем начальную температуру объекта

		w_t[0] = 0; ///< приравниваем к нулю, т.к. прошлого момента ещё нет
		w_t[1] = w_0; ///< устанавливаем начальное входное тепло
	}
};

/**
* \brief Конкретный класс регулятора 
*
* Для расчёта e(t) - разницы между текущим и желаемым значением температуры и u(t) - управляющего воздействия
*/
class Regulator
{
	double target_value; ///< желаемое значение
	double e_t[3]; ///< массив, содержащий разницу целевой температуры и температуры в настоящий, прошлый и позапрошлый моменты времени
	double u_t[2]; ///< массив, содержащий управляющее воздействие в настоящий и прошлый моменты времени
	

	double K = 15; ///< коэффициент передачи
	double Ti = 20; ///< постоянная интегрирования
	double Td = 5; ///< постоянная дифференцирования 
	double T0 = 1.0; ///< временной шаг
 
	double q0 = K * (1 + Td / T0); ///< хранит значение параметра q0
	double q1 = -K * (1 + 2 * (Td / T0) - (T0 / Ti)); ///<хранит значение параметра q1
	double q2 = K * Td / T0; ///< хранит значение параметра q2

	/**
	* для расчёта управляющего воздействия
	*/
	double count_u_t() {
		return u_t[0] + (q0 * e_t[2] + q1 * e_t[1] + q2 * e_t[0]);
	}

public:
	/**
	* для расчёта параметров регулятора с поступлением нового значения температуры
	*/
	double temp_temperature; ///< для хранения переданной в регулятор температуры 
	void set_y_t(double y) {
		this->temp_temperature = y; ///< устанавливаем тепературу в текущий момент времени

		this->e_t[0] = this->e_t[1]; ///< сохраняем разницу e(t) на прошлом моменте времени
		this->e_t[1] = this->e_t[2]; ///< сохраняем разницу e(t) на прошлом моменте времени
		this->e_t[2] = this->target_value - y; ///< рассчитываем разницу e(t) в текущий момент времени

		this->u_t[0] = this->u_t[1]; ///< сохраняем управляющее воздействие на прошлом моменте времени
		this->u_t[1] = this->count_u_t(); ///< расчитываем управляющее воздействие в текущий момент времени
	}

	/**
	* для возврата разницы между желаемым значением и текущей температурой
	*/
	double get_e_t() { return e_t[2]; }
	/**
	* для возврата управляющего воздействия
	*/
	double get_u_t() { return u_t[1]; }

	/**
	* Конструктор объекта регулирования
	* \param t_0 начальная температура объекта
	* \param w_0 начальное тепло, поступающее объекту
	*/
	Regulator(double temperature, double target_value) {
		this->temp_temperature = temperature;
		this->target_value = target_value; ///< устанавилваем желаемое значение 
		
		this->e_t[0] = 0; ///< приравниваем к нулю, т.к. позапрошлого момента ещё нет
		this->e_t[1] = 0; ///< приравниваем к нулю, т.к. прошлого момента ещё нет
		this->e_t[2] = target_value - temperature; ///< устанавиваем начальную разницу e(t)
		
		this->u_t[0] = 0; ///< приравниваем к нулю, т.к. прошлого момента ещё нет
		this->u_t[1] = this->count_u_t(); ///< устанавливаем начальное управляющее воздействие
	}
};

/**
* для вывода параметров ПИД-регулятора
* \param regulator регулятор
* \param pid объект регулирования
* \param i момент времени
*/
void print_pid(Regulator& regulator, Rocket& pid, int i = 0) {
	cout << "t = " << i << '\t'
		<< "y(t) = " << pid.get_y_t() << '\t'
		<< "e(t) = " << regulator.get_e_t() << '\t'
		<< "u(t) = " << regulator.get_u_t() << '\n';
}

int main() {
	int moment; ///< для указания последного момента времени
	double value; ///< для указания желаемого значения
	double start_temperature; ///< для указания начальной температуры объекта 

	cout << "Time: "; cin >> moment; ///< вводим время
	cout << "Goal value: "; cin >> value; ///< вводим желаемое значение
	cout << "Start temperature: "; cin >> start_temperature; ///< вводим начальную температуру

	Rocket pid_rocket(start_temperature, 0); ///< создаём объект регулирования
	Regulator regulator(pid_rocket.get_y_t(), value); ///< создаём регулятор
	regulator.set_y_t(pid_rocket.get_y_t()); ///< передаём регулятору начальную температуру объекта
	pid_rocket.set_w_t(regulator.get_u_t()); ///< передаём объекту начальное управляющее воздействие 

	print_pid(regulator, pid_rocket); ///< выводим сстояние объекта и регутора в начальный момент времени

	/**
	* производим расчёт температуры на каждом моменте времени 
	*/
	for (int i = 1; i <= moment; i++) { 
		pid_rocket.count_y_t(); ///< расчитываем температуру в текущий момент времени
		regulator.set_y_t(pid_rocket.get_y_t()); ///< передаём регулятору текущую температуру объекта
		pid_rocket.set_w_t(regulator.get_u_t()); ///< передаём объекту текущее управляющее воздействие

		print_pid(regulator, pid_rocket, i); ///< выводим состояние объекта и регулятора в итый момент времени
	}

	return 0;
}
