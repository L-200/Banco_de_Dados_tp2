#ifndef HASHING_HPP
#define HASHING_HPP

#include <string>
#include <fstream>
#include "record.hpp" // struct do Artigo está em record.hpp

// long para ponteiros em arquivo
using f_ptr = long;

//unidade mínima de leitura/escrita no arquivo de dados, contendo varios registros de arquivossss
const int RECORDS_PER_BLOCK = 10;

struct DataBlock {
    Artigo records[RECORDS_PER_BLOCK]; // array de registros de artigos
    int record_count;                  // quantos registros estão atualmente neste bloco

    //construtor para inicializar um bloco vazio
    DataBlock() : record_count(0) {}
};


// classe que gerencia o arquivo de dados com hashing
class HashingFile {
public:
    // abre/cria o arquivo de dados
    HashingFile(const std::string& data_file_path, long total_blocks);

    // fecha o arquivo
    ~HashingFile();

    // insere um novo artigo no arquivo de dados, retorna o ponteiro de onde o registro foi de fato inserido
    f_ptr insert(const Artigo& new_artigo);

    // busca um artigo pelo ID, retorna o Artigo encontrado e o número de blocos lidos
    Artigo find_by_id(int id, int& blocks_read);

private:
    std::fstream data_file; // gerencia conexão ler e escrever no arquivo de dados
    long total_blocks;      // número total de blocos no arquivo (tamanho da tabela hash)

    // recebe uma chave e retorna o número do bloco inicial
    long hash_function(int key);

    // lê um bloco do arquivo de dados
    DataBlock read_block(long block_number);

    // escreve um bloco no arquivo de dados.
    void write_block(long block_number, const DataBlock& block);
};

#endif // HASHING_HPP