#include <iostream>
#include <hashing.hpp>
#include <record.hpp>

HashingFile::HashingFile(const std::string& data_file_path, long num_total_blocks) {
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
