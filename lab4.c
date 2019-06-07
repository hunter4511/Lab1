#include <pthread.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <stddef.h>

void *commandant();
void *student(void *studentId);

typedef struct Student{
	bool inRoom;
} Student;

typedef struct Commandant{
	bool inRoom;
} Commandant;

pthread_cond_t commandant_condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t commandant_mutex = PTHREAD_MUTEX_INITIALIZER;

Student *_students;
Commandant _commandant;

int students_cap;
int students_amount;



int main(int argc, char **argv){
	
	students_amount = 6;
	srand(time(NULL));

	if (argc != 2) {
		students_cap = 4;
	}
	else
	{
		students_cap = atoi(argv[1]);
	}
	long i;

	pthread_t students_thread[students_amount];
	pthread_t commandant_thread;
	
	_students = (Student*)calloc(students_amount, sizeof(Student));

	
	for (i = 0; i < students_amount; i++) {
		pthread_create(students_thread + i, NULL, &student, (void*)i);
	}
	pthread_create(&commandant_thread, NULL, commandant, NULL);
	
	pthread_join(commandant_thread, NULL);
	for (i = 0; i < students_amount; i++) {
		pthread_join(students_thread + i, NULL);
	}
	

	return 0;
}

void *student(void *studentId) {
	long id = studentId;
	for (;;) {
		sleep(1);

		if (!_students[id].inRoom) {
			int chanceToEnter = rand();
			if (chanceToEnter % 5 == 0) {
				pthread_mutex_lock(&commandant_mutex);
				if (!_commandant.inRoom) {
					_students[id].inRoom = true;
					print_time_message("Student has enterd the room");
				}
				else {
					print_time_message("Student cant enter, commandant is in the room");
				}

				pthread_mutex_unlock(&commandant_mutex);
			}
		}
		else {
			int chanceToLeave = rand();
			if (chanceToLeave % 2 == 0) {
				_students[id].inRoom = false;
				print_time_message("Student has left the room");
				pthread_cond_signal(&commandant_condition);
				int a = getStudentsInRoomAmount();
				printf(" number of students %d \n", a);
			}

		}
	}
}

void *commandant() {
	
	for (;;) {
		
		sleep(1);
		
		int studentsAmountInRoom = getStudentsInRoomAmount();
		bool areStudentsCrazy = studentsAmountInRoom >= students_cap;
		if (studentsAmountInRoom == 0) {
			print_time_message("Search is in the room ");
			pthread_mutex_lock(&commandant_mutex);
			_commandant.inRoom = true;
			pthread_mutex_unlock(&commandant_mutex);
			sleep(1);
			pthread_mutex_lock(&commandant_mutex);
			_commandant.inRoom = false;
			pthread_mutex_unlock(&commandant_mutex);
			print_time_message("Search is off ");
		}
		else if (areStudentsCrazy) {
			print_time_message("Commandant has came to kick ass of studens");
			pthread_mutex_lock(&commandant_mutex);
			_commandant.inRoom = true;
			//pthread_mutex_unlock(&commandant_mutex);
			for (;;) {
				pthread_cond_wait(&commandant_condition, &commandant_mutex);
				int newStudents = getStudentsInRoomAmount();
				if (!newStudents) {

					_commandant.inRoom = false;
					pthread_mutex_unlock(&commandant_mutex);
					print_time_message("Commandant has left the room");
					
					break;
				}
			}
		}
		else {
			print_time_message("Commandant went through");
		}
	}
}

int getStudentsInRoomAmount() {
	if (_students == NULL) {
		return 0;
	}
	int i;
	int amount = 0;
	for (i = 0; i < students_amount; i++) {
		if (_students[i].inRoom) {
			amount++;
		}
	}
	return amount;
}

void print_time_message(char* message) {
	print_time();
	puts(message);
}

void print_time() {
	time_t timer;
	char buffer[26];
	struct tm *tm_info;

	time(&timer);
	tm_info = localtime(&timer);

	strftime(buffer, 26, "ok", tm_info);
	printf(buffer);
}
