#include "record.hpp"
#include <cstring> 
#include <iostream>


const size_t RECORD_SIZE = sizeof(Artigo); 

// 1. SERIALIZAÇÃO (Struct Artigo -> Bytes)
void serialize_record(const Artigo& artigo, char* buffer) {
    // Copia a struct Artigo inteira para o buffer de bytes.
    // O buffer deve ter pelo menos o tamanho de RECORD_SIZE.
    std::memcpy(buffer, &artigo, RECORD_SIZE);
}

// 2. DESSERIALIZAÇÃO (Bytes -> Struct Artigo)
Artigo deserialize_record(const char* buffer) {
    Artigo artigo;
    // Copia os bytes do buffer para a struct Artigo.
    std::memcpy(&artigo, buffer, RECORD_SIZE);
    return artigo;
}
