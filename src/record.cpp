#include "record.hpp"
#include <cstring> // Para memcpy

// 1. SERIALIZAÇÃO (Struct para Bytes para salvar no disco)
void serialize_record(const Artigo& artigo, char* buffer) {
    // Usa memcpy para copiar o struct diretamente para o buffer.
    // Isso é o método mais simples e direto para structs C++ de tamanho fixo.
    // O tamanho total do registro é sizeof(Artigo).
    std::memcpy(buffer, &artigo, sizeof(Artigo));
}

// 2. DESSERIALIZAÇÃO (Bytes do disco de volta para Struct)
Artigo deserialize_record(const char* buffer) {
    Artigo artigo;
    // Copia os bytes do buffer para a struct Artigo
    std::memcpy(&artigo, buffer, sizeof(Artigo));
    return artigo;
}

// Você pode adicionar uma constante global em record.hpp para o tamanho fixo:
// const size_t RECORD_SIZE = sizeof(Artigo);