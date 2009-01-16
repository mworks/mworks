#include "Lockable.h"
using namespace mw;

Lockable::Lockable() {
    pthread_mutex_init(&mutex, NULL);
}
  
Lockable::~Lockable() {
    pthread_mutex_destroy(&mutex);
}
