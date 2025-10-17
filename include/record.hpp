#ifndef RECORD_HPP
#define RECORD_HPP

// --- INCLUDES CORRETOS (Aqui, fora de qualquer struct ou função) ---
#include <string>
#include <chrono> 
#include <ctime>  

// FUNÇÕES OBRIGATÓRIAS PARA MANIPULAÇÃO DE DISCO (Assinaturas virão depois da struct)
// ----------------------------------------------------------------------

// DEFINIÇÃO DA ESTRUTURA Artigo
struct Artigo {
    // Membros REAIS e COMPLETOS
    int ID;                                         
    char Titulo[301];                               
    int Ano;                                        
    char Autores[151];                              
    int Citacoes;                                   
    time_t Atualizacao_timestamp;                   
    char Snippet[1025];                             

    // Construtor padrão (Obrigatório para compilação!)
    Artigo() : ID(0), Ano(0), Citacoes(0), Atualizacao_timestamp(0) {
        // Inicialização de strings C
        Titulo[0] = '\0';
        Autores[0] = '\0';
        Snippet[0] = '\0';
    }
};


// DECLARAÇÕES DAS FUNÇÕES (que serão implementadas no src/record.cpp)

/**
 * @brief Serializa um objeto Artigo para um array de bytes de tamanho fixo.
 */
void serialize_record(const Artigo& artigo, char* buffer);

/**
 * @brief Desserializa um array de bytes (lido do disco) de volta para um objeto Artigo.
 */
Artigo deserialize_record(const char* buffer);


#endif // RECORD_HPP