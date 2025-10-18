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
    // Ignorar linhas vazias
    if (line.empty()) return false;

    // Detectar delimitador: se tiver ';', usa ';', senão usa ','
    char delimiter = (line.find(';') != std::string::npos) ? ';' : ',';

    std::stringstream ss(line);
    std::string field;
    std::string fields[7];
    int i = 0;

    // Extrair até 7 campos (esperados)
    while (std::getline(ss, field, delimiter) && i < 7) {
        // Remover aspas iniciais e finais, se houver
        if (!field.empty() && field.front() == '"') field.erase(0, 1);
        if (!field.empty() && field.back() == '"') field.pop_back();

        // Substituir "NULL" por string vazia
        if (field == "NULL") field.clear();

        fields[i++] = field;
    }

    // Se a linha tiver menos de 7 campos, descarta
    if (i < 7) {
        std::cerr << "Linha ignorada (campos insuficientes): " << line << std::endl;
        return false;
    }

    try {
        // 1. ID (inteiro)
        artigo.ID = std::atoi(fields[0].c_str());

        // 2. Título (alfa 300)
        std::strncpy(artigo.Titulo, fields[1].c_str(), 300);
        artigo.Titulo[300] = '\0';

        // 3. Ano (inteiro)
        artigo.Ano = std::atoi(fields[2].c_str());

        // 4. Autores (alfa 150)
        std::strncpy(artigo.Autores, fields[3].c_str(), 150);
        artigo.Autores[150] = '\0';

        // 5. Citações (inteiro)
        artigo.Citacoes = std::atoi(fields[4].c_str());

        // 6. Atualização (timestamp)
        artigo.Atualizacao_timestamp = std::atol(fields[5].c_str());

        // 7. Snippet (alfa 1024)
        std::strncpy(artigo.Snippet, fields[6].c_str(), 1024);
        artigo.Snippet[1024] = '\0';

        return true;
    } catch (...) {
        std::cerr << "ERRO DE PARSING: Linha mal formatada: " << line << std::endl;
        return false;
    }
}

int main(int argc, char* argv[]) {
    
    // 1. Verificar argumentos (espera o caminho para o CSV)
    if (argc != 2) {
        std::cerr << "Uso: " << argv[0] << " <caminho_para_o_arquivo_csv>" << std::endl;
        return 1;
    }
    // A variável input_csv_path precisa ser definida AQUI:
    const std::string input_csv_path = argv[1]; 

    std::cout << "--- INÍCIO DA CARGA DE DADOS ---" << std::endl;
    std::cout << "Arquivo de entrada: " << input_csv_path << std::endl;

    std::ifstream input_file(input_csv_path);
    if (!input_file.is_open()) {
        std::cerr << "ERRO: Não foi possível abrir o arquivo CSV: " << input_csv_path << std::endl;
        return 1;
    }
    
    // --- INICIALIZAÇÃO E LOOP PRINCIPAL ---

    // OBS: O 'initial_blocks' deve ser calculado. Usamos 1000 como placeholder.
    long initial_blocks = 775000; 

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