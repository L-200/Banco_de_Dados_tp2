#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "hashing.hpp" 
#include "record.hpp"

// construtor 
HashingFile::HashingFile(const std::string& data_file_path, long num_total_blocks) {
    total_blocks = num_total_blocks;
    //tentando abrir data file
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

//fechando o arquivo
HashingFile::~HashingFile() {
    if (data_file.is_open()) {
        data_file.close();
    }
}

f_ptr HashingFile::insert(const Artigo& new_artigo) {
    //calculando endereço do bloco inicial
    long initial_block = hash_function(new_artigo.ID);
    long current_block_num = initial_block;

    for (int i = 0; i < total_blocks; i++) { 
        DataBlock block = read_block(current_block_num); 

        if (block.record_count < RECORDS_PER_BLOCK) { //achamos onde vamos inserir
            int record_pos = block.record_count;
            block.records[record_pos] = new_artigo;
            block.record_count++;
            write_block(current_block_num, block);
            return (current_block_num * sizeof(DataBlock)) + (record_pos * sizeof(Artigo)); //retornando o endereço exato onde inserimos 
        } 

        //se o bloco estiver cheio, tentamos inserir ao proximo bloco, se chegar no final de arquivo volta ao começo 
        current_block_num = (current_block_num + 1) % total_blocks;
        //se voltamos ao bloco inicial não temos mais espaço para inserir o novo arquivo
        if (current_block_num == initial_block) {
            std::cerr << "ERRO: Arquivo de dados está cheio!" <<std::endl;
            return -1;
        }
    }
    return -1; //falha na inserção
}

Artigo HashingFile::find_by_id(int id, int& blocks_read) {
    blocks_read = 0;
    long initial_block = hash_function(id);
    long current_block_num = initial_block;

    for (int i = 0; i < total_blocks; i++) { //loop seguindo a mesma logica do insert
        DataBlock block = read_block(current_block_num);
        blocks_read++; 

        for (int j = 0; j < block.record_count; j++) {
            if (block.records[j].ID == id) { //checa se o artigo está no bloco
                return block.records[j];
            }
        }

        if (block.record_count < RECORDS_PER_BLOCK) { //artigo deveria estar nesse bloco
            break;
        }

        current_block_num = (current_block_num + 1) % total_blocks;

        if (current_block_num == initial_block) {
            break; //demos a volta no arquivo todo e ainda não achamos
        }
    }

    Artigo not_found_artigo;
    not_found_artigo.ID = -1;
    return not_found_artigo;
    
}

//FUNÇÕES PRIVADAS 

long HashingFile::hash_function(int key) { //padrão da industria, tenta gerar um numero bastante unico
    return key % total_blocks;
}

DataBlock HashingFile::read_block(long block_number) {
    DataBlock block;

    //calculando o endereço do arquivo e mudando o cursor de leitura para a posição
    f_ptr offset = block_number * sizeof(DataBlock);
    data_file.seekg(offset);
    //lendo e retornando o que leu 
    data_file.read(reinterpret_cast<char*>(&block), sizeof(DataBlock));
    return block;
}

void HashingFile::write_block(long block_number, const DataBlock& block) {
    //calculando o endereço e mudando o cursor de escritura para ele
    f_ptr offset = block_number * sizeof(DataBlock);
    data_file.seekp(offset);

    //escrevendo
    data_file.write(reinterpret_cast<const char*>(&block), sizeof(DataBlock));
}