#ifndef HASHING_HPP
#define HASHING_HPP

#include "record.hpp" // AGORA ELE INCLUI O SEU ARQUIVO!
#include <string>
#include <fstream>

using f_ptr = long;
const int RECORDS_PER_BLOCK = 10;

struct DataBlock {
    Artigo records[RECORDS_PER_BLOCK];
    int record_count;
    DataBlock() : record_count(0) {}
};

class HashingFile {
public:

    // Destrutor (para o erro da linha 42)
    ~HashingFile(); 

    // Construtor (que corrigimos antes)
    HashingFile(const std::string& data_file_path, long num_total_blocks); 

    // Inserção (para o erro da linha 72)
    f_ptr insert(const Artigo& new_artigo);

    // Busca (para o erro da linha 79)
    Artigo find_by_id(int id, int& blocks_read);
private:
    std::fstream data_file;
    long total_blocks;
    long hash_function(int key);
    DataBlock read_block(long block_number);
    void write_block(long block_number, const DataBlock& block);
};

#endif // HASHING_HPP