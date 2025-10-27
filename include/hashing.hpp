#ifndef HASHING_HPP
#define HASHING_HPP

#include "record.hpp" // Inclui a definição de artigo
#include <string> 
#include <fstream>  // Biblioteca para manipular arquivos de disco
#include <unordered_map>

using f_ptr = long; // Endereço dentro de um arquivo

//id(4) + titulo(301) + ano(4) + autores(151) + citacoes(4) + atualização(20) + snippet(1025) ≃ 1509 bytes
// 1059*N + record_count(4) = 4096
//N <= 2.71
const int RECORDS_PER_BLOCK = 2;

struct DataBlock {
    Artigo records[RECORDS_PER_BLOCK]; // Lista para guardar os registros de artigos
    int record_count; // Quantos registros estão ocupando o bloco
    DataBlock() : record_count(0) {} // Construtor que começa a struct com 0 artigos
};

// Classe que vai gerenciar todo o hashing
class HashingFile {
public:

    // Construtor: prepara o arquivo para o uso 
    HashingFile(const std::string& data_file_path, long num_total_blocks); 

    // Destrutor: fecha o arquivo quando o objeto é destruido
    ~HashingFile();

    // Inserção: insere novo artigo no arquivo
    f_ptr insert(const Artigo& new_artigo);

    // Busca pelo ID: retorna o artigo encontrado pelo ID e quantos blocos foram lidos
    // Se não encontrar o artigo retorna um artigo com ID -1
    Artigo find_by_id(int id, int& blocks_read);

private:

    std::unordered_map<long, DataBlock> block_cache; // Bloco_número -> bloco em memória
    const size_t CACHE_LIMIT = 10000; // Maior que os outros por conta das colisões constantes
    std::fstream data_file; // Gerencia a conexão para ler e escrever
    long total_blocks;  // Quantidade total de blocos atualmente 


    long hash_function(int key); // Transforma a key em um ID

    DataBlock read_block(long block_number); // Lê um bloco do disco

    void flush_cache(); // Transfere as mudanças feitas no bloco cache para o bloco no disco

    void write_block(long block_number, const DataBlock& block); // Escreve um bloco no disco

};

#endif 