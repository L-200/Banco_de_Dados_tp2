#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "hashing.hpp" 
#include "record.hpp"

// Implementação do Construtor (Já fornecido)
HashingFile::HashingFile(const std::string& data_file_path, long num_total_blocks) {
    // ... (o código do construtor que você já tinha) ...
    total_blocks = num_total_blocks;
    data_file.open(data_file_path, std::ios::in | std::ios::out | std::ios::binary);

    if (!data_file.is_open()) {
        // criando novo arquivo zerado
        std::ofstream create_file(data_file_path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!create_file) {
            throw std::runtime_error("ERRO: não foi possível criar o arquivo de dados");
        }
        create_file.close();

        // reabrindo o arquivo para leitura e escrita
        data_file.open(data_file_path, std::ios::in | std::ios::out | std::ios::binary);
        if (!data_file.is_open()) {
            throw std::runtime_error("ERRO: não foi possível reabrir o arquivo de dados");
        }

        // inicializando todos os blocos vazios
        DataBlock empty_block{};
        for (long i = 0; i < total_blocks; i++) {
            write_block(i, empty_block);
        }
        data_file.flush();
    }
}

HashingFile::~HashingFile() {
    if (data_file.is_open()) {
        data_file.close();
    }
}

f_ptr HashingFile::insert(const Artigo& new_artigo) {
    //calculando endereço do bloco inicial
    long initial_block = hash_function(new_artigo.id);
    long current_block_num = initial_block;

    for (int i = 0; i < total_blocks; i++) { 
        DataBlock block = read_block(current_block_num); 

        if (block.record_count < RECORDS_PER_BLOCK) { //achamos onde vamos inserir
            int record_pos = block.record_count;
            block.records[record_pos] = new_artigo;
            block.record_count++;
            write_block(current_block_num, block);
            return (current_block_num * sizeof(DataBlock)) + (record_pos * sizeof(Artigo)); //retornando o endereço exato onde inserimos 
        } else {
            //IMPLEMENTAR AS COLISÕES
        }
    }
}