#ifndef RECORD_HPP
#define RECORD_HPP

#include <string>
#include <chrono> 
#include <ctime>  

// DEFINIÇÃO DA ESTRUTURA Artigo
struct Artigo {
    int ID;                                         
    char Titulo[301];                               
    int Ano;                                        
    char Autores[151];                              
    int Citacoes;                                   
    time_t Atualizacao_timestamp;                   
    char Snippet[1025];                             


    Artigo() : ID(0), Ano(0), Citacoes(0), Atualizacao_timestamp(0) {
        
        Titulo[0] = '\0';
        Autores[0] = '\0';
        Snippet[0] = '\0';
    }
};



/**
 * @brief Serializa um objeto Artigo para um array de bytes de tamanho fixo
 */
void serialize_record(const Artigo& artigo, char* buffer);

/**
 * @brief Desserializa um array de bytes (lido do disco) de volta para um objeto Artigo
 */
Artigo deserialize_record(const char* buffer);


#endif 