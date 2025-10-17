#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <stdexcept>
#include <cstdlib> // Para a função exit()
#include <cstring> // Para std::strncp
// Incluindo headers necessários do seu projeto
#include "record.hpp" 
#include "hashing.hpp"
#include "BPlusTree.hpp" 
#include "upload.hpp" 

// IMPORTANTE: Esta é a implementação da função que você DECLAROU em include/upload.hpp
bool parse_csv_line(const std::string& line, Artigo& artigo) {
    std::stringstream ss(line);
    std::string field;
    
    // Tentativa de parsing
    try {
        // 1. ID (inteiro)
        if (!std::getline(ss, field, ',')) return false;
        artigo.ID = std::atoi(field.c_str());

        // 2. Título (alfa 300)
        if (!std::getline(ss, field, ',')) return false;
        std::strncpy(artigo.Titulo, field.c_str(), 300);
        artigo.Titulo[300] = '\0'; // Garantir terminação em C

        // 3. Ano (inteiro)
        if (!std::getline(ss, field, ',')) return false;
        artigo.Ano = std::atoi(field.c_str());

        // 4. Autores (alfa 150)
        if (!std::getline(ss, field, ',')) return false;
        std::strncpy(artigo.Autores, field.c_str(), 150);
        artigo.Autores[150] = '\0';

        // 5. Citações (inteiro)
        if (!std::getline(ss, field, ',')) return false;
        artigo.Citacoes = std::atoi(field.c_str());

        // 6. Atualização (data e hora - time_t)
        if (!std::getline(ss, field, ',')) return false;
        // Assume que a string de data/hora é grande, simplificamos para atol por enquanto
        artigo.Atualizacao_timestamp = std::atol(field.c_str());

        // 7. Snippet (alfa 1024 - último campo)
        if (!std::getline(ss, field)) return false; 
        std::strncpy(artigo.Snippet, field.c_str(), 1024);
        artigo.Snippet[1024] = '\0';

        return true;
    } catch (...) {
        // Captura qualquer erro inesperado durante a conversão
        std::cerr << "ERRO DE PARSING: Linha mal formatada." << std::endl;
        return false;
    }
}
int main(int argc, char* argv[]) {
    
    // ... (Verificação de argumentos e definição de input_csv_path) ...

    std::cout << "--- INÍCIO DA CARGA DE DADOS ---" << std::endl;
    std::cout << "Arquivo de entrada: " << input_csv_path << std::endl;

    std::ifstream input_file(input_csv_path);
    if (!input_file.is_open()) {
        std::cerr << "ERRO: Não foi possível abrir o arquivo CSV: " << input_csv_path << std::endl;
        return 1;
    }
    
    // --- INICIALIZAÇÃO E LOOP PRINCIPAL ---

    // OBS: O 'initial_blocks' deve ser calculado. Usamos 1000 como placeholder.
    long initial_blocks = 1000; 

    try {
        // Inicializa o arquivo de dados (Hashing)
        // O construtor abrirá ou criará data/data_file.dat
        HashingFile data_file("data/data_file.dat", initial_blocks);
        
        // Inicializa os índices (B+Trees - o nome dos arquivos deve ser definido pelo grupo)
        // Se o seu colega não implementou os construtores da BPlusTree, pode ser necessário 
        // deixá-los comentados ou criar implementações vazias também.
        // BPlusTree primary_index("data/primary_index.idx");
        // BPlusTree secondary_index("data/secondary_index.idx");
        
        std::string line;
        
        // 1. Ignorar a linha de cabeçalho do CSV
        std::getline(input_file, line); 

        // 2. Loop para ler e processar os artigos
        int inserted_count = 0;
        while (std::getline(input_file, line)) {
            Artigo new_artigo;
            
            if (parse_csv_line(line, new_artigo)) {
                // A. Insere no Arquivo de Dados (Hashing)
                f_ptr data_ptr = data_file.insert(new_artigo); 

                // B. Insere nos Índices (Descomente quando as classes BPlusTree estiverem prontas)
                // primary_index.insert(new_artigo.ID, data_ptr);
                // secondary_index.insert(new_artigo.Titulo, data_ptr); 
                
                inserted_count++;
            }
        }
        input_file.close();

        std::cout << "Carga de dados concluída com sucesso." << std::endl;
        std::cout << "Total de artigos processados e inseridos: " << inserted_count << std::endl;

    } catch (const std::runtime_error& e) {
        std::cerr << "ERRO FATAL DURANTE A CARGA: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}