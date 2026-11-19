#include <iostream>
#include <pthread.h>
#include <unistd.h>
#include <queue>

using namespace std;

const int CAPACIDAD = 5;
const int NUM_PRODUCTORES = 2;
const int NUM_CONSUMIDORES = 2;
const int ELEMENTOS_POR_HILO = 5;

queue<int> bufferCompartido;
pthread_mutex_t mutexBuffer;
pthread_mutex_t mutexSalida;
pthread_cond_t bufferNoLleno;
pthread_cond_t bufferNoVacio;

int siguienteDato = 1;

void imprimir(const string& mensaje) {
    pthread_mutex_lock(&mutexSalida);
    cout << mensaje << endl;
    pthread_mutex_unlock(&mutexSalida);
}

void* productor(void* argumento) {
    int id = *static_cast<int*>(argumento);

    for (int i = 0; i < ELEMENTOS_POR_HILO; ++i) {
        pthread_mutex_lock(&mutexBuffer);

        while (bufferCompartido.size() == CAPACIDAD) {
            pthread_cond_wait(&bufferNoLleno, &mutexBuffer);
        }

        int dato = siguienteDato++;
        bufferCompartido.push(dato);

        imprimir("Productor " + to_string(id) +
                 " produjo: " + to_string(dato));

        pthread_cond_signal(&bufferNoVacio);
        pthread_mutex_unlock(&mutexBuffer);

        sleep(1);
    }

    return nullptr;
}

void* consumidor(void* argumento) {
    int id = *static_cast<int*>(argumento);

    for (int i = 0; i < ELEMENTOS_POR_HILO; ++i) {
        pthread_mutex_lock(&mutexBuffer);

        while (bufferCompartido.empty()) {
            pthread_cond_wait(&bufferNoVacio, &mutexBuffer);
        }

        int dato = bufferCompartido.front();
        bufferCompartido.pop();

        imprimir("Consumidor " + to_string(id) +
                 " consumio: " + to_string(dato));

        pthread_cond_signal(&bufferNoLleno);
        pthread_mutex_unlock(&mutexBuffer);

        sleep(2);
    }

    return nullptr;
}

int main() {
    pthread_t productores[NUM_PRODUCTORES];
    pthread_t consumidores[NUM_CONSUMIDORES];

    int idsProductores[NUM_PRODUCTORES] = {1, 2};
    int idsConsumidores[NUM_CONSUMIDORES] = {1, 2};

    pthread_mutex_init(&mutexBuffer, nullptr);
    pthread_mutex_init(&mutexSalida, nullptr);
    pthread_cond_init(&bufferNoLleno, nullptr);
    pthread_cond_init(&bufferNoVacio, nullptr);

    for (int i = 0; i < NUM_PRODUCTORES; ++i) {
        pthread_create(&productores[i], nullptr, productor,
                       &idsProductores[i]);
    }

    for (int i = 0; i < NUM_CONSUMIDORES; ++i) {
        pthread_create(&consumidores[i], nullptr, consumidor,
                       &idsConsumidores[i]);
    }

    for (int i = 0; i < NUM_PRODUCTORES; ++i) {
        pthread_join(productores[i], nullptr);
    }

    for (int i = 0; i < NUM_CONSUMIDORES; ++i) {
        pthread_join(consumidores[i], nullptr);
    }

    pthread_cond_destroy(&bufferNoLleno);
    pthread_cond_destroy(&bufferNoVacio);
    pthread_mutex_destroy(&mutexSalida);
    pthread_mutex_destroy(&mutexBuffer);

    cout << "Todos los productores y consumidores han terminado." << endl;
    return 0;
}
