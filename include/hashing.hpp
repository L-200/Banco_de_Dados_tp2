#ifndef HASHING_HPP
#define HASHING_HPP

#include "record.hpp" //inclui a definição de artigo
#include <string> 
#include <fstream>  //biblioteca para manipular arquivos de disco
#include <unordered_map>

using f_ptr = long; //endereço dentro de um arquivo

//id(4) + titulo(301) + ano(4) + autores(151) + citacoes(4) + atualização(20) + snippet(1025) ≃ 1509 bytes
// 1059*N + record_count(4) = 4096
//N <= 2.71
const int RECORDS_PER_BLOCK = 2;

struct DataBlock {
    Artigo records[RECORDS_PER_BLOCK]; //lista para guardar os registros de artigos
    int record_count; //quantos registros estão ocupando o bloco
    DataBlock() : record_count(0) {} // construtor que começa a struct com 0 artigos
};

//classe que vai gerenciar todo o hashing
class HashingFile {
public:

    //construtor - prepara o arquivo para o uso 
    HashingFile(const std::string& data_file_path, long num_total_blocks); 

    //destrutor - fecha o arquivo quando o objeto é destruido
    ~HashingFile();

    //inserção - insere novo artigo no arquivo
    f_ptr insert(const Artigo& new_artigo);

    //busca pelo id - retorna o artigo encontrado pelo id e quantos blocos foram lidos
    //se não encontrar o artigo retorna um artigo com id -1
    Artigo find_by_id(int id, int& blocks_read);

private:

    std::unordered_map<long, DataBlock> block_cache; // bloco_número -> bloco em memória
    const size_t CACHE_LIMIT = 1024; // número máximo de blocos no cache antes de flush
    std::fstream data_file; //gerencia a conexão para ler e escrever
    long total_blocks;  //quantidade total de blocos atualmente 


    long hash_function(int key); //transforma a key em um id 

    DataBlock read_block(long block_number); // lê um bloco do disco

    void flush_cache();

    void write_block(long block_number, const DataBlock& block); //escreve um bloco no disco

};

#endif // HASHING_HPP