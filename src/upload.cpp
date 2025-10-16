#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cstdlib> // Para a função exit()

// Incluindo headers necessários do seu projeto
#include "record.hpp" 
#include "hashing.hpp"
#include "BPlusTree.hpp" 
#include "upload.hpp" 

/**
 * @brief Ponto de entrada do programa 'upload'. 
 * Faz a carga inicial dos dados do CSV e cria os arquivos de dados e índices.
 */
int main(int argc, char* argv[]) {
    // 1. Verificar argumentos (espera o caminho para o CSV, conforme o makefile)
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <caminho_para_o_arquivo_csv>" << std::endl;
        return 1;
    }
    const std::string input_csv_path = argv[1]; 

    std::cout << "--- INÍCIO DA CARGA DE DADOS ---" << std::endl;
    std::cout << "Arquivo de entrada: " << input_csv_path << std::endl;

    // A Lógica principal do seu trabalho (leitura do CSV e inserção) virá aqui.
    // Por enquanto, apenas testamos se o arquivo pode ser aberto.
    
    std::ifstream input_file(input_csv_path);
    if (!input_file.is_open()) {
        std::cerr << "ERRO: Não foi possível abrir o arquivo CSV: " << input_csv_path << std::endl;
        // Se o makefile for executado antes de você criar data/input.csv, isso falhará aqui.
        return 1;
    }
    input_file.close();

    // OBS: O código do seu colega HashingFile::write_block() agora DEVE existir, 
    // mas a lógica dele ainda estará vazia (conforme corrigimos anteriormente).
    
    // A implementação da lógica do seu upload.cpp virá aqui!
    
    std::cout << "Carga de dados concluída com sucesso (Implementação da lógica em andamento)." << std::endl;
    return 0;
}