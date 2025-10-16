// struct do artigo científico 
// funções para serializar (converter o objeto para bytes para salvar em disco) e
// desserializar (converter os bytes lidos do disco para o objeto original
#ifndef RECORD_HPP
#define RECORD_HPP

// ... includes ...

// DEFINIÇÃO DA ESTRUTURA
struct Artigo {
    // ... (campos que você definiu: ID, Titulo, Ano, etc.) ...
};


// FUNÇÕES OBRIGATÓRIAS PARA MANIPULAÇÃO DE DISCO

/**
 * @brief Serializa um objeto Artigo para um array de bytes de tamanho fixo.
 * @param artigo O objeto Artigo a ser serializado.
 * @param buffer O array de bytes onde os dados serão escritos (tamanho fixo).
 */
void serialize_record(const Artigo& artigo, char* buffer);

/**
 * @brief Desserializa um array de bytes (lido do disco) de volta para um objeto Artigo.
 * @param buffer O array de bytes lido do disco.
 * @return O objeto Artigo reconstruído.
 */
Artigo deserialize_record(const char* buffer);


#endif // RECORD_HPP