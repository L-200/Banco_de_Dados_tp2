#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>

#include "hashing.hpp" 
#include "record.hpp"

// Construtor 
HashingFile::HashingFile(const std::string& data_file_path, long num_total_blocks) {
    total_blocks = num_total_blocks;
    //Tentando abrir data file
    data_file.open(data_file_path, std::ios::in | std::ios::out | std::ios::binary);

    if (!data_file.is_open()) {
        // Criando novo arquivo zerado
        std::ofstream create_file(data_file_path, std::ios::out | std::ios::binary | std::ios::trunc);
        if (!create_file) {
            throw std::runtime_error("ERRO: não foi possível criar o arquivo de dados");
        }
        create_file.close();

        // Reabrindo o arquivo para leitura e escrita
        data_file.open(data_file_path, std::ios::in | std::ios::out | std::ios::binary);
        if (!data_file.is_open()) {
            throw std::runtime_error("ERRO: não foi possível reabrir o arquivo de dados");
        }

        // Inicializando todos os blocos vazios
        DataBlock empty_block{};
        for (long i = 0; i < total_blocks; i++) {
            write_block(i, empty_block);
        }
        data_file.flush();
    }
}

//Fechando o arquivo
HashingFile::~HashingFile() {
    if (!block_cache.empty()) {
        flush_cache();
    }
    if (data_file.is_open()) {
        data_file.close();
    }
}

f_ptr HashingFile::insert(const Artigo& new_artigo) {
    //Calculando endereço do bloco inicial
    long initial_block = hash_function(new_artigo.ID);
    long current_block_num = initial_block;

    for (int i = 0; i < total_blocks; i++) { 
        DataBlock block = read_block(current_block_num); 

        if (block.record_count < RECORDS_PER_BLOCK) { //Achamos onde vamos inserir
            int record_pos = block.record_count;
            block.records[record_pos] = new_artigo;
            block.record_count++;
            write_block(current_block_num, block);
            return (current_block_num * sizeof(DataBlock)) + (record_pos * sizeof(Artigo)); //Retornando o endereço exato onde inserimos 
        } 

        //Se o bloco estiver cheio, tentamos inserir ao proximo bloco, se chegar no final de arquivo volta ao começo 
        current_block_num = (current_block_num + 1) % total_blocks;
        //Se voltamos ao bloco inicial, não temos mais espaço para inserir o novo arquivo
        if (current_block_num == initial_block) {
            std::cerr << "ERRO: Arquivo de dados está cheio!" <<std::endl;
            return -1;
        }
    }
    return -1; //Falha na inserção
}

Artigo HashingFile::find_by_id(int id, int& blocks_read) {
    blocks_read = 0;
    long initial_block = hash_function(id);
    long current_block_num = initial_block;

    for (int i = 0; i < total_blocks; i++) { //Loop seguindo a mesma logica do insert
        DataBlock block = read_block(current_block_num);
        blocks_read++; 

        for (int j = 0; j < block.record_count; j++) {
            if (block.records[j].ID == id) { //Checa se o artigo está no bloco
                return block.records[j];
            }
        }

    
        if (block.record_count < RECORDS_PER_BLOCK) { //Artigo deveria estar nesse bloco
            break;
        }
        

        current_block_num = (current_block_num + 1) % total_blocks;

        if (current_block_num == initial_block) {
            break; // Foi dado a volta no arquivo todo e ainda não achado
        }
    }

    Artigo not_found_artigo;
    not_found_artigo.ID = -1;
    return not_found_artigo;
    
}

//FUNÇÕES PRIVADAS 

long HashingFile::hash_function(int key) { // Padrão da indústria, tenta gerar um número bastante único
    return key % total_blocks;
}

DataBlock HashingFile::read_block(long block_number) {
    // Procurando bloco no cache
    auto it = block_cache.find(block_number);
    //Verificando se o bloco foi encontrado no cache
    if (it != block_cache.end()) { 
        return it->second; // Retorna o bloco diretamente da memória
    }

    //Se o bloco não está no cache precisamos ler ele do arquivo
    DataBlock block;
    f_ptr offset = block_number * sizeof(DataBlock);
    data_file.seekg(offset);
    data_file.read(reinterpret_cast<char*>(&block), sizeof(DataBlock));

    return block;
}

// Escreve todos os blocos dentro do cache de volta no disco
void HashingFile::flush_cache() {
    for (const auto& pair : block_cache) {
        long block_number = pair.first; // Pegando o número do bloco
        const DataBlock& block = pair.second; // Pegando para o bloco de dados

        f_ptr offset = block_number * sizeof(DataBlock);
        data_file.seekp(offset);
        data_file.write(reinterpret_cast<const char*>(&block), sizeof(DataBlock));
    }

    data_file.flush();
    block_cache.clear();
}

void HashingFile::write_block(long block_number, const DataBlock& block) {
    f_ptr offset = block_number * sizeof(DataBlock);
    data_file.seekp(offset); // Posiciona o leitor de escritura

    if (!data_file.write(reinterpret_cast<const char*>(&block), sizeof(DataBlock))) {
        throw std::runtime_error ("ERRO HASHING WRITE: Falha ao escrever bloco ");
    }
    data_file.flush(); 
}