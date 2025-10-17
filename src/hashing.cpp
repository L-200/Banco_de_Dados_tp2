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

// IMPLEMENTAÇÕES VAZIAS PARA RESOLVER ERROS DE LINKAGEM

// Implementação do Destrutor
HashingFile::~HashingFile() {
    if (data_file.is_open()) {
        data_file.close();
    }
}

// Implementação VAZIA de read_block
DataBlock HashingFile::read_block(long block_number) {
    // A Lógica real de leitura de disco deve ser implementada aqui
    std::cerr << "AVISO: HashingFile::read_block não implementada, retornando bloco vazio." << std::endl;
    return DataBlock{};
}

// Implementação VAZIA de write_block (CORRIGE O UNDEFINED REFERENCE)
void HashingFile::write_block(long block_number, const DataBlock& block) {
    // A Lógica real de escrita de disco deve ser implementada aqui
    std::cerr << "AVISO: HashingFile::write_block não implementada. Bloco " << block_number << " não foi escrito." << std::endl;
}

// Implementação VAZIA de hash_function
long HashingFile::hash_function(int key) {
    // Lógica real da função hash deve ser implementada aqui
    std::cerr << "AVISO: HashingFile::hash_function não implementada, usando modulo simples." << std::endl;
    if (total_blocks > 0) {
        return key % total_blocks;
    }
    return 0;
}

// Implementação VAZIA de insert
f_ptr HashingFile::insert(const Artigo& new_artigo) {
    // Lógica real de inserção deve ser implementada aqui
    std::cerr << "AVISO: HashingFile::insert não implementada." << std::endl;
    return 0; 
}

// Implementação VAZIA de find_by_id
Artigo HashingFile::find_by_id(int id, int& blocks_read) {
    // Lógica real de busca deve ser implementada aqui
    std::cerr << "AVISO: HashingFile::find_by_id não implementada." << std::endl;
    blocks_read = 0;
    return Artigo{}; // Retorna artigo vazio
}