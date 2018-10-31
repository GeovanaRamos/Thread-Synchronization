#include<stdio.h>
#include <stdlib.h>
#include<pthread.h>
#include<unistd.h>
#include<semaphore.h>

#define SLEEPING 0
#define HELPING 1

sem_t mutex, ae_sem, *std_sem;
int current_student, line_count = 0, num_std, *line, ae_state;

int getFirstStudentFromLine(){
    int first = line[0];

    //remover da fila e passar os outros uma posição a frente
    for (int i = 0; i < (num_std/2 - 1); i++) {
        line[i] = line[i+1];
    }

    line[(num_std/2) -1] = 0;
    line_count--;
    printf("AE está chamando primeiro estudante da fila\n");
    return first - 1;
}

void goToLine(int id){

    line[line_count] = id + 1;
    line_count++;

}

void help(int id){

    int tm = (rand() % 10) + 1;
    printf("Estudante %d está sendo ajudado por %d segundos\n", id, tm);
    sleep(tm);

}


void code(int id){

    int tm = (rand() % 20) + 1;
    printf("Estudante %d programando por %d segundos\n", id, tm );
    sleep(tm);

}

void *AEAction(void *num_std){
    int helps = *(int*)num_std * 1; //TODO AUMENTAR CONTADOR
    int count = 0, sem_id;

    while(count < helps){
        if (line_count > 0) {
            sem_id = getFirstStudentFromLine();
            current_student = sem_id;
        } else {
            ae_state = SLEEPING;
            printf("AE foi dormir\n");
            sem_wait(&ae_sem); // dormindo até estudante chamar
            printf("AE acordou\n");
        }
        ae_state = HELPING;
        help(current_student);
        printf("Estudante %d terminou de ser ajudado\n", current_student);
        sem_post(&std_sem[current_student]); // libera estudante que terminou de ser ajudado
        count++;
    }

    pthread_exit(0);
}

void *StudentAction(void *i){
    int id = *(int*)i;

    //TODO AUMENTAR LOOP
    for(int i=0; i<1; i++){
        code(id);
        sem_wait(&mutex);
        printf("Estudante %d foi pedir ajuda\n", id);
        if(ae_state == SLEEPING){
            //ae está dormindo, então acordar e pedir ajuda
            current_student = id; // passa o estudante para o AE
            sem_post(&ae_sem); // acorda o AE
            sem_post(&mutex); // destrava os outros alunos
            sem_wait(&std_sem[id]); // espera terminar de ser ajudado
        } else if (line_count < num_std/2){
            //AE está ocupado, mas tem lugar na fila
            goToLine(id);
            printf("Estudante %d está na fila na posição %d\n", id, line_count);
            sem_post(&mutex);//libera outros estudantes
            sem_wait(&std_sem[id]);//espera na fila
        } else {
            //não cabe na fila, volta a programar
            printf("Fila está cheia, estudante %d foi programar\n", id );
            i--;
            sem_post(&mutex);
        }


    }

    pthread_exit(0);
}

int main(){
    srand(time(NULL));

    //NÚMERO DE ESTUDANTES // TODO AUMENTAR
    num_std = (rand() % 10) + 3;
    printf("Número de  estudantes: %d\n", num_std );

    //FILA
    line = (int*)calloc(num_std/2, sizeof(int));

    //SEMÁFOROS
    sem_init(&mutex, 0, 1);
    sem_init(&ae_sem, 0, 0);
    std_sem = (sem_t*)malloc(num_std*sizeof(sem_t));
    for (int i = 0; i < num_std; i++) {
        sem_init(std_sem, 0, 0);
    }

    //THREADS
    pthread_t AE_th, students_th[num_std];
    pthread_create(&AE_th, NULL, AEAction, (void*)&num_std);
    int arg[num_std];
    for(int i=0; i < num_std; i++){
        arg[i] = i;
        pthread_create(&students_th[i], NULL, StudentAction, (void*)&arg[i]);
    }

    //JOIN
    for(int i=0; i < num_std; i++){
        pthread_join(students_th[i], NULL);
    }
    pthread_join(AE_th, NULL);

    //TODO SEM_DESTROY
    sem_destroy(&mutex);
    sem_destroy(&ae_sem);
    for (int i = 0; i < num_std; i++) {
        sem_destroy(std_sem);
    }


    return 0;
}
